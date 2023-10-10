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

#include "MyFilterByAtomicMass.hh"

MyFilterByAtomicMass::MyFilterByAtomicMass(G4String name, TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM,
										   TsFilterManager* fM, TsVGenerator* generator, TsVScorer* scorer, TsVFilter* parentFilter)
:TsVFilter(name, pM, mM, gM, fM, generator, scorer, parentFilter) {
	ResolveParameters();
	if (name=="onlyincludeparticlesnotoftwiceatomicnumber") fInvert = true;
}


MyFilterByAtomicMass::~MyFilterByAtomicMass()
{;}


void MyFilterByAtomicMass::ResolveParameters() {
	fAtomicMass = fPm->GetIntegerParameter(GetFullParmName(GetName()));
	TsVFilter::ResolveParameters();
}


G4bool MyFilterByAtomicMass::Accept(const G4Step* aStep) const {
	if (fParentFilter && !fParentFilter->Accept(aStep)) return false;

	return AcceptTrack(aStep->GetTrack());
}


G4bool MyFilterByAtomicMass::AcceptTrack(const G4Track* aTrack) const {
	if (fParentFilter && !fParentFilter->AcceptTrack(aTrack)) return false;

	if (aTrack->GetDefinition()->GetAtomicMass() == fAtomicMass) {
		if (fInvert) return false;
		else return true;
	}

	if (fInvert) return true;
	else return false;
}
