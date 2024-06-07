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

#include "TsStackingAction.hh"

#include "TsGeometryManager.hh"
#include "TsVGeometryComponent.hh"
#include "TsGeneratorManager.hh"
#include "TsVarianceManager.hh"

#include "TsVFilter.hh"

#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4RegionStore.hh"
#include "G4VProcess.hh"
#include "G4StackManager.hh"
#include "G4DNAChemistryManager.hh"

TsStackingAction::TsStackingAction(TsParameterManager *pm, TsGeometryManager *, TsGeneratorManager *pgM, TsVarianceManager *vM)
	: fPm(pm), fPgm(pgM), fVm(vM),
	  fStage(0), fPrimaryCounter(0), fKillTrack(false),
	  fDBSActive(false), fRangeRejection(false)
{
	fKillTrack = fVm->UseKillOtherParticles();
	fDBSActive = fVm->UseDirectionalRussianRoulette();
	fRangeRejection = fVm->UseRangeRejection();
}


TsStackingAction::~TsStackingAction()
{
}


void TsStackingAction::PrepareNewEvent()
{
	fStage = 0;
	fPrimaryCounter = 0;
}


G4ClassificationOfNewTrack TsStackingAction::ClassifyNewTrack(const G4Track* track)
{
	// At stage=0, which means start of event, kill any tracks that fail the appropriate source filter.
	if (fStage==0) {
		fPrimaryCounter++;
		TsVFilter* filter = fPgm->GetFilter(fPrimaryCounter);

		if (filter==0 || filter->AcceptTrack(track))
			return fWaiting;
		else
			return fKill;
	}

	if ( fKillTrack )
		return fVm->ApplyKillOtherParticles(track);
	
	if ( fRangeRejection )
		return fVm->ApplyRangeRejection(track);

	if ( fDBSActive )
		return fVm->ApplyDirectionalRussianRoulette(track);

	return fUrgent;
}


void TsStackingAction::NewStage()
{
	if ( fPm->NeedsChemistry() && stackManager->GetNTotalTrack() == 0 )
		G4DNAChemistryManager::Instance()->Run();

	fStage++;
}


void TsStackingAction::Quit(G4String name, G4String message) {
	G4cerr << "Topas is exiting due to a serious error in variance reduction setup." << G4endl;
	G4cerr << "Parameter name: " << name << G4endl;
	G4cerr << message << G4endl;
	fPm->AbortSession(1);
}
