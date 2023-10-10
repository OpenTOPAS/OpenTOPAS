//
// ********************************************************************
// *                                                                  *
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

#ifndef TsModularPhysicsList_hh
#define TsModularPhysicsList_hh

#include "G4VModularPhysicsList.hh"

#include "TsTopasConfig.hh"

#include <vector>

class TsExtensionManager;
class TsGeometryManager;
class TsParameterManager;
class TsVarianceManager;
class TsGeometrySampler;

class G4GeometrySampler;

// Utility class for wrapping
class VPhysicsCreator
{
public:
	virtual G4VPhysicsConstructor* operator()() = 0;
	virtual ~VPhysicsCreator() = default;

	G4String GetPhysicsName()
	{
        const G4VPhysicsConstructor* phys = this->operator()();
		G4String name = phys->GetPhysicsName();
		delete phys;
		return name;
	}
};

template <class T> class Creator;
template <class T> class CreatorWithPm;

class TsModularPhysicsList : public G4VModularPhysicsList
{
public:
	TsModularPhysicsList(TsParameterManager* pM, TsExtensionManager* eM, TsGeometryManager* gM, TsVarianceManager* vM, G4String name);
	~TsModularPhysicsList() override = default;

	void AddModule(const G4String& name);

	void AddBiasing(std::vector<G4GeometrySampler*> gs) { fGeomSamplers = gs; }
	void AddBiasing(std::vector<TsGeometrySampler*> pgs) { fProtonGeomSamplers = pgs; }

private:
	void ConstructProcess() override;
	void ConstructParticle() override;
	void ActiveG4DNAPerRegion(G4String moduleName);
	void ActiveG4EmModelPerRegion(G4String moduleName);
	void AddTransportationAndParallelScoring();
	void SetUpParallelWorldProcess(G4String worldName, G4bool useLMG);
	void SetCuts() override;
	G4String GetFullParmName(const char* parmName);

	TsParameterManager* fPm;
	TsExtensionManager* fEm;
	TsGeometryManager* fGm;
	TsVarianceManager* fVm;

	G4String fName;
	G4int fNumberBuilt;
	G4bool fTransportationOnly;

	std::vector<G4GeometrySampler*> fGeomSamplers;
	std::vector<TsGeometrySampler*> fProtonGeomSamplers;

	std::map<G4String, VPhysicsCreator*> fPhysicsTable;

protected:
	void AddBiasingProcess();

	void SetEmParameters();

	std::map<G4String, VPhysicsCreator*>::const_iterator LocatePhysicsModel(G4String model, G4bool allow_extensions = true);

private:
	G4bool IsPhysicsRegistered(const std::vector<G4VPhysicsConstructor*>* const, G4VPhysicsConstructor*) const;
};

// Inline methods and templates
#include "TsModularPhysicsList.icc"

#endif
