// Scorer for Charge
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

#include "MyScoreCharge.hh"

#include "TsVGeometryComponent.hh"

MyScoreCharge::MyScoreCharge(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager *eM,
										   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("e+");
}


MyScoreCharge::~MyScoreCharge() {;}


G4bool MyScoreCharge::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	G4bool status = false;

	// Add charge when particle stops
	if (aStep->GetPostStepPoint()->GetKineticEnergy() == 0) {
		G4double CellCharge = aStep->GetTrack()->GetParticleDefinition()->GetPDGCharge();
		CellCharge *= aStep->GetPreStepPoint()->GetWeight();
		AccumulateHit(aStep, CellCharge);
		status = true;
	}

	// Subtract charge when particle is generated, except for primaries
	if (aStep->GetTrack()->GetCurrentStepNumber() == 1 && aStep->GetTrack()->GetParentID() != 0) {
		G4double CellCharge = aStep->GetTrack()->GetParticleDefinition()->GetPDGCharge();
		CellCharge *= aStep->GetPreStepPoint()->GetWeight();
		CellCharge *= -1.0;
		AccumulateHit(aStep, CellCharge);
		status = true;
	}

	return status;
}
