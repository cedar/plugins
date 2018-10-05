/*======================================================================================================================

    THIS FILE CONTAINS PROPRIETARY CODE BY SCHUNK. LIMITED CIRCULATION ONLY.
    We cannot distribute this code under the GPL license.

========================================================================================================================

    Institute:   Ruhr-Universitaet Bochum
                 Institut fuer Neuroinformatik

    File:        SMBCanBusChannel.cpp

    Maintainer:  Mathis Richter
    Email:       mathis.richter@ini.rub.de
    Date:        2014 12 02

    Description:

    Credits:     Ceyhun Pekis; Schunk

======================================================================================================================*/

// LOCAL INCLUDES
#include "devices/schunk/SMPCanBusChannel.h"

// SYSTEM INCLUDES
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <QMutex>
#include <ntcan.h>

//! transmit queue size for CAN frames
#define CAN_ESD_TXQUEUESIZE 512
//! receive queue size for CAN frames
#define CAN_ESD_RXQUEUESIZE 1024

//----------------------------------------------------------------------------------------------------------------------
// constructors and destructor
//----------------------------------------------------------------------------------------------------------------------

cedar::dev::schunk::SMPCanBusChannel::SMPCanBusChannel()
:
mIsOpen(false),
_mModuleMap(),
_mTimeout(new cedar::aux::IntParameter(this, "timeout", 25, cedar::aux::IntParameter::LimitType(0, 100000))),
_mBaudrate(new cedar::aux::IntParameter(this, "baudrate", 500000, cedar::aux::IntParameter::LimitType(10000, 1000000))),
_mCanPort(new cedar::aux::UIntParameter(this, "can port", 0, cedar::aux::UIntParameter::LimitType(0, 1)))
{
  for (unsigned int i = 0; i < 256; ++i)
  {
    g_smp_buffer[i] = NULL;
  }
}

cedar::dev::schunk::SMPCanBusChannel::~SMPCanBusChannel()
{
  prepareChannelDestructAbsolutelyRequired();
}

// todo remove
using namespace std;

//----------------------------------------------------------------------------------------------------------------------
// methods
//----------------------------------------------------------------------------------------------------------------------
void cedar::dev::schunk::SMPCanBusChannel::openHook()
{
  NTCAN_RESULT rc;

  {
    // block the channel
    QMutexLocker mutex_locker(&mMutex);

    // open the interface
    rc = canOpen
         (
           static_cast<int>(_mCanPort->getValue()), // can port
           0,                                       // flags
           CAN_ESD_TXQUEUESIZE,                     // txquesize
           CAN_ESD_RXQUEUESIZE,                     // rxquesize
           _mTimeout->getValue(),
           _mTimeout->getValue(),
           &mHandle
         );
  }

  if (rc != NTCAN_SUCCESS)
  {
    std::cout << "Error opening CAN port!" << std::endl;
    return;
  }

  {
    // block the channel
    QMutexLocker mutex_locker(&mMutex);
    // set the baud rate
    rc = canSetBaudrate(mHandle, baudrateToCode(_mBaudrate->getValue()));
  }

  if (rc != NTCAN_SUCCESS)
  {
    std::cout << "Error setting baudrate!" << std::endl;
    return;
  }

  // read all responses
  while (processInput())
  {
    ;
  }

  this->mIsOpen = true;
  std::cout << "CAN bus is open!" << std::endl;
}

void cedar::dev::schunk::SMPCanBusChannel::addModules(const std::vector<unsigned int>& moduleMap)
{
  _mModuleMap = moduleMap;
}

void cedar::dev::schunk::SMPCanBusChannel::postOpenHook()
{
  for (unsigned int i = 0; i < _mModuleMap.size(); ++i)
  {
    acknowledge(_mModuleMap.at(i));

    // only reference modules that are not already referenced
    if (getref(_mModuleMap.at(i)))
    {
      std::cout << "Module " << _mModuleMap.at(i) << " is already referenced." << std::endl;
    }
    else
    {
      reference(_mModuleMap.at(i));
    }
  }

  // add all CAN IDs of the modules we are interested in
  uSMPMessageID message_id;
  message_id.value = 0;
  message_id.is_module_id = 1;     // we are only interested in module ids, not group ids
  message_id.from_module = 1;      // we are only interested in receiving from modules

  {
    // block the channel
    QMutexLocker mutex_locker(&mMutex);

    NTCAN_RESULT rc;

    for (unsigned int i = 0; i < _mModuleMap.size(); ++i)
    {
      message_id.module_id = _mModuleMap.at(i);
      // we are interested in normal messages and diagnosis messages
      for (int is_normal = 0; is_normal <= 1; ++is_normal)
      {
        message_id.is_normal = is_normal;
        int can_id = message_id.value;

        rc = canIdAdd(mHandle, can_id);

        if (rc != NTCAN_SUCCESS)
        {
          cout << "Cannot add module with id " << _mModuleMap.at(i) << endl;
        }
      }
    }
  }

  // message id used to send SMP requests
  uSMPMessageID request_id;

  // prepare fixed part of the message id for requests
  request_id.is_module_id = 1;
  request_id.from_module = 0;
  request_id.is_normal = 1;
  for (unsigned int i = 0; i < _mModuleMap.size(); ++i)
  {
    // prepare module specific part of the message id for next request
    request_id.module_id = _mModuleMap.at(i);

    // prepare parameters for request
    float period;
    sGetStateMode mode;
    period = 0.0; // make module reply only once (not periodically)
    mode.Position = 1; // make module reply its current position
    mode.Velocity = 0;
    mode.Current = 0;

    this->getState(request_id, &period, &mode);
  }

  // read all responses
  while (processInput())
  {
    ;
  }
}

