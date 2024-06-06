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

#include "TsScoreOpticalPhotonCount.hh"

#include "G4ProcessManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4VProcess.hh"

TsScoreOpticalPhotonCount::TsScoreOpticalPhotonCount(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
													 G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVNtupleScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	fExpectedNextStatus = Undefined;

	fBounceLimit = 10000;
	if ( fPm->ParameterExists( GetFullParmName( "BounceLimit" ) ) )
		fBounceLimit = fPm->GetIntegerParameter( GetFullParmName( "BounceLimit" ));

	fCounterBounce = -1;

	fNtuple->RegisterColumnF(&fPosX, "Local position X", "cm");
	fNtuple->RegisterColumnF(&fPosY, "Local position Y", "cm");
	fNtuple->RegisterColumnF(&fPosZ, "Local position Z", "cm");
	fNtuple->RegisterColumnF(&fWaveLength, "Wavelength", "nm");
	fNtuple->RegisterColumnF(&fTime, "Arrival time", "ns");
	fNtuple->RegisterColumnI(&fOpticalProcessID, "ProcessID: 1 Scintillation, 2 Cerenkov, 3 Absorption");
}


TsScoreOpticalPhotonCount::~TsScoreOpticalPhotonCount() {;}


G4bool TsScoreOpticalPhotonCount::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	G4Track* aTrack = aStep->GetTrack();
	G4OpBoundaryProcessStatus boundaryStatus=Undefined;
#ifdef TOPAS_MT
	static G4ThreadLocal G4OpBoundaryProcess* boundary=NULL;
#else
	static G4OpBoundaryProcess* boundary=NULL;
#endif

	if(!boundary){
		G4ProcessManager* procManager = aTrack->GetDefinition()->GetProcessManager();
		G4int nprocesses = procManager->GetProcessListLength();
		G4ProcessVector* procVector = procManager->GetProcessList();
		for( int i = 0; i < nprocesses; i++){
			if((*procVector)[i]->GetProcessName()=="OpBoundary"){
				boundary = (G4OpBoundaryProcess*)(*procVector)[i];
				break;
			}
		}
	}

	G4ParticleDefinition* particleType = aTrack->GetDefinition();
	if( particleType == G4OpticalPhoton::OpticalPhotonDefinition() ) {
		if (boundary == nullptr) {
			G4cerr << "TOPAS is exiting due to a serious error in TsScoreOpticalPhotonCount::ProcessHits" << G4endl;
			G4cerr << "G4OpBoundaryProcess* boundary == nullptr" << G4endl;
			fPm->AbortSession(1);
		}
		boundaryStatus=boundary->GetStatus();
		if(fExpectedNextStatus==StepTooSmall){
			if(boundaryStatus!=StepTooSmall){
				G4cout << "TOPAS is exiting due a serious error in geometry" << G4endl;
				G4cout << "Something is wrong with the surface normal" << G4endl;
				G4cout << "Ensure that surface is correctly defined" << G4endl;
				fPm->AbortSession(1);
			}
		}

		fExpectedNextStatus = Undefined;
		G4ThreeVector pos, localpos;

		G4TouchableHandle theTouchable;
		G4StepPoint* preStepPoint = 0;
		const G4VProcess* creator = aTrack->GetCreatorProcess();

		switch(boundaryStatus){
			case Detection: {

				preStepPoint = aStep->GetPreStepPoint();

				if ( creator && creator->GetProcessName() == "Scintillation" )
					fOpticalProcessID = 1;
				else if ( creator && creator->GetProcessName() == "Cerenkov" )
					fOpticalProcessID = 2;
				else if ( creator && creator->GetProcessName() == "Absorption" )
					fOpticalProcessID = 3;
				else
					fOpticalProcessID = 0;

				fWaveLength = 12.4e-4/(aTrack->GetKineticEnergy()/eV);
				fTime   = aTrack->GetGlobalTime();
				theTouchable = preStepPoint->GetTouchableHandle();
				pos = preStepPoint->GetPosition();
				localpos = theTouchable->GetHistory()->GetTopTransform().TransformPoint(pos);

				fPosX = localpos.x();
				fPosY = localpos.y();
				fPosZ = localpos.z();

				fNtuple->Fill();

				return true;
			}
			case FresnelReflection:
				fCounterBounce++;
				break;
			case TotalInternalReflection:
				if ( fBounceLimit > 0 && fCounterBounce >= fBounceLimit) {
					aTrack->SetTrackStatus(fStopAndKill);
					G4cout << "\n Bounce Limit Exceeded. Track eliminated." << G4endl;
					return false;
				}
				break;
			default:
				return false;
		}
	}
	return false;
}

