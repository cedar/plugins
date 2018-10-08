/*======================================================================================================================

    THIS FILE CONTAINS PROPRIETARY CODE BY SCHUNK. LIMITED CIRCULATION ONLY.
    We cannot distribute this code under the GPL license.

========================================================================================================================

    Institute:   Ruhr-Universitaet Bochum
                 Institut fuer Neuroinformatik

    File:        SMBCanBusChannel.h

    Maintainer:  Mathis Richter
    Email:       mathis.richter@ini.rub.de
    Date:        2014 12 02

    Description:

    Credits:     Ceyhun Pekis; Schunk

======================================================================================================================*/

#ifndef CEDAR_DEV_SCHUNK_SMP_CAN_BUS_CHANNEL_H
#define CEDAR_DEV_SCHUNK_SMP_CAN_BUS_CHANNEL_H

// LOCAL INCLUDES
#include "devices/schunk/SMPCanBusChannel.fwd.h"

// CEDAR INCLUDES
#include <cedar/auxiliaries/UIntParameter.h>
#include <cedar/auxiliaries/IntParameter.h>
#include <cedar/devices/Channel.h>

// SYSTEM INCLUDES
#include <ntcan.h>
#include <QMutex>


/*!@brief This class provides CAN bus access for robotic hardware implementing the Schunk motion protocol (SMP).
 *
 * @todo describe more.
 */
class cedar::dev::schunk::SMPCanBusChannel : public cedar::dev::Channel
{
  //--------------------------------------------------------------------------------------------------------------------
  // nested types
  //--------------------------------------------------------------------------------------------------------------------

  //! datatype to describe a SMP message ID
  typedef union
  {
    struct
    {
      unsigned short module_id    : 8; //!< Bits 0-7: SCHUNK group or module ID
      unsigned short is_module_id : 1; //!< Bit    8: flag, if set then \a module_id is a module ID else a group ID
      unsigned short from_module  : 1; //!< Bit   10: flag, if set then message is from module (slave) to master else from master to module (slave)
      unsigned short is_normal    : 1; //!< Bit   11: flag, if set then message is a 'normal' message, else a 'diagnosis' message
      unsigned short dummy        : 5; //!< remaining bits are unused and ignored
    };

    unsigned short value;              //!< the value of all bits combined
  } uSMPMessageID;

  //! mode selector for GET_STATE requests/indications
  typedef struct
  {
    unsigned char Position : 1; //!< flag, if 1 then position is requested and will be included in the response
    unsigned char Velocity : 1; //!< flag, if 1 then velocity is requested and will be included in the response
    unsigned char Current  : 1; //!< flag, if 1 then electric current is requested and will be included in the response
    unsigned char dummy    : 5; //!< unused
  } sGetStateMode;

  //! buffer for a complete SMP message with data
  typedef struct
  {
    uSMPMessageID message_id;     //!< the message_id of the (fragmented) SMP message
    unsigned int  d_len;          //!< the (final) d_len of the (fragmented) SMP message (not all bytes might be present (yet) in \a data, see \a d_len_received
    unsigned int  d_len_received; //!< the current data length in \a data of a partially received SMP message
    unsigned char data[256];      //!< the data of a (fragmented) SMP message
  } sSMPBuffer;

  // comm mode
  enum eCommModeType
  {
    COMM_MODE_AUTO,
    COMM_MODE_RS232,
    COMM_MODE_CAN,
    COMM_MODE_PROFI_BUS,
    COMM_MODE_RS232_NO_IMPULSE_MESSAGE,
    COMM_MODE_CANOPEN,
    COMM_MODE_NONE,
    MAX_COMMMODE
  };

  // reference type
  enum eReferenceType
  {
    REF_SWITCH_INTERN_LEFT,
    REF_SWITCH_INTERN_RIGHT,
    REF_SWITCH_EXTERN_LEFT,
    REF_SWITCH_EXTERN_RIGHT,
    REF_VEL_LEFT,
    REF_VEL_RIGHT,
    REF_VEL_LEFT_DIST,
    REF_VEL_RIGHT_DIST,
    REF_CURRENT_LEFT,
    REF_CURRENT_RIGHT,
    REF_CURRENT_LEFT_DIST,
    REF_CURRENT_RIGHT_DIST,
    REF_NONE,
    REF_HAND,
    MAX_REFTYPE
  };

