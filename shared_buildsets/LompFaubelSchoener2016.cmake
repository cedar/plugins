#
# This is the buildset accompanying the publication
#
# === UNPUBLISHED (in review) ===
#
# "A neural-dynamic architecture for concurrent estimation of object pose and identity"
#
# by Oliver Lomp [1,∗], Christian Faubel [1] and Gregor Schoener [1]
#
# [*] to whom correspondence should be addressed via oliver.lomp@ini.ruhr-uni-bochum.de
# [1] Institut fuer Neuroinformatik, Ruhr-University Bochum, Building NB, 44801 Bochum, Germany
#
# ----------------------------------------------------------------------------------------------------------------------
#
# This buildset provides all necessary components for loading the architecture used for generating the data in the
# publication.
#
# Detailed instructions on how to get the architecture will be provided online if the publication is accepted.
#

DECLARE_PLUGIN(LompFaubelSchoener2016)

ADD_TO_PLUGIN(STEPS cedar::proc::steps::CrossCorrelation
                    cedar::proc::steps::GeometricImageTransform
                    cedar::proc::steps::LabelString
                    cedar::proc::steps::MatrixThreadDecoupler
                    cedar::proc::steps::MaxPooling
                    cedar::proc::steps::ReceptiveFieldHistogram
                    cedar::proc::steps::SteerableFilter
                    cedar::proc::steps::SteerC2C3
                    cedar::proc::steps::SteeringAngle
                    cedar::proc::steps::SteeringEnergy
                    cedar::proc::steps::ViewCombination
                    cedar::proc::steps::WeightedSumOfSlices
                    cedar::dyn::InhibitoryNeuron
                    cedar::dyn::steps::PatternMemory
                    )

ADD_TO_PLUGIN(PLOTS cedar::aux::gui::StringPlot)

ADD_TO_PLUGIN(GROUP_TEMPLATES "steering filter bank"
                              "histogram label matching"
                              "edge rotation estimation"
                              "histogram shift estimation"
                              "localized histogram extraction"
                              "localized histogram forward shift"
                              "edge histogram rotation"
                              )

ADD_TO_PLUGIN(KERNEL cedar::aux::kernel::SteerableKernel)
