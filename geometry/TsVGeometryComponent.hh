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

#ifndef TsVGeometryComponent_hh
#define TsVGeometryComponent_hh

#include "G4RotationMatrix.hh"
#include "G4Point3D.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4VisExtent.hh"

class TsParameterManager;
class TsExtensionManager;
class TsMaterialManager;
class TsGeometryManager;

class TsVMagneticField;
class TsVElectroMagneticField;

class G4Step;
class G4VisAttributes;
class G4UserLimits;
class G4LogicalVolume;
class G4VTouchable;
class G4LogicalSkinSurface;
class G4Material;
class G4BoundingExtentScene;

class TsVGeometryComponent
{
public:
	TsVGeometryComponent(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
						 TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name);
	virtual ~TsVGeometryComponent();

	// Construct the geometry (called by GeometryManager immediately after this component is instantiated)
	virtual G4VPhysicalVolume* Construct() = 0;

	// Must be called as first part of construction
	void BeginConstruction();

	// If there is a parameter other than position or color that can be updated without having to entirely
	// destroy and recreate the comopnent, handle that parameter here.
	// Examples is leaf position for TsMultiLeafCollimator.
	virtual void UpdateForSpecificParameterChange(G4String parameter);
	virtual void UpdateForSpecificMagneticFieldParameterChange(G4String parameter);
	virtual void UpdateForSpecificElectroMagneticFieldParameterChange(G4String parameter);

	// Gets the material name, or, for cases of material=Parent or World, gets that material name
	G4String GetResolvedMaterialName();
	G4String GetResolvedMaterialName(G4String& subComponentName, G4bool allowUndefined = false);

	// Instantiate and constuct all children of the current component
	void InstantiateChildren();

	// Given a partial parameter name, such as "TransX",
	// return the full parameter name for this component, such as "Ge/World/TransX"
	G4String GetFullParmName(const char* parmName);

	// Get cubic volume of the envelope volume
	// or, if a Group, returns sum of child component envelope volumes
	G4double GetCubicVolume();

	// Get a material
	G4Material* GetMaterial(G4String name);
	G4Material* GetMaterial(const char* name);

	// Get extent. fExent is "cached" and must be "zeroed" if component changes.
	// E.g., fExtent = G4VisExtent::GetNullExtent() in UpdateForNewRun.
	const G4VisExtent& GetExtent();

protected:
	// Same as GetFullParmName, but allows for a name other than the component name.
	// Use if want a subcomponent such as "Ge/ModWheel/Track1/Color"
	// when the component name was just "ModWheel"
	G4String GetFullParmName(G4String& subComponentName, const char* parmName);
	G4String GetFullParmName(const char* subComponentName, const char* parmName);

	// Create a Logial Volume for the Component
	G4LogicalVolume* CreateLogicalVolume(G4VSolid* solid);

	// Create a Logial Volume for a subComponent
	G4LogicalVolume* CreateLogicalVolume(const char* subComponentName, G4VSolid* solid);
	G4LogicalVolume* CreateLogicalVolume(G4String& subComponentName, G4VSolid* solid);

	// Create a Logial Volume for a subComponent using specified material
	G4LogicalVolume* CreateLogicalVolume(const char* subComponentName, G4String& materialName, G4VSolid* solid);
	G4LogicalVolume* CreateLogicalVolume(G4String& subComponentName, G4String& materialName, G4VSolid* solid);

	// Create a Physical Volume for the Component
	G4VPhysicalVolume* CreatePhysicalVolume(G4LogicalVolume* lVol);

	// Create a Physical Volume for a subComponent, centered in the given parent volume
	G4VPhysicalVolume* CreatePhysicalVolume(const char* subComponentName, G4LogicalVolume* lVol, G4VPhysicalVolume* parent);
	G4VPhysicalVolume* CreatePhysicalVolume(G4String& subComponentName, G4LogicalVolume* lVol, G4VPhysicalVolume* parent);

	// Create a Physical Volume for a subComponent, offset or rotated relative to the given parent volume
	G4VPhysicalVolume* CreatePhysicalVolume(const char* subComponentName, G4LogicalVolume* lVol,
											G4RotationMatrix* rot, G4ThreeVector* trans, G4VPhysicalVolume* parent);
	G4VPhysicalVolume* CreatePhysicalVolume(G4String& subComponentName, G4LogicalVolume* lVol,
											G4RotationMatrix* rot, G4ThreeVector* trans, G4VPhysicalVolume* parent);

