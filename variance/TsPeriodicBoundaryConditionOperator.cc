//
// ********************************************************************
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * TOPAS collaboration.                                             *
// * Use or redistribution of this code is not permitted without the  *
// * explicit approval of the TOPAS collaboration.                    *
// * Contact: Joseph Perl, perl@slac.stanford.edu                     *
// *                                                                  *
// ********************************************************************
//

#include "TsParameterManager.hh"

#include "TsPeriodicBoundaryConditionOperator.hh"
#include "TsPeriodicBoundaryConditionProcess.hh"

#include "G4BiasingProcessInterface.hh"
#include "G4GenericMessenger.hh"

TsPeriodicBoundaryConditionOperator::TsPeriodicBoundaryConditionOperator(TsParameterManager* pM, G4String name)
: G4VBiasingOperator(name), fPm(pM)
{
	fKillingOperation = new TsPeriodicBoundaryConditionProcess(fPm, name);
	fNamesOfRegions = fPm->GetStringVector("Vr/" + name + "/ApplyBiasingInRegionsNamed");
	fNumberOfRegions = fPm->GetVectorLength("Vr/" + name + "/ApplyBiasingInRegionsNamed");
	fAcceptedLogicalVolumeNames = fPm->GetStringVector("Vr/" + name + "/ApplyBiasingInVolumesNamed");
	fNbOfAcceptedLogicalVolumeNames = fPm->GetVectorLength("Vr/" + name + "/ApplyBiasingInVolumesNamed");
	
}


TsPeriodicBoundaryConditionOperator::~TsPeriodicBoundaryConditionOperator()
{
}


void TsPeriodicBoundaryConditionOperator::StartRun()
{
	G4cout << GetName() << " : starting run " << G4endl;
	((TsPeriodicBoundaryConditionProcess*)fKillingOperation)->SetNumberOfRegions(fNumberOfRegions);
	((TsPeriodicBoundaryConditionProcess*)fKillingOperation)->SetRegions(fNamesOfRegions);
}


G4VBiasingOperation* TsPeriodicBoundaryConditionOperator::ProposeNonPhysicsBiasingOperation(const G4Track*,
																			   const G4BiasingProcessInterface* )
{
	return fKillingOperation;
}


G4bool TsPeriodicBoundaryConditionOperator::IsApplicable(G4LogicalVolume* logicalVolume) {
	G4bool applicable = false;
	for ( int i = 0; i < fNbOfAcceptedLogicalVolumeNames; i++ )
		if ( logicalVolume->GetName() == fAcceptedLogicalVolumeNames[i] )
			applicable = true;
	return applicable;
}
