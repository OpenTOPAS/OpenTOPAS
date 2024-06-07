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

#ifndef TsSourcePhaseSpaceOld_hh
#define TsSourcePhaseSpaceOld_hh

#include "TsSource.hh"

#include "TsPrimaryParticle.hh"

#include <queue>

class TsSourcePhaseSpaceOld : public TsSource
{
public:
	TsSourcePhaseSpaceOld(TsParameterManager* pM, TsSourceManager* psM, G4String sourceName);
	~TsSourcePhaseSpaceOld();

	void ResolveParameters();

	G4long GetNumberOfHistoriesToReadFromFile();

	// Returns True if there is still more data in file
	G4bool ReadSomeDataFromFileToBuffer(std::queue<TsPrimaryParticle>* particleBuffer);

private:
	G4String fFileName;
	G4bool fIsBinary;
	G4bool fIsLimited;
	G4bool fLimitedHasZ;
	G4bool fLimitedHasWeight;
	G4bool fLimitedAssumePhotonIsNewHistory;
	G4int fRecordLength;
	G4long fNumberOfOriginalHistories;
	G4long fNumberOfHistoriesInFile;
	G4long fNumberOfParticlesInFile;
	G4long fPreCheckNumberOfParticlesInFile;
	G4long fPreCheckNumberOfHistoriesInFile;
	G4int fMultipleUse;
	G4long fNumberOfHistoriesToReadFromFile;
	G4long fRemainingNumberOfParticlesInFile;
	G4long fPhaseSpaceBufferSize;
	G4double fPhaseSpaceScaleXPosBy;
	G4double fPhaseSpaceScaleYPosBy;
	G4double fPhaseSpaceScaleZPosBy;
	G4bool fPhaseSpaceInvertXAxis;
	G4bool fPhaseSpaceInvertYAxis;
	G4bool fPhaseSpaceInvertZAxis;
	G4bool fIgnoreUnsupportedParticles;
	G4bool fIncludeEmptyHistories;
	G4bool fPreCheck;

	std::streampos fFilePosition;
};

#endif
