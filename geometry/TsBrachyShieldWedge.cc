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

#include "TsBrachyShieldWedge.hh"

#include "TsParameterManager.hh"

#include "G4SystemOfUnits.hh"
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4MultiUnion.hh"

TsBrachyShieldWedge::TsBrachyShieldWedge(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
					   TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name)
:TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name)
{
}

TsBrachyShieldWedge::~TsBrachyShieldWedge()
{
}


G4VPhysicalVolume* TsBrachyShieldWedge::Construct()
{
	BeginConstruction();

	const G4double  rMin				= fPm->GetDoubleParameter( GetFullParmName("RMin"), "Length");
	const G4double  rMax				= fPm->GetDoubleParameter( GetFullParmName("RMax"), "Length");
	const G4double  hL					= fPm->GetDoubleParameter( GetFullParmName("HL"), "Length");
	const G4double  angleStart			= fPm->GetDoubleParameter( GetFullParmName("AngleStart"), "Angle");
	const G4double  angleDistance		= fPm->GetDoubleParameter( GetFullParmName("AngleDistance"), "Angle");
	const G4int		numberOfRadialHoles = fPm->GetIntegerParameter( GetFullParmName("NumberOfRadialHoles"));
	const G4double  holeRadius			= fPm->GetDoubleParameter( GetFullParmName("HoleRadius"), "Length");
	const G4double  holeOffset			= fPm->GetDoubleParameter( GetFullParmName("HoleOffset"), "Length");

	// Ring
	G4Tubs* ringSolid = new G4Tubs("Ring", holeRadius, rMin, hL*.999, 0.*deg, 360.*deg);

	// Wedge with no holes
	G4Tubs* wedgeWithoutHolesSolid = new G4Tubs("WedgeWithoutHoles", rMin, rMax, hL*.999, angleStart, angleDistance);
	
	// Combine ring and wedge into shield without holes
	G4RotationMatrix ra = G4RotationMatrix();
	G4ThreeVector pa = G4ThreeVector(0., 0., 0.);
	G4Transform3D ta = G4Transform3D(ra,pa);
	G4UnionSolid* shieldWithoutHolesSolid = new G4UnionSolid("ShieldWithoutHoles", ringSolid, wedgeWithoutHolesSolid, ta);

	// Set of holes
	G4MultiUnion* setOfHolesSolid = new G4MultiUnion("SetOfHoles");

	G4Tubs* holeSolid = new G4Tubs("Hole", 0., holeRadius, hL, 0.0*deg, 360.0*deg);

	G4double angle;
	G4double angleBetweenHoles = 360.*deg/numberOfRadialHoles;
	G4double transX;
	G4double transY;
	for (G4int holeNum = 0; holeNum < numberOfRadialHoles; holeNum++) {
		angle = angleBetweenHoles*holeNum;
		transX = sin(angle)*holeOffset;
		transY = cos(angle)*holeOffset;
		pa = G4ThreeVector(transX, transY, 0.);
		ta = G4Transform3D(ra,pa);
		setOfHolesSolid->AddNode(*holeSolid, ta);
	}
	setOfHolesSolid->Voxelize();

	// Subtract set of holes from shield
	pa = G4ThreeVector(0., 0., 0.);
	ta = G4Transform3D(ra,pa);
	G4SubtractionSolid* shieldSolid = new G4SubtractionSolid("Shield", shieldWithoutHolesSolid, setOfHolesSolid, ta);
	
	// Create the logical and physical volumes for the shield
	fEnvelopeLog = CreateLogicalVolume(shieldSolid);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

  	return fEnvelopePhys;
}
