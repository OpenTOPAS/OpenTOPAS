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

#ifndef TsSourcePhaseSpace_hh
#define TsSourcePhaseSpace_hh

#include "TsSource.hh"

#include "TsPrimaryParticle.hh"

#include <queue>
#include <fstream>

class TsSourcePhaseSpace : public TsSource
{
public:
	TsSourcePhaseSpace(TsParameterManager* pM, TsSourceManager* psM, G4String sourceName);
	~TsSourcePhaseSpace();

	void ResolveParameters();

	void ReadSomeDataFromFileToBuffer(std::queue<TsPrimaryParticle>* particleBuffer);

    G4bool ReadOneParticle(std::queue<TsPrimaryParticle>* particleBuffer);

    G4long GetFileSize(std::string filename);

private:
	G4String fFileName;
    G4int fRecordLength;
    G4long fFileSize;
    std::ifstream fDataFile;
    std::streampos fFilePosition;
    G4String fAsciiLine;
    G4bool fIgnoreUnsupportedParticles;
    G4bool fIncludeEmptyHistories;
	G4long fNumberOfEmptyHistoriesToAppend;
	G4long fNumberOfEmptyHistoriesAppended;
    G4bool fPreviousHistoryWasEmpty;
	G4int fMultipleUse;
	G4bool fIsBinary;
	G4bool fIsLimited;
	G4bool fLimitedHasZ;
	G4bool fLimitedHasWeight;
	G4bool fLimitedAssumePhotonIsNewHistory;
	G4bool fLimitedAssumeEveryParticleIsNewHistory;
	G4bool fLimitedAssumeFirstParticleIsNewHistory;
    G4bool fPreCheck;
    G4long fPreCheckNumberOfHistories;
    G4long fPreCheckNumberOfNonEmptyHistories;
    G4long fPreCheckNumberOfParticles;
	G4int fPreCheckShowParticleCountAtInterval;
    G4long fHeaderNumberOfHistories;
	G4long fHeaderNumberOfNonEmptyHistories;
	G4long fHeaderNumberOfParticles;
	G4double fPhaseSpaceScaleXPosBy;
	G4double fPhaseSpaceScaleYPosBy;
	G4double fPhaseSpaceScaleZPosBy;
	G4bool fPhaseSpaceInvertXAxis;
	G4bool fPhaseSpaceInvertYAxis;
	G4bool fPhaseSpaceInvertZAxis;
    TsPrimaryParticle fPrimaryParticle;
};

#endif
