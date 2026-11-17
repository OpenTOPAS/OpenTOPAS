//
// ********************************************************************
// *                                                                  *
// * Copyright 2026 The TOPAS Collaboration                           *
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

#include "TsFilterByCreatorProcess.hh"

#include "TsTrackInformation.hh"

#include "G4VProcess.hh"
#include "G4ProcessVector.hh"
#include "G4ProcessTable.hh"
#include "G4LossTableManager.hh"
#include "G4VEmProcess.hh"

TsFilterByCreatorProcess::TsFilterByCreatorProcess(G4String name, TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM,
												   TsFilterManager* fM, TsVGenerator* generator, TsVScorer* scorer, TsVFilter* parentFilter, G4bool invert,
												   G4bool includeAncestors)
:TsVFilter(name, pM, mM, gM, fM, generator, scorer, parentFilter, invert), fIncludeAncestors(includeAncestors), fProcessCacheVersion(1) {
	ResolveParameters();

	if (fIncludeAncestors)
		pM->SetNeedsTrackingAction();
}


TsFilterByCreatorProcess::~TsFilterByCreatorProcess()
{;}


void TsFilterByCreatorProcess::ResolveParameters() {
	fProcessNames.clear();

	G4String* processNames = fPm->GetStringVector(GetFullParmName(GetName()));
	G4int length = fPm->GetVectorLength(GetFullParmName(GetName()));

	for (G4int i = 0; i < length; i++) {
		// Check gamma subprocesses first: they are not registered individually
		// in G4ProcessTable, which would emit a misleading not-found message.
		G4VEmProcess* gammaGeneralProcess = G4LossTableManager::Instance()->GetGammaGeneralProcess();
		G4bool found = gammaGeneralProcess && gammaGeneralProcess->GetEmProcess(processNames[i]);
		if (!found) {
			G4ProcessVector* oneVector = G4ProcessTable::GetProcessTable()->FindProcesses(processNames[i]);
			found = oneVector->size() > 0;
			delete oneVector;
		}

		if (!found) {
			G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
			G4cerr << GetName() << " = " << processNames[i] << " refers to an unknown process." << G4endl;
			fPm->AbortSession(1);
		}

		fProcessNames.push_back(processNames[i]);
	}

	// Invalidate every thread's cache, including threads other than this one.
	fProcessCacheVersion++;
	TsVFilter::ResolveParameters();
}


void TsFilterByCreatorProcess::UpdateForNewRun(G4bool rebuiltSomeComponents) {
	// Physics may have been rebuilt since the previous run.
	// Updates take place between runs, while workers are not evaluating filters.
	fProcessCacheVersion++;
	TsVFilter::UpdateForNewRun(rebuiltSomeComponents);
}


const std::vector<G4VProcess*>& TsFilterByCreatorProcess::GetProcesses() const {
	ProcessCache& cache = fProcessCache.Get();
	if (cache.version != fProcessCacheVersion) {
		cache.processes.clear();
		G4ProcessTable* theProcessTable = G4ProcessTable::GetProcessTable();
		for (size_t i = 0; i < fProcessNames.size(); i++) {
			// Resolve gamma subprocesses before querying the process table.
			G4VEmProcess* gammaGeneralProcess = G4LossTableManager::Instance()->GetGammaGeneralProcess();
			G4VEmProcess* process = gammaGeneralProcess ? gammaGeneralProcess->GetEmProcess(fProcessNames[i]) : nullptr;
			G4bool found = process != nullptr;
			if (found) {
				cache.processes.push_back(process);
			} else {
				G4ProcessVector* oneVector = theProcessTable->FindProcesses(fProcessNames[i]);
				found = oneVector->size() > 0;
				for (size_t j = 0; j < oneVector->size(); j++)
					cache.processes.push_back((*oneVector)[j]);
				delete oneVector;
			}

			if (!found) {
				G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
				G4cerr << GetName() << " = " << fProcessNames[i] << " refers to an unknown process." << G4endl;
				fPm->AbortSession(1);
			}
		}
		cache.version = fProcessCacheVersion;
	}

	return cache.processes;
}


G4bool TsFilterByCreatorProcess::Accept(const G4Step* aStep) const {
	if (fParentFilter && !fParentFilter->Accept(aStep)) return false;

	// Resolve pointers in the thread that evaluates the filter.
	const std::vector<G4VProcess*>& processes = GetProcesses();
	for (size_t i = 0; i < processes.size(); i++)
		if (aStep->GetTrack()->GetCreatorProcess() == processes[i]) {
			if (fInvert) return false;
			else return true;
	    }

	if (fIncludeAncestors) {
		TsTrackInformation* parentInformation = (TsTrackInformation*)(aStep->GetTrack()->GetUserInformation());
		if (parentInformation) {
			std::vector<G4VProcess*> ancestorProcesses = parentInformation->GetCreatorProcesses();
			for (size_t i=0; i < ancestorProcesses.size(); i++)
				for (size_t j = 0; j < processes.size(); j++)
					if ( ancestorProcesses[i]  == processes[j] ) {
						if (fInvert) return false;
						else return true;
	                }
		}
	}

	if (fInvert) return true;
	else return false;
}


G4bool TsFilterByCreatorProcess::AcceptTrack(const G4Track*) const {
	G4cerr << "Topas is exiting due to a serious error in source setup." << G4endl;
	G4cerr << "Sources cannot be filtered by " << GetName() << G4endl;
	fPm->AbortSession(1);
	return false;
}
