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

#include "TsApertureArray.hh"

#include "TsParameterManager.hh"

#include "G4Trd.hh"
#include "G4MultiUnion.hh"
#include "G4SubtractionSolid.hh"
#include "G4Transform3D.hh"

#include <fstream>

TsApertureArray::TsApertureArray(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
					   TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name)
:TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name)
{
}

TsApertureArray::~TsApertureArray()
{
}


G4VPhysicalVolume* TsApertureArray::Construct()
{
	BeginConstruction();

	G4double widthBeamletAtIso = fPm->GetDoubleParameter(GetFullParmName("WidthBeamletAtIso"),"Length");
	G4double spacingBeamletAtIso = fPm->GetDoubleParameter(GetFullParmName("SpacingBeamletAtIso"),"Length");
	G4int nBeamletsWidth = fPm->GetIntegerParameter(GetFullParmName("NBeamletsWidth"));
	G4int nBeamletsLength = fPm->GetIntegerParameter(GetFullParmName("NBeamletsLength"));
	G4double distCollimatorDownstreamFaceToIso = fPm->GetDoubleParameter(GetFullParmName("DistCollimatorDownstreamFaceToIso"),"Length");
	G4double hlx = fPm->GetDoubleParameter(GetFullParmName("HLX"),"Length");
	G4double hly = fPm->GetDoubleParameter(GetFullParmName("HLY"),"Length");
	G4double collimatorTransZ = fPm->GetDoubleParameter(GetFullParmName("TransZ"),"Length");
	G4double collimatorThickness = fPm->GetDoubleParameter(GetFullParmName("CollimatorThickness"),"Length");
	G4double distBremTargetToIso = fPm->GetDoubleParameter(GetFullParmName("DistBremTargetToIso"),"Length");
	G4double distBeamletVirtualFocusToIso  = fPm->GetDoubleParameter(GetFullParmName("DistBeamletVirtualFocusToIso"),"Length");
	G4double angleOffset  = fPm->GetDoubleParameter(GetFullParmName("AngleOffset"),"Angle");
	G4bool useFullLengthBeamlets = fPm->GetBooleanParameter(GetFullParmName("UseFullLengthBeamlets"));

	G4String geometryMethod = fPm->GetStringParameter(GetFullParmName("GeometryMethod")); // addbeamlets, subtractbeamlets or onlybeamlets
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(geometryMethod);
#else
	geometryMethod.toLower();
#endif

	G4int totalBeamlets = nBeamletsWidth * nBeamletsLength;
	if (geometryMethod != "addbeamlets" && totalBeamlets==1) {
		geometryMethod = "addbeamlets";
		G4cout << "Switched GeometryMethod to AddBeamlets." << G4endl;
		G4cout << "This is the only method that works when have just a single beamlet." << G4endl;
	}
	
	if (geometryMethod != "addbeamlets" && totalBeamlets > 300) {
		G4cout << "Warning: The TsApertureArray component " << GetName() << " may have poor performance." << G4endl;
		G4cout << "Its geometry method may be very slow to build as it has so many beamlets." << G4endl;
		G4cout << "Only the GeometryMethod AddBeamlets works efficiently with such a large number of beamlets." << G4endl;
	}
	
	if (geometryMethod != "addbeamlets" && (spacingBeamletAtIso < widthBeamletAtIso)) {
		geometryMethod = "addbeamlets";
		G4cout << "Switched GeometryMethod to AddBeamlets." << G4endl;
		G4cout << "This is the only method that works when SpacingBeamletAtIso < WidthBeamletAtIso." << G4endl;
	}

	G4double distanceFocusToDownstreamFace = distBremTargetToIso - distCollimatorDownstreamFaceToIso;
	G4double distanceFocusToUpstreamFace = distanceFocusToDownstreamFace - collimatorThickness;
	G4double fractionOfDistanceToDownstreamFace = distanceFocusToDownstreamFace / distBremTargetToIso;
	G4double fractionOfDistanceToUpstreamFace   = distanceFocusToUpstreamFace   / distBremTargetToIso;
	
	G4double distBeamletVirtualFocusToBeamletCenter;
	if (useFullLengthBeamlets)
		distBeamletVirtualFocusToBeamletCenter = distBeamletVirtualFocusToIso - distBremTargetToIso / 2.;
	else
		distBeamletVirtualFocusToBeamletCenter = distBeamletVirtualFocusToIso - distCollimatorDownstreamFaceToIso - collimatorThickness / 2.;

	G4double angleBetweenTwoBeamlets = 2. * atan2(spacingBeamletAtIso/2., distBeamletVirtualFocusToIso);

	G4double startAngleX = - angleBetweenTwoBeamlets * ( nBeamletsWidth  -1) / 2.;
	G4double startAngleY = - angleBetweenTwoBeamlets * ( nBeamletsLength -1) / 2.;
	
	G4Box* envelopeSolid = 0;
	G4MultiUnion* multiUnionSolid;

	if (geometryMethod == "addbeamlets" || geometryMethod == "subtractbeamlets")
		envelopeSolid = new G4Box(fName, hlx, hly, collimatorThickness / 2.);
	
	if (geometryMethod == "subtractbeamlets" || geometryMethod == "onlybeamlets")
		multiUnionSolid = new G4MultiUnion("UnionOfBeamlets");
	
	if (geometryMethod == "addbeamlets") {
		fEnvelopeLog = CreateLogicalVolume(envelopeSolid);
		fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);
	}

	// Beamlets will be filled with the sphinx's parent material (probably air or vacuum)
	G4String beamletMaterial = fParentComponent->GetResolvedMaterialName();

	// Loop over all the beamlets
	for (G4int iWidth = 0; iWidth < nBeamletsWidth; iWidth++ ) {
		for (G4int iLength = 0; iLength < nBeamletsLength; iLength++ ) {
			G4double angleX = startAngleX + iWidth * angleBetweenTwoBeamlets;
			G4double angleY = startAngleY + iLength * angleBetweenTwoBeamlets + angleOffset;
			G4RotationMatrix* rot = new G4RotationMatrix();
			G4ThreeVector* angleVector = new G4ThreeVector(-angleY, angleX, 0.);
			G4double angleScalar = sqrt(angleX*angleX + angleY*angleY);
			if (geometryMethod != "addbeamlets")
				rot->rotate(-angleScalar, angleVector);
			else
				rot->rotate(angleScalar, angleVector);

			//RegisterRotation(rot);

			G4double transZ;
			
			G4VSolid* beamletSolid;
			if (useFullLengthBeamlets) {
				beamletSolid = new G4Trd("OneBeamlet", widthBeamletAtIso/2., 0.,
										 widthBeamletAtIso/2., 0., distBremTargetToIso / cos(angleScalar) / 2.);

				// Keep in mind that translation is relative to center of Collimator
				transZ = (distBremTargetToIso - distBeamletVirtualFocusToIso) / 2. - collimatorTransZ;
			} else {
				G4double halfWidthBeamletAtDownstreamFace = widthBeamletAtIso/2. * fractionOfDistanceToDownstreamFace;
				G4double halfWidthBeamletAtUpstreamFace   = widthBeamletAtIso/2. * fractionOfDistanceToUpstreamFace;

				// Beamlet needs to extend past collimator on both sides, so that angled ends are both fully outside.
				G4double extraLengthToExtendPastCollimator = (halfWidthBeamletAtDownstreamFace + halfWidthBeamletAtUpstreamFace) * sin(angleScalar);
				
				G4double extraFactorForDownstreamExtension = (distanceFocusToDownstreamFace + extraLengthToExtendPastCollimator) / distanceFocusToDownstreamFace;
				G4double extraFactorForUpstreamExtension   = (distanceFocusToUpstreamFace   - extraLengthToExtendPastCollimator) / distanceFocusToUpstreamFace;

				G4double halfLengthBeamlet = collimatorThickness / cos(angleScalar) / 2. + extraLengthToExtendPastCollimator;

				beamletSolid = new G4Trd("OneBeamlet",  halfWidthBeamletAtDownstreamFace * extraFactorForDownstreamExtension,
														halfWidthBeamletAtUpstreamFace   * extraFactorForUpstreamExtension,
														halfWidthBeamletAtDownstreamFace * extraFactorForDownstreamExtension,
														halfWidthBeamletAtUpstreamFace   * extraFactorForUpstreamExtension,
														halfLengthBeamlet);

				transZ = 0.;
			}
			
			G4double transX = distBeamletVirtualFocusToBeamletCenter * tan(angleX);
			G4double transY = distBeamletVirtualFocusToBeamletCenter * tan(angleY);

			G4ThreeVector* position = new G4ThreeVector(transX, transY, transZ);
			//RegisterTranslation(position);
			G4Transform3D* transform3D = new G4Transform3D(*rot,*position);
			//RegisterTransform(transform3D);
			
			if (geometryMethod == "addbeamlets") {
				G4LogicalVolume* beamletLog = CreateLogicalVolume("Aperture", beamletMaterial, beamletSolid);
				CreatePhysicalVolume("Beamlet", beamletLog, rot, position, fEnvelopePhys);
			} else {
				multiUnionSolid->AddNode(*beamletSolid, *transform3D);
			}
		}
	}

	if (geometryMethod != "addbeamlets")
		multiUnionSolid->Voxelize();

	if (geometryMethod == "onlybeamlets") {
		fEnvelopeLog = CreateLogicalVolume(multiUnionSolid);
		fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);
	} else if (geometryMethod == "subtractbeamlets") {
		G4RotationMatrix* ra = new G4RotationMatrix();
		
		G4ThreeVector* pa = new G4ThreeVector(0., 0., 0.);
		G4Transform3D* ta = new G4Transform3D(*ra,*pa);
		
		G4SubtractionSolid* subtractionSolid = new G4SubtractionSolid("Collimator", envelopeSolid, multiUnionSolid, *ta);
		
		fEnvelopeLog = CreateLogicalVolume(subtractionSolid);
		fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);
	}

  	return fEnvelopePhys;
}
