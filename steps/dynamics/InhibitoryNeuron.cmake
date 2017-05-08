DECLARE_STEP(cedar::dyn::InhibitoryNeuron
             MOC
             CATEGORY         "Dynamics"
             DESCRIPTION      "A single neuron with the dynamics tau * dv/dt = -sigmoid(s) * tanh(beta * (v - s)) - (1 - sigmoid(s)) * v, where v is the state of the system, beta defines the slope of change and s is the input, projected to zero dimensions by summing."
             MAINTAINER       "Oliver Lomp"
             DEPRECATED_NAME  dynamics::InhibitoryNeuron
            )
