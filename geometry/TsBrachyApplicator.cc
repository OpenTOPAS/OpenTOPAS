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

#include "TsBrachyApplicator.hh"

#include "TsParameterManager.hh"

#include "G4SystemOfUnits.hh"
#include "G4UIcommand.hh"
#include "G4Orb.hh"
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4MultiUnion.hh"

TsBrachyApplicator::TsBrachyApplicator(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
					   TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name)
:TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name)
{
}

TsBrachyApplicator::~TsBrachyApplicator()
{
}


G4VPhysicalVolume* TsBrachyApplicator::Construct()
{
	BeginConstruction();

	const G4double  cylinderLength		= fPm->GetDoubleParameter( GetFullParmName("CylinderLength"), "Length");
	const G4double  radius				= fPm->GetDoubleParameter( GetFullParmName("Radius"), "Length");
	const G4int		numberOfRadialHoles = fPm->GetIntegerParameter( GetFullParmName("NumberOfRadialHoles"));
	const G4double  holeRadius			= fPm->GetDoubleParameter( GetFullParmName("HoleRadius"), "Length");
	const G4double  holeOffset			= fPm->GetDoubleParameter( GetFullParmName("HoleOffset"), "Length");

	G4double holeLength = cylinderLength/2. + radius;
	
	// Applicator without holes is composed of cylinder plus endcap
	G4Tubs* applicatorCylinderSolid = new G4Tubs("ApplicatorCylinder", 0., radius, cylinderLength/2., 0.0*deg, 360.0*deg);
	G4Orb* applicatorEndcapSolid = new G4Orb("ApplicatorEndcap", radius);
	G4RotationMatrix ra = G4RotationMatrix();
	G4ThreeVector pa = G4ThreeVector(0., 0., -cylinderLength/2.);
	G4Transform3D ta = G4Transform3D(ra,pa);
	G4UnionSolid* applicatorWithoutHolesSolid = new G4UnionSolid("ApplicatorWithoutHoles", applicatorCylinderSolid, applicatorEndcapSolid, ta);

	// Set of holes
	G4MultiUnion* setOfHolesSolid = new G4MultiUnion("SetOfHoles");;
	
	// Add center hole to setOfHolesSolid
	G4Tubs* holeSolid = new G4Tubs("Hole", 0., holeRadius, holeLength, 0.0*deg, 360.0*deg);
	pa = G4ThreeVector(0., 0., 0.);
	ta = G4Transform3D(ra,pa);
	setOfHolesSolid->AddNode(*holeSolid, ta);
	
	// Add radial holes to setOfHolesSolid
	G4double transX, transY;
	G4String transName, transValue;
	for (G4int holeNum = 1; holeNum < numberOfRadialHoles+1; holeNum++) {
		transX = sin(360*deg*holeNum/numberOfRadialHoles)*holeOffset;
		transY = cos(360.*deg*holeNum/numberOfRadialHoles)*holeOffset;
		pa = G4ThreeVector(transX, transY, 0.);
		ta = G4Transform3D(ra,pa);
		setOfHolesSolid->AddNode(*holeSolid, ta);
		
		// Save position into parameters for use in placing other components
		transName  = "dc:Ge/" + fName + "/Hole" + G4UIcommand::ConvertToString(holeNum) + "/TransX";
		transValue = G4UIcommand::ConvertToString(transX / mm) + " mm";
		fPm->AddParameter(transName, transValue);
		transName  = "dc:Ge/" + fName + "/Hole" + G4UIcommand::ConvertToString(holeNum) + "/TransY";
		transValue = G4UIcommand::ConvertToString(transY / mm) + " mm";
		fPm->AddParameter(transName, transValue);
	}

	// Done building setOfHolesSolid
	setOfHolesSolid->Voxelize();

	// Subtract setOfHolesSolid from overallSolid
	pa = G4ThreeVector(0., 0., 0.);
	ta = G4Transform3D(ra,pa);
	G4SubtractionSolid* applicatorSolid = new G4SubtractionSolid("Applicator", applicatorWithoutHolesSolid, setOfHolesSolid, ta);
	
	// Create the logical and physical volumes for the applicator
	fEnvelopeLog = CreateLogicalVolume(applicatorSolid);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

	InstantiateChildren();

  	return fEnvelopePhys;
}
