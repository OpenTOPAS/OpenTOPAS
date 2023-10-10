if(Geant4_VERSION_MAJOR GREATER 9)
    add_library( chemistry
	TsChemistryManager.cc
	TsChemistryManager.hh
	TsChemSteppingAction.cc
	TsChemSteppingAction.hh
	TsChemTimeStepAction.cc
	TsChemTimeStepAction.hh
	TsChemTrackingManager.cc
	TsChemTrackingManager.hh
	TsChemTrackingAction.cc
	TsChemTrackingAction.hh
    )
else()
    add_library( chemistry
	TsChemistryManager.cc
	TsChemistryManager.hh
	TsChemSteppingAction.cc
	TsChemSteppingAction.hh
	TsChemTrackingAction.cc
	TsChemTrackingAction.hh
    )
endif()

