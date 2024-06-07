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

#ifndef TsSourceDistributed_hh
#define TsSourceDistributed_hh

#include "TsSource.hh"

#include "G4Point3D.hh"
#include <vector>

class TsSourceDistributed : public TsSource
{
public:
	TsSourceDistributed(TsParameterManager* pM, TsSourceManager* psM, G4String sourceName);
	~TsSourceDistributed();

	void UpdateForNewRun(G4bool rebuiltSomeComponents);

	void ResolveParameters();

	void PrepareSampledPoints();
	
	std::vector<G4Point3D*> fSampledPoints;

private:
	G4int fNumberOfSourcePoints;
	G4int fPreviousNumberOfSourcePoints;
	G4bool fRedistributePointsOnNewRun;
	
	enum DistributionType { FLAT, GAUSSIAN };
	DistributionType fPointDistribution;

	G4double fPointDistributionSigma;
};

#endif
