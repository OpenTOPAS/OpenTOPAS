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

#ifndef TsGeometryManager_hh
#define TsGeometryManager_hh

#include <map>
#include <list>
#include <set>

#include "G4VUserDetectorConstruction.hh"

class TsParameterManager;
class TsExtensionManager;
class TsMaterialManager;
class TsScoringManager;
class TsSequenceManager;

class TsVGeometryComponent;
class TsVMagneticField;
class TsVElectroMagneticField;
class TsBorderSurface;
class TsGeometryHub;
class TsVarianceManager;

class G4VPhysicalVolume;
class G4LogicalVolume;
class G4Material;
class G4OpticalSurface;

class TsGeometryManager : public G4VUserDetectorConstruction
{
public:
	TsGeometryManager(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mm);
	~TsGeometryManager();

	TsGeometryHub* GetGeometryHub();

	G4VPhysicalVolume* GetPhysicalVolume(G4String volumeName);

	void AddToReoptimizeList(G4LogicalVolume*);

	G4VPhysicalVolume* Construct();
	void ConstructSDandField();
	void Initialize();
	void SetScoringManager(TsScoringManager* scM);

	void SetCurrentComponent(TsVGeometryComponent* currentComponent);
	void SetCurrentMagneticField(TsVMagneticField* currentMagneticField);
	void SetCurrentElectroMagneticField(TsVElectroMagneticField* currentElectroMagneticField);
	void NoteAnyUseOfChangeableParameters(const G4String& name);
	void UpdateForSpecificParameterChange(G4String parameter);
	void UpdateWorldForNewRun();
	G4bool UpdateForNewRun(TsSequenceManager* sqm, G4bool forceUpdatePlacement);

	void RemoveFromReoptimizeList(G4LogicalVolume*);

	G4VPhysicalVolume* CreateParallelWorld(G4String parallelWorldName, G4bool componentIsGroup);

	G4OpticalSurface* GetOpticalSurface(G4String opticalSurfaceName);
	void RegisterBorderSurface(G4String borderSurfaceName);
	void CheckForEffectOnBorderSurfaceDestinations(G4String toComponentName);

	TsVGeometryComponent* GetComponent(G4String& nameWithCopyId);
	std::vector<G4String> GetComponentNames();
	G4String GetCopyIdFromBinning(G4int* nBins);
	G4String GetCopyIdFromBinning(G4int i, G4int j, G4int k);

	void UnRegisterComponent(G4String& name);

	std::vector<G4String> GetChildComponentsOf(G4String& parentName);

	void SetHaveParallelComponentsThatAreNotGroups();
	G4bool HaveParallelComponentsThatAreNotGroups();

	void SetParallelWorldHasMaterial(G4String worldName);
	std::set<G4String>* GetParallelWorldsWithMaterial();

	void RegisterToIgnoreInUnusedComponentCheck(G4String name);
    
    void SetGeometricalTolerance(G4double newTolerance);

	void SetTooComplexForOGLS();
	G4bool IsTooComplexForOGLS();

	void SetHasDividedCylinderOrSphere();
	G4bool HasDividedCylinderOrSphere();

	G4bool GeometryHasOverlaps();
	
	void SetVarianceManager(TsVarianceManager* vM);

private:
	void CheckForUnusedComponents();
	G4bool IgnoreInUnusedComponentCheck(G4String);

	G4String GetFullSurfaceParmName(G4String surfaceName, const char* parmName);
	void Quit(const G4String& name, const char* message);

	TsParameterManager* fPm;
	TsExtensionManager* fEm;
	TsMaterialManager* fMm;
	TsScoringManager* fScm;
	TsVarianceManager* fVm;

	G4int fVerbosity;

	TsGeometryHub* fGeometryHub;
	TsVGeometryComponent* fWorldComponent;
	G4VPhysicalVolume* fWorldVolume;

	G4bool fHaveParallelComponentsThatAreNotGroups;
	std::set<G4String>* fParallelWorldsWithMaterial;
	std::set<G4String>* fNamesToIgnoreInUnusedComponentCheck;

	std::map<G4String, TsBorderSurface*>* fBorderSurfaces;
	std::map<G4String, G4OpticalSurface*>* fOpticalSurfaces;

	G4bool fTooComplexForOGLS;
	G4bool fHasDividedCylinderOrSphere;

	G4bool fAlreadyConstructed;
	std::list<G4LogicalVolume*> fReoptimizeVolumes;

	std::map<G4String,TsVGeometryComponent*>* fStore;
	TsVGeometryComponent* fCurrentComponent;
	std::multimap< G4String, std::pair<G4String,G4String> > fChangeableParameterMap;
	TsVMagneticField* fCurrentMagneticField;
	std::multimap< G4String, std::pair<G4String,G4String> > fChangeableMagneticFieldParameterMap;
	TsVElectroMagneticField* fCurrentElectroMagneticField;
	std::multimap< G4String, std::pair<G4String,G4String> > fChangeableElectroMagneticFieldParameterMap;
};

#endif
