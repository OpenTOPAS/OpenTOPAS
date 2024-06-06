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

#ifndef TsPhysicsManager_hh
#define TsPhysicsManager_hh

#include "globals.hh"
#include <vector>

class TsParameterManager;
class TsExtensionManager;
class TsGeometryManager;
class TsGeometrySampler;
class TsVarianceManager;

class G4VUserPhysicsList;
class G4GeometrySampler;

class TsPhysicsManager
{
public:
	TsPhysicsManager(TsParameterManager* pM, TsExtensionManager* eM, TsGeometryManager* gM);
	~TsPhysicsManager();

	G4VUserPhysicsList* GetPhysicsList();
	void AddBiasing(std::vector<G4GeometrySampler*> mgs) {fGeomSamplers = mgs;};
	void AddBiasing(std::vector<TsGeometrySampler*> pmgs) {fProtonGeomSamplers = pmgs;};
	void SetVarianceManager(TsVarianceManager* vM);
	
private:
	TsParameterManager* fPm;
	TsExtensionManager* fEm;
	TsGeometryManager*	fGm;
	TsVarianceManager*  fVm;

	G4String fPhysicsListName;

	void SetEmParameters();

	G4String GetFullParmName(const char* parmName);

	std::vector<G4GeometrySampler*> fGeomSamplers;
	std::vector<TsGeometrySampler*> fProtonGeomSamplers;
};
#endif