void cedar::dev::schunk::SMPCanBusChannel::closeHook()
{
  // close the port if it is open
  if (this->isOpen())
  {
    // block the channel
    QMutexLocker mutex_locker(&mMutex);
    canClose(mHandle);
    this->mIsOpen = false;
  }
}

void cedar::dev::schunk::SMPCanBusChannel::send(uSMPMessageID message_id, unsigned int d_len, unsigned char* data)
{
  // message is too long, send it fragmented
  if (d_len > 7)
  {
    sendFragmented(message_id, d_len, data);
    return;
  }

  message_id.dummy = 0; // set bits to ignore to 0
  CMSG cmsg;
  cmsg.id = message_id.value;
  cmsg.len = d_len+1;        // CAN message length must include the d_len parameter
  cmsg.data[0] = d_len;      // the SMP d_len is stored as first data byte
  for (unsigned int j = 0; j < d_len; j++)
  {
    cmsg.data[ j+1 ] = data[j];
  }

  NTCAN_RESULT rc;
  int len_cmsgs = 1;

  {
    // block the channel
    QMutexLocker mutex_locker(&mMutex);
    // write the message to CAN
    rc = canWrite(mHandle, &cmsg, &len_cmsgs, NULL);
  }

  if (rc != NTCAN_SUCCESS)
  {
    std::cout << "Failed to write stuff - no success" << std::endl;
  }
  if (len_cmsgs != 1)
  {
    std::cout << "Failed to write stuff - length not right" << std::endl;
  }
}

void cedar::dev::schunk::SMPCanBusChannel::sendFragmented(uSMPMessageID message_id, unsigned int d_len, unsigned char* data)
{
  // prepare constant part of CMSG
  message_id.dummy = 0; // set bits to ignore to 0
  CMSG cmsg;
  cmsg.id = message_id.value;

  unsigned char frag_cmd = FRAG_START;  // prepare first fragmentation command code
  int len_cmsgs;
  NTCAN_RESULT rc;
  while (d_len > 0)
  {
    //------------------------
    // prepare next fragment to send
    cmsg.len = 0;                            // initialize counter
    cmsg.data[ cmsg.len++ ] = (unsigned char) d_len; // the remaining SMP d_len is stored as first data byte
    cmsg.data[ cmsg.len++ ] = frag_cmd;      // the SMP fragmentation command code is stored as second data byte

    // pack bytes from data into cmsg until that is full or no more data to send
    while ( cmsg.len < 8  &&  d_len > 0 )
    {
      cmsg.data[ cmsg.len++ ] = *(data++);
      d_len--;
    }

    // send the prepared fragment
    len_cmsgs = 1;

    {
      // block the channel
      QMutexLocker mutex_locker(&mMutex);
      // write the message to CAN
      rc = canWrite(mHandle, &cmsg, &len_cmsgs, NULL);
    }

    if (rc != NTCAN_SUCCESS)
    {
      std::cout << "Error while writing fragmented message" << std::endl;
    }
    if (len_cmsgs != 1)
    {
      std::cout << "Error while writing fragmented message" << std::endl;
    }

    // prepare next fragmentation command code depending on amount of remaining data to send
    if ( d_len > 6 )
      frag_cmd = FRAG_MIDDLE;  // more than one more fragment needed
    else
      frag_cmd = FRAG_END;     // next one will be the last fragment
    //------------------------
  }
  return;
}

bool cedar::dev::schunk::SMPCanBusChannel::receive
   (
       uSMPMessageID* message_id,
       unsigned int* d_len,
       unsigned char* data,
       int max_data_len,
       unsigned char // do_reset
   )
{
  CMSG cmsg;

  // loop until timeout or a complete SMP message that is not
  do
  {
    // receive a frame:
    if (!this->receiveFrame(&cmsg))
    {
      // if rc!=NTCAN_SUCCESS or rc==NTCAN_RX_TIMEOUT or len_cmsgs!=1
      return false;
    }

    // copy to intermediate message_id
    uSMPMessageID rec_message_id;
    rec_message_id.value = cmsg.id;

    unsigned int rec_d_len = (unsigned int) cmsg.data[0];
    unsigned char rec_cmd_code = cmsg.data[1];

    if ( rec_cmd_code == FRAG_START  ||  rec_cmd_code == FRAG_MIDDLE  || rec_cmd_code == FRAG_END )
    {
      // cmsg is part of a fragmented message, so copy to internal buffer first.

      // use internal buffer according to module_id
      sSMPBuffer* smp_buffer_p = g_smp_buffer[ rec_message_id.module_id ];

      switch (rec_cmd_code)
      {
      case FRAG_START:
        if ( smp_buffer_p == NULL )
        {
          // no memory in internal buffer, so allocate that first
          smp_buffer_p = (sSMPBuffer*) malloc( sizeof( sSMPBuffer ) );
          g_smp_buffer[ rec_message_id.module_id ] = smp_buffer_p;
        }
        if ( smp_buffer_p == NULL )
        {
          std::cout << "allocating of memory buffer failed" << std::endl;
          return false;
        }

        // copy first fragment to the smp_buffer_p, this will overwrite any leftover data
        smp_buffer_p->message_id     = rec_message_id;
        smp_buffer_p->d_len          = rec_d_len;
        smp_buffer_p->d_len_received = 0;
        // copying of data is done below as it is common to all fragment types
        break;

      case FRAG_MIDDLE: // no break here
      case FRAG_END:
        if ( smp_buffer_p == NULL  ||  smp_buffer_p->d_len == 0 )
        {
          // we havent received a FRAG_START for this message_id, so forget the current fragment
          continue;
        }

        // check if message id matches (the module id matches of course, but the other bits might not)
        if ( smp_buffer_p->message_id.value != rec_message_id.value )
        {
          std::cout << "Some mix-up in fragmented messages" << std::endl;
          continue;
        }

        // check if d_len matches (it might not if fragments were lost)
        if ( smp_buffer_p->d_len != rec_d_len + smp_buffer_p->d_len_received )
        {
          std::cout << "Some mix-up in fragmented messages" << std::endl;
          continue;
        }
        // copying of data is done below as it is common to all fragment types
        break;
      } // end of switch

      // copy data of fragment to smp_buffer_p:
      memcpy( smp_buffer_p->data + smp_buffer_p->d_len_received, cmsg.data+2, cmsg.len-2 );
      smp_buffer_p->d_len_received += cmsg.len-2;

      if ( rec_cmd_code == FRAG_END )
      {
        // a fragmented messages was received completely, so
        // copy info to user provided data:
        *message_id = smp_buffer_p->message_id;
        *d_len      = smp_buffer_p->d_len;
        memcpy( data, smp_buffer_p->data, *d_len );

        return true; // indicate that a complete message was received correctly
      }

    } // end of is fragment
    else
    {
      // non fragmented message, so it is complete and can be returned to the caller
      if ( rec_d_len > (unsigned int)max_data_len )
      {
        std::cout << "error in receiving stuff" << std::endl;
        return false;
      }

      // copy info to user provided data:
      *message_id = rec_message_id;
      *d_len      = rec_d_len;
      memcpy( data, cmsg.data+1, *d_len );
      return true; // indicate that a complete message was received correctly
    }

  } while (1);// try as long as no timeout occurs
  return true;
}

