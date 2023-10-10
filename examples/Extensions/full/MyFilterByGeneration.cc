// Filter for OnlyIncludeParticlesOfGeneration
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
