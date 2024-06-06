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

#ifndef TsUniformSplitting_hh
#define TsUniformSplitting_hh

#include "TsVBiasingProcess.hh"

#include <vector>

class TsParameterManager;
class TsGeometryManager;

class TsUniformSplitting : public TsVBiasingProcess
{
public:
	TsUniformSplitting(G4String name, TsParameterManager*, TsGeometryManager*);
	~TsUniformSplitting();

	void ResolveParameters();
	void AddBiasingProcess() { return; };
	void Initialize();
	void Clear();
	void SetUniformSplitting();

private:
	TsParameterManager* fPm;
	
	G4String fName;
	G4String fType;
	G4bool fUseDirectionalSplitting;
	std::vector<G4String> fRegionName;
	std::vector<G4String> fProcesses;
	std::vector<G4double> fSplitNumber;
	std::vector<G4double> fLowerEnergyLimitForRussianRoulette;
	G4double fTransX;
	G4double fTransY;
	G4double fTransZ;
	G4double fRMax;
};
#endif