	// Create a Physical Volume for one of several versions of a subComponent, offset or rotated relative to the given parent volume
	// Use this method when a different logical volume object has been created for each version of the physical volume
	G4VPhysicalVolume* CreatePhysicalVolume(const char* subComponentName, G4int copy, G4bool reuseLogical, G4LogicalVolume* lVol,
											G4RotationMatrix* rot, G4ThreeVector* trans, G4VPhysicalVolume* parent);
	G4VPhysicalVolume* CreatePhysicalVolume(G4String& subComponentName, G4int copy, G4bool reuseLogical, G4LogicalVolume* lVol,
											G4RotationMatrix* rot, G4ThreeVector* trans, G4VPhysicalVolume* parent);
	G4VPhysicalVolume* CreatePhysicalVolume(const char* subComponentName, G4int copy, G4bool reuseLogical, G4LogicalVolume* lVol,
											G4Transform3D transform, G4VPhysicalVolume* parent);
	G4VPhysicalVolume* CreatePhysicalVolume(G4String& subComponentName, G4int copy, G4bool reuseLogical, G4LogicalVolume* lVol,
											G4Transform3D transform, G4VPhysicalVolume* parent);

	G4VPhysicalVolume* CreatePhysicalVolume(const char* subComponentName, G4int copy, G4bool reuseLogical, G4LogicalVolume* lVol,
											G4RotationMatrix* rot, G4ThreeVector* trans, G4LogicalVolume* parent);
	G4VPhysicalVolume* CreatePhysicalVolume(G4String& subComponentName, G4int copy, G4bool reuseLogical, G4LogicalVolume* lVol,
											G4RotationMatrix* rot, G4ThreeVector* trans, G4LogicalVolume* parent);

	// Create a Physical Volume for a subComponent that uses parameterized placement
	G4VPhysicalVolume* CreatePhysicalVolume(const char* subComponentName, G4LogicalVolume* lVol, G4VPhysicalVolume* parent,
											const EAxis pAxis, const G4int nReplicas, G4VPVParameterisation* pParam);
	G4VPhysicalVolume* CreatePhysicalVolume(G4String& subComponentName, G4LogicalVolume* lVol, G4VPhysicalVolume* parent,
											const EAxis pAxis, const G4int nReplicas, G4VPVParameterisation* pParam);

	// Create a Physical Volume for a subComponent that uses replica placement
	G4VPhysicalVolume *CreatePhysicalVolume(const char *subComponentName, G4LogicalVolume *lVol, G4VPhysicalVolume *parent,
											const EAxis pAxis, const G4int nReplicas, G4double width, G4double offset = 0);

	// Explicitely set a vector of logical volumes to be used for scoring for sub-components
	void SetLogicalVolumeToBeSensitive(G4LogicalVolume*);

	// Register any G4RotationMatrix that you create so that its lifetime can be managed
	void RegisterRotation(G4RotationMatrix* rot);

	// Register any Translation that you create so that its lifetime can be managed
	void RegisterTranslation(G4ThreeVector* translation);

	// Register any G4Transform3D that you create so that its lifetime can be managed
	void RegisterTransform(G4Transform3D* transform);

	// Quit with error message about parameter being out of range
	void OutOfRange(G4String parameterName, G4String requirement);

	// Most components will never need to directly call this method.
	// It is only to be used in the rare case that the component directly calls AddParameter
	// for a parameter that may affect placement (such as when the compensator resets its own thickness)
	void CalculatePlacement();

	// Call to indicate that OpenGL graphics that this component is so complex that OGLI (immediate mode)
	// should be used rather than the default OGLS (stored mode)
	void SetTooComplexForOGLS();

	// Call for any logical volume that you move in your component's UpdateForSpecificParameterChange method
	void AddToReoptimizeList(G4LogicalVolume*);

	// Get the default color of a material
	G4String GetDefaultMaterialColor(G4String name);
	G4String GetDefaultMaterialColor(const char* name);

	// Pointer to parameter manager
	TsParameterManager* fPm;

	// User classes should not access any methods or data beyond this point

public:
	G4RotationMatrix* GetRotRelToWorld();
	G4Point3D* GetTransRelToWorld();

