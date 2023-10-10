// Scorer for MyBinnedScorer1
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

#include "MyBinnedScorer1.hh"

MyBinnedScorer1::MyBinnedScorer1(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
						  G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
						 : TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("MeV"); 
}


MyBinnedScorer1::~MyBinnedScorer1() {;}


G4bool MyBinnedScorer1::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}
	
	G4double edep = aStep->GetTotalEnergyDeposit();
	if ( edep > 0. ) {
		edep *= 2. * aStep->GetPreStepPoint()->GetWeight();
		
		AccumulateHit(aStep, edep);
		
		return true;
	}
	return false;	
}
