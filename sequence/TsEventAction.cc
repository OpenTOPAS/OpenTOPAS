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

#include "TsEventAction.hh"

#include "TsParameterManager.hh"
#include "TsExtensionManager.hh"
#include "TsSequenceManager.hh"

#include "TsVScorer.hh"
#include "TsSteppingAction.hh"

#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4Event.hh"

TsEventAction::TsEventAction(TsParameterManager* pM, TsExtensionManager* eM, TsSequenceManager* sqM)
: fPm(pM), fEm(eM), fSqm(sqM), fIsFirstEventInThisThread(true), fNumberOfAnomalousHistoriesInARow(0)
{
	fInterval = fPm->GetIntegerParameter("Ts/ShowHistoryCountAtInterval");
}


TsEventAction::~TsEventAction()
{;}


void TsEventAction::BeginOfEventAction(const G4Event* event) {
	G4int counter;
	if (fPm->IsRandomMode())
		counter = fPm->GetRunID();
	else
		counter = event->GetEventID();

	if (fInterval!=0 && std::fmod(counter, fInterval)==0) {
		if (fPm->GetBooleanParameter("Ts/IncludeTimeInHistoryCount")) {
			char       buf[80];
			time_t     now = time(0);
			struct tm  tstruct = *localtime(&now);
			strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
			G4cout << buf << "  ";
		}

		if (fPm->GetBooleanParameter("Ts/ShowHistoryCountOnSingleLine"))
			G4cout << "Begin processing for Run: " << fPm->GetRunID() << ", History: " <<  event->GetEventID() << '\r';
		else
			G4cout << "Begin processing for Run: " << fPm->GetRunID() << ", History: " <<  event->GetEventID() << G4endl;
	}

	if (fPm->GetBooleanParameter("Ts/ShowHistoryCountLessFrequentlyAsSimulationProgresses") && counter == fInterval*10) {
		G4int intervalWas = fInterval;
		fInterval *= 10;
		fInterval = fmin(fPm->GetIntegerParameter("Ts/MaxShowHistoryCountInterval"), fInterval);
		if (fInterval != intervalWas)
			G4cout << "Resetting history count interval to: " << fInterval << G4endl;
	}

	if (fIsFirstEventInThisThread) {
		fIsFirstEventInThisThread = false;
		fPm->HandleFirstEvent();
	}

	((TsSteppingAction*)G4RunManager::GetRunManager()->GetUserSteppingAction())->ClearStepCount();
	
	// User hook for begin of history
	fEm->BeginHistory(fPm, G4RunManager::GetRunManager()->GetCurrentRun(), event);
}


void TsEventAction::EndOfEventAction(const G4Event* event) {
	if (event->IsAborted())
		fSqm->NoteInterruptedHistory();

	std::vector<TsVScorer*>::iterator iter;
	for (iter=fScorers.begin(); iter!=fScorers.end(); iter++)
		(*iter)->AccumulateEvent();

	if ((((TsSteppingAction*)G4RunManager::GetRunManager()->GetUserSteppingAction())->GetStepCount() == 1) &&
		(((TsSteppingAction*)G4RunManager::GetRunManager()->GetUserSteppingAction())->GetMostRecentStep()->GetPostStepPoint()->GetTouchable()->GetVolume())) {
		fNumberOfAnomalousHistoriesInARow++;
		if (fPm->GetBooleanParameter("Ts/QuitIfManyHistoriesSeemAnomalous") &&
			(fNumberOfAnomalousHistoriesInARow >= fPm->GetIntegerParameter("Ts/NumberOfAnomalousHistoriesToAllowInARow"))) {
			G4cerr << "" << G4endl;
			G4cerr << "TOPAS has detected a condition that may mean Geant4 is no longer correctly transporting particles." << G4endl;
			G4cerr << "There have been " << fNumberOfAnomalousHistoriesInARow << " histories in a row that each had only a single track," << G4endl;
			G4cerr << "and the track had only a two steps, and the last step did not exit the world." << G4endl;
			G4cerr << "The last such case was Run: " << fPm->GetRunID() << ", History: " <<  event->GetEventID() << G4endl;
			G4cerr << "Step Length: " << ((TsSteppingAction*)G4RunManager::GetRunManager()->GetUserSteppingAction())->GetMostRecentStep()->GetStepLength() / mm << " mm" << G4endl;
			G4cerr << "Ending Volume: " << ((TsSteppingAction*)G4RunManager::GetRunManager()->GetUserSteppingAction())->GetMostRecentStep()->GetPostStepPoint()->GetTouchable()->GetVolume()->GetName() << G4endl;
			G4cerr << "" << G4endl;
			G4cerr << "We do  not yet understand what causes this condition in Geant4." << G4endl;
			G4cerr << "Please report to us that you encountered this condition," << G4endl;
			G4cerr << "as your example may help us to solve this mystery." << G4endl;
			G4cerr << "Contact us on the TOPAS user forum or by sending email to Jose.RamosMendez@ucsf.edu or JSchuemann@mgh.harvard.edu" << G4endl;
			G4cerr << "" << G4endl;
			G4cerr << "For now you may be able to work around the issue by changing the random seed." << G4endl;
			G4cerr << "You can also increase the number of such histories permitted in a row by adjusting the parameter:" << G4endl;
			G4cerr << "i:Ts/NumberOfAnomalousHistoriesToAllowInARow. The default value is 10." << G4endl;
			G4cerr << "Or you can turn off this check entirely by setting: b:Ts/QuitIfManyHistoriesSeemAnomalous = \"False\"" << G4endl;
			fPm->AbortSession(1);
		}
	} else {
		fNumberOfAnomalousHistoriesInARow = 0;
	}
	
	// User hook for end of history
	fEm->EndHistory(fPm, G4RunManager::GetRunManager()->GetCurrentRun(), event);
}


void TsEventAction::RegisterScorer(TsVScorer* scorer) {
	fScorers.push_back(scorer);
}
