// Scorer for MyNtupleScorer2
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

#include "MyNtupleScorer2.hh"

#include "TsTrackInformation.hh"

#include "G4PSDirectionFlag.hh"
#include "G4VProcess.hh"

MyNtupleScorer2::MyNtupleScorer2(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
                          G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
                         : TsVNtupleScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
    SetSurfaceScorer();
	pM->SetNeedsSteppingAction();
	
	fNtuple->RegisterColumnF(&fPosX, "Position X", "cm");
	fNtuple->RegisterColumnF(&fPosY, "Position Y", "cm");
	fNtuple->RegisterColumnF(&fPosZ, "Position Z", "cm");
	fNtuple->RegisterColumnF(&fCosX, "Direction Cosine X", "");
	fNtuple->RegisterColumnF(&fCosY, "Direction Cosine Y", "");
	fNtuple->RegisterColumnF(&fEnergy, "Energy", "MeV");
	fNtuple->RegisterColumnF(&fWeight, "Weight", "");
	fNtuple->RegisterColumnI(&fPType, "Particle Type (in PDG Format)");
	fNtuple->RegisterColumnB(&fCosZIsNegative, "Flag to tell if Third Direction Cosine is Negative (1 means true)");
	fNtuple->RegisterColumnS(&fOriginProcessName, "Origin Process");
	fNtuple->RegisterColumnS(&fLastVolumeName, "Last Volume");
}


MyNtupleScorer2::~MyNtupleScorer2() {;}


G4bool MyNtupleScorer2::ProcessHits(G4Step* aStep,G4TouchableHistory*)
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

		G4ThreeVector pos       = theStepPoint->GetPosition();
		G4ThreeVector mom       = theStepPoint->GetMomentumDirection();
		G4ThreeVector vertexPos = aStep->GetTrack()->GetVertexPosition();
		G4ThreeVector vertexMom = aStep->GetTrack()->GetVertexMomentumDirection();
		
		fPType          = aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
		fPosX           = pos.x();
		fPosY           = pos.y();
		fPosZ           = pos.z();
		fCosX           = mom.x();
		fCosY           = mom.y();
		fCosZIsNegative = mom.z() < 0.;
		fEnergy	        = theStepPoint->GetKineticEnergy();
		fWeight	        = theStepPoint->GetWeight();
	
		const G4VProcess* originProcess = aStep->GetTrack()->GetCreatorProcess();
		if (originProcess)
			fOriginProcessName = originProcess->GetProcessName();
		else
			fOriginProcessName = "Primary";
		
		TsTrackInformation* parentInformation = (TsTrackInformation*)(aStep->GetTrack()->GetUserInformation());
		if (parentInformation) {
			std::vector<G4VPhysicalVolume*> volumes = parentInformation->GetInteractionVolumes();
			if (volumes.size()==0)
				fLastVolumeName = aStep->GetTrack()->GetOriginTouchable()->GetVolume()->GetName();
			else
				fLastVolumeName = volumes.front()->GetName();
		} else {
			fLastVolumeName = aStep->GetTrack()->GetOriginTouchable()->GetVolume()->GetName();
		}

        fNtuple->Fill();
        return true;
    }
    return false;   
}
