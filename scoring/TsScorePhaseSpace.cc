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

#include "TsScorePhaseSpace.hh"

#include "TsVGeometryComponent.hh"
#include "TsVScorer.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4UIcommand.hh"
#include "G4Tokenizer.hh"
#include "G4SystemOfUnits.hh"
#include "G4PSDirectionFlag.hh"

TsScorePhaseSpace::TsScorePhaseSpace(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
                                     G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
    : TsVNtupleScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer),
      fPosX(0.), fPosY(0.), fPosZ(0.), fCosX(0.), fCosY(0.), fCosZIsNegative(false), fEnergy(0.), fWeight(0.), fPType(0),
      fNumberOfSequentialEmptyHistories(0), fIsEmptyHistory(true), fIsNewHistory(false), fTOPASTime(0.), fTimeOfFlight(0.),
      fRunID(0), fEventID(0), fTrackID(0), fParentID(0), fCharge(0.), fCreatorProcess(""),
      fVertexKE(0.), fVertexPosX(0.), fVertexPosY(0.), fVertexPosZ(0.), fVertexCosX(0.), fVertexCosY(0.), fVertexCosZ(0.),
      fSeedPart1(0), fSeedPart2(0), fSeedPart3(0), fSeedPart4(0), fSignedEnergy(0.), fSignedPType(0),
      fIncludeEmptyHistoriesInSequence(false), fIncludeEmptyHistoriesAtEndOfRun(false),
      fIncludeEmptyHistoriesAtEndOfFile(false),
      fKillAfterPhaseSpace(false), fOutputToLimited(false), fIncludeTOPASTime(false),
      fIncludeTimeOfFlight(false), fIncludeTrackID(false), fIncludeParentID(false), fIncludeCharge(false),
      fIncludeCreatorProcess(false), fIncludeVertexInfo(false), fIncludeSeed(false),
      fPrevRunID(-1), fPrevEventID(-1), fNumberOfHistoriesThatMadeItToPhaseSpace(0)
{
	SetSurfaceScorer();

	if (fPm->ParameterExists(GetFullParmName("IncludeEmptyHistories"))) {
		G4String includeEmptyHistories = fPm->GetStringParameter(GetFullParmName("IncludeEmptyHistories"));
		G4String includeEmptyHistoriesLower = includeEmptyHistories;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(includeEmptyHistoriesLower);
#else
		includeEmptyHistoriesLower.toLower();
#endif
		if (includeEmptyHistoriesLower != "none") {
			if (includeEmptyHistoriesLower == "insequence") {
				if (fOutFileType == "limited") {
					G4cerr << GetFullParmName("IncludeEmptyHistories") << " has value: " << includeEmptyHistories << G4endl;
					G4cerr << " but this option is not supported for output type Limited." << G4endl;
					fPm->AbortSession(1);
				}
				fIncludeEmptyHistoriesInSequence = "true";
			} else if (includeEmptyHistoriesLower == "atendofrun") {
				if (fOutFileType == "limited") {
					G4cerr << GetFullParmName("IncludeEmptyHistories") << " has value: " << includeEmptyHistories << G4endl;
					G4cerr << " but this option is not supported for output type Limited." << G4endl;
					fPm->AbortSession(1);
				}
				fIncludeEmptyHistoriesAtEndOfRun = "true";
			} else if (includeEmptyHistoriesLower == "atendoffile") {
				if (fOutFileType == "limited") {
					G4cerr << GetFullParmName("IncludeEmptyHistories") << " has value: " << includeEmptyHistories << G4endl;
					G4cerr << " but this option is not supported for output type Limited." << G4endl;
					fPm->AbortSession(1);
				}
				fIncludeEmptyHistoriesAtEndOfFile = "true";
			} else {
				G4cerr << "Serious error in scoring setup." << G4endl;
				G4cerr << GetFullParmName("IncludeEmptyHistories") << " has unsupported value: " << includeEmptyHistories << G4endl;
				G4cerr << "Value must be one of: None, InSequence, AtEndOfRun or AtEndOfFile" << G4endl;
				fPm->AbortSession(1);
			}
		}
	}

	if (fPm->ParameterExists(GetFullParmName("KillAfterPhaseSpace")) && fPm->GetBooleanParameter(GetFullParmName("KillAfterPhaseSpace")))
		fKillAfterPhaseSpace = true;

	if (fOutFileType == "limited") {
		fOutputToLimited = true;
		fNtuple->RegisterColumnI8(&fSignedPType, "Particle Type (sign from z direction)");
		fNtuple->RegisterColumnF(&fSignedEnergy, "Energy (-ve if new history)", "MeV");
		fNtuple->RegisterColumnF(&fPosX, "Position X", "cm");
		fNtuple->RegisterColumnF(&fPosY, "Position Y", "cm");
		fNtuple->RegisterColumnF(&fPosZ, "Position Z", "cm");
		fNtuple->RegisterColumnF(&fCosX, "Direction Cosine X", "");
		fNtuple->RegisterColumnF(&fCosY, "Direction Cosine Y", "");
		fNtuple->RegisterColumnF(&fWeight, "Weight", "");
	} else {
		fNtuple->RegisterColumnF(&fPosX, "Position X", "cm");
		fNtuple->RegisterColumnF(&fPosY, "Position Y", "cm");
		fNtuple->RegisterColumnF(&fPosZ, "Position Z", "cm");
		fNtuple->RegisterColumnF(&fCosX, "Direction Cosine X", "");
		fNtuple->RegisterColumnF(&fCosY, "Direction Cosine Y", "");
		fNtuple->RegisterColumnF(&fEnergy, "Energy", "MeV");
		fNtuple->RegisterColumnF(&fWeight, "Weight", "");
		fNtuple->RegisterColumnI(&fPType, "Particle Type (in PDG Format)");
		fNtuple->RegisterColumnB(&fCosZIsNegative, "Flag to tell if Third Direction Cosine is Negative (1 means true)");
		fNtuple->RegisterColumnB(&fIsNewHistory, "Flag to tell if this is the First Scored Particle from this History (1 means true)");

		if (fPm->ParameterExists(GetFullParmName("IncludeTOPASTime")) && fPm->GetBooleanParameter(GetFullParmName("IncludeTOPASTime"))) {
			fIncludeTOPASTime = true;
			fNtuple->RegisterColumnF(&fTOPASTime, "TOPAS Time", "s");
		}

		if (fPm->ParameterExists(GetFullParmName("IncludeTimeOfFlight")) && fPm->GetBooleanParameter(GetFullParmName("IncludeTimeOfFlight"))) {
			fIncludeTimeOfFlight = true;
			fNtuple->RegisterColumnF(&fTimeOfFlight, "Time of Flight", "ns");
		}

		if (fPm->ParameterExists(GetFullParmName("IncludeRunID")) && fPm->GetBooleanParameter(GetFullParmName("IncludeRunID")))
			fNtuple->RegisterColumnI(&fRunID, "Run ID");

		if (fPm->ParameterExists(GetFullParmName("IncludeEventID")) && fPm->GetBooleanParameter(GetFullParmName("IncludeEventID")))
			fNtuple->RegisterColumnI(&fEventID, "Event ID");

		if (fPm->ParameterExists(GetFullParmName("IncludeTrackID")) && fPm->GetBooleanParameter(GetFullParmName("IncludeTrackID"))) {
			fIncludeTrackID = true;
			fNtuple->RegisterColumnI(&fTrackID, "Track ID");
		}

		if (fPm->ParameterExists(GetFullParmName("IncludeParentID")) && fPm->GetBooleanParameter(GetFullParmName("IncludeParentID"))) {
			fIncludeParentID = true;
			fNtuple->RegisterColumnI(&fParentID, "Parent ID");
		}

		if (fPm->ParameterExists(GetFullParmName("IncludeCharge")) && fPm->GetBooleanParameter(GetFullParmName("IncludeCharge"))) {
			fIncludeCharge = true;
			fNtuple->RegisterColumnF(&fCharge, "Charge", "e+");
		}

		if (fPm->ParameterExists(GetFullParmName("IncludeCreatorProcess")) && fPm->GetBooleanParameter(GetFullParmName("IncludeCreatorProcess"))) {
			fIncludeCreatorProcess = true;
			fNtuple->RegisterColumnS(&fCreatorProcess, "Creator Process Name");
		}

		if (fPm->ParameterExists(GetFullParmName("IncludeVertexInfo")) && fPm->GetBooleanParameter(GetFullParmName("IncludeVertexInfo"))) {
			fIncludeVertexInfo = true;
			fNtuple->RegisterColumnF(&fVertexKE, "Initial Kinetic Energy", "MeV");
			fNtuple->RegisterColumnF(&fVertexPosX, "Vertex Position X", "cm");
			fNtuple->RegisterColumnF(&fVertexPosY, "Vertex Position Y", "cm");
			fNtuple->RegisterColumnF(&fVertexPosZ, "Vertex Position Z", "cm");
			fNtuple->RegisterColumnF(&fVertexCosX, "Initial Direction Cosine X", "");
			fNtuple->RegisterColumnF(&fVertexCosY, "Initial Direction Cosine Y", "");
			fNtuple->RegisterColumnF(&fVertexCosZ, "Initial Direction Cosine Z", "");
		}

		if (fPm->ParameterExists(GetFullParmName("IncludeSeed")) && fPm->GetBooleanParameter(GetFullParmName("IncludeSeed"))) {
			fIncludeSeed = true;
			fNtuple->RegisterColumnI(&fSeedPart1, "Seed Part 1");
			fNtuple->RegisterColumnI(&fSeedPart2, "Seed Part 2");
			fNtuple->RegisterColumnI(&fSeedPart3, "Seed Part 3");
			fNtuple->RegisterColumnI(&fSeedPart4, "Seed Part 4");
		}
	}
}