  // motor type
  enum eMotorType
  {
    MOTOR_DC,
    MOTOR_BLDC,
    MOTOR_PMSM,
    MOTOR_TORQUE,
    MOTOR_STEPPER,
    MAX_MOTORTYPE
  };

  // pos system type
  enum ePosType
  {
    POS_ENCODER,
    POS_ENCODER_INDEX,
    POS_RESOLVER,
    POS_SSI,
    POS_ENDAT,
    POS_SIN_COS,
    POS_ENCODER_DIFF,
    POS_ENCODER_DIFF_INDEX,
    POS_ANALOG,
    POS_HALL,
    POS_MASTER_MOTOR,
    POS_SLAVE_MOTOR,
    MAX_POSTYPE
  };

  // pos system mount
  enum ePosMount
  {
    MOUNT_INPUT_SIDE,
    MOUNT_OUTPUT_SIDE,
    MOUNT_MIDDLE_SIDE,
    MOUNT_POWER_BALL,
    MAX_POSMOUNT
  };

  // serial high baud
  enum eSerialHighBaud
  {
    SERIAL_HIGH_BAUD_UNKNOWN1,
    SERIAL_HIGH_BAUD_NO,
    SERIAL_HIGH_BAUD_YES,
    SERIAL_HIGH_BAUD_UNKNOWN2,
    MAX_SERIAL_HIGH_BAUD
  };

  // ramp mode
  enum eRampModeType
  {
    RAMP_TRAPEZ,
    RAMP_JERK,
    RAMP_TRAPEZ_SRU,
    RAMP_NO_RAMP,
    RAMP_LINEAR,
    MAX_RAMPMODE
  };

  // unit type
  enum eUnitType
  {
    UNIT_MM,
    UNIT_M,
    UNIT_INCH,
    UNIT_RAD,
    UNIT_GRAD,
    UNIT_INTERN,
    UNIT_MICRO_M_INTEGER,
    UNIT_MICRO_GRAD_INTEGER,
    UNIT_MICRO_INCH_INTEGER,
    UNIT_MILLI_GRAD_INTEGER,
    MAX_UNITSYSTEM
  };

  // user type
  enum eUserType
  {
    USER_GUEST,
    USER_DIAG,
    USER_PROFI,
    USER_ADVANCED,
    USER_ROOT,
    USER_WIZARD,
    MAX_USER
  };

  // controller type
  enum eControllerType
  {
    CONTROLLER_CURRENT_SPEED,
    CONTROLLER_CASCADE,
    CONTROLLER_SPEED_CUR_LIMIT,
    CONTROLLER_SPEED_PWM_LIMIT,
    CONTROLLER_POS_CASCADE,
    MAX_CONTROL_MODE
  };

  // brake usage
  enum eBrakeUsageType
  {
    BRAKE_USE_NO_USE,
    BRAKE_USE_ERROR_ONLY,
    BRAKE_USE_NORMAL,
    MAX_BRAKE_USAGE
  };

  // brake type
  enum eBrakeType
  {
    BRAKE_TYPE_NONE,
    BRAKE_TYPE_MAGNETIC_12V,
    BRAKE_TYPE_MAGNETIC_24V,
    BRAKE_TYPE_MAGNETIC_48V,
    MAX_BRAKETYPE
  };

  // digital input usage
  enum eDigitalINType
  {
    DIGITAL_IN_NORMAL,
    DIGITAL_IN_PROGRAM,
    DIGITAL_IN_HW1_EXT,
    DIGITAL_IN_HW1_HW2_EXT,
    DIGITAL_IN_HW1_REF_EXT,
    DIGITAL_IN_HW1_REF_HW2_EXT,
    DIGITAL_IN_TRIGGER_INPUT,
    MAX_IN_TYPE
  };

  // digital output usage
  enum eDigitalOUTType
  {
    DIGITAL_OUT_NORMAL,
    DIGITAL_OUT_STATE_OUT2_MOVING,
    DIGITAL_OUT_STATE_OUT2_POS_REACHED,
    DIGITAL_OUT_STATE_OUT2_BRAKE,
    DIGITAL_OUT_STATE_OUT2_WARNING,
    DIGITAL_OUT_STATE_OUT2_PHRASE_MODE,
    MAX_OUT_TYPE
  };

  // analog output usage
  enum eAnalogOutType
  {
    ANALOG_OUT_NONE,
    ANALOG_OUT_POS,
    ANALOG_OUT_SPEED,
    ANALOG_OUT_CURRENT,
    ANALOG_OUT_MAX,
    MAX_ANALOG_OUT_TYPE
  };