bool cedar::dev::schunk::SMPCanBusChannel::receiveFrame(CMSG* cmsg)
{
  int len_cmsgs = 1;
  NTCAN_RESULT rc;

  {
    // block the channel
    QMutexLocker mutex_locker(&mMutex);
    // read the message on CAN
    rc = canRead(mHandle, cmsg, &len_cmsgs, NULL);
  }

  if (rc == NTCAN_RX_TIMEOUT)
  {
    return false;
  }

  if (rc != NTCAN_SUCCESS)
  {
    std::cout << "error while reading frame - did not receive NTCAN_SUCCESS" << std::endl;
    return false;
  }

  if (len_cmsgs != 1)
  {
    std::cout << "error while reading frame - length of cmsgs not 1" << std::endl;
    return false;
  }
  return true;
}

unsigned int cedar::dev::schunk::SMPCanBusChannel::baudrateToCode(unsigned long baudrate)
{
  switch (baudrate)
  {
    case 1000000 : return NTCAN_BAUD_1000;
    case  800000 : return NTCAN_BAUD_800;
    case  500000 : return NTCAN_BAUD_500;
    case  250000 : return NTCAN_BAUD_250;
    case  125000 : return NTCAN_BAUD_125;
    case  100000 : return NTCAN_BAUD_100;
    case   50000 : return NTCAN_BAUD_50;
    case   20000 : return NTCAN_BAUD_20;
    case   10000 : return NTCAN_BAUD_10;
  }
  return NTCAN_BAUD_500;
}

bool cedar::dev::schunk::SMPCanBusChannel::getState(unsigned int moduleId, bool position, bool velocity, bool current)
{
  // prepare fixed part of the message id for requests:
  uSMPMessageID message_id;
  message_id.module_id    = static_cast<unsigned short>(moduleId);
  message_id.is_module_id = 1;
  message_id.from_module  = 0;
  message_id.is_normal    = 1;

  sGetStateMode mode;
  float period=0.0;

  // set the variable to 1 when we reclaim it:
  if (position)
    mode.Position=1;
  else
    mode.Position=0;

  if (velocity)
    mode.Velocity=1;
  else
    mode.Velocity=0;

  if (current)
    mode.Current=1;
  else
    mode.Current=0;

  // create and fill buffer with data to send:
  this->getState(message_id, &period, &mode);
  return true;
}

void cedar::dev::schunk::SMPCanBusChannel::getState(uSMPMessageID message_id, float* period, sGetStateMode* mode)
{
  // add buffer space
  unsigned int d_len = sizeof(unsigned char)       // GET_STATE
                       + ((period != NULL) ? sizeof(float)         : 0)  // period (if given)
                       + ((mode   != NULL) ? sizeof(sGetStateMode) : 0); // mode (if given)

  unsigned char buffer[d_len]; // create buffer of desired length
  int index = 0;

  // fill buffer with data to send:
  buffer[index++] = GET_STATE;

  if (period != NULL)
  {
    memcpy(buffer + index, period, sizeof(float));
    index += sizeof( float );
  }

  if (mode != NULL)
  {
    memcpy(buffer+index, mode, sizeof(sGetStateMode));
    index += sizeof(sGetStateMode);
  }
  //------------------------

  // adjust message id:
  message_id.from_module = 0;

  // send prepared buffer:
  this->send(message_id, d_len, buffer);

  // read all responses
  while (processInput())
  {
    usleep(100);
  }
}

void cedar::dev::schunk::SMPCanBusChannel::movePos(unsigned int moduleId, float position, float velocity)
{
  uSMPMessageID message_id;
  message_id.module_id = static_cast<unsigned short>(moduleId);
  message_id.is_module_id = 1;
  message_id.from_module  = 0;
  message_id.is_normal    = 1;
  //float velocity = 50.0;
  float acceleration = 100.0;
  float current = 2.0;

  this->movePos(message_id, &position, &velocity, &acceleration, &current, NULL);
}

