// Scorer for KineticEnergyAtSurface
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

#include "MyScoreKineticEnergyAtSurface.hh"

MyScoreKineticEnergyAtSurface::MyScoreKineticEnergyAtSurface(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
															 G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("MeV");
}


MyScoreKineticEnergyAtSurface::~MyScoreKineticEnergyAtSurface() {;}


G4bool MyScoreKineticEnergyAtSurface::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	G4double weight = 0.;
	G4TouchableHandle theTouchable = aStep->GetPreStepPoint()->GetTouchableHandle();
	if (aStep->GetPreStepPoint()->GetStepStatus() == fGeomBoundary) {
		weight = 1.;
		weight *= aStep->GetPreStepPoint()->GetWeight();

		if ( weight > 0. ) {
			ResolveSolid(aStep);

			AccumulateHit(aStep, weight);

			return true;
		}
	}
	return false;
}