  // analog input usage
  enum eAnalogInType
  {
    EXTERN_IN_NOT_USED,
    EXTERN_IN_POS,
    EXTERN_IN_SPEED,
    EXTERN_IN_CURRENT,
    EXTERN_IN_PWM,
    MAX_EXTERN_IN_TYPE
  };

  // internal switch usage
  enum eInternalInUsage
  {
    INTERNAL_SWITCH_NONE,
    INTERNAL_SWITCH_HW1,
    INTERNAL_SWITCH_HW1_HW2,
    MAX_INTRNAL_SWITCH_TYPE
  };

  // commutation modes
  enum eCommutationMode
  {
    COMMUTATION_STEPPER,
    COMMUTATION_INDUCTANCE,
    COMMUTATION_DITHER,
    COMMUTATION_NONE,
    MAX_COMMUTATION_MODE
  };

  // Zwischenanschlag ZP
  enum eIntermediateStop
  {
    INTERMEDIATE_STOP_NONE,
    INTERMEDIATE_STOP_ACTIVE,
    INTERMEDIATE_STOP_MAX
  };

  // extern Start
  enum eExternStart
  {
    EXTERN_START_NONE,
    EXTERN_START_DIGITAL_IN_FROM_POS_B,
    EXTERN_START_DIGITAL_IN_FROM_POS_C,
    EXTERN_START_DIGITAL_IN_FROM_POS_B_C,
    EXTERN_START_COMMAND_FROM_POS_B,
    EXTERN_START_COMMAND_FROM_POS_C,
    EXTERN_START_COMMAND_FROM_POS_B_C,
    EXTERN_START_MAX
  };

  // valve status
  enum eValveState
  {
    VALVE_STATE_OFF,
    VALVE_STATE_OVEREXCITATION,
    VALVE_STATE_HOLD,MAX_VALVE_STATE
  };

  // wait time conditions in POSB and POSC
  enum eDirectionWaitPosBPosCType
  {
    COMING_FROM_FORWARD,
    COMING_FROM_BACKWARD,
    COMING_FROM_FORWARD_AND_BACKWARD,
    MAX_COMING_FROM
  };

  // gripper
  enum eGripperAvailable
  {
    GRIPPER_NONE,GRIPPER_UNIT,
    GRIPPER_ROTATION_UNIT_ROTATE_RIGHT,
    GRIPPER_ROTATION_UNIT_ROTATE_LEFT,
    GIPPER_UNIT_MAX
  };

  enum eGrippPosAPosD
  {
    GRIPP_IN_POSA,
    GRIPP_IN_POSD,
    GRIPP_MAX
  };

  // program states
  enum eValveProgramState
  {
    PROG_INIT,
    START,
    UH_RIGHT,
    ZP_RIGHT,
    UV_RIGHT,
    UH_LEFT,
    UV_LEFT,
    ZP_LEFT,
    GRIPPER_ROTATE_IN_POSB,
    GRIPPER_ROTATE_IN_POSC,
    GRIPPER_OPEN,
    GRIPPER_CLOSE,
    LOOP_END,
    GRIPP_IN_POS_A,
    GRIPP_IN_POS_D,
    END
  };

  struct sDEVICE_FLAGS
  {
    enum eUnitType        unitSystem               : 5;  // 0..4    | Offset 0
    enum eCommModeType    commMode                 : 5;  // 4..9    | Offset 0/1
    bool                  invertMotorDir           : 1;  // 10      | Offset 1
    bool                  invertPosSystem          : 1;  // 11      | Offset 1
    enum eRampModeType    posRampType              : 3;  // 12..14  | Offset 1
    bool                  startFromEEPROM          : 1;  // 15      | Offset 1
    bool                  endless                  : 1;  // 16      | Offset 2
    bool                  moveZeroAfterReferencing : 1;  // 17      | Offset 2
    enum eControllerType  controlerStructur        : 3;  // 18..20  | Offset 2
    enum eDigitalINType   digitalInUsage           : 3;  // 21..23  | Offset 2
    enum eDigitalOUTType  digitalOutUsage          : 3;  // 24..26  | Offset 3
    enum eAnalogOutType   analogOut_InUsage        : 3;  // 27..29  | Offset 3
    enum eInternalInUsage internalInUsage          : 2;  // 30..31  | Offset 3
  };