TsScorePhaseSpace::~TsScorePhaseSpace()
{;}


G4bool TsScorePhaseSpace::ProcessHits(G4Step* aStep, G4TouchableHistory*)
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
		fRunID = GetRunID();
		fEventID = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();

		if (fIncludeTOPASTime)
			fTOPASTime = GetTime();

		if (fIncludeTimeOfFlight)
			fTimeOfFlight   = aStep->GetTrack()->GetGlobalTime();

		if (fIncludeTrackID)
			fTrackID = aStep->GetTrack()->GetTrackID();

		if (fIncludeParentID)
			fParentID = aStep->GetTrack()->GetParentID();

		if (fIncludeCharge)
			fCharge = aStep->GetTrack()->GetDynamicParticle()->GetCharge();
		
		if (fIncludeCreatorProcess) {
			const G4VProcess* creatorProcess = aStep->GetTrack()->GetCreatorProcess();
			if (creatorProcess)
				fCreatorProcess = creatorProcess->GetProcessName();
			else
				fCreatorProcess = "Primary";
		}
		
		if (fIncludeVertexInfo) {
			fVertexKE       = aStep->GetTrack()->GetVertexKineticEnergy();
			fVertexPosX     = vertexPos.x();
			fVertexPosY     = vertexPos.y();
			fVertexPosZ     = vertexPos.z();
			fVertexCosX     = vertexMom.x();
			fVertexCosY     = vertexMom.y();
			fVertexCosZ     = vertexMom.z();
		}

		if (fIncludeSeed) {
			G4Tokenizer next(GetRandomNumberStatusForThisEvent());
			next();
			next();
			G4String token = next();
			fSeedPart1 = G4UIcommand::ConvertToInt(token);
			token = next();
			fSeedPart2 = G4UIcommand::ConvertToInt(token);
			token = next();
			fSeedPart3 = G4UIcommand::ConvertToInt(token);
			token = next();
			fSeedPart4 = G4UIcommand::ConvertToInt(token);
		}

		// Check if this is a new history
		if (fEventID != fPrevEventID || fRunID != fPrevRunID) {
			fIsNewHistory = true;
			fNumberOfHistoriesThatMadeItToPhaseSpace++;
			fPrevEventID = fEventID;
			fPrevRunID = fRunID;
		} else {
			fIsNewHistory = false;
		}

		if (fOutputToLimited) {
			switch(fPType)
			{
				case 22:
					fPType = 1;  // gamma
					break;
				case 11:
					fPType = 2;  // electron
					break;
				case -11:
					fPType = 3;  // positron
					break;
				case 2112:
					fPType = 4;  // neutron
					break;
				case 2212:
					fPType = 5;  // proton
					break;
				default:
					return false;
			}
			fSignedEnergy = fIsNewHistory ? -fEnergy : fEnergy;
			fSignedPType = fCosZIsNegative ? -fPType : fPType;
		}

		fNtuple->Fill();
		fIsEmptyHistory = false;

		// Record some additional statistics
		G4int particleEncoding = aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
		fNumberOfParticles[particleEncoding]++;
		if (fEnergy > fMaximumKE[particleEncoding]) fMaximumKE[particleEncoding] = fEnergy;
		if (fMinimumKE[particleEncoding]==0.0) fMinimumKE[particleEncoding] = fEnergy;
		else if (fEnergy < fMinimumKE[particleEncoding]) fMinimumKE[particleEncoding] = fEnergy;

		if ( fKillAfterPhaseSpace ) aStep->GetTrack()->SetTrackStatus(fStopAndKill);
		return true;
	}
	return false;
}


