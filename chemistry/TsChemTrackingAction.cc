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

#include "TsChemTrackingAction.hh"
#include "TsParameterManager.hh"
#include "TsVScorer.hh"

#include "G4Track.hh"
#include "G4Molecule.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4UIcommand.hh"
#include <vector>

TsChemTrackingAction::TsChemTrackingAction(TsParameterManager* pM)
: G4UserTrackingAction(), fPm(pM), fReportPreChemistry(false)
{
	if ( fPm->ParameterExists("Ch/ReportOriginOfMoleculesToAsciiFile")
#ifdef TOPAS_MT
		&& G4Threading::IsWorkerThread()
#endif
		)
	{
		fReportPreChemistry = true;
		G4String outFile = fPm->GetStringParameter("Ch/ReportOriginOfMoleculesToAsciiFile");
		
#ifdef TOPAS_MT
		outFile += G4UIcommand::ConvertToString( G4Threading::G4GetThreadId() ) + ".chem";
#else
		outFile += ".chem";
#endif
		fOutFile.open(outFile);
		fOutFile << "#runID eventID trackID moleculeName parent1TrackID parent2TrackID creationTime(ps) originVolume" << G4endl;
	}
}


TsChemTrackingAction::~TsChemTrackingAction()
{}


void TsChemTrackingAction::PreUserTrackingAction(const G4Track* aTrack)
{
	if ( fReportPreChemistry ) {
		G4String currentVolume = aTrack->GetVolume()->GetName();
		G4int a, b;
		GetMolecule(aTrack)->GetParentID(a, b);
		fOutFile  << fPm->GetRunID() << "\t"
		<< G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID() << "\t"
		<< aTrack->GetTrackID() << "\t" << GetMolecule(aTrack)->GetName() << "\t"
		<< a << "\t" << b << "\t" << aTrack->GetGlobalTime()/picosecond  << "\t"
		<< currentVolume << G4endl;
	}

	std::vector<TsVScorer*>::iterator iter;
	for (iter=fScorers.begin(); iter!=fScorers.end(); iter++)
		(*iter)->UserHookForBeginOfChemicalTrack(aTrack);
}


void TsChemTrackingAction::PostUserTrackingAction(const G4Track* aTrack) {
    for (auto iter=fScorers.begin(); iter != fScorers.end(); iter++)
        (*iter)->UserHookForEndOfChemicalTrack(aTrack);
}


void TsChemTrackingAction::RegisterScorer(TsVScorer* scorer) {
	fScorers.push_back(scorer);
}
