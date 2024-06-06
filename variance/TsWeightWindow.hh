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

#ifndef TsWeightWindow_hh
#define TsWeightWindow_hh

#include "G4GeometryCell.hh"
#include "G4VPhysicalVolume.hh"
#include "G4GeometrySampler.hh"

#include "TsVBiasingProcess.hh"

#include <vector>

class TsParameterManager;
class TsGeometryManager;

class TsWeightWindow : public TsVBiasingProcess
{
public:
	TsWeightWindow(G4String name, TsParameterManager*, TsGeometryManager*);
	~TsWeightWindow();

	void ResolveParameters();
	void SetGeometrySampler();
	void Clear();
	void SetGeometry();
	void Initialize();
	void SetWeightWindow();
	void AddBiasingProcess();
	
	G4String GetTypeName() {return fType;};
	G4String GetName() {return fName;};
	
private:
	TsGeometryManager*	fGm;
	TsParameterManager* fPm;

	G4String fName;
	
	std::vector<G4GeometrySampler*> fGeometrySampler;
	
	G4String* fParticleName;
	G4int fParticleNameLength;
	std::vector<G4VPhysicalVolume*> fPhysVol;
	G4String fType;
	G4String fBiasingWorld;
	G4String fComponentName;
	G4String* fSubComponentNames;
	G4int fSubCompSize;
	G4bool fIsParallel;
	
	G4bool fGeometryAlreadyBuilt;
	G4bool fClearSampling;
	
	G4int fVerbosity;
};
#endif
