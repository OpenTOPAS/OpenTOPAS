// Filter for OnlyIncludeParticlesOfTwiceAtomicNumber,OnlyIncludeParticlesNotOfTwiceAtomicNumber
//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * This file was obtained from Topas MC Inc under the license       *
// * agreement set forth at http://www.topasmc.org/registration       *
// * Any use of this file constitutes full acceptance of              *
// * this TOPAS MC license agreement.                                 *
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
