// Scorer for Fluence
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

#include "MyScoreFluence.hh"

MyScoreFluence::MyScoreFluence(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
							   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("/mm2");
}


MyScoreFluence::~MyScoreFluence() {;}


G4bool MyScoreFluence::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	G4double quantity = aStep->GetStepLength();

	if ( quantity > 0.) {
		ResolveSolid(aStep);

		quantity /= fSolid->GetCubicVolume();
		quantity *= aStep->GetPreStepPoint()->GetWeight();

		AccumulateHit(aStep, quantity);

		return true;
	}
	return false;
}