void TsScorePhaseSpace::AccumulateEvent()
{
	TsVNtupleScorer::AccumulateEvent();

	if (fIsEmptyHistory) {
		if (fIncludeEmptyHistoriesAtEndOfRun || fIncludeEmptyHistoriesAtEndOfFile) {
			fNumberOfSequentialEmptyHistories++;
		} else if (fIncludeEmptyHistoriesInSequence) {
			fPType          = 0;
			fPosX           = 0.;
			fPosY           = 0.;
			fPosZ           = 0.;
			fCosX           = 0.;
			fCosY           = 0.;
			fCosZIsNegative = false;
			fEnergy	        = 0.;
			fWeight	        = -1.;
			fRunID = GetRunID();
			fEventID = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();

			fIsNewHistory = true;

			if (fIncludeTrackID)
				fTrackID = 0;

			if (fIncludeParentID)
				fParentID = 0;

			if (fIncludeCharge)
				fCharge = 0.;

			if (fIncludeCreatorProcess)
				fCreatorProcess = "emptyHistory";

			if (fIncludeVertexInfo) {
				fVertexKE       = 0.;
				fVertexPosX     = 0.;
				fVertexPosY     = 0.;
				fVertexPosZ     = 0.;
				fVertexCosX     = 0.;
				fVertexCosY     = 0.;
				fVertexCosZ     = 0.;
			}

			if (fIncludeTOPASTime)
				fTOPASTime = GetTime();

			if (fIncludeTimeOfFlight)
				fTimeOfFlight = 0.;

			if (fIncludeSeed) {
				G4Tokenizer next(GetRandomNumberStatusForThisEvent());
				next();
				next();
				G4String token = next();
				fSeedPart1 = G4UIcommand::ConvertToInt(token);
				token = next();
				fSeedPart2 = G4UIcommand::ConvertToInt(token);
				token = next();
				fSeedPart3 = G4UIcommand::ConvertToInt(token);
				token = next();
				fSeedPart4 = G4UIcommand::ConvertToInt(token);
			}

			fNtuple->Fill();
		}
	}

	fIsEmptyHistory = true;
}