  union uDEVICE_FLAGS
  {
    unsigned int          all;                           //         | Offset 0..3
    struct sDEVICE_FLAGS  bit;                           //         | Offset 0..3
  };

  struct sSTATE_FLAGS
  {
    bool           REFERENCED   : 1;
    bool           MOVING       : 1;
    bool           PHRASE_MODE  : 1;
    bool           WARNING      : 1;
    bool           QUIT         : 1;
    bool           BRAKE        : 1;
    bool           MOVE_BLOCKED : 1;
    bool           POS_REACHED  : 1;
    unsigned short low          : 8;
  };

  union uSTATE_FLAGS
  {
    unsigned short      all;
    struct sSTATE_FLAGS bit;
  };

  //! commands supported by SMP
  enum eCmdCode
  {
    DUMMY_CMD,          //0x00
    STATE_MASK_POS,     //0x01
    STATE_MASK_VEL,     //0x02
    UNKNOWN_CMD_03,     //0x03
    STATE_MASK_CUR,     //0x04
    UNKNOWN_CMD_05,     //0x05
    UNKNOWN_CMD_06,     //0x06
    UNKNOWN_CMD_07,     //0x07
    UNKNOWN_CMD_08,     //0x08
    UNKNOWN_CMD_09,     //0x09
    UNKNOWN_CMD_0A,     //0x0A
    UNKNOWN_CMD_0B,     //0x0B
    UNKNOWN_CMD_0C,     //0x0C
    UNKNOWN_CMD_0D,     //0x0D
    UNKNOWN_CMD_0E,     //0x0E
    UNKNOWN_CMD_0F,     //0x0F

    UNKNOWN_CMD_10,     //0x10
    UNKNOWN_CMD_11,     //0x11
    UNKNOWN_CMD_12,     //0x12
    UNKNOWN_CMD_13,     //0x13
    UNKNOWN_CMD_14,     //0x14
    UNKNOWN_CMD_15,     //0x15
    UNKNOWN_CMD_16,     //0x16
    UNKNOWN_CMD_17,     //0x17
    UNKNOWN_CMD_18,     //0x18
    UNKNOWN_CMD_19,     //0x19
    UNKNOWN_CMD_1A,     //0x1A
    UNKNOWN_CMD_1B,     //0x1B
    UNKNOWN_CMD_1C,     //0x1C
    UNKNOWN_CMD_1D,     //0x1D
    UNKNOWN_CMD_1E,     //0x1E
    UNKNOWN_CMD_1F,     //0x1F

    UNKNOWN_CMD_20,     //0x20
    UNKNOWN_CMD_21,     //0x21
    UNKNOWN_CMD_22,     //0x22
    UNKNOWN_CMD_23,     //0x23
    UNKNOWN_CMD_24,     //0x24
    UNKNOWN_CMD_25,     //0x25
    UNKNOWN_CMD_26,     //0x26
    UNKNOWN_CMD_27,     //0x27
    UNKNOWN_CMD_28,     //0x28
    UNKNOWN_CMD_29,     //0x29
    UNKNOWN_CMD_2A,     //0x2A
    UNKNOWN_CMD_2B,     //0x2B
    UNKNOWN_CMD_2C,     //0x2C
    UNKNOWN_CMD_2D,     //0x2D
    UNKNOWN_CMD_2E,     //0x2E
    UNKNOWN_CMD_2F,     //0x2F

    UNKNOWN_CMD_30,     //0x30
    UNKNOWN_CMD_31,     //0x31
    UNKNOWN_CMD_32,     //0x32
    UNKNOWN_CMD_33,     //0x33
    UNKNOWN_CMD_34,     //0x34
    UNKNOWN_CMD_35,     //0x35
    UNKNOWN_CMD_36,     //0x36
    UNKNOWN_CMD_37,     //0x37
    UNKNOWN_CMD_38,     //0x38
    UNKNOWN_CMD_39,     //0x39
    UNKNOWN_CMD_3A,     //0x3A
    UNKNOWN_CMD_3B,     //0x3B
    UNKNOWN_CMD_3C,     //0x3C
    UNKNOWN_CMD_3D,     //0x3D
    UNKNOWN_CMD_3E,     //0x3E
    UNKNOWN_CMD_3F,     //0x3F