void cedar::dev::schunk::SMPCanBusChannel::movePos
     (
       uSMPMessageID message_id,
       float* position,
       float* velocity,
       float* acceleration,
       float* current,
       float* jerk
     )
{
  // add buffer space for:
  unsigned int d_len = sizeof(unsigned char)                           // MOVE_POS
                       + ((position     != NULL) ? sizeof(float) : 0)  // position (if given)
                       + ((velocity     != NULL) ? sizeof(float) : 0)  // velocity (if given)
                       + ((acceleration != NULL) ? sizeof(float) : 0)  // acceleration (if given)
                       + ((current      != NULL) ? sizeof(float) : 0)  // current (if given)
                       + ((jerk         != NULL) ? sizeof(float) : 0); // jerk (if given)

  unsigned char buffer[d_len];  // create buffer
  int index = 0;

  // fill buffer with data to send:
  buffer[index++] = MOVE_POS;

  if (position != NULL)
  {
    memcpy(buffer+index, position, sizeof(float));
    index += sizeof(float);
  }

  if (velocity != NULL)
  {
    memcpy(buffer+index, velocity, sizeof(float));
    index += sizeof(float);
  }

  if (acceleration != NULL)
  {
    memcpy(buffer+index, acceleration, sizeof(float));
    index += sizeof(float);
  }

  if (current != NULL)
  {
    memcpy(buffer+index, current, sizeof(float));
    index += sizeof(float);
  }

  if (jerk != NULL)
  {
    memcpy(buffer+index, jerk, sizeof(float));
    index += sizeof(float);
  }
  //------------------------

  // adjust message id:
  message_id.from_module = 0;

  // send prepared buffer:
  this->send(message_id, d_len, buffer);

  // read all responses
  while (processInput())
  {
    ;
  }
}

void cedar::dev::schunk::SMPCanBusChannel::moveVel(unsigned int moduleId, float velocity)
{
  uSMPMessageID message_id;
  message_id.module_id    = static_cast<unsigned short>(moduleId);
  message_id.is_module_id = 1;
  message_id.from_module  = 0;
  message_id.is_normal    = 1;
  float current = 2.0;

  this->moveVel(message_id, &velocity, &current );
}

void cedar::dev::schunk::SMPCanBusChannel::moveVel( uSMPMessageID message_id, float* velocity, float* current )
{
    // add buffer space for:
    unsigned int d_len = sizeof(unsigned char)                       // MOVE_VEL
                         + ((velocity != NULL) ? sizeof(float) : 0)  // velocity (if given)
                         + ((current  != NULL) ? sizeof(float) : 0); // current (if given)

    unsigned char buffer[d_len]; // create buffer
    int index = 0;

    // fill buffer with data to send:
    buffer[index++] = MOVE_VEL;

    if (velocity != NULL)
    {
      memcpy(buffer+index, velocity, sizeof(float));
      index += sizeof(float);
    }

    if (current != NULL)
    {
      memcpy(buffer+index, current, sizeof(float));
      index += sizeof(float);
    }
    //------------------------

    // adjust message id:
    message_id.from_module = 0;

    // send prepared buffer:
    this->send(message_id, d_len, buffer);

    // read all responses
    while (processInput())
    {
      ;
    }
}

bool cedar::dev::schunk::SMPCanBusChannel::processInput()
{
  // get response from module

  uSMPMessageID response_id;
  unsigned int d_len;
  unsigned char data[256];

  // try to receive next message from interface:
  bool rc = this->receive(&response_id, &d_len, data, 256, 1);

  if (rc)
  {
    // and handle it:
    this->parseResponse(response_id, d_len, data);
  }
  else
  {
    // probably time-out, do nothing
  }

  return rc;
}

void cedar::dev::schunk::SMPCanBusChannel::acknowledge(unsigned int moduleId)
{
  uSMPMessageID message_id;
  message_id.module_id    = static_cast<unsigned short>(moduleId);
  message_id.is_module_id = 1;
  message_id.from_module  = 0;
  message_id.is_normal    = 1;

  this->acknowledge(message_id);
}

void cedar::dev::schunk::SMPCanBusChannel::acknowledge(uSMPMessageID message_id)
{
  unsigned int d_len = sizeof(unsigned char); // CMD_ACK

  unsigned char buffer[d_len];  // create buffer
  int index = 0;

  // fill buffer with data to send:
  buffer[index++] = CMD_ACK;

  // adjust message id:
  message_id.from_module = 0;

  // send prepared buffer:
  this->send(message_id, d_len, buffer);

  // read all responses
  while (processInput())
  {
    ;
  }
}

void cedar::dev::schunk::SMPCanBusChannel::reference(unsigned int moduleId)
{
  uSMPMessageID message_id;
  message_id.module_id = static_cast<unsigned short>(moduleId);
  message_id.is_module_id = 1;
  message_id.from_module  = 0;
  message_id.is_normal    = 1;

  this->reference(message_id);
}

void cedar::dev::schunk::SMPCanBusChannel::reference(uSMPMessageID message_id)
{
  // add buffer space for:
  unsigned int d_len = sizeof(unsigned char); // CMD_REFERENCE

  unsigned char buffer[d_len]; // create buffer
  int index = 0;

  // fill buffer with data to send:
  buffer[index++] = CMD_REFERENCE;

  // adjust message id:
  message_id.from_module = 0;

  // send prepared buffer:
  this->send(message_id, d_len, buffer);

  // read all responses:
  while (processInput())
  {
    ;
  }
}

double cedar::dev::schunk::SMPCanBusChannel::getLastJointPosition(unsigned int i)
{
  //returns the stored position:
  return mpos[i];
}

double cedar::dev::schunk::SMPCanBusChannel::getLastJointVelocity(unsigned int i)
{
  //returns the stored velocity:
  return mvel[i];
}

