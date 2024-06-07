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

#include "TsFilterByIncidentParticleCreatorProcess.hh"

#include "G4VProcess.hh"
#include "G4ProcessVector.hh"
#include "G4ProcessTable.hh"

TsFilterByIncidentParticleCreatorProcess::TsFilterByIncidentParticleCreatorProcess(G4String name, TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM,
												   TsFilterManager* fM, TsVGenerator* generator, TsVScorer* scorer, TsVFilter* parentFilter, G4bool invert)
:TsVFilter(name, pM, mM, gM, fM, generator, scorer, parentFilter, invert) {
	fProcesses = new G4ProcessVector();

	ResolveParameters();
}


TsFilterByIncidentParticleCreatorProcess::~TsFilterByIncidentParticleCreatorProcess()
{;}


void TsFilterByIncidentParticleCreatorProcess::ResolveParameters() {
	fProcesses->clear();

	G4String* processNames = fPm->GetStringVector(GetFullParmName(GetName()));
	G4int length = fPm->GetVectorLength(GetFullParmName(GetName()));

	for (G4int i = 0; i < length; i++) {
		G4ProcessTable* theProcessTable = G4ProcessTable::GetProcessTable();
		G4ProcessVector* oneVector = theProcessTable->FindProcesses(processNames[i]);

		for (size_t j = 0; j < oneVector->size(); j++)
			fProcesses->insert((*oneVector)[j]);

		if (fProcesses->size()==0) {
			G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
			G4cerr << GetName() << " = " << processNames[i] << " refers to an unknown process." << G4endl;
			fPm->AbortSession(1);
		}
	}

	TsVFilter::ResolveParameters();
}


G4bool TsFilterByIncidentParticleCreatorProcess::Accept(const G4Step* aStep) const {
	if (fParentFilter && !fParentFilter->Accept(aStep)) return false;

	for (size_t i = 0; i < fProcesses->size(); i++)
		if (fScorer->GetIncidentParticleCreatorProcess() == (*fProcesses)[i]) {
			if (fInvert) return false;
			else return true;
	    }

	if (fInvert) return true;
	else return false;
}


G4bool TsFilterByIncidentParticleCreatorProcess::AcceptTrack(const G4Track*) const {
	G4cerr << "Topas is exiting due to a serious error in source setup." << G4endl;
	G4cerr << "Sources cannot be filtered by " << GetName() << G4endl;
	fPm->AbortSession(1);
	return false;
}