    UNKNOWN_CMD_40,     //0x40
    UNKNOWN_CMD_41,     //0x41
    UNKNOWN_CMD_42,     //0x42
    UNKNOWN_CMD_43,     //0x43
    UNKNOWN_CMD_44,     //0x44
    UNKNOWN_CMD_45,     //0x45
    UNKNOWN_CMD_46,     //0x46
    UNKNOWN_CMD_47,     //0x47
    UNKNOWN_CMD_48,     //0x48
    UNKNOWN_CMD_49,     //0x49
    UNKNOWN_CMD_4A,     //0x4A
    UNKNOWN_CMD_4B,     //0x4B
    UNKNOWN_CMD_4C,     //0x4C
    UNKNOWN_CMD_4D,     //0x4D
    UNKNOWN_CMD_4E,     //0x4E
    UNKNOWN_CMD_4F,     //0x4F

    UNKNOWN_CMD_50,     //0x50
    UNKNOWN_CMD_51,     //0x51
    UNKNOWN_CMD_52,     //0x52
    UNKNOWN_CMD_53,     //0x53
    UNKNOWN_CMD_54,     //0x54
    UNKNOWN_CMD_55,     //0x55
    UNKNOWN_CMD_56,     //0x56
    UNKNOWN_CMD_57,     //0x57
    UNKNOWN_CMD_58,     //0x58
    UNKNOWN_CMD_59,     //0x59
    UNKNOWN_CMD_5A,     //0x5A
    UNKNOWN_CMD_5B,     //0x5B
    UNKNOWN_CMD_5C,     //0x5C
    UNKNOWN_CMD_5D,     //0x5D
    UNKNOWN_CMD_5E,     //0x5E
    UNKNOWN_CMD_5F,     //0x5F

    UNKNOWN_CMD_60,     //0x60
    UNKNOWN_CMD_61,     //0x61
    UNKNOWN_CMD_62,     //0x62
    CMD_ASCII,          //0x63
    UNKNOWN_CMD_64,     //0x64
    UNKNOWN_CMD_65,     //0x65
    UNKNOWN_CMD_66,     //0x66
    UNKNOWN_CMD_67,     //0x67
    UNKNOWN_CMD_68,     //0x68
    UNKNOWN_CMD_69,     //0x69
    UNKNOWN_CMD_6A,     //0x6A
    UNKNOWN_CMD_6B,     //0x6B
    UNKNOWN_CMD_6C,     //0x6C
    UNKNOWN_CMD_6D,     //0x6D
    UNKNOWN_CMD_6E,     //0x6E
    UNKNOWN_CMD_6F,     //0x6F

    UNKNOWN_CMD_70,     //0x70
    UNKNOWN_CMD_71,     //0x71
    UNKNOWN_CMD_72,     //0x72
    UNKNOWN_CMD_73,     //0x73
    UNKNOWN_CMD_74,     //0x74
    UNKNOWN_CMD_75,     //0x75
    UNKNOWN_CMD_76,     //0x76
    UNKNOWN_CMD_77,     //0x77
    UNKNOWN_CMD_78,     //0x78
    UNKNOWN_CMD_79,     //0x79
    UNKNOWN_CMD_7A,     //0x7A
    UNKNOWN_CMD_7B,     //0x7B
    UNKNOWN_CMD_7C,     //0x7C
    UNKNOWN_CMD_7D,     //0x7D
    UNKNOWN_CMD_7E,     //0x7E
    UNKNOWN_CMD_7F,     //0x7F

    GET_CONFIG,         //0x80
    SET_CONFIG,         //0x81
    GET_CONFIG_EXT,     //0x82
    SET_CONFIG_EXT,     //0x83
    FRAG_START,         //0x84
    FRAG_MIDDLE,        //0x85
    FRAG_END,           //0x86
    FRAG_ACK,           //0x87
    CMD_ERROR,          //0x88
    CMD_WARNING,        //0x89
    CMD_INFO,           //0x8A
    CMD_ACK,            //0x8B
    CMD_DEFAULT,        //0x8C
    UNKNOWN_CMD_8D,     //0x8D
    UNKNOWN_CMD_8E,     //0x8E
    UNKNOWN_CMD_8F,     //0x8F

