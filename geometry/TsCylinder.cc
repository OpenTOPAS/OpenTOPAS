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

#include "TsCylinder.hh"

#include "TsParameterManager.hh"
#include "TsSequenceManager.hh"

#include "TsTopasConfig.hh"
#include "TsParameterisation.hh"

#include "G4Tubs.hh"
#include "G4Step.hh"
#include "G4VTouchable.hh"
#include "G4GeometryTolerance.hh"
#include "G4SystemOfUnits.hh"

TsCylinder::TsCylinder(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
							 TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name) :
TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name)
{
	fIsDividable = true;
	fCanCalculateSurfaceArea = true;
	fHasDifferentVolumePerDivision = true;

	fDivisionNames[0] = "R";
	fDivisionNames[1] = "Phi";
	fDivisionNames[2] = "Z";
	fDivisionUnits[0] = "cm";
	fDivisionUnits[1] = "deg";
	fDivisionUnits[2] = "cm";
}


TsCylinder::~TsCylinder()
{
}


G4VPhysicalVolume* TsCylinder::Construct()
{
	BeginConstruction();

	fPhiDivisionRotations.clear();

	if (!fIsCopy) {
		for (G4int i = 0; i<3; i++) {
			if (fPm->ParameterExists(GetBinParmName(i))) {
				fDivisionCounts[i] = fPm->GetIntegerParameter(GetBinParmName(i));
				if (fDivisionCounts[i] <= 0.) OutOfRange( GetBinParmName(i), "must be larger than zero");
			}
		}
	}

	if (fPm->ParameterExists(GetFullParmName("VoxelMaterials"))) {
		G4int nDivisions = fDivisionCounts[0] * fDivisionCounts[1] * fDivisionCounts[2];
		if (fPm->GetVectorLength(GetFullParmName("VoxelMaterials")) != nDivisions) {
			G4cerr << "Topas is exiting due to a serious error in geometry setup." << G4endl;
			G4cerr << GetName() << " has " << nDivisions << " voxels," << G4endl;
			G4cerr << "but " << GetFullParmName("VoxelMaterials") << " has length of " << fPm->GetVectorLength(GetFullParmName("VoxelMaterials")) << G4endl;
			fPm->AbortSession(1);
		}
	}

	if (fDivisionCounts[0] > 1 || fDivisionCounts[1] > 1)
		SetHasDividedCylinderOrSphere();

	if (fPm->ParameterExists(GetFullParmName("RMin")))
		fTotalRMin = fPm->GetDoubleParameter(GetFullParmName("RMin"),"Length");
	else
		fTotalRMin = 0.;

	G4double totalRMax = fPm->GetDoubleParameter(GetFullParmName("RMax"),"Length");
	G4double totalHL   = fPm->GetDoubleParameter(GetFullParmName("HL"),"Length");

	G4double totalSPhi;
	if (fPm->ParameterExists(GetFullParmName("SPhi")))
		totalSPhi = fPm->GetDoubleParameter(GetFullParmName("SPhi"),"Angle");
	else
		totalSPhi = 0. * deg;

	G4double totalDPhi;
	if (fPm->ParameterExists(GetFullParmName("DPhi")))
		totalDPhi = fPm->GetDoubleParameter(GetFullParmName("DPhi"),"Angle");
	else
		totalDPhi = 360. * deg;

	if (fTotalRMin < 0.       ) OutOfRange( GetFullParmName("RMin"), "can not be negative");
	if (totalRMax < fTotalRMin) OutOfRange( GetFullParmName("RMax"), "can not be less than RMin");
	if (totalHL <= 0.         ) OutOfRange( GetFullParmName("HL"), "must be larger than zero");
	if (totalSPhi < 0.        ) OutOfRange( GetFullParmName("SPhi"), "can not be negative");
	if (totalSPhi > 360. * deg) OutOfRange( GetFullParmName("SPhi"), "can not be greater than 360 degrees");
	if (totalDPhi < 0.        ) OutOfRange( GetFullParmName("DPhi"), "can not be negative");
	if (totalDPhi > 360. * deg) OutOfRange( GetFullParmName("DPhi"), "can not be greater than 360 degrees");

	fFullWidths[0] = totalRMax - fTotalRMin;
	fFullWidths[1] = totalDPhi;
	fFullWidths[2] = 2. * totalHL;

	G4Tubs* envelopeSolid = new G4Tubs(fName, fTotalRMin, totalRMax, totalHL, totalSPhi, totalDPhi);
	fEnvelopeLog = CreateLogicalVolume(envelopeSolid);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

	G4int nDivisions = fDivisionCounts[0] * fDivisionCounts[1] * fDivisionCounts[2];
	if (nDivisions > 1) {
		G4String envelopeMaterialName = GetResolvedMaterialName();

		// Calculate divisions
		G4double deltaPhi = totalDPhi / fDivisionCounts[1];
		G4double deltaZ = totalHL / fDivisionCounts[2];

		G4String divisionName;

		if (fDivisionCounts[0] == 1 && fDivisionCounts[1] == 1) {
			// Use Replica for Z division
			divisionName = fName + "_Z_Division";

			// If necessary, propagate MaxStepSize to divisions
			G4String ParmNameForDivisionMaxStepSize = "Ge/" + fName + "/" + divisionName + "/MaxStepSize";
			if ((!fPm->ParameterExists(ParmNameForDivisionMaxStepSize)) &&
				fPm->ParameterExists(GetFullParmName("MaxStepSize")))
				fPm->AddParameter("d:" + ParmNameForDivisionMaxStepSize, GetFullParmName("MaxStepSize") + + " " + fPm->GetUnitOfParameter(GetFullParmName("MaxStepSize")));

			G4VSolid* ZDivisionSolid = new G4Tubs(divisionName, fTotalRMin, totalRMax, deltaZ, totalSPhi, totalDPhi);
			G4LogicalVolume* ZDivisionLog = CreateLogicalVolume(divisionName, envelopeMaterialName, ZDivisionSolid);
			CreatePhysicalVolume(divisionName, ZDivisionLog, fEnvelopePhys, kZAxis, fDivisionCounts[2], 2. * deltaZ);
			ZDivisionLog->SetVisAttributes(GetVisAttributes(""));
		} else {
			// Use Parameterisation for all three divisions
			divisionName = fName + "_Division";

			// If necessary, propagate MaxStepSize to divisions
			G4String ParmNameForDivisionMaxStepSize = "Ge/" + fName + "/" + divisionName + "/MaxStepSize";
			if ((!fPm->ParameterExists(ParmNameForDivisionMaxStepSize)) &&
				fPm->ParameterExists(GetFullParmName("MaxStepSize")))
				fPm->AddParameter("d:" + ParmNameForDivisionMaxStepSize, GetFullParmName("MaxStepSize") + + " " + fPm->GetUnitOfParameter(GetFullParmName("MaxStepSize")));

			G4VSolid* DivisionSolid = new G4Tubs(divisionName, fTotalRMin, totalRMax, deltaZ, totalSPhi, deltaPhi);
			G4LogicalVolume* DivisionLog = CreateLogicalVolume(divisionName, envelopeMaterialName, DivisionSolid);
			CreatePhysicalVolume(divisionName, DivisionLog, fEnvelopePhys, kUndefined, fDivisionCounts[0]*fDivisionCounts[1]*fDivisionCounts[2], new TsParameterisation(this));
			DivisionLog->SetVisAttributes(GetVisAttributes(""));

			// Precalculate rotation matrices for all phi rotations.
			// Be sure to do this after first placement so that these rotations do not appear as fRotations(0).
			for (int iPhi=0; iPhi<fDivisionCounts[1]; iPhi++) {
				G4RotationMatrix* rot = new G4RotationMatrix();
				rot->rotateZ( - iPhi * fFullWidths[1] / fDivisionCounts[1] );
				fPhiDivisionRotations.push_back(rot);

				// Also save into virtual geometry component's overall list of rotation vectors,
				// used when update method needs to delete all rotation vectors.
				RegisterRotation(rot);
			}
		}
	}

	// Instantiate children unless this was the World component.
	// The World's children are constructed in a separate call from the Geometry Manager.
	// This makes the World available to Geant4 when our other constructions ask Geant4 for parallel worlds.
	if (fParentVolume)
		InstantiateChildren();

  	return fEnvelopePhys;
}


