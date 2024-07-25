// Filter for OnlyIncludeParticlesOfGeneration
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

#include "MyFilterByGeneration.hh"

MyFilterByGeneration::MyFilterByGeneration(G4String name, TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM,
										   TsFilterManager* fM, TsVGenerator* generator, TsVScorer* scorer, TsVFilter* parentFilter)
:TsVFilter(name, pM, mM, gM, fM, generator, scorer, parentFilter) {
	ResolveParameters();
}


MyFilterByGeneration::~MyFilterByGeneration()
{;}


void MyFilterByGeneration::ResolveParameters() {
	fAcceptPrimary = false;
	fAcceptSecondary = false;

	G4String acceptGeneration = fPm->GetStringParameter(GetFullParmName(GetName()));
	acceptGeneration.toLower();
	if (acceptGeneration=="primary") fAcceptPrimary = true;
	if (acceptGeneration=="secondary") fAcceptSecondary = true;

	TsVFilter::ResolveParameters();
}


G4bool MyFilterByGeneration::Accept(const G4Step* aStep) const {
	if (fParentFilter && !fParentFilter->Accept(aStep)) return false;

	if (fAcceptPrimary   && aStep->GetTrack()->GetParentID() == 0 ) {
		if (fInvert) return false;
		else return true;
	}

	if (fAcceptSecondary && aStep->GetTrack()->GetParentID() != 0 ) {
		if (fInvert) return false;
		else return true;
	}

	if (fInvert) return true;
	else return false;
}


G4bool MyFilterByGeneration::AcceptTrack(const G4Track*) const {
	G4cerr << "Topas is exiting due to a serious error in source setup." << G4endl;
	G4cerr << "Sources cannot be filtered by " << GetName() << G4endl;
	exit(1);
}
