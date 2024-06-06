//
// ********************************************************************
// *                                                                  *
// * Copyright 2024 The TOPAS Collaboration                           *
// * Copyright 2022 The TOPAS Collaboration                           *
// *                                                                  *
// * Permission is hereby granted, free of charge, to any person      *
// * obtaining a copy of this software and associated documentation   *
// * files (the "Software"), to deal in the Software without          *
// * restriction, including without limitation the rights to use,     *
// * copy, modify, merge, publish, distribute, sublicense, and/or     *
// * sell copies of the Software, and to permit persons to whom the   *
// * Software is furnished to do so, subject to the following         *
// * conditions:                                                      *
// *                                                                  *
// * The above copyright notice and this permission notice shall be   *
// * included in all copies or substantial portions of the Software.  *
// *                                                                  *
// * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,  *
// * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES  *
// * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND         *
// * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT      *
// * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,     *
// * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
// * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR    *
// * OTHER DEALINGS IN THE SOFTWARE.                                  *
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
