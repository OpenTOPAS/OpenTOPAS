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

#ifndef TsCylinder_hh
#define TsCylinder_hh

#include "TsVGeometryComponent.hh"

class TsCylinder : public TsVGeometryComponent
{
public:
	TsCylinder(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
				  TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name);
	~TsCylinder();

	G4VPhysicalVolume* Construct();

	// Methods used by scorers
	G4int GetIndex(G4Step* aStep);
	G4int GetIndex(G4int iR, G4int iPhi, G4int iZ);
	G4int GetBin(G4int index, G4int iBin);
	SurfaceType GetSurfaceID(G4String surfaceName);
	G4bool IsOnBoundary(G4ThreeVector localpos, G4VSolid* solid, SurfaceType surfaceID);
	G4double GetAreaOfSelectedSurface(G4VSolid* solid, SurfaceType surfaceID, G4int i, G4int j, G4int k);

	// Methods used by parameterization
	G4Material* ComputeMaterial(const G4int repNo, G4VPhysicalVolume* pvol, const G4VTouchable* parent);
	void ComputeTransformation(const G4int, G4VPhysicalVolume* pvol) const;
	void ComputeDimensions(G4Tubs&, const G4int, const G4VPhysicalVolume*) const;
	void ComputeDimensions(G4Sphere&, const G4int, const G4VPhysicalVolume*) const{};

	static void CreateDefaults(TsParameterManager* pM, G4String& childName, G4String& parentName);

private:
	G4double fTotalRMin;

	std::vector<G4RotationMatrix*> fPhiDivisionRotations;
};

#endif