G4int TsCylinder::GetIndex(G4Step* aStep) {
	if (fDivisionCounts[0] * fDivisionCounts[1] * fDivisionCounts[2] == 1) {
		return 0;
	} else if (fDivisionCounts[0] == 1 && fDivisionCounts[1] == 1) {
		const G4VTouchable* touchable = aStep->GetPreStepPoint()->GetTouchable();
		G4int iZ = touchable->GetReplicaNumber(0);

		if (iZ < 0 || iZ >= fDivisionCounts[2]) {
			fPm->GetSequenceManager()->NoteIndexError(aStep->GetTotalEnergyDeposit(),
													  GetNameWithCopyId(), "Z", iZ, fDivisionCounts[2] - 1);
			return -1;
		}

		return iZ;
	} else {
		const G4VTouchable* touchable = aStep->GetPreStepPoint()->GetTouchable();

		if (IsParameterized() && !aStep->GetPreStepPoint()->GetTouchable()->GetVolume()->IsParameterised()) {
			fPm->GetSequenceManager()->NoteParameterizationError(aStep->GetTotalEnergyDeposit(),
																	 GetNameWithCopyId(),
																	 aStep->GetPreStepPoint()->GetTouchable()->GetVolume()->GetName());
			return -1;
		}

		G4int index = touchable->GetReplicaNumber(0);

		if (index < 0 || index >= fDivisionCounts[0] * fDivisionCounts[1] * fDivisionCounts[2]) {
			fPm->GetSequenceManager()->NoteIndexError(aStep->GetTotalEnergyDeposit(),
													  GetNameWithCopyId(), "", index,
													  fDivisionCounts[0] * fDivisionCounts[1] * fDivisionCounts[2] - 1);
			return -1;
		}

		return index;
	}
}