// called for each worker at the end of a run
void TsScorePhaseSpace::AbsorbResultsFromWorkerScorer(TsVScorer* workerScorer)
{
	TsVNtupleScorer::AbsorbResultsFromWorkerScorer(workerScorer);

	// Absorb additional statistics
	TsScorePhaseSpace* workerPhaseSpaceScorer = dynamic_cast<TsScorePhaseSpace*>(workerScorer);

	fNumberOfHistoriesThatMadeItToPhaseSpace += workerPhaseSpaceScorer->fNumberOfHistoriesThatMadeItToPhaseSpace;

	fNumberOfSequentialEmptyHistories += workerPhaseSpaceScorer->fNumberOfSequentialEmptyHistories;

	std::map<G4int,G4long>::iterator wIter1;
	std::map<G4int,G4long>::iterator mIter1;
	for ( wIter1 = workerPhaseSpaceScorer->fNumberOfParticles.begin(); wIter1 != workerPhaseSpaceScorer->fNumberOfParticles.end(); wIter1++) {
		mIter1 = fNumberOfParticles.find( wIter1->first);
		if (mIter1 == fNumberOfParticles.end())
			fNumberOfParticles.insert(std::pair<G4int, G4long>( wIter1->first, wIter1->second));
		else
			mIter1->second += wIter1->second;
	}

	std::map<G4int,G4double>::iterator wIter2;
	std::map<G4int,G4double>::iterator mIter2;
	for ( wIter2 = workerPhaseSpaceScorer->fMinimumKE.begin(); wIter2 != workerPhaseSpaceScorer->fMinimumKE.end(); wIter2++) {
		mIter2 = fMinimumKE.find( wIter2->first);
		if (mIter2 == fMinimumKE.end())
			fMinimumKE.insert(std::pair<G4int, G4double>( wIter2->first, wIter2->second));
		else if (wIter2->second < mIter2->second)
			mIter2->second = wIter2->second;
	}

	for ( wIter2 = workerPhaseSpaceScorer->fMaximumKE.begin(); wIter2 != workerPhaseSpaceScorer->fMaximumKE.end(); wIter2++) {
		mIter2 = fMaximumKE.find( wIter2->first);
		if (mIter2 == fMaximumKE.end())
			fMaximumKE.insert(std::pair<G4int, G4double>( wIter2->first, wIter2->second));
		else if (wIter2->second > mIter2->second)
			mIter2->second = wIter2->second;
	}

	// Clear additional statistics of worker scorer
	workerPhaseSpaceScorer->fNumberOfHistoriesThatMadeItToPhaseSpace = 0;
	workerPhaseSpaceScorer->fNumberOfSequentialEmptyHistories = 0;
	workerPhaseSpaceScorer->fNumberOfParticles.clear();
	workerPhaseSpaceScorer->fMinimumKE.clear();
	workerPhaseSpaceScorer->fMaximumKE.clear();
}


