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

#ifndef TsScorePhaseSpace_hh
#define TsScorePhaseSpace_hh

#include "TsVNtupleScorer.hh"

#include <stdint.h>

class TsScorePhaseSpace : public TsVNtupleScorer
{
public:
	TsScorePhaseSpace(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
					  G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer);
	~TsScorePhaseSpace();

	virtual G4bool ProcessHits(G4Step*, G4TouchableHistory*);

	void AccumulateEvent();

	void AbsorbResultsFromWorkerScorer(TsVScorer* workerScorer);

	void UpdateForEndOfRun();

protected:
	void Output();
	void Clear();

	G4float fPosX;
	G4float fPosY;
	G4float fPosZ;
	G4float fCosX;
	G4float fCosY;
	G4bool fCosZIsNegative;
	G4float fEnergy;
	G4float fWeight;
	G4int fPType;
	G4int fNumberOfSequentialEmptyHistories;
	G4bool fIsEmptyHistory;
	G4bool fIsNewHistory;
	G4float fTOPASTime;
	G4float fTimeOfFlight;
	G4int fRunID;
	G4int fEventID;
	G4int fTrackID;
	G4int fParentID;
	G4float fCharge;
	G4String fCreatorProcess;
	G4float fVertexKE;
	G4float fVertexPosX;
	G4float fVertexPosY;
	G4float fVertexPosZ;
	G4float fVertexCosX;
	G4float fVertexCosY;
	G4float fVertexCosZ;
	G4int fSeedPart1;
	G4int fSeedPart2;
	G4int fSeedPart3;
	G4int fSeedPart4;
	G4float fSignedEnergy;
	int8_t fSignedPType;

	G4bool fIncludeEmptyHistoriesInSequence;
	G4bool fIncludeEmptyHistoriesAtEndOfRun;
	G4bool fIncludeEmptyHistoriesAtEndOfFile;
	G4bool fKillAfterPhaseSpace;
	G4bool fOutputToLimited;
	G4bool fIncludeTOPASTime;
	G4bool fIncludeTimeOfFlight;
	G4bool fIncludeTrackID;
	G4bool fIncludeParentID;
	G4bool fIncludeCharge;
	G4bool fIncludeCreatorProcess;
	G4bool fIncludeVertexInfo;
	G4bool fIncludeSeed;

	G4int fPrevRunID;
	G4int fPrevEventID;

	G4long fNumberOfHistoriesThatMadeItToPhaseSpace;
	std::map<G4int, G4long> fNumberOfParticles; // # of particles of each type
	std::map<G4int, G4double> fMinimumKE;  // min KE of each particle type
	std::map<G4int, G4double> fMaximumKE;  // max KE of each particle type
};

#endif