G4int TsCylinder::GetIndex(G4int iR, G4int iPhi, G4int iZ) {
	if (fDivisionCounts[0] * fDivisionCounts[1] * fDivisionCounts[2] == 1) {
		return 0;
	} else {
		return iR * fDivisionCounts[1] * fDivisionCounts[2] + iPhi * fDivisionCounts[2] + iZ;
	}
}


G4int TsCylinder::GetBin(G4int index, G4int iBin) {
	G4int binR = int( index / ( fDivisionCounts[1] * fDivisionCounts[2] ) );
	if (iBin==0) return binR;

	G4int binPhi = int( ( index - binR * fDivisionCounts[1] * fDivisionCounts[2] ) / fDivisionCounts[2]);
	if (iBin==1) return binPhi;

	G4int binZ = index - binR * fDivisionCounts[1] * fDivisionCounts[2] - binPhi * fDivisionCounts[2];
	if (iBin==2) return binZ;

	return -1;
}


TsVGeometryComponent::SurfaceType TsCylinder::GetSurfaceID(G4String surfaceName) {
	SurfaceType surfaceID;
	G4String surfaceNameLower = surfaceName;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(surfaceNameLower);
#else
	surfaceNameLower.toLower();
#endif
	if (surfaceNameLower=="zplussurface")
		surfaceID = ZPlusSurface;
	else if (surfaceNameLower=="zminussurface")
		surfaceID = ZMinusSurface;
	else if (surfaceNameLower=="outercurvedsurface")
		surfaceID = OuterCurvedSurface;
	else if (surfaceNameLower=="innercurvedsurface")
		surfaceID = InnerCurvedSurface;
	else if (surfaceNameLower=="phiplussurface")
		surfaceID = PhiPlusSurface;
	else if (surfaceNameLower=="phiminussurface")
		surfaceID = PhiMinusSurface;
	else if (surfaceNameLower=="anysurface")
		surfaceID = AnySurface;
	else {
		surfaceID = None;
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Scorer name: " << GetName() << " has unknown surface name: " << surfaceName << G4endl;
		fPm->AbortSession(1);
	}
	return surfaceID;
}


