// Filter for OnlyIncludeParticlesCharged,OnlyIncludeParticlesNotCharged
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

#include "MyFilterByCharge.hh"

MyFilterByCharge::MyFilterByCharge(G4String name, TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM,
								   TsFilterManager* fM, TsVGenerator* generator, TsVScorer* scorer, TsVFilter* parentFilter)
:TsVFilter(name, pM, mM, gM, fM, generator, scorer, parentFilter) {
	ResolveParameters();
	if (name=="onlyincludeparticlesnotcharged") fInvert = true;
}


MyFilterByCharge::~MyFilterByCharge()
{;}


void MyFilterByCharge::ResolveParameters() {
	fAcceptNegative = false;
	fAcceptNeutral = false;
	fAcceptPositive = false;

	G4String* acceptCharges = fPm->GetStringVector(GetFullParmName(GetName()));
	G4int length = fPm->GetVectorLength(GetFullParmName(GetName()));

	for (G4int i = 0; i < length; i++) {
		acceptCharges[i].toLower();
		if (acceptCharges[i]=="negative") fAcceptNegative = true;
		if (acceptCharges[i]=="neutral") fAcceptNeutral = true;
		if (acceptCharges[i]=="positive") fAcceptPositive = true;
	}

	TsVFilter::ResolveParameters();
}


G4bool MyFilterByCharge::Accept(const G4Step* aStep) const {
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


G4bool MyFilterByCharge::AcceptTrack(const G4Track* aTrack) const {
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
