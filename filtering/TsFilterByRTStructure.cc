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

#include "TsFilterByRTStructure.hh"

#include "TsVGeometryComponent.hh"
#include "TsVScorer.hh"

TsFilterByRTStructure::TsFilterByRTStructure(G4String name, TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM,
										   TsFilterManager* fM, TsVGenerator* generator, TsVScorer* scorer, TsVFilter* parentFilter, G4bool invert)
:TsVFilter(name, pM, mM, gM, fM, generator, scorer, parentFilter, invert) {
	fStructureIDs = new std::vector<G4int>;
	ResolveParameters();

	// Scorer will need a pointer back to this filter to get StructureIDs at output time
	fScorer->SetRTStructureFilter(this);
}


TsFilterByRTStructure::~TsFilterByRTStructure()
{;}


void TsFilterByRTStructure::ResolveParameters() {
	fNames = fPm->GetStringVector(GetFullParmName(GetName()));
	fNamesLength = fPm->GetVectorLength(GetFullParmName(GetName()));

	CacheGeometryPointers();
}


void TsFilterByRTStructure::CacheGeometryPointers() {
	fStructureIDs->clear();

	fComponent = fScorer->GetComponent();

	for (G4int i = 0; i < fNamesLength; i++) {
		G4int j = fComponent->GetStructureID(fNames[i]);
		if (j==-1) {
			G4cerr << "Topas is exiting due to a serious error in scorer setup." << G4endl;
			G4cerr << "Component: " << fComponent->GetNameWithCopyId() << " does not have stucture: " << fNames[i] << G4endl;
			fPm->AbortSession(1);
		}
		fStructureIDs->push_back(j);
	}
}


G4bool TsFilterByRTStructure::Accept(const G4Step* aStep) const {
	if (fParentFilter && !fParentFilter->Accept(aStep)) return false;

	// See if this index number is in any of the specified structures
	for (G4int i = 0; i < (int)fStructureIDs->size(); i++) {
		if (fComponent->IsInNamedStructure((*fStructureIDs)[i], aStep)) {
			if (fInvert) return false;
			else return true;
		}
	}

	if (fInvert) return true;
	else return false;
}


G4bool TsFilterByRTStructure::AcceptTrack(const G4Track*) const {
	G4cerr << "Topas is exiting due to a serious error in source setup." << G4endl;
	G4cerr << "Sources cannot be filtered by " << GetName() << G4endl;
	fPm->AbortSession(1);
	return false;
}


std::vector<G4int>* TsFilterByRTStructure::GetStructureIDs() {
	return fStructureIDs;
}


G4bool TsFilterByRTStructure::IsInverted() {
	return fInvert;
}