G4bool TsCylinder::IsOnBoundary(G4ThreeVector localpos, G4VSolid* solid, SurfaceType surfaceID) {
	G4double kCarTolerance = G4GeometryTolerance::GetInstance()->GetSurfaceTolerance();

	switch (surfaceID) {

		case AnySurface:
			return true;

		case ZPlusSurface:
			return (std::fabs(localpos.z() - ((G4Tubs*)(solid))->GetZHalfLength()) < kCarTolerance);

		case ZMinusSurface:
			return (std::fabs(localpos.z() + ((G4Tubs*)(solid))->GetZHalfLength()) < kCarTolerance);

		case OuterCurvedSurface: {
			G4double localR2 = localpos.perp2();
			G4double radius = ((G4Tubs*)(solid))->GetOuterRadius();
			return (localR2 > (radius-kCarTolerance)*(radius-kCarTolerance)
					&& localR2 < (radius+kCarTolerance)*(radius+kCarTolerance));
		}

		case InnerCurvedSurface: {
			G4double localR2 = localpos.perp2();
			G4double radius = ((G4Tubs*)(solid))->GetInnerRadius();
			return (localR2 > (radius-kCarTolerance)*(radius-kCarTolerance)
					&& localR2 < (radius+kCarTolerance)*(radius+kCarTolerance));
		}

		case PhiPlusSurface: {
			G4double localPhi = localpos.phi();
			G4double sPhi = ((G4Tubs*)(solid))->GetStartPhiAngle();
			G4double dPhi = ((G4Tubs*)(solid))->GetDeltaPhiAngle();
			return (localPhi > (sPhi + dPhi - kCarTolerance)
					&& localPhi < (sPhi + dPhi + kCarTolerance));
		}

		case PhiMinusSurface: {
			G4double localPhi = localpos.phi();
			G4double sPhi = ((G4Tubs*)(solid))->GetStartPhiAngle();
			return (localPhi > (sPhi-kCarTolerance)
					&& localPhi < (sPhi+kCarTolerance));
		}

		default:
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "TsCylinder::IsOnBoundary called for unknown surface of component: " << fName << G4endl;
			fPm->AbortSession(1);
			return false;
	}
}


G4double TsCylinder::GetAreaOfSelectedSurface(G4VSolid* solid, SurfaceType surfaceID, G4int, G4int, G4int) {
	G4double r_inner   = ((G4Tubs*)(solid))->GetInnerRadius();
	G4double r_outer   = ((G4Tubs*)(solid))->GetOuterRadius();
	G4double delta_phi = ((G4Tubs*)(solid))->GetDeltaPhiAngle();
	G4double delta_z   = 2. * ((G4Tubs*)(solid))->GetZHalfLength();

	switch (surfaceID) {

		case ZPlusSurface:
		case ZMinusSurface:
			return 0.5 * delta_phi * (r_outer*r_outer - r_inner*r_inner);

		case OuterCurvedSurface:
			return delta_phi * r_outer * delta_z;

		case InnerCurvedSurface:
			return delta_phi * r_inner * delta_z;

		case PhiPlusSurface:
		case PhiMinusSurface:
			return (r_outer - r_inner) * delta_z;

		case AnySurface:
			return (delta_phi * (r_outer*r_outer - r_inner*r_inner))
				 + (delta_phi * r_outer * delta_z)
				 + (delta_phi * r_inner * delta_z)
				 + (2. * (r_outer - r_inner) * delta_z);

		default:
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "TsCylinder::GetAreaOfSelectedSurface called for unknown surface of component: " << fName << G4endl;
			fPm->AbortSession(1);
			return 0.;
	}
}