void TsScorePhaseSpace::UpdateForEndOfRun() {
	if (fIncludeEmptyHistoriesAtEndOfRun) {
		fPType          = 0;
		fPosX           = 0.;
		fPosY           = 0.;
		fPosZ           = 0.;
		fCosX           = 0.;
		fCosY           = 0.;
		fCosZIsNegative = false;
		fEnergy	        = 0.;
		fWeight	        = -1. * fNumberOfSequentialEmptyHistories;
		fRunID = GetRunID();
		fEventID = 0;

		fIsNewHistory = true;

		if (fIncludeTrackID)
			fTrackID = 0;

		if (fIncludeParentID)
			fParentID = 0;

		if (fIncludeCharge)
			fCharge = 0.;

		if (fIncludeCreatorProcess)
			fCreatorProcess = "emptyHistory";

		if (fIncludeVertexInfo) {
			fVertexKE       = 0.;
			fVertexPosX     = 0.;
			fVertexPosY     = 0.;
			fVertexPosZ     = 0.;
			fVertexCosX     = 0.;
			fVertexCosY     = 0.;
			fVertexCosZ     = 0.;
		}

		if (fIncludeTOPASTime)
			fTOPASTime = GetTime();

		if (fIncludeTimeOfFlight)
			fTimeOfFlight = 0.;

		if (fIncludeSeed) {
			G4Tokenizer next(GetRandomNumberStatusForThisEvent());
			fSeedPart1 = 0;
			fSeedPart2 = 0;
			fSeedPart3 = 0;
			fSeedPart4 = 0;
		}

		fNtuple->Fill();

		fNumberOfSequentialEmptyHistories = 0;
	}

	TsVScorer::UpdateForEndOfRun();
}

