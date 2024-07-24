// Scorer for MyNtupleScorer2
//
// ********************************************************************
// *                                                                  *
// * Copyright 2024 The TOPAS Collaboration                           *
// * Copyright 2022 The TOPAS Collaboration                           *
// *                                                                  *
// * Permission is hereby granted, free of charge, to any person      *
// * obtaining a copy of this software and associated documentation   *
// * files (the "Software"), to deal in the Software without          *
// * restriction, including without limitation the rights to use,     *
// * copy, modify, merge, publish, distribute, sublicense, and/or     *
// * sell copies of the Software, and to permit persons to whom the   *
// * Software is furnished to do so, subject to the following         *
// * conditions:                                                      *
// *                                                                  *
// * The above copyright notice and this permission notice shall be   *
// * included in all copies or substantial portions of the Software.  *
// *                                                                  *
// * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,  *
// * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES  *
// * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND         *
// * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT      *
// * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,     *
// * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
// * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR    *
// * OTHER DEALINGS IN THE SOFTWARE.                                  *
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
