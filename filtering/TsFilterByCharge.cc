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

#include "TsFilterByCharge.hh"

TsFilterByCharge::TsFilterByCharge(G4String name, TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM,
								   TsFilterManager* fM, TsVGenerator* generator, TsVScorer* scorer, TsVFilter* parentFilter, G4bool invert)
:TsVFilter(name, pM, mM, gM, fM, generator, scorer, parentFilter, invert) {
	ResolveParameters();
}


TsFilterByCharge::~TsFilterByCharge()
{;}


void TsFilterByCharge::ResolveParameters() {
	fAcceptNegative = false;
	fAcceptNeutral = false;
	fAcceptPositive = false;

	G4String* acceptCharges = fPm->GetStringVector(GetFullParmName(GetName()));
	G4int length = fPm->GetVectorLength(GetFullParmName(GetName()));

	for (G4int i = 0; i < length; i++) {
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(acceptCharges[i]);
#else
		acceptCharges[i].toLower();
#endif
		if (acceptCharges[i]=="negative") fAcceptNegative = true;
		if (acceptCharges[i]=="neutral") fAcceptNeutral = true;
		if (acceptCharges[i]=="positive") fAcceptPositive = true;
	}

	TsVFilter::ResolveParameters();
}


G4bool TsFilterByCharge::Accept(const G4Step* aStep) const {
	if (fParentFilter && !fParentFilter->Accept(aStep)) return false;

	if (fAcceptNegative && aStep->GetPreStepPoint()->GetCharge() < 0. ) {
		if (fInvert) return false;
		else return true;
	}

	if (fAcceptNeutral  && aStep->GetPreStepPoint()->GetCharge() == 0. ) {
		if (fInvert) return false;
		else return true;
	}

	if (fAcceptPositive && aStep->GetPreStepPoint()->GetCharge() > 0. ) {
		if (fInvert) return false;
		else return true;
	}

	if (fInvert) return true;
	else return false;
}


G4bool TsFilterByCharge::AcceptTrack(const G4Track* aTrack) const {
	if (fParentFilter && !fParentFilter->AcceptTrack(aTrack)) return false;

	if (fAcceptNegative && aTrack->GetDynamicParticle()->GetCharge() < 0. ) {
		if (fInvert) return false;
		else return true;
	}

	if (fAcceptNeutral  && aTrack->GetDynamicParticle()->GetCharge() == 0. ) {
		if (fInvert) return false;
		else return true;
	}

	if (fAcceptPositive && aTrack->GetDynamicParticle()->GetCharge() > 0. ) {
		if (fInvert) return false;
		else return true;
	}

	if (fInvert) return true;
	else return false;
}