G4Material* TsCylinder::ComputeMaterial(const G4int repNo, G4VPhysicalVolume* pvol, const G4VTouchable* parent) {
	if (parent==0)
		G4cerr << "TsCylinder::ComputeMaterial called with parent touchable zero for repNo: " << repNo << ", pvol: " << pvol->GetName() << G4endl;

	//G4cout << "TsCylinder::ComputeMaterial called with repNo: " << repNo << ", pvol: " << pvol << ", parent: " << parent << G4endl;
	if (parent==0 || !fHasVariableMaterial) return pvol->GetLogicalVolume()->GetMaterial();

	unsigned int matIndex = (*fCurrentMaterialIndex)[repNo];

	G4Material* material = (*fMaterialList)[ matIndex ];

	pvol->GetLogicalVolume()->SetVisAttributes(fPm->GetColor(GetDefaultMaterialColor(material->GetName())));

	return material;
}


void TsCylinder::ComputeTransformation(const G4int copyNo, G4VPhysicalVolume* physVol) const
{
	// copyNo = iR * nBinsPhi * nBinsZ + iPhi * nBinsZ + iZ
	G4int iR = int( copyNo / ( fDivisionCounts[1] * fDivisionCounts[2] ) );
	G4int iPhi = int( ( copyNo - iR * fDivisionCounts[1] * fDivisionCounts[2] ) / fDivisionCounts[2] );
	G4int iZ = copyNo - iR * fDivisionCounts[1] * fDivisionCounts[2] - iPhi * fDivisionCounts[2];

	if (fPhiDivisionRotations.size()>0) physVol->SetRotation(fPhiDivisionRotations[iPhi]);

	if (fDivisionCounts[2]>1) {
		G4double transZ = (iZ + 0.5) * fFullWidths[2] / fDivisionCounts[2] - fFullWidths[2] / 2;
		G4ThreeVector position(0.,0., transZ);
		physVol->SetTranslation(position);
	}
}


void TsCylinder::ComputeDimensions(G4Tubs& tubs, const G4int copyNo, const G4VPhysicalVolume* pvol) const
{
	if (copyNo < 0) {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "TsCylinder::ComputeDimensions called for volume: " << pvol->GetName() << " with invalid copyNo: " << copyNo << G4endl;
		fPm->AbortSession(1);
	}

	// copyNo = iR * nBinsPhi * nBinsZ + iPhi * nBinsZ + iZ
	G4int iR = int( copyNo / ( fDivisionCounts[1] * fDivisionCounts[2] ) );

	tubs.SetInnerRadius( fTotalRMin + iR * fFullWidths[0] / fDivisionCounts[0]);
	tubs.SetOuterRadius( fTotalRMin + (iR+1) * fFullWidths[0] / fDivisionCounts[0] );
}


void TsCylinder::CreateDefaults(TsParameterManager* pM, G4String& childName, G4String&) {
	G4String parameterName;
	G4String transValue;

	parameterName = "dc:Ge/" + childName + "/RMax";
	transValue = "80. cm";
	pM->AddParameter(parameterName, transValue);

	parameterName = "dc:Ge/" + childName + "/RMin";
	transValue = "0. cm";
	pM->AddParameter(parameterName, transValue);

	parameterName = "dc:Ge/" + childName + "/HL";
	transValue = "20. cm";
	pM->AddParameter(parameterName, transValue);

	parameterName = "ic:Ge/" + childName + "/RBins";
	transValue = "1";
	pM->AddParameter(parameterName, transValue);

	parameterName = "ic:Ge/" + childName + "/PhiBins";
	transValue = "1";
	pM->AddParameter(parameterName, transValue);

	parameterName = "ic:Ge/" + childName + "/ZBins";
	transValue = "1";
	pM->AddParameter(parameterName, transValue);

	parameterName = "dc:Ge/" + childName + "/SPhi";
	transValue = "0. deg";
	pM->AddParameter(parameterName, transValue);

	parameterName = "dc:Ge/" + childName + "/DPhi";
	transValue = "360. deg";
	pM->AddParameter(parameterName, transValue);
}