    CMD_FAST_STOP,      //0x90
    CMD_STOP,           //0x91
    CMD_REFERENCE,      //0x92
    CMD_MOVE_BLOCKED,   //0x93
    CMD_POS_REACHED,    //0x94
    GET_STATE,          //0x95
    GET_DETAILED_ERROR_INFO, //0x96
    CMD_REFERENCE_HAND, //0x97
    GET_STATE_AXIS,     //0x98
    GET_OTP_AREA,       //0x99
    GET_CYCLE_TIME,     //0x9A
    UNKNOWN_CMD_9B,     //0x9B
    UNKNOWN_CMD_9C,     //0x9C
    UNKNOWN_CMD_9D,     //0x9D
    UNKNOWN_CMD_9E,     //0x9E
    UNKNOWN_CMD_9F,     //0x9F

    SET_TARGET_VEL,     //0xA0
    SET_TARGET_ACC,     //0xA1
    SET_TARGET_JERK,    //0xA2
    SET_TARGET_CUR,     //0xA3
    SET_TARGET_TIME,    //0xA4
    SET_TARGET_FORCE,   //0xA5
    SET_TARGET_POS,     //0xA6
    SET_TARGET_POS_REL, //0xA7
    UNKNOWN_CMD_A8,     //0xA8
    UNKNOWN_CMD_A9,     //0xA9
    UNKNOWN_CMD_AA,     //0xAA
    UNKNOWN_CMD_AB,     //0xAB
    UNKNOWN_CMD_AC,     //0xAC
    UNKNOWN_CMD_AD,     //0xAD
    UNKNOWN_CMD_AE,     //0xAE
    UNKNOWN_CMD_AF,     //0xAF

    MOVE_POS,           //0xB0
    MOVE_POS_TIME,      //0xB1
    MOVE_SRU_POS,       //0xB2
    MOVE_CUR,           //0xB3
    MOVE_FORCE,         //0xB4
    MOVE_VEL,           //0xB5
    MOVE_PWM,           //0xB6
    MOVE_GRIP,          //0xB7
    MOVE_POS_REL,       //0xB8
    MOVE_POS_TIME_REL,  //0xB9
    MOVE_POS_LOOP,      //0xBA
    MOVE_POS_TIME_LOOP, //0xBB
    MOVE_POS_REL_LOOP,  //0xBC
    MOVE_POS_TIME_REL_LOOP, //0xBD
    UNKNOWN_CMD_BE,     //0xBE
    MOVE_CUR_PWM,       //0xBF
    SET_PHRASE,         //0xC0
    EXE_PHRASE,         //0xC1
    GET_PHRASES,        //0xC2
    PRG_GOTO,           //0xC3
    #if _INTPROG
      PRG_TOGGLE_WAITSTATE, //0xC4
      PRG_END_SEQUENCE, //0xC5
      PRG_TOGGLE_DEBUG, //0xC6
      PRG_ACKNOWLEDGE,  //0xC7
    #else
      PRG_WAIT,         //0xC4
      PRG_END,          //0xC5
      CONFIG_VES,       //0xC6
      OPCODE,           //0xC7
    #endif
    PRG_GET_PROGRAM_NO, //0xC8
    PRG_SET_PARAMETER,  //0xC9
    PRG_GET_PARAMETER,  //0xCA
    PRG_END_PROGRAM,    //0xCB
    UNKNOWN_CMD_CC,     //0xCC
    UNKNOWN_CMD_CD,     //0xCD
    UNKNOWN_CMD_CE,     //0xCE
    #if _INTPROG
      EXE_SEQUENCE,     //0xCF
    #else
      PRG_EXE,          //0xCF
    #endif
    EXE_PHRASE0,        //0xD0
    EXE_PHRASE1,        //0xD1
    EXE_PHRASE2,        //0xD2
    EXE_PHRASE3,        //0xD3
    EXE_PHRASE4,        //0xD4
    EXE_PHRASE5,        //0xD5
    EXE_PHRASE6,        //0xD6
    EXE_PHRASE7,        //0xD7
    EXE_PHRASE8,        //0xD8
    EXE_PHRASE9,        //0xD9
    EXE_PHRASE10,       //0xDA
    EXE_PHRASE11,       //0xDB
    EXE_PHRASE12,       //0xDC
    EXE_PHRASE13,       //0xDD
    EXE_PHRASE14,       //0xDE
    EXE_PHRASE15,       //0xDF

