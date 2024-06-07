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

#include "TsEye.hh"

#include "TsParameterManager.hh"

#include "G4Tubs.hh"
#include "G4Orb.hh"
#include "G4Ellipsoid.hh"
#include "G4Polycone.hh"
#include "G4UnionSolid.hh"
#include "G4IntersectionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4LogicalVolume.hh"

TsEye::TsEye(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
			 TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name) :
TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name)
{
}


TsEye::~ TsEye(){}


G4VPhysicalVolume* TsEye::Construct()
{
	BeginConstruction();

    // Basic shapes that will be used to build each subcomponent

	// tumor basic shapes
	G4double tumorRadius = fPm->GetDoubleParameter(GetFullParmName("Tumor/Radius"), "Length");
	G4Orb* tumorFrontSphere = new G4Orb("tumor_sphere", tumorRadius);

    // sclera basic shapes
	G4double scleraRadius = fPm->GetDoubleParameter(GetFullParmName("Sclera/Radius"), "Length");
    G4Orb* scleraOuterSphere = new G4Orb("outside_sclera", scleraRadius);
	G4Tubs* scleraSizedTubForVariusCuts = new G4Tubs("sclera_tub", 0, scleraRadius, scleraRadius, 0, 2*pi);

	// vitreous basic shapes
	G4double vitreousRMajor = fPm->GetDoubleParameter(GetFullParmName("Vitreous/RMajor"), "Length");
	G4double vitreousRMinor = fPm->GetDoubleParameter(GetFullParmName("Vitreous/RMinor"), "Length");
	G4Ellipsoid* vitreousEllipsoid = new G4Ellipsoid("vitreous_ellipsoid", vitreousRMinor, vitreousRMajor, vitreousRMajor);
	
    // aqueous basic shapes
	G4double aqueousRadius = fPm->GetDoubleParameter(GetFullParmName("Aqueous/Radius"), "Length");
    G4Orb* aqueousSphere = new G4Orb("aqueous_front", aqueousRadius);

    // cornea basic shapes
	G4double corneaFrontRadius = fPm->GetDoubleParameter(GetFullParmName("Cornea/FrontRadius"), "Length");
    G4Orb* corneaFrontSphere = new G4Orb("outside_cornea", corneaFrontRadius);
	G4double corneaBackRadius = fPm->GetDoubleParameter(GetFullParmName("Cornea/BackRadius"), "Length");
    G4Orb* corneaBackSphere = new G4Orb("inside_cornea", corneaBackRadius);

    // iris basic shape
	G4double irisInnerRadius = fPm->GetDoubleParameter(GetFullParmName("Iris/InnerRadius"), "Length");
	G4double irisOuterRadius = fPm->GetDoubleParameter(GetFullParmName("Iris/OuterRadius"), "Length");
	G4double irisLength = fPm->GetDoubleParameter(GetFullParmName("Iris/Length"), "Length");
    G4Tubs* irisTub = new G4Tubs("iris_shape", irisInnerRadius, irisOuterRadius, irisLength, 0, 2*pi);

    // lens basic shapes
	G4double lensFrontRadius = fPm->GetDoubleParameter(GetFullParmName("Lens/FrontRadius"), "Length");
    G4Orb* lensFrontSphere = new G4Orb("lens_Front_sphere", lensFrontRadius);
	G4double lensBackRadius = fPm->GetDoubleParameter(GetFullParmName("Lens/BackRadius"), "Length");
    G4Orb* lensBackSphere = new G4Orb("lens_Back_sphere", lensBackRadius);


    // Build the subcomponent solids from the above basic shapes
	
    // tumor
	G4double tumorOffset = fPm->GetDoubleParameter(GetFullParmName("Tumor/Offset"), "Length");
	G4ThreeVector tumorTrans1(0, 0, -1. * tumorOffset);
	G4double tumorRotX = fPm->GetDoubleParameter(GetFullParmName("Tumor/RotX"), "Angle");
	G4double tumorRotY = fPm->GetDoubleParameter(GetFullParmName("Tumor/RotY"), "Angle");
	tumorTrans1.rotateX(tumorRotX);
	tumorTrans1.rotateY(tumorRotY);
    G4IntersectionSolid* tumorSolid = new G4IntersectionSolid("int_ell_sphere", vitreousEllipsoid, tumorFrontSphere, 0, tumorTrans1);

    // sclera
    G4SubtractionSolid* scleraShellSolid = new G4SubtractionSolid("sclera_shell", scleraOuterSphere, vitreousEllipsoid);
    G4RotationMatrix* scleraRot1 = new G4RotationMatrix;
    scleraRot1->rotateY(2*M_PI*rad);
	G4double scleraOffset = fPm->GetDoubleParameter(GetFullParmName("Sclera/Offset"), "Length");
    G4ThreeVector scleraTrans1(scleraOffset, 0, 0);
    G4SubtractionSolid* scleraSolid = new G4SubtractionSolid("sclera_solid", scleraShellSolid, corneaFrontSphere, scleraRot1, scleraTrans1);

    // cornea
    G4RotationMatrix* corneaRot1 = new G4RotationMatrix;
    corneaRot1->rotateY(2*M_PI*rad);
	G4double corneaOffset = fPm->GetDoubleParameter(GetFullParmName("Cornea/Offset"), "Length");
    G4ThreeVector corneaTrans1(corneaOffset, 0, 0);
    G4SubtractionSolid* corneaShellSolid = new G4SubtractionSolid("outside_minus_inside", corneaFrontSphere, corneaBackSphere, corneaRot1, corneaTrans1);
	G4ThreeVector corneaTrans2(-1. * scleraOffset, 0, 0);
    G4SubtractionSolid* corneaSolid = new G4SubtractionSolid("cornea_shell_minus_ellipsoid", corneaShellSolid, vitreousEllipsoid, corneaRot1, corneaTrans2);
	
    // iris
    G4RotationMatrix* irisRot1 = new G4RotationMatrix;
    irisRot1->rotateY(-M_PI/2*rad);
	G4double irisFrontOffset = fPm->GetDoubleParameter(GetFullParmName("Iris/FrontOffset"), "Length");
	G4double irisBackOffset = fPm->GetDoubleParameter(GetFullParmName("Iris/BackOffset"), "Length");
    G4ThreeVector irisTrans1(0, 0, irisFrontOffset);
    G4SubtractionSolid* irisSolid = new G4SubtractionSolid("iris_tube_minus_cornea", irisTub, corneaSolid, irisRot1, irisTrans1);
    G4RotationMatrix* irisRot2 = new G4RotationMatrix;
    irisRot2->rotateY(M_PI/2*rad);
	
    // lens
    G4RotationMatrix* lensRot1 = new G4RotationMatrix;
    lensRot1->rotateY(2*M_PI*rad);
	G4double lensFrontOffset = fPm->GetDoubleParameter(GetFullParmName("Lens/FrontOffset"), "Length");
	G4double lensBackOffset = fPm->GetDoubleParameter(GetFullParmName("Lens/BackOffset"), "Length");
    G4ThreeVector lensTrans1(-1 * lensFrontOffset, 0, 0);
    G4IntersectionSolid* lensSolid = new G4IntersectionSolid("lens_antSphere_postSphere", lensBackSphere, lensFrontSphere, lensRot1, lensTrans1);

    // vitreous
    G4RotationMatrix* vitreousRot1 = new G4RotationMatrix;
    vitreousRot1->rotateY(M_PI/2*rad);
	G4double vitreousFrontCut = fPm->GetDoubleParameter(GetFullParmName("Vitreous/FrontCut"), "Length");
    G4ThreeVector vitreousTrans1(vitreousFrontCut, 0, 0);
    G4SubtractionSolid* vitreousA = new G4SubtractionSolid("outside_minus_inside", vitreousEllipsoid, scleraSizedTubForVariusCuts, vitreousRot1, vitreousTrans1);
    G4RotationMatrix* vitreousRot2 = new G4RotationMatrix;
    vitreousRot2->rotateY(2*M_PI*rad);
    G4ThreeVector vitreousTrans2(lensBackOffset, 0, 0);
    G4SubtractionSolid* vitreousB = new G4SubtractionSolid("VitreousA_minus_lens_back", vitreousA, lensBackSphere, vitreousRot2, vitreousTrans2);
    G4SubtractionSolid* vitrousSolid = new G4SubtractionSolid("VitreousB_minus_tumor", vitreousB, tumorSolid, 0, tumorTrans1);

    // aqueous
    G4RotationMatrix* aqueousRot1 = new G4RotationMatrix;
    aqueousRot1->rotateY(M_PI/2*rad);
	G4double aqueousBackCut = fPm->GetDoubleParameter(GetFullParmName("Aqueous/BackCut"), "Length");
	G4ThreeVector aqueousTrans1(-1. * aqueousBackCut, 0, 0);
    G4SubtractionSolid* aqueousSolidA = new G4SubtractionSolid("ellipsoid_cut", vitreousEllipsoid, scleraSizedTubForVariusCuts, aqueousRot1, aqueousTrans1);
    G4RotationMatrix* aqueousRot2 = new G4RotationMatrix;
    aqueousRot2->rotateY(2*M_PI*rad);
	G4double aqueousOffset = fPm->GetDoubleParameter(GetFullParmName("Aqueous/Offset"), "Length");
    G4ThreeVector aqueousTrans2(aqueousOffset, 0, 0);
    G4UnionSolid* aqueousSolidB = new G4UnionSolid("union_cutEllispoid_sphere", aqueousSolidA, aqueousSphere, aqueousRot2, aqueousTrans2);
	G4ThreeVector aqueousTrans3(-1. * aqueousBackCut, 0, 0);
    G4SubtractionSolid* aqueousSolidC = new G4SubtractionSolid("aqu_cut", aqueousSolidB, scleraSizedTubForVariusCuts, aqueousRot1, aqueousTrans3);
	G4double aqueousLensOffset = fPm->GetDoubleParameter(GetFullParmName("Aqueous/LensOffset"), "Length");
	G4ThreeVector aqueousTrans4(-1. * aqueousLensOffset, 0, 0);
    G4SubtractionSolid* aqueousSolidD = new G4SubtractionSolid("aqu_minus_ant_lens", aqueousSolidC, lensFrontSphere, aqueousRot2, aqueousTrans4);
    G4ThreeVector aqueousTrans5(irisBackOffset, 0, 0);
    G4SubtractionSolid* aqueousSolid = new G4SubtractionSolid("aqu_minus_iris", aqueousSolidD, irisTub, aqueousRot1, aqueousTrans5);

    // optical nerve
	G4int nPoints = fPm->GetVectorLength(GetFullParmName("OpticNerve/Z"));
	if (fPm->GetVectorLength(GetFullParmName("OpticNerve/R")) != nPoints) {
		G4cerr << "Topas is exiting due to a serious error in geometry setup." << G4endl;
		G4cerr << GetFullParmName("OpticNerve/Z") << " has different number of values than " <<
		GetFullParmName("OpticNerve/R") << G4endl;
		fPm->AbortSession(1);
	}

	G4double* polyconeZ = fPm->GetDoubleVector(GetFullParmName("OpticNerve/Z"), "Length");
	G4double* polyconeROuter = fPm->GetDoubleVector(GetFullParmName("OpticNerve/R"), "Length");
	G4double* polyconeRInner = new G4double[fPm->GetVectorLength(GetFullParmName("OpticNerve/R"))]{0.};
	G4Polycone* opticNerveSolid = new G4Polycone("optic_nerve_polycone", 0., 360.*deg, 8,
												 polyconeZ, polyconeRInner, polyconeROuter);
    G4RotationMatrix* opticNerveRot1 = new G4RotationMatrix;
	G4double opticNerveAngleX = fPm->GetDoubleParameter(GetFullParmName("OpticNerve/AngleX"), "Angle");
	G4double opticNerveAngleY = fPm->GetDoubleParameter(GetFullParmName("OpticNerve/AngleY"), "Angle");
    opticNerveRot1->rotateY(opticNerveAngleY);
    opticNerveRot1->rotateX(opticNerveAngleX);
	G4double opticNerveOffsetX = fPm->GetDoubleParameter(GetFullParmName("OpticNerve/OffsetX"), "Length");
	G4double opticNerveOffsetY = fPm->GetDoubleParameter(GetFullParmName("OpticNerve/OffsetY"), "Length");
	G4double opticNerveOffsetZ = fPm->GetDoubleParameter(GetFullParmName("OpticNerve/OffsetZ"), "Length");
	G4ThreeVector* opticNerveTrans1 = new G4ThreeVector(-1. * opticNerveOffsetX, -1. * opticNerveOffsetY, -1. * opticNerveOffsetZ);


	// Create overall eye envelope. Requires union of sclera, cornea and optic nerve
	G4UnionSolid* envelopeScleraPlusCorneaSolid = new G4UnionSolid("envelope_solid", scleraOuterSphere, corneaFrontSphere, scleraRot1, scleraTrans1);
	G4UnionSolid* envelopeSolid = new G4UnionSolid("envelope_solid", envelopeScleraPlusCorneaSolid, opticNerveSolid, opticNerveRot1, *opticNerveTrans1);
	fEnvelopeLog = CreateLogicalVolume(envelopeSolid);
	fEnvelopeLog->SetVisAttributes(fPm->GetInvisible());
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

	// Add the subcomponent volumes
    G4LogicalVolume* scleraLog = CreateLogicalVolume("Sclera", scleraSolid);
    CreatePhysicalVolume("Sclera", scleraLog, fEnvelopePhys);

    G4LogicalVolume* corneaLog = CreateLogicalVolume("Cornea", corneaSolid);
	CreatePhysicalVolume("Cornea", corneaLog, 0, new G4ThreeVector(scleraOffset, 0,0), fEnvelopePhys);

    G4LogicalVolume* vitreousLog = CreateLogicalVolume("Vitreous", vitrousSolid);
    CreatePhysicalVolume("Vitreous", vitreousLog, fEnvelopePhys);

    G4LogicalVolume* aqueousLog = CreateLogicalVolume("Aqueous", aqueousSolid);
    CreatePhysicalVolume("Aqueous", aqueousLog, fEnvelopePhys);

    G4LogicalVolume* irisLog = CreateLogicalVolume("Iris", irisSolid);
	CreatePhysicalVolume("Iris", irisLog, irisRot2, new G4ThreeVector(irisBackOffset, 0,0), fEnvelopePhys);

    G4LogicalVolume* lensLog = CreateLogicalVolume("Lens", lensSolid);
	CreatePhysicalVolume("Lens", lensLog, 0, new G4ThreeVector(lensBackOffset, 0,0), fEnvelopePhys);

    G4LogicalVolume* tumorLog = CreateLogicalVolume("Tumor", tumorSolid);
    CreatePhysicalVolume("Tumor", tumorLog, fEnvelopePhys);

    G4LogicalVolume* opticNerveLog = CreateLogicalVolume("OpticNerve", opticNerveSolid);
	CreatePhysicalVolume("OpticNerve", opticNerveLog, opticNerveRot1, opticNerveTrans1, fEnvelopePhys);
	
	return fEnvelopePhys;
}
