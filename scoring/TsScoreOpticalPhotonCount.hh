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

#ifndef TsScoreOpticalPhotonCount_hh
#define TsScoreOpticalPhotonCount_hh

#include "TsVNtupleScorer.hh"
#include "G4OpBoundaryProcess.hh"

class TsScoreOpticalPhotonCount : public TsVNtupleScorer
{
public:
	TsScoreOpticalPhotonCount(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
				   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer);

	~TsScoreOpticalPhotonCount();

	virtual G4bool ProcessHits(G4Step*,G4TouchableHistory*);

protected:

	G4float fPosX;
	G4float fPosY;
	G4float fPosZ;
	G4float fWaveLength;
	G4float fTime;
	G4int   fOpticalProcessID;

private:
	G4OpBoundaryProcessStatus fExpectedNextStatus;
	G4int fBounceLimit;
	G4int fCounterBounce;
};
#endif
