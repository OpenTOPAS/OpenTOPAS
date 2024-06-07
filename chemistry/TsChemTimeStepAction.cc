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

#include "TsChemTimeStepAction.hh"
#include "TsParameterManager.hh"
#include "TsVScorer.hh"

#include "G4Scheduler.hh"
#include "G4Track.hh"
#include "G4Molecule.hh"

TsChemTimeStepAction::TsChemTimeStepAction(TsParameterManager* pM, G4String name)
:G4UserTimeStepAction(), fPm(pM)
{
	
	G4String parmName = "Ch/" + name + "/";
	
	if ( fPm->ParameterExists(parmName + "ChemicalStageTimeStepsHighEdges") &&
		fPm->ParameterExists(parmName + "ChemicalStageTimeStepsResolutions")) {
		
		G4int nSteps = fPm->GetVectorLength(parmName + "ChemicalStageTimeStepsHighEdges");
		
		if ( nSteps != fPm->GetVectorLength(parmName + "ChemicalStageTimeStepsResolutions")) {
			G4cerr << "TOPAS is exiting due to an error in parameters definition" << G4endl;
			G4cerr << "Length of double vector parameter: " << parmName + "ChemicalStageTimeStepsHighEdges" <<
			" does not match with legth of double vector parameter: " <<
			parmName + "ChemicalStageTimeStepsResolutions" << G4endl;
		}
		
		G4double* highEdges = fPm->GetDoubleVector(parmName + "ChemicalStageTimeStepsHighEdges","Time");
		G4double* resolution = fPm->GetDoubleVector(parmName + "ChemicalStageTimeStepsResolutions","Time");
		
		for ( int i = 0; i < nSteps; i++ )
			AddTimeStep(highEdges[i], resolution[i]);
	}

	else {
		AddTimeStep(     1*ps,0.1*ps);
		AddTimeStep(    10*ps,1.0*ps);
		AddTimeStep(   100*ps,3.0*ps);
		AddTimeStep(  1000*ps, 10*ps);
		AddTimeStep( 10000*ps,100*ps);
		AddTimeStep(100000*ps,100*ps);
		AddTimeStep(999999*ps,100*ps);
	}
	
}


TsChemTimeStepAction::~TsChemTimeStepAction()
{}


TsChemTimeStepAction& TsChemTimeStepAction::operator=(const TsChemTimeStepAction& rhs)
{
	if ( this == &rhs )
		return *this;
	
	return *this;
}


void TsChemTimeStepAction::UserPreTimeStepAction()
{
	std::vector<TsVScorer*>::iterator iter;
	for (iter=fScorers.begin(); iter!=fScorers.end(); iter++)
		(*iter)->UserHookForPreTimeStepAction();
}


void TsChemTimeStepAction::UserPostTimeStepAction()
{
	std::vector<TsVScorer*>::iterator iter;
	for (iter=fScorers.begin(); iter!=fScorers.end(); iter++)
		(*iter)->UserHookForPostTimeStepAction();
}


void TsChemTimeStepAction::UserReactionAction(const G4Track& trackA, const G4Track& trackB, const std::vector<G4Track*>* products) {
    for (auto iter = fScorers.begin(); iter != fScorers.end(); iter++) {
        (*iter)->UserHookForChemicalReaction(trackA, trackB, products);
    }
}


void TsChemTimeStepAction::RegisterScorer(TsVScorer* scorer) {
	fScorers.push_back(scorer);
}