    CMD_REBOOT,         //0xE0
    CMD_DIO,            //0xE1
    CMD_FLASH_MODE,     //0xE2
    CHANGE_USER,        //0xE3
    CHECK_MC_PC_COMMUNICATION, //0xE4
    CHECK_PC_MC_COMMUNICATION, //0xE5
    CMD_DISCONNECT,     //0xE6
    CMD_TOGGLE_IMPULSE_MESSAGE, //0xE7
    CMD_FLASH_MODE_SWITCH, //0xE8
    CMD_GET_DIO,        //0xE9
    CMD_SET_DIO,        //0xEA
    CMD_DIO_AXIS,       //0xEB
    CMD_SLAVE_MODE,     //0xEC
    CMD_MASTER_MODE,    //0xED

    CMD_MAG_PROTOCOL_ACTIVE, //0xEE
    GET_SENSOR_TEMPERATURE, //0xEF

    GET_TEMPERATURE,    //0xF0
    CMD_HWTEST,         //0xF1
    CMD_MSM_PARAM_READ, //0xF2
    CMD_MSM_PARAM_WRITE, //0xF3
    CMD_MSM_CONTROL,    //0xF4
    CMD_MAG_SET_VALVE,  //0xF5
    CMD_LASE,           //0xF6
    CMD_LASE_IMPULSE,   //0xF7
    CAMAT_CHANGE_PROGRAM, //0xF8
    CAMAT_SETTINGS_CHANGED, //0xF9
    CAMAT_RES_MEASUREMENT_BLOCK, //0xFA
    CMD_ROBOT,          //0xFB
    CMD_ROBOT_PARAM,    //0xFC
    CMD_ROBOT_RESERVED, //0xFD
    CAMAT_TRIGGER,      //0xFE
    //UNKNOWN_CMD_FF    //0xFF
    ERASE_CYCLE_COUNT   // 0xFF
  };

  //--------------------------------------------------------------------------------------------------------------------
  // constructors and destructor
  //--------------------------------------------------------------------------------------------------------------------
public:
  //!@brief The standard constructor.
  SMPCanBusChannel();

  //!@brief Destructor
  virtual ~SMPCanBusChannel();

  //--------------------------------------------------------------------------------------------------------------------
  // public methods
  //--------------------------------------------------------------------------------------------------------------------
public:
  //! this function opens a CAN port with given id
  void openHook();

  //! this function closes a CAN port if it is open
  void closeHook();

  //! adds modules that are to be controlled
  void addModules(const std::vector<unsigned int>& modules);

  /*! prepares fixed part of the message id to sends a request for only the position, if we don't set the velocity and current
   *@param  moduleId    the number of the module
   *@param  position    returns the position, if it's true
   *@param  velocity    returns the velocity, if it's true (here we need also the position for the calculation)
   *@param  current     returns the current, if it's true (here we need also the position and the velocity for the calculation)
   */
  bool getState(unsigned int moduleId, bool position, bool velocity, bool current);

  //! returns the last position
  //@param  moduleId    the number of the module
  float getLastJointPosition(unsigned int moduleId);

  //! returns the last velocity
  //@param  moduleId    the number of the module
  float getLastJointVelocity(unsigned int moduleId);

  //! returns the last current
  //@param  moduleId    the number of the module
  float getLastJointCurrent(unsigned int moduleId);

  //! returns the duration of the last moving module
  //@param  moduleId    the number of the module
  float getLastJointDuration(unsigned int moduleId);

  //! sends an ACK to a specified module
  //@param  moduleId    the number of the module
  void acknowledge(unsigned int moduleId);

  //! sends a reference request to a module
  //@param  moduleId    the number of the module
  void reference(unsigned int moduleId);

  /*! prepares fixed part of the message id and sets the parameter values to move a module to the given position
   *@param  moduleId    the number of the module
   *@param  position    the position of the joint (in degree)
   *@param  velocity    the velocity of the joint (if not given, 50.0)
   */
  void movePos(unsigned int moduleId, float position, float velocity=50.0);

  /*! prepares fixed part of the message id and sets the parameter values to move a module with the given velocity
   *@param  moduleId    the number of the module
   *@param  velocity    the joint velocity of the joint (in degree/s (?))
   */
  void moveVel(unsigned int moduleId, float velocity);

  //! returns true, if it's already referenced
  bool getref(unsigned int id);

  //! returns the state of the port
  inline bool isOpen() const
  {
    return this->mIsOpen;
  }

  //--------------------------------------------------------------------------------------------------------------------
  // protected methods
  //--------------------------------------------------------------------------------------------------------------------
protected:
  void postOpenHook();

