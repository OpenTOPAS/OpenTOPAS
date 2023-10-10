// Scorer for EnergyFluence
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

#include "MyScoreEnergyFluence.hh"

MyScoreEnergyFluence::MyScoreEnergyFluence(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
										   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("MeV/mm2");
}


MyScoreEnergyFluence::~MyScoreEnergyFluence() {;}


G4bool MyScoreEnergyFluence::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	G4double quantity = aStep->GetStepLength();

	if ( quantity > 0.) {
		G4double energy = aStep->GetTrack()->GetKineticEnergy();

		if ( energy > 0. ) {
			ResolveSolid(aStep);

			quantity /= fSolid->GetCubicVolume();
			quantity *= aStep->GetPreStepPoint()->GetWeight();
			quantity *= energy;

			AccumulateHit(aStep, quantity);

			return true;
		}
	}
	return false;
}