// called for master only at the end of a run
void TsScorePhaseSpace::Output()
{
	G4long totalNumberOfParticles = 0;
	std::map<G4int,G4long>::iterator itr;
	for (itr=fNumberOfParticles.begin();itr!=fNumberOfParticles.end();++itr) {
		totalNumberOfParticles += itr->second;
	}

	if (fIncludeEmptyHistoriesAtEndOfFile) {
		fPType          = 0;
		fPosX           = 0.;
		fPosY           = 0.;
		fPosZ           = 0.;
		fCosX           = 0.;
		fCosY           = 0.;
		fCosZIsNegative = false;
		fEnergy	        = 0.;
		fWeight	        = -1. * fNumberOfSequentialEmptyHistories;
		fRunID = GetRunID();
		fEventID = 0;

		fIsNewHistory = true;

		if (fIncludeTrackID)
			fTrackID = 0;

		if (fIncludeParentID)
			fParentID = 0;

		if (fIncludeCharge)
			fCharge = 0.;

		if (fIncludeCreatorProcess)
			fCreatorProcess = "emptyHistory";

		if (fIncludeVertexInfo) {
			fVertexKE       = 0.;
			fVertexPosX     = 0.;
			fVertexPosY     = 0.;
			fVertexPosZ     = 0.;
			fVertexCosX     = 0.;
			fVertexCosY     = 0.;
			fVertexCosZ     = 0.;
		}

		if (fIncludeTOPASTime)
			fTOPASTime = GetTime();

		if (fIncludeTimeOfFlight)
			fTimeOfFlight = 0.;

		if (fIncludeSeed) {
			G4Tokenizer next(GetRandomNumberStatusForThisEvent());
			fSeedPart1 = 0;
			fSeedPart2 = 0;
			fSeedPart3 = 0;
			fSeedPart4 = 0;
		}

		fNtuple->Fill();

		fNumberOfSequentialEmptyHistories = 0;
	}

	if (fOutputToLimited) {
		std::ostringstream header;

		header << "$TITLE:" << G4endl;
		header << "TOPAS Phase Space in \"limited\" format. " <<
		"Should only be used when it is necessary to read or write from restrictive older codes." << G4endl;

		header << "$RECORD_CONTENTS:" << G4endl;
		header << "    1     // X is stored ?" << G4endl;
		header << "    1     // Y is stored ?" << G4endl;
		header << "    1     // Z is stored ?" << G4endl;
		header << "    1     // U is stored ?" << G4endl;
		header << "    1     // V is stored ?" << G4endl;
		header << "    1     // W is stored ?" << G4endl;
		header << "    1     // Weight is stored ?" << G4endl;
		header << "    0     // Extra floats stored ?" << G4endl;
		header << "    0     // Extra longs stored ?" << G4endl;

		header << "$RECORD_LENGTH:" << G4endl;
		G4int recordLength = 7*sizeof(G4float) + 1;
		header << recordLength << G4endl;

		header << "$ORIG_HISTORIES:" << G4endl;
		header << GetScoredHistories() << G4endl;

		header << "$PARTICLES:" << G4endl;
		header << totalNumberOfParticles << G4endl;

		header << "$EXTRA_FLOATS:" << G4endl;
		header << "0" << G4endl;

		header << "$EXTRA_INTS:" << G4endl;
		header << "0" << G4endl;

		fNtuple->fHeaderPrefix = header.str();
		fNtuple->SuppressColumnDescription(true);
		fNtuple->Write();
	}

	// Collect prefix statistics
	std::ostringstream prefix;
	prefix << "Number of Original Histories: " << GetScoredHistories() << G4endl;
	prefix << "Number of Original Histories that Reached Phase Space: " << fNumberOfHistoriesThatMadeItToPhaseSpace << G4endl;
	prefix << "Number of Scored Particles: " << totalNumberOfParticles << G4endl;

	// Collect suffix statistics
	std::ostringstream suffix;
	for (itr=fNumberOfParticles.begin();itr!=fNumberOfParticles.end();++itr) {
		if (itr->first)
			suffix << "Number of " << G4ParticleTable::GetParticleTable()->FindParticle(itr->first)->GetParticleName() << ": " << itr->second << G4endl;
		else
			suffix << "Number of particles with PDG code zero: " << itr->second << G4endl;
	}
	suffix << std::endl;
	std::map<G4int,G4double>::iterator itr_d;
	for (itr_d=fMinimumKE.begin();itr_d!=fMinimumKE.end();++itr_d) {
		if (itr_d->first)
			suffix << "Minimum Kinetic Energy of " << G4ParticleTable::GetParticleTable()->FindParticle(itr_d->first)->GetParticleName() << ": " << itr_d->second/MeV << " MeV" << G4endl;
		else
			suffix << "Minimum Kinetic Energy of particles with PDG code zero: " << itr_d->second/MeV << " MeV" << G4endl;
	}
	suffix << std::endl;
	for (itr_d=fMaximumKE.begin();itr_d!=fMaximumKE.end();++itr_d) {
		if (itr_d->first)
			suffix << "Maximum Kinetic Energy of " << G4ParticleTable::GetParticleTable()->FindParticle(itr_d->first)->GetParticleName() << ": " << itr_d->second/MeV << " MeV" << G4endl;
		else
			suffix << "Maximum Kinetic Energy of particles with PDG code zero: " << itr_d->second/MeV << " MeV" << G4endl;
	}

	if (!fOutputToLimited) {
		std::ostringstream title;
		if (fOutFileType == "ascii")
			title << "TOPAS ASCII Phase Space" << G4endl << G4endl;
		else if (fOutFileType == "binary")
			title << "TOPAS Binary Phase Space" << G4endl << G4endl;

		fNtuple->fHeaderPrefix = title.str() + prefix.str();
		fNtuple->fHeaderSuffix = suffix.str();
		fNtuple->Write();
	}

	// report additional statistics
	G4cout << G4endl;
	G4cout << "Scorer: " << GetNameWithSplitId() << G4endl;
	if (fNtuple->HasHeaderFile())
		G4cout << "Header   has been written to file: " << fNtuple->GetHeaderFileName() << G4endl;
	G4cout << "Contents has been written to file: " << fNtuple->GetDataFileName() << G4endl;
	G4cout << "Scored on surface: " << fComponent->GetName() << "/" << GetSurfaceName() << G4endl;
	G4cout << G4endl;
	G4cout << prefix.str() << G4endl;
	G4cout << suffix.str() << G4endl;

	if (fHitsWithNoIncidentParticle > 0) {
		G4cout << "Warning, one of your filters was based on the energy or momentum of the parent particle that was incident on the scoring component," << G4endl;
		G4cout << "but at least one hit resulted from a primary that was already inside the scoring component when it was generated." << G4endl;
		G4cout << "Such hits are always left out by this incident particle filter. Total number of such hits:" << fHitsWithNoIncidentParticle << G4endl;
	}

	UpdateFileNameForUpcomingRun();
}


void TsScorePhaseSpace::Clear()
{
	fScoredHistories = 0;
	fNumberOfHistoriesThatMadeItToPhaseSpace = 0;
	fNumberOfSequentialEmptyHistories = 0;
	fNumberOfParticles.clear();
	fMinimumKE.clear();
	fMaximumKE.clear();
}
