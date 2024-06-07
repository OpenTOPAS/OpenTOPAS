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

#ifndef TsGeometricalParticleSplit_hh
#define TsGeometricalParticleSplit_hh

#include "G4GeometryCell.hh"
#include "G4VPhysicalVolume.hh"
#include "TsGeometrySampler.hh"

#include "TsVBiasingProcess.hh"

#include <vector>

class TsParameterManager;
class TsGeometryManager;

class TsGeometricalParticleSplit : public TsVBiasingProcess
{
public:
	TsGeometricalParticleSplit(G4String name, TsParameterManager*, TsGeometryManager*);
	~TsGeometricalParticleSplit();

	void ResolveParameters();
	void SetBiasingProcess(std::vector<TsGeometrySampler*> tgs);
	void Clear();
	void SetGeometry();
	void Initialize();
	void SetGeometricalParticleSplit();
	void AddBiasingProcess();
	
	G4String GetName() {return fName;};
	G4String GetTypeName() {return fType;};
	
	std::vector<TsGeometrySampler*> GetGeometrySampler();
	
private:
	TsGeometryManager*	fGm;
	TsParameterManager* fPm;

	G4String fName;
	G4String* fParticleName;
	G4int fParticleNameLength;
	G4String fType;
	G4String fWorldBiasingName;
	G4String fComponentName;
	G4String* fSubComponentNames;
	G4int fSubCompSize;
	G4int fVerbosity;
	
	std::vector<G4VPhysicalVolume*> fPhysVol;
	std::vector<TsGeometrySampler*> fGeometrySampler;
};
#endif
