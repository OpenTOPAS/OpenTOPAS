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

#include "TsBrachyShieldStar.hh"

#include "TsParameterManager.hh"

#include "G4SystemOfUnits.hh"
#include "G4UnionSolid.hh"
#include "G4MultiUnion.hh"

TsBrachyShieldStar::TsBrachyShieldStar(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
					   TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name)
:TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name)
{
}

TsBrachyShieldStar::~TsBrachyShieldStar()
{
}


G4VPhysicalVolume* TsBrachyShieldStar::Construct()
{
	BeginConstruction();

	const G4double  holeRadius		= fPm->GetDoubleParameter( GetFullParmName("holeRadius"), "Length");
	const G4double  rMin			= fPm->GetDoubleParameter( GetFullParmName("RMin"), "Length");
	const G4double  rMax			= fPm->GetDoubleParameter( GetFullParmName("RMax"), "Length");
	const G4double  hL				= fPm->GetDoubleParameter( GetFullParmName("HL"), "Length");
	const G4int		numberOfSpokes	= fPm->GetIntegerParameter( GetFullParmName("NumberOfRadialHoles"));
	const G4double  spokeWidth		= fPm->GetDoubleParameter( GetFullParmName("SpokeWidth"), "Length");

	G4double spokeLength = rMax - holeRadius;
	G4double angleBetweenSpokes = 360.*deg/numberOfSpokes;

	// Overall shield
	G4MultiUnion* shieldSolid = new G4MultiUnion("Shield");
	
	// Add ring
	G4Tubs* ringSolid = new G4Tubs("Ring", holeRadius, rMin, hL, 0.*deg, 360.*deg);
	G4RotationMatrix ra = G4RotationMatrix();
	G4ThreeVector pa = G4ThreeVector(0., 0., 0.);
	G4Transform3D ta = G4Transform3D(ra,pa);
	shieldSolid->AddNode(*ringSolid, ta);

	// First spoke is rotated by half the angle between spokes
	ra.rotateZ(-angleBetweenSpokes/2.);

	// Add spokes
	G4Box* spokeSolid = new G4Box("Spoke", spokeWidth/2., spokeLength/1.8, hL);
	G4double angle;
	G4double transX;
	G4double transY;
	for (G4int spokeNum = 0; spokeNum < numberOfSpokes; spokeNum++) {
		angle = angleBetweenSpokes*(spokeNum + 0.5);
		transX = sin(angle)*(rMin + spokeLength/2.);
		transY = cos(angle)*(rMin + spokeLength/2.);
		pa = G4ThreeVector(transX, transY, 0.);
		ta = G4Transform3D(ra,pa);
		shieldSolid->AddNode(*spokeSolid, ta);
		ra.rotateZ(-angleBetweenSpokes);
	}
	shieldSolid->Voxelize();
	
	// Create the logical and physical volumes for the shield
	fEnvelopeLog = CreateLogicalVolume(shieldSolid);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

  	return fEnvelopePhys;
}
