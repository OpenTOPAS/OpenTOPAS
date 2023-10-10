// Scorer for MyNtupleScorer1
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

#include "MyNtupleScorer1.hh"

#include "G4PSDirectionFlag.hh"

MyNtupleScorer1::MyNtupleScorer1(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
						  G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
						 : TsVNtupleScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetSurfaceScorer();

	fNtuple->RegisterColumnD(&fEnergy, "Energy", "MeV");
	fNtuple->RegisterColumnF(&fWeight, "Weight", "");
	fNtuple->RegisterColumnI(&fParticleType, "Particle Type (in PDG Format)");
	fNtuple->RegisterColumnB(&fIsNewHistory, "Flag to tell if this is the First Scored Particle from this History (1 means true)");
}


MyNtupleScorer1::~MyNtupleScorer1() {;}


G4bool MyNtupleScorer1::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	ResolveSolid(aStep);

	if (IsSelectedSurface(aStep)) {
		G4StepPoint* theStepPoint=0;
		G4int direction = GetDirection();
		if (direction == fFlux_In) theStepPoint = aStep->GetPreStepPoint();
		else if (direction == fFlux_Out) theStepPoint = aStep->GetPostStepPoint();
		else return false;

		fEnergy       = theStepPoint->GetKineticEnergy();
		fWeight       = theStepPoint->GetWeight();
		fParticleType = aStep->GetTrack()->GetDefinition()->GetPDGEncoding();

		// Check if this is a new history
		fRunID   = GetRunID();
		fEventID = GetEventID();
		if (fEventID != fPrevEventID || fRunID != fPrevRunID) {
			fIsNewHistory = true;
			fPrevEventID = fEventID;
			fPrevRunID = fRunID;
		} else {
			fIsNewHistory = false;
		}

		fNtuple->Fill();
		return true;
	}
	return false;
}
