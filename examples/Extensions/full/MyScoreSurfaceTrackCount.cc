// Scorer for SurfaceTrackCurrent
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

#include "MyScoreSurfaceTrackCount.hh"

#include "G4PSDirectionFlag.hh"

MyScoreSurfaceTrackCount::MyScoreSurfaceTrackCount(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
												   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetSurfaceScorer();
	SetUnit("");
}


MyScoreSurfaceTrackCount::~MyScoreSurfaceTrackCount() {;}


G4bool MyScoreSurfaceTrackCount::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	ResolveSolid(aStep);

	if (IsSelectedSurface(aStep)) {
		G4double weight = aStep->GetPreStepPoint()->GetWeight();

		if ( weight != 0. ) {
			AccumulateHit(aStep, weight);
			return true;
		}
	}
	return false;
}