double cedar::dev::schunk::SMPCanBusChannel::getLastJointCurrent(unsigned int i)
{
  //returns the stored current:
  return mcur[i];
}

double cedar::dev::schunk::SMPCanBusChannel::getLastJointDuration(unsigned int i)
{
  //returns the stored duration:
  return mdur[i];
}

bool cedar::dev::schunk::SMPCanBusChannel::getref(unsigned int id)
{
  //returns true if it is already referenced:
  return ref[id].bit.REFERENCED;
}

bool cedar::dev::schunk::SMPCanBusChannel::parseResponse(uSMPMessageID message_id, unsigned int d_len, unsigned char* data)
{
  // there must be at least one data byte, the command code:
  if (d_len < 1)
  {
    cout << "SMPRC INVALID PARAMETER" << endl;
    return false;
  }

  // we parse a response message, so it must be from module, not from master:
  if (message_id.from_module != 1)
  {
    cout << "SMPRC PARSE ERROR" << endl;
    return false;
  }

  int index = 1;               // parameters (if any) start at index 1
  unsigned char still_matches; // Flag, whether the actual message still matches the currently tested one'

  switch (data[0])                        // switch according to command code in data byte 0
  {
    case CMD_ACK:
    {
      // There exist several messages with command code CMD_ACK!
      // Try to determine message from its d_len (and values of fixed parameters):

      if ( d_len == sizeof( unsigned char ) + sizeof( char[2] ) )
      {
        still_matches = 1;                // init flag
            index         = 1;                // reinit index of parameters

            // extract and check the parameters of the message (if any):

            // first space for some helper variables (needed since values must be aligned in memory):
            char _OK[2] = { 'O', 'K' };                     // set to fixed expected value for comparison

            if ( still_matches  &&  index + sizeof( char[2] ) <= d_len )
            {
              // fixed parameters are just checked, not forwarded to the callback function:
              if ( memcmp( _OK, data+index, sizeof( char[2] ) ) != 0 )
                // Not the expected value, so actual message does no longer match the currently tested one.
                // (=>try another message with same command code and length or indicate error)
                still_matches = 0;
              index += sizeof( char[2] );
            }

            // finally call the callback function - if it still matches - with the extracted parameters:
            if ( still_matches )
              return true;
      }
      if ( d_len == sizeof( unsigned char ) + sizeof( unsigned char ) )
      {
        still_matches = 1;                              // init flag
        index         = 1;                              // reinit index of parameters

        // extract and check the parameters of the message (if any):

        // first space for some helper variables (needed since values must be aligned in memory):
        unsigned char command_code;
        unsigned char error_code;

        if ( still_matches  &&  index + sizeof( unsigned char ) <= d_len )
        {
          // copy from data buffer to local variable (necessary due to memory-alignment)
          // (command code is at index 0)
          memcpy( &command_code, data+0, sizeof( unsigned char ) );
          cout << "Command Code: " << static_cast<unsigned int>(command_code) << endl;
        }

        if ( still_matches  &&  index + sizeof( unsigned char ) <= d_len )
        {
          // copy from data buffer to local variable (necessary due to memory-alignment)
          memcpy( &error_code, data+index, sizeof( unsigned char ) );
          index += sizeof( unsigned char );
          cout << "Error Code: " << static_cast<unsigned int>(error_code) << endl;
        }

        // finally call the callback function - if it still matches - with the extracted parameters:
        if ( still_matches )
        {
          cout << "Received ERROR CODE" << endl;
          return false;
        }

      }
      // None of the above if clauses matched or incorrect data in message!
      // (unknown d_len or wrong values of fixed parameters)
      cout << "SMPRC PARSE ERROR CMD_ACK" << endl;
      return false;

    }  // end of case
    //--------------------------------------------
    case CMD_REFERENCE:
    {
      // There exist several messages with command code CMD_REFERENCE!
      // Try to determine message from its d_len (and values of fixed parameters):

      if ( d_len == sizeof( unsigned char ) + sizeof( char[2] ) )
      {
        still_matches = 1;                              // init flag
        index         = 1;                              // reinit index of parameters

        // extract and check the parameters of the message (if any):

        // first space for some helper variables (needed since values must be aligned in memory):
        char _OK[2] = { 'O', 'K' };                     // set to fixed expected value for comparison

        if ( still_matches  &&  index + sizeof( char[2] ) <= d_len )
        {
          // fixed parameters are just checked, not forwarded to the callback function:
          if ( memcmp( _OK, data+index, sizeof( char[2] ) ) != 0 )
            // Not the expected value, so actual message does no longer match the currently tested one.
            // (=>try another message with same command code and length or indicate error)
            still_matches = 0;
          index += sizeof( char[2] );
        }

        // finally call the callback function - if it still matches - with the extracted parameters:
        if ( still_matches )
          return true;
      }
      if ( d_len == sizeof( unsigned char ) + sizeof( unsigned char ) )
      {
        still_matches = 1;                              // init flag
        index         = 1;                              // reinit index of parameters

        // extract and check the parameters of the message (if any):

        // first space for some helper variables (needed since values must be aligned in memory):
        unsigned char command_code;
        unsigned char error_code;

        if ( still_matches  &&  index + sizeof( unsigned char ) <= d_len )
        {
          // copy from data buffer to local variable (necessary due to memory-alignment)
          // (command code is at index 0)
          memcpy( &command_code, data+0, sizeof( unsigned char ) );
          cout << "Command Code: " << static_cast<unsigned int>(command_code) << endl;
        }

        if ( still_matches  &&  index + sizeof( unsigned char ) <= d_len )
        {
          // copy from data buffer to local variable (necessary due to memory-alignment)
          memcpy( &error_code, data+index, sizeof( unsigned char ) );
          index += sizeof( unsigned char );
          cout << "Error Code: " << static_cast<unsigned int>(error_code) << endl;
        }

        // finally call the callback function - if it still matches - with the extracted parameters:
        if ( still_matches )
        {
          cout << "Received ERROR CODE" << endl;
          return false;
        }

      }

      // None of the above if clauses matched or incorrect data in message!
      // (unknown d_len or wrong values of fixed parameters)
      cout << "SMPRC PARSE ERROR CMD_REFERENCE" << endl;
      return false;

    }  // end of case
    //--------------------------------------------
    case MOVE_POS:
    {
      // There exist several messages with command code MOVE_POS!
      // Try to determine message from its d_len (and values of fixed parameters):

      if ( d_len == sizeof( unsigned char ) + sizeof( char[2] ) )
      {
        still_matches = 1;                              // init flag
        index         = 1;                              // reinit index of parameters

        // extract and check the parameters of the message (if any):

        // first space for some helper variables (needed since values must be aligned in memory):
        char _OK[2] = { 'O', 'K' };                     // set to fixed expected value for comparison

        if ( still_matches  &&  index + sizeof( char[2] ) <= d_len )
        {
          // fixed parameters are just checked, not forwarded to the callback function:
          if ( memcmp( _OK, data+index, sizeof( char[2] ) ) != 0 )
            // Not the expected value, so actual message does no longer match the currently tested one.
            // (=>try another message with same command code and length or indicate error)
            still_matches = 0;
          index += sizeof( char[2] );
        }

      }
      if ( d_len == sizeof( unsigned char ) + sizeof( float ) )
      {
        still_matches = 1;                              // init flag
        index         = 1;                              // reinit index of parameters

        // extract and check the parameters of the message (if any):

        // first space for some helper variables (needed since values must be aligned in memory):
        float duration;

        if ( still_matches  &&  index + sizeof( float ) <= d_len )
        {
          // copy from data buffer to local variable (necessary due to memory-alignment)
          memcpy( &duration, data+index, sizeof( float ) );
          index += sizeof( float );
          // store the duration
          mdur[message_id.module_id]=duration;
        }

        // finally call the callback function - if it still matches - with the extracted parameters:
        if ( still_matches )
        {
          //cout << "Moving module " << message_id.module_id << "will take " << duration << "seconds" << endl;
          return true;
        }

      }
      if ( d_len == sizeof( unsigned char ) + sizeof( unsigned char ) )
      {
        still_matches = 1;                              // init flag
        index         = 1;                              // reinit index of parameters

        // extract and check the parameters of the message (if any):

        // first space for some helper variables (needed since values must be aligned in memory):
        unsigned char command_code;
        unsigned char error_code;

        if ( still_matches  &&  index + sizeof( unsigned char ) <= d_len )
        {
          // copy from data buffer to local variable (necessary due to memory-alignment)
          // (command code is at index 0)
          memcpy( &command_code, data+0, sizeof( unsigned char ) );
          cout << "Command Code: " << static_cast<unsigned int>(command_code) << endl;
        }

        if ( still_matches  &&  index + sizeof( unsigned char ) <= d_len )
        {
          // copy from data buffer to local variable (necessary due to memory-alignment)
          memcpy( &error_code, data+index, sizeof( unsigned char ) );
          index += sizeof( unsigned char );
          cout << "Error Code: " << static_cast<unsigned int>(error_code) << endl;
        }

        // finally call the callback function - if it still matches - with the extracted parameters:
        if ( still_matches )
        {
          cout << "Received ERROR CODE in MOVE_POS" << endl;
          return false;
        }

      }

      // None of the above if clauses matched or incorrect data in message!
      // (unknown d_len or wrong values of fixed parameters)
      cout << "SMPRC PARSE ERROR MOVE_POS" << endl;
      return false;

    }  // end of case
    //--------------------------------------------
    case GET_STATE:
    {
      // There exist several messages with command code GET_STATE!
      // Try to determine message from its d_len (and values of fixed parameters):

      if ( d_len == sizeof( unsigned char ) + sizeof( union uSTATE_FLAGS )
          || d_len == sizeof( unsigned char ) + sizeof( float ) + sizeof( union uSTATE_FLAGS )
          || d_len == sizeof( unsigned char ) + sizeof( float ) + sizeof( float ) + sizeof( union uSTATE_FLAGS )
          || d_len == sizeof( unsigned char ) + sizeof( float ) + sizeof( float ) + sizeof( float ) + sizeof( union uSTATE_FLAGS ) )
      {
        still_matches = 1;                              // init flag
        index         = 1;                              // reinit index of parameters

        // extract and check the parameters of the message (if any):

        // first space for some helper variables (needed since values must be aligned in memory):
        float* position = NULL;
        float position_value;
        float* velocity = NULL;
        float velocity_value;
        float* current = NULL;
        float current_value;
        union uSTATE_FLAGS status;

        if ( still_matches  &&  index + sizeof( float ) <= d_len )
        {
          // copy from data buffer to local variable (necessary due to memory-alignment)
          memcpy( &position_value, data+index, sizeof( float ) );
          position = &position_value;
          index += sizeof( float );
          still_matches = (unsigned int)index < d_len;
          // store the current position (degree must be converted to radian)
          mpos[message_id.module_id]=((*position)*(M_PI)/180);
        }

        if ( still_matches  &&  index + sizeof( float ) <= d_len )
        {
          // copy from data buffer to local variable (necessary due to memory-alignment)
          memcpy( &velocity_value, data+index, sizeof( float ) );
          velocity = &velocity_value;
          index += sizeof( float );
          still_matches = (unsigned int)index < d_len;
          // store the current velocity (degree must be converted to radian)
          mvel[message_id.module_id]=((*velocity)*(M_PI)/180);
        }

        if ( still_matches  &&  index + sizeof( float ) <= d_len )
        {
          // copy from data buffer to local variable (necessary due to memory-alignment)
          memcpy( &current_value, data+index, sizeof( float ) );
          current = &current_value;
          index += sizeof( float );
          still_matches = (unsigned int)index < d_len;
          // store the current
          mcur[message_id.module_id]=*current;
        }

        if ( still_matches  &&  index + sizeof( union uSTATE_FLAGS ) <= d_len )
        {
          // copy from data buffer to local variable (necessary due to memory-alignment)
          memcpy( &status, data+index, sizeof( union uSTATE_FLAGS ) );
          index += sizeof( union uSTATE_FLAGS );
          // when it is already referenced, store
          ref[message_id.module_id].bit.REFERENCED=status.bit.REFERENCED;
        }

        // finally call the callback function - if it still matches - with the extracted parameters:
        if ( still_matches )
          return true;
      }
      if ( d_len == sizeof( unsigned char ) + sizeof( unsigned char ) )
      {
        still_matches = 1;                              // init flag
        index         = 1;                              // reinit index of parameters

        // extract and check the parameters of the message (if any):

        // first space for some helper variables (needed since values must be aligned in memory):
        unsigned char command_code;
        unsigned char error_code;

        if ( still_matches  &&  index + sizeof( unsigned char ) <= d_len )
        {
          // copy from data buffer to local variable (necessary due to memory-alignment)
          // (command code is at index 0)
          memcpy( &command_code, data+0, sizeof( unsigned char ) );
          cout << "Command Code: " << static_cast<unsigned int>(command_code) << endl;
        }

        if ( still_matches  &&  index + sizeof( unsigned char ) <= d_len )
        {
          // copy from data buffer to local variable (necessary due to memory-alignment)
          memcpy( &error_code, data+index, sizeof( unsigned char ) );
          index += sizeof( unsigned char );
          cout << "Error Code: " << static_cast<unsigned int>(error_code) << endl;
        }

        // finally call the callback function - if it still matches - with the extracted parameters:
        if ( still_matches )
        {
          //SMP_RETURN( SMP_Confirm(  smp_if, message_id, command_code, error_code ) );
          cout << "Received ERROR CODE in GET_STATE" << endl;
          return false;
        }

      }

      // None of the above if clauses matched or incorrect data in message!
      // (unknown d_len or wrong values of fixed parameters)
      cout << "SMPRC PARSE ERROR GET_STATE" << endl;
      return false;
    } // end of case
    //--------------------------------------------
    case SET_TARGET_ACC:
    {
      //cout << "Der Befehl SET TARGET ACC wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case MOVE_PWM:
    {
      //cout << "Der Befehl MOVE PWM wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case MOVE_VEL:
    {
      // There exist several messages with command code MOVE_VEL!
      // Try to determine message from its d_len (and values of fixed parameters):

      if ( d_len == sizeof( unsigned char ) + sizeof( char[2] ) )
      {
        still_matches = 1;                              // init flag
        index         = 1;                              // reinit index of parameters

        // extract and check the parameters of the message (if any):

        // first space for some helper variables (needed since values must be aligned in memory):
        char _OK[2] = { 'O', 'K' };                     // set to fixed expected value for comparison

        if ( still_matches  &&  index + sizeof( char[2] ) <= d_len )
        {
          // fixed parameters are just checked, not forwarded to the callback function:
          if ( memcmp( _OK, data+index, sizeof( char[2] ) ) != 0 )
            // Not the expected value, so actual message does no longer match the currently tested one.
            // (=>try another message with same command code and length or indicate error)
            still_matches = 0;
          index += sizeof( char[2] );
        }

        // finally call the callback function - if it still matches - with the extracted parameters:
        if ( still_matches )
          return true;
      }
      if ( d_len == sizeof( unsigned char ) + sizeof( unsigned char ) )
      {
        still_matches = 1;                              // init flag
        index         = 1;                              // reinit index of parameters

        // extract and check the parameters of the message (if any):

        // first space for some helper variables (needed since values must be aligned in memory):
        unsigned char command_code;
        unsigned char error_code;

        if ( still_matches  &&  index + sizeof( unsigned char ) <= d_len )
        {
          // copy from data buffer to local variable (necessary due to memory-alignment)
          // (command code is at index 0)
          memcpy( &command_code, data+0, sizeof( unsigned char ) );
        }

        if ( still_matches  &&  index + sizeof( unsigned char ) <= d_len )
        {
          // copy from data buffer to local variable (necessary due to memory-alignment)
          memcpy( &error_code, data+index, sizeof( unsigned char ) );
          index += sizeof( unsigned char );
          cout << "Error Code: " << static_cast<unsigned int>(error_code) << endl;
        }

        // finally call the callback function - if it still matches - with the extracted parameters:
        if ( still_matches )
        {
          //SMP_RETURN( SMP_Confirm(  smp_if, message_id, command_code, error_code ) );
          cout << "Received ERROR CODE in MOVE_VEL" << endl;
          return false;
        }

      }

      // None of the above if clauses matched or incorrect data in message!
      // (unknown d_len or wrong values of fixed parameters)
      cout << "SMPRC PARSE ERROR MOVE_VEL" << endl;
      return false;
    }  // end of case
    //--------------------------------------------
      case CMD_STOP:
    {
      //cout << "Der Befehl CMD STOP wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case CHECK_PC_MC_COMMUNICATION:
    {
      //cout << "Der Befehl CHECK PC MC COMMUNICATION wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case GET_TEMPERATURE:
    {
      //cout << "Der Befehl GET TEMPERATURE wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case CHANGE_USER:
    {
      //cout << "Der Befehl CHANGE USER wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case CMD_WARNING:
    {
      //cout << "Der Befehl CMD WARNING wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case CMD_INFO:
    {
      //cout << "Der Befehl CMD INFO wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case CMD_GET_DIO:
    {
      //cout << "Der Befehl CMD GET DIO wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case CMD_DIO:
    {
      //cout << "Der Befehl CMD DIO wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case CMD_MOVE_BLOCKED:
    {
      //cout << "Der Befehl CMD MOVE BLOCKED wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case SET_TARGET_JERK:
    {
      //cout << "Der Befehl SET TARGET JERK wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case GET_DETAILED_ERROR_INFO:
    {
      //cout << "Der Befehl GET DETAILED ERROR INFO wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case SET_TARGET_POS_REL:
    {
      //cout << "Der Befehl SET TARGET POS REL wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case CMD_TOGGLE_IMPULSE_MESSAGE:
    {
      //cout << "Der Befehl CMD TOGGLE IMPULSE MESSAGE wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case SET_TARGET_TIME:
    {
      //cout << "Der Befehl SET TARGET TIME wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case MOVE_POS_TIME:
    {
      //cout << "Der Befehl MOVE POS TIME wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case MOVE_SRU_POS:
    {
      //cout << "Der Befehl MOVE SRU POS wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case CMD_DISCONNECT:
    {
      //cout << "Der Befehl CMD DISCONNECT wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case MOVE_POS_REL:
    {
      //cout << "Der Befehl MOVE POS REL wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case SET_CONFIG:
    {
      //cout << "Der Befehl SET CONFIG wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case SET_CONFIG_EXT:
    {
      //cout << "Der Befehl SET CONFIG EXT wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case CMD_SET_DIO:
    {
      //cout << "Der Befehl CMD SET DIO wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case CMD_ERROR:
    {
      //cout << "Der Befehl CMD ERROR wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case GET_CONFIG_EXT:
    {
      //cout << "Der Befehl GET CONFIG EXT wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case MOVE_CUR:
    {
      //cout << "Der Befehl MOVE CUR wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case SET_TARGET_POS:
    {
      //cout << "Der Befehl SET TARGET POS wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case SET_TARGET_VEL:
    {
      //cout << "Der Befehl SET TARGET VEL wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case SET_TARGET_CUR:
    {
      //cout << "Der Befehl SET TARGET CUR wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case GET_CONFIG:
    {
      //cout << "Der Befehl GET CONFIG wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case CHECK_MC_PC_COMMUNICATION:
    {
      //cout << "Der Befehl CHECK MC PC COMMUNICATION wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case CMD_FLASH_MODE:
    {
      //cout << "Der Befehl CMD FLASH MODE wurde erkannt" << endl;
      return true;
    }  // end of case
    //--------------------------------------------
    case CMD_POS_REACHED:
    {
      // There exist several messages with command code CMD_POS_REACHED!
      // Try to determine message from its d_len (and values of fixed parameters):

      if ( d_len == sizeof( unsigned char ) + sizeof( float ) )
      {
        still_matches = 1;                              // init flag
        index         = 1;                              // reinit index of parameters

        // extract and check the parameters of the message (if any):

        // first space for some helper variables (needed since values must be aligned in memory):
        float position;

        if ( still_matches  &&  index + sizeof( float ) <= d_len )
        {
          // copy from data buffer to local variable (necessary due to memory-alignment)
          memcpy( &position, data+index, sizeof( float ) );
          index += sizeof( float );
          //cout << "Position: " << position << endl;
          //mpos[message_id.module_id]=position;
        }

        // finally call the callback function - if it still matches - with the extracted parameters:
        if ( still_matches )
          return true;
      }
      if ( d_len == sizeof( unsigned char ) + sizeof( unsigned char ) )
      {
        still_matches = 1;                              // init flag
        index         = 1;                              // reinit index of parameters

        // extract and check the parameters of the message (if any):

        // first space for some helper variables (needed since values must be aligned in memory):
        unsigned char command_code;
        unsigned char error_code;

        if ( still_matches  &&  index + sizeof( unsigned char ) <= d_len )
        {
          // copy from data buffer to local variable (necessary due to memory-alignment)
          // (command code is at index 0)
          memcpy( &command_code, data+0, sizeof( unsigned char ) );
          cout << "Command Code: " << static_cast<unsigned int>(command_code) << endl;
        }

        if ( still_matches  &&  index + sizeof( unsigned char ) <= d_len )
        {
          // copy from data buffer to local variable (necessary due to memory-alignment)
          memcpy( &error_code, data+index, sizeof( unsigned char ) );
          index += sizeof( unsigned char );
          cout << "Error Code: " << static_cast<unsigned int>(error_code) << endl;
        }

        // finally call the callback function - if it still matches - with the extracted parameters:
        if ( still_matches )
        {
          //SMP_RETURN( SMP_Confirm(  smp_if, message_id, command_code, error_code ) );
          cout << "Received ERROR CODE in POS_REACHED" << endl;
          return false;
        }

      }

      // None of the above if clauses matched or incorrect data in message!
      // (unknown d_len or wrong values of fixed parameters)
      cout << "SMPRC PARSE ERROR CMD_POS_REACHED" << endl;
      return false;

    }  // end of case
    //--------------------------------------------
    default:
    {
      // unknown command code:
      cout << "SMPRC UNKNOWN COMMAND" << endl;
      return false;
    }
  } // end of switch command-code

  // we should never get here:
  cout << "SMPRC UNKNOWN ERROR" << endl;
  return false;
}
