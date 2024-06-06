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

#include "TsChemistryManager.hh"
#include "TsParameterManager.hh"

#include "TsChemTrackingManager.hh"
#include "TsChemTrackingAction.hh"
#include "TsChemSteppingAction.hh"

#include "TsChemTimeStepAction.hh"

#include "G4Scheduler.hh"
#include "G4DNAChemistryManager.hh"

#include "G4SystemOfUnits.hh"

TsChemistryManager::TsChemistryManager(TsParameterManager* pM)
: fPm(pM), fName("Default"), fActiveTransport(false), fMaxChemicalStepNumber(1000),
fMaxChemicalStageTime(100*nanosecond), fUseTimmingCut(false), fUseStepNumberingCut(false), fVerbosity(0)
{
	ResolveParameters();
}


TsChemistryManager::~TsChemistryManager()
{}


void TsChemistryManager::ResolveParameters() {
	
	if ( fPm->ParameterExists("Ch/ChemistryName") )
		fName = fPm->GetStringParameter("Ch/ChemistryName");
	
	if (fPm->ParameterExists(GetFullParmName("ChemicalStageTransportActive")))
		fActiveTransport = fPm->GetBooleanParameter(GetFullParmName("ChemicalStageTransportActive"));
	
	if ( fPm->ParameterExists(GetFullParmName("ChemicalStageTimeEnd")) ) {
		fMaxChemicalStageTime = fPm->GetDoubleParameter(GetFullParmName("ChemicalStageTimeEnd"), "Time");
		fUseTimmingCut = true;
	}
	
	if ( fPm->ParameterExists(GetFullParmName("ChemicalStageMaximumStepNumber"))) {
		fMaxChemicalStepNumber = fPm->GetIntegerParameter(GetFullParmName("ChemicalStageMaximumStepNumber"));
		fUseStepNumberingCut = true;
	}
	
	if ( fUseTimmingCut && fUseStepNumberingCut ) {
		G4cerr << "TOPAS is exiting due to an incompatibility in parameters." << G4endl;
		G4cerr << "Parameter: " << GetFullParmName("ChemicalStageTimeEnd") << " cannot be used in conjunction with "
		<< "parameter: " << GetFullParmName("ChemicalStageMaximumStepNumber") << G4endl;
		fPm->AbortSession(1);
	}
	
	if ( fPm->ParameterExists("Ts/ChemistryVerbosity") )
		fVerbosity = fPm->GetIntegerParameter("Ts/ChemistryVerbosity");
	
}


void TsChemistryManager::Configure() {
	if ( G4DNAChemistryManager::IsActivated() ) {

		G4Scheduler::Instance()->SetUserAction(new TsChemTimeStepAction(fPm, fName));
		
		if ( fUseTimmingCut )
			G4Scheduler::Instance()->SetEndTime(fMaxChemicalStageTime);
		
		if ( fUseStepNumberingCut )
			G4Scheduler::Instance()->SetMaxNbSteps(fMaxChemicalStepNumber);
		
		if ( 0 < fVerbosity )
			G4Scheduler::Instance()->SetVerbose(fVerbosity-1);
		else
			G4Scheduler::Instance()->SetVerbose(0);
		
		if ( fActiveTransport ) {
			TsChemTrackingManager* chemTrackingManager = new TsChemTrackingManager();
			chemTrackingManager->SetUserAction(new TsChemSteppingAction);
			chemTrackingManager->SetUserAction(new TsChemTrackingAction(fPm));
			G4Scheduler::Instance()->SetInteractivity(chemTrackingManager);
		}
		
		if ( fPm->ParameterExists("Ch/ReportPreChemicalStageToAsciiFile") )  {
			G4String fileName_mt = fPm->GetStringParameter("Ch/ReportPreChemicalStageToAsciiFile");
			fileName_mt += ".chem";
			
			G4DNAChemistryManager::Instance()->WriteInto(fileName_mt);
		}
	}
}


G4String TsChemistryManager::GetFullParmName(G4String suffix) {
	G4String fullParName = "Ch/" + fName + "/" + suffix;
	return fullParName;
}