  //--------------------------------------------------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------------------------------------------------
private:
  /*! getState is used to request information from a specific module on the bus
   *@param  message_id    fixed part of the message id (sending information)
   *@param  period      the period (if we don't want make a module reply only once)
   *@param  mode      is the information about what we claim back (set in getState())
   */
  void getState(uSMPMessageID message_id, float* period, sGetStateMode* mode);

  /*! sends a SMP message using the CAN port
   *@param  message_id    fixed part of the message id (sending information)
   *@param  d_len     the length of a SMP message
   *@param  data      the data of a SMP message
   */
  void send(uSMPMessageID message_id, unsigned int d_len, unsigned char* data);

  /*! if an SMP message exceeds the maximum amount of allowed bytes, this function sends a message in pieces
   *@param  message_id    fixed part of the message id (sending information)
   *@param  d_len     the length of a SMP message
   *@param  data      the data of a SMP message
   */
  void sendFragmented(uSMPMessageID message_id, unsigned int d_len, unsigned char* data);

  /*! receives a single message from the CAN bus
   *@param  message_id    fixed part of the message id (sending information)
   *@param  d_len     a length for a received SMP message
   *@param  data      a data for a received SMP message
   *@param  max_data_len  the max. size of data (256)
   *@param  do_reset
   */
  bool receive(uSMPMessageID* message_id, unsigned int* d_len, unsigned char* data, int max_data_len, unsigned char do_reset);

  //! receives a byte-sized frame of a message
  //@param  cmsg      an object for a received SMP message
  bool receiveFrame(CMSG* cmsg);

  /*! assembles a message to move a module to the given position and sends the message
   *@param  message_id    fixed part of the message id (sending information)
   *@param  position    the position of the joint (in degree)
   *@param  velocity    the velocity of the joint (in degree)
   *@param  acceleration  the acceleration of the joint (in degree)
   *@param  current     the current
   *@param  jerk      the jerk
   */
  void movePos(uSMPMessageID message_id, float* position, float* velocity, float* acceleration, float* current, float* jerk);

  /*! assembles a message to move a module with the given velocity and sends the message
   *@param  message_id    fixed part of the message id (sending information)
   *@param  velocity    the velocity of the joint (in degree)
   *@param  current     the current
   */
  void moveVel(uSMPMessageID message_id, float* velocity, float* current);

  //! assembles an ACK message and sends it to a module
  //@param  message_id    fixed part of the message id (sending information)
  void acknowledge(uSMPMessageID message_id);

  //! assembles a message to calibrate a module and sends the message
  //@param  message_id    fixed part of the message id (sending information)
  void reference(uSMPMessageID message_id);

  //! translates baud rate to an enum code
  unsigned int baudrateToCode(unsigned long);

  //! processes incoming messages
  bool processInput();

  /*! parses a received message
   *@param  message_id    fixed part of the message id (sending information)
   *@param  d_len     the length of the received SMP message
   *@param  data      the data of the received SMP message
   */
  bool parseResponse(uSMPMessageID message_id, unsigned int d_len, unsigned char* data);

  //--------------------------------------------------------------------------------------------------------------------
  // members
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet
private:
  //! the handle to a can interface
  NTCAN_HANDLE mHandle;

  //! a message buffer
  sSMPBuffer* g_smp_buffer[255+1];

  //! flag containing the state of the can bus
  bool mIsOpen;

  //! a buffer for the position
  float mpos[31];

  //! a buffer for the velocity
  float mvel[31];

  //! a buffer for the current
  float mcur[31];

  //! a buffer for the duration of the position
  float mdur[31];

  //! a buffer for the referenced module
  uSTATE_FLAGS ref[31];

  std::vector<unsigned int> _mModuleMap;

  mutable QMutex mMutex;

  //--------------------------------------------------------------------------------------------------------------------
  // parameters
  //--------------------------------------------------------------------------------------------------------------------
protected:
  // none yet

private:
  //! a timeout for sending/receiving messages
  cedar::aux::IntParameterPtr _mTimeout;

  //! the baud rate of communication
  cedar::aux::IntParameterPtr _mBaudrate;

  //! the CAN port
  cedar::aux::UIntParameterPtr _mCanPort;
}; // class cedar::dev::schunk::SMPCanBusChannel

#endif // CEDAR_DEV_SCHUNK_SMP_CAN_BUS_CHANNEL_H