	G4VPhysicalVolume* GetParentVolume();
	G4VPhysicalVolume* GetPhysicalVolume(G4String& name);
	std::vector<G4VPhysicalVolume*> GetAllPhysicalVolumes(G4bool recursivelyIncludeSubcomponents = false);
	void RecursivelyAddVolumes(std::vector<G4VPhysicalVolume*>* volumes);

	std::vector<G4LogicalVolume*> GetLogicalVolumesToBeSensitive();

	G4String GetName();
	G4String GetNameWithCopyId();

	TsVGeometryComponent* GetOriginalComponent();

	G4bool SimplestCreateWasCalled();

	// Deprecated form. Instead use the version that takes no arguments
	void InstantiateChildren(G4VPhysicalVolume* pvol);

	void InstantiateFields();

	G4String GetFullParmNameLower(const char* parmName);

	void SetMaterialList(std::vector<G4Material*>* materialList);
	void SetMaterialIndex(std::vector<unsigned short>* materialIndex);

	enum SurfaceType {
		None,
		XPlusSurface,
		XMinusSurface,
		YPlusSurface,
		YMinusSurface,
		ZPlusSurface,
		ZMinusSurface,
		InnerCurvedSurface,
		OuterCurvedSurface,
		PhiPlusSurface,
		PhiMinusSurface,
		ThetaPlusSurface,
		ThetaMinusSurface,
		AnySurface
	};

	virtual G4LogicalVolume* GetScoringVolume();
	virtual G4int GetIndex(G4Step* aStep);
	virtual G4int GetIndex(G4int i, G4int j, G4int k);
	virtual G4int GetBin(G4int index, G4int iBin);
	virtual SurfaceType GetSurfaceID(G4String surfaceName);
	virtual G4bool IsOnBoundary(G4ThreeVector localpos, G4VSolid* solid, SurfaceType surfaceID);
	virtual G4double GetAreaOfSelectedSurface(G4VSolid* solid, SurfaceType surfaceID, G4int i, G4int j, G4int k);
	G4double GetFullWidth(G4int index);
	G4int GetDivisionCount(G4int index);
	G4String GetDivisionName(G4int index);
	G4String GetBinHeader(G4int index, G4int nBins);

	G4int GetNumberOfMaterials() const;
	G4Material* GetMaterialInVoxel(G4int idx) const;
	virtual G4Material* ComputeMaterial(const G4int repNo, G4VPhysicalVolume* pvol, const G4VTouchable* parent);
	virtual void ComputeTransformation(const G4int copyNo, G4VPhysicalVolume* pvol) const;
	virtual void ComputeDimensions(G4Tubs& tubs, const G4int copyNo, const G4VPhysicalVolume*) const;
	virtual void ComputeDimensions(G4Sphere& sphere, const G4int copyNo, const G4VPhysicalVolume*) const;

	void SetParallelScoringCopyDivisions(G4int* nDivisions);

	G4VisAttributes* GetVisAttributes(G4String subComponentName);

	G4int GetStructureID(G4String structureName);
	G4bool IsInNamedStructure(G4int structureID, const G4Step* aStep);
	G4bool IsInNamedStructure(G4int structureID, G4int index);

	G4String GetWorldName();
	G4VPhysicalVolume* GetEnvelopePhysicalVolume();
	G4LogicalVolume* GetEnvelopeLogicalVolume();

	G4bool IsCopy();
	TsVGeometryComponent* OriginalComponent();
	void SetOriginalComponent(TsVGeometryComponent* originalComponent);

	G4bool HasDifferentVolumePerDivision();
	G4bool IsParallel();
	G4bool HasChildrenUsedByScorers();
	G4bool IsUsedByScorer();
	void SetUsedByScorer();
	G4bool HasPropagatingScorer();
	void SetHasPropagatingScorer();
	void HasCopy();
	void MarkAsHavingParallelWorldDescendent();
	G4bool HasParallelWorldDescendents();
	void SetVisAttribute(G4VisAttributes* visAtt);

	void MarkAsNeedToUpdatePlacement();
	G4bool RebuildIfNeeded();
	virtual void UpdateForNewRun(G4bool force);
	void MarkAllVolumesForReoptimize();

	TsVGeometryComponent* InstantiateChild(G4String childName);
	void InstantiateAndConstructChild(G4String childName);

