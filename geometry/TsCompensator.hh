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

#ifndef TsCompensator_hh
#define TsCompensator_hh

#include "TsVGeometryComponent.hh"

class TsCompensator : public TsVGeometryComponent
{
public:
	TsCompensator(TsParameterManager* pM, TsExtensionManager* Em,
				  TsMaterialManager* mM, TsGeometryManager* gM,
				  TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name);
	~TsCompensator();

	G4VPhysicalVolume* Construct();

private:
	void BuildAsPolyhedra();
	void BuildAsExtrudedSolid();
	void BuildAsUnionSolid();
	void BuildAsSubtractionSolid();
	std::vector<G4Tubs*> GetDrillHoleCylinders();
	std::vector<G4ThreeVector*> GetDrillHoleLocations();

	G4String fFileName;
	G4double fConversionFactor;
	G4double fMainCylinderThickness;
	G4double fMainCylinderRadius;
	G4double fMainCylinderRadiusSquared;
	G4double fDrillHoleRadius;
	G4VSolid* fMainCylinder;

	G4int fNumRows;
	// One per row
	std::vector<G4double> fXSteps;
	std::vector<G4double> fXStarts;
	std::vector<G4double> fYStarts;
	// per row and per column
	std::vector< std::vector<G4double> > fDepths;
};

#endif
