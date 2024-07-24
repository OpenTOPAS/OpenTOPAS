// Filter for OnlyIncludeParticlesOfTwiceAtomicNumber,OnlyIncludeParticlesNotOfTwiceAtomicNumber
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

#include "MyFilter1.hh"

MyFilter1::MyFilter1(G4String name, TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM,
						 TsFilterManager* fM, TsVGenerator* generator, TsVScorer* scorer, TsVFilter* parentFilter)
:TsVFilter(name, pM, mM, gM, fM, generator, scorer, parentFilter)
{
	ResolveParameters();
	if (name=="onlyincludeparticlesnotoftwiceatomicnumber") fInvert = true;
}


MyFilter1::~MyFilter1()
{;}


void MyFilter1::ResolveParameters()
{
	fAtomicNumber = 2 * fPm->GetIntegerParameter(GetFullParmName(GetName()));
}


G4bool MyFilter1::Accept(const G4Step* aStep) const
{
	// This line is absolutely required in all filters even if you are not filtering on steps
	if (fParentFilter && !fParentFilter->Accept(aStep)) return false;

	return AcceptTrack(aStep->GetTrack());
}


G4bool MyFilter1::AcceptTrack(const G4Track* aTrack) const
{
	// This line is absolutely required in all filters
	if (fParentFilter && !fParentFilter->AcceptTrack(aTrack)) return false;

	if (aTrack->GetDefinition()->GetAtomicNumber() == fAtomicNumber) {
		if (fInvert) return false;
		else return true;
	}

	if (fInvert) return true;
	else return false;
}
