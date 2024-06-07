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

#include "TsFilterByAtomicNumber.hh"

#include "TsTrackInformation.hh"

TsFilterByAtomicNumber::TsFilterByAtomicNumber(G4String name, TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM,
											   TsFilterManager* fM, TsVGenerator* generator, TsVScorer* scorer, TsVFilter* parentFilter, G4bool invert,
											   G4bool includeAncestors)
:TsVFilter(name, pM, mM, gM, fM, generator, scorer, parentFilter, invert), fIncludeAncestors(includeAncestors) {
	ResolveParameters();

	if (fIncludeAncestors)
		pM->SetNeedsTrackingAction();
}


TsFilterByAtomicNumber::~TsFilterByAtomicNumber()
{;}


void TsFilterByAtomicNumber::ResolveParameters() {
	fAtomicNumber = fPm->GetIntegerParameter(GetFullParmName(GetName()));
	TsVFilter::ResolveParameters();
}


G4bool TsFilterByAtomicNumber::Accept(const G4Step* aStep) const {
	if (fParentFilter && !fParentFilter->Accept(aStep)) return false;

	if (aStep->GetTrack()->GetDefinition()->GetAtomicNumber() == fAtomicNumber) {
		if (fInvert) return false;
		else return true;
	}

	if (fIncludeAncestors) {
		TsTrackInformation* parentInformation = (TsTrackInformation*)(aStep->GetTrack()->GetUserInformation());
		if (parentInformation) {
			std::vector<G4ParticleDefinition*> ancestorParticleDefs = parentInformation->GetParticleDefs();
			for (size_t i=0; i < ancestorParticleDefs.size(); i++)
			if (ancestorParticleDefs[i]->GetAtomicNumber() == fAtomicNumber) {
				if (fInvert) return false;
				else return true;
			}
		}
	}

	if (fInvert) return true;
	else return false;
}


G4bool TsFilterByAtomicNumber::AcceptTrack(const G4Track* aTrack) const {
	if (fParentFilter && !fParentFilter->AcceptTrack(aTrack)) return false;

	if (aTrack->GetDefinition()->GetAtomicNumber() == fAtomicNumber) {
		if (fInvert) return false;
		else return true;
	}

	if (fInvert) return true;
	else return false;
}