	void CheckForOverlaps(G4VPhysicalVolume* pvol);

	G4bool CanCalculateSurfaceArea();

	G4bool IsParameterized();

protected:
	void AddOpticalSurfaces();

	G4RotationMatrix* GetRotForPlacement();
	G4ThreeVector* GetTransForPlacement();

	void RegisterVisAtt(G4VisAttributes* attribute);

	void DeleteContents();

	G4String GetBinParmName(G4int i);

	void SetHasDividedCylinderOrSphere();

	std::vector<G4double> Logspace(G4double startValue, G4double endValue, G4int num, G4double base = 10);

	enum RadialBinning
	{
		Equal,
		Log,
		Custom
	};

	TsExtensionManager* fEm;

	TsVGeometryComponent* fParentComponent;
	G4VPhysicalVolume* fParentVolume;
	G4VPhysicalVolume* fEnvelopePhys;
	G4LogicalVolume* fEnvelopeLog;
	G4String fName;

	G4bool fIsCopy;
	TsVGeometryComponent* fOriginalComponent;

	G4String fCopyId;
	G4bool fIsGroup;
	G4bool fIsDividable;
	G4bool fCanCalculateSurfaceArea;
	G4bool fHasDifferentVolumePerDivision;
	G4LogicalVolume* fScoringVolume;

	G4String fDivisionNames[3];
	G4String fDivisionUnits[3];
	G4int fDivisionCounts[3];
	G4double fFullWidths[3];

	G4int fMaximumNumberOfDetailedErrorReports;
	G4int fNumberOfDetailedErrorReports;

	G4bool fHasVariableMaterial;
	std::vector<G4Material*>* fMaterialList;
	std::vector<unsigned short>* fCurrentMaterialIndex;
	G4int fImageIndex;

	std::vector<G4String> fStructureNames;
	std::vector<G4String> fMaterialByRTStructMaterialNames;
	std::vector<G4Material*> fMaterialByRTStructMaterials;
	std::vector<G4int> fMaterialByRTStructNamesIndexIntoIsInNamedStructure;
	std::vector<G4VisAttributes*> fStructureColors;
	std::vector< std::vector< std::vector<G4bool> > > fIsInNamedStructure;

	G4int fVerbosity;

private:
	G4String GetCopyIdFromBinning(G4int* nBins);

	G4RotationMatrix* GetRotRelToParentComponent();
	G4ThreeVector* GetTransRelToParentComponent();

	void AddVolumesToScene(G4BoundingExtentScene& scene);

	void Quit(const G4String& parameterName, const char* message);

	TsMaterialManager* fMm;
	TsGeometryManager* fGm;

	G4bool fIsUsedByScorer;
	G4bool fHasPropagatingScorer;
	G4bool fHasCopy;

	std::vector<G4RotationMatrix*> fRotations;
	std::vector<G4ThreeVector*> fTranslations;
	std::vector<G4Transform3D*> fTransforms;

	G4bool fIsParallel;
	G4bool fHasParallelWorldDescendents;
	G4bool fNeedToRebuild;
	G4bool fNeedToUpdatePlacement;
	G4bool fSimplestCreateWasCalled;
	G4String fWorldName;
	G4VisExtent fExtent;

	G4RotationMatrix* fRotRelToWorld;
	G4Point3D* fTransRelToWorld;
	G4RotationMatrix* fRotRelToLastNonGroupComponent;
	G4Point3D* fTransRelToLastNonGroupComponent;
	G4RotationMatrix* fRotRelToParentComponent;
	G4ThreeVector* fTransRelToParentComponent;

	std::vector<G4VSolid*> fSolids;
	std::vector<G4VisAttributes*> fVisAtts;
	std::vector<G4UserLimits*> fUserLimits;
	std::vector<G4LogicalVolume*> fLogicalVolumes;
	std::vector<G4VPhysicalVolume*> fPhysicalVolumes;
	std::vector<G4LogicalVolume*> fLogicalVolumesToBeSensitive;
	std::vector<G4VPVParameterisation*> fParameterizations;
	G4LogicalSkinSurface* fLogicalSkinSurface;

	std::vector<TsVGeometryComponent*> fChildren;
	std::vector<TsVMagneticField*> fMagneticFields;
	std::vector<TsVElectroMagneticField*> fElectroMagneticFields;
};

#endif
