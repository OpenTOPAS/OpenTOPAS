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

#ifndef TsBox_hh
#define TsBox_hh

#include "TsVGeometryComponent.hh"

class TsBox : public TsVGeometryComponent
{
public:
	TsBox(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
				  TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name);
	~TsBox();

	G4VPhysicalVolume* Construct();

	// Methods used by scorers
	G4int GetIndex(G4Step* aStep);
	G4int GetIndex(G4int iX, G4int iY, G4int iZ);
	G4int GetBin(G4int index, G4int iBin);
	SurfaceType GetSurfaceID(G4String surfaceName);
	G4bool IsOnBoundary(G4ThreeVector localpos, G4VSolid* solid, SurfaceType surfaceID);
	G4double GetAreaOfSelectedSurface(G4VSolid* solid, SurfaceType surfaceID, G4int iX, G4int iY, G4int iZ);

	// Methods used by parameterization
	G4Material* ComputeMaterial(const G4int repNo, G4VPhysicalVolume* pvol, const G4VTouchable* parent);
	void ComputeTransformation(const G4int, G4VPhysicalVolume* pvol) const;

	G4Point3D GetTransFirstVoxelCenterRelToComponentCenter() { return fTransFirstVoxelCenterRelToComponentCenter; }

	void SetShowSpecificSlicesX(G4bool*);
	void SetShowSpecificSlicesY(G4bool*);
	void SetShowSpecificSlicesZ(G4bool*);

	static void CreateDefaults(TsParameterManager* pM, G4String& childName, G4String& parentName);
protected:
	void ConstructVoxelStructure();

	G4bool fShowOnlyOutline;
	G4bool fConstructParameterized;
	G4Point3D fTransFirstVoxelCenterRelToComponentCenter;

private:
	G4bool* fShowSpecificSlicesX;
	G4bool* fShowSpecificSlicesY;
	G4bool* fShowSpecificSlicesZ;
};

#endif
