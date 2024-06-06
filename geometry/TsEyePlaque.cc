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

#include "TsEyePlaque.hh"

#include "TsParameterManager.hh"

#include "G4Tubs.hh"
#include "G4Sphere.hh"

#include "G4UnionSolid.hh"
#include "G4IntersectionSolid.hh"
#include "G4SubtractionSolid.hh"

#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"

#include "G4PVPlacement.hh"

#include <G4PhysicalVolumeStore.hh>

#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "Randomize.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4Material.hh"
#include "G4NistManager.hh"


TsEyePlaque::TsEyePlaque(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
			 TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name) :
TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name)
{
}

TsEyePlaque::~TsEyePlaque(){}

G4VPhysicalVolume* TsEyePlaque::Construct()
{
	BeginConstruction();

	G4double eyeRadius = fPm->GetDoubleParameter(GetFullParmName("Eye/Radius"), "Length");
	G4Sphere* eyeSphere = new G4Sphere("eye_solid", 0, eyeRadius, 0, 2*pi, 0, pi);

	G4double sphereROuter = fPm->GetDoubleParameter(GetFullParmName("Sphere/ROuter"), "Length");
    G4Sphere* outerSphere = new G4Sphere("ep_outer_sph_solid", 0, sphereROuter, 0, 2*pi, pi/2, pi/2);

	G4double cylinderROuter = fPm->GetDoubleParameter(GetFullParmName("Cylinder/ROuter"), "Length");
	G4double cylinderLength = fPm->GetDoubleParameter(GetFullParmName("Cylinder/Length"), "Length");
    G4Tubs* outerCylinder = new G4Tubs("ep_outer_tub_solid", 0, cylinderROuter, cylinderLength, 0, 2*pi);

	G4double sphereRInner = fPm->GetDoubleParameter(GetFullParmName("Sphere/RInner"), "Length");
    G4Sphere* innerSphere = new G4Sphere("ep_sph_solid", 0, sphereRInner, 0, 2*pi, pi/2, pi/2);

	G4double cylinderRInner = fPm->GetDoubleParameter(GetFullParmName("Cylinder/RInner"), "Length");
	G4Tubs* innerCylinder = new G4Tubs("ep_tub_solid", 0, cylinderRInner, cylinderLength, 0, 2*pi);
	
	G4double sphereCutCylinderROuter = fPm->GetDoubleParameter(GetFullParmName("SphereCutCylinder/ROuter"), "Length");
	G4double sphereCutCylinderRInner = fPm->GetDoubleParameter(GetFullParmName("SphereCutCylinder/RInner"), "Length");
	G4Tubs* cutCylinder = new G4Tubs("cutting_tube", 0, sphereCutCylinderROuter, sphereCutCylinderRInner, 0, 2*pi);

    // back
    G4IntersectionSolid* sld_back1 = new G4IntersectionSolid("ep_back1_solid",outerSphere,outerCylinder);
    G4SubtractionSolid* sld_back2 = new G4SubtractionSolid("ep_back2_solid",sld_back1,eyeSphere);
    G4SubtractionSolid* sld_back = new G4SubtractionSolid("ep_back_solid",sld_back2,cutCylinder);
	G4LogicalVolume* log_back = CreateLogicalVolume("Back", sld_back);

    // lip
    G4IntersectionSolid* sld_lip1 = new G4IntersectionSolid("ep_lip1_solid",innerSphere,innerCylinder);
    G4SubtractionSolid* sld_lip2 = new G4SubtractionSolid("ep_lip2_solid",sld_lip1,eyeSphere);
    G4SubtractionSolid* sld_lip = new G4SubtractionSolid("ep_lip_solid", sld_lip2,cutCylinder);
	G4LogicalVolume* log_lip = CreateLogicalVolume("Lip", sld_lip);

	// Create overall envelope. Requires union of back and lip
	G4UnionSolid* envelopeSolid = new G4UnionSolid("envelope_solid", sld_back, sld_lip);
	fEnvelopeLog = CreateLogicalVolume(envelopeSolid);
	fEnvelopeLog->SetVisAttributes(fPm->GetInvisible());
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

	G4VPhysicalVolume* pvol_back = CreatePhysicalVolume("ep_back_log",log_back,fEnvelopePhys);
	InstantiateChildren(pvol_back);

	CreatePhysicalVolume("ep_lip_log",log_lip,fEnvelopePhys);

	return fEnvelopePhys;
}
