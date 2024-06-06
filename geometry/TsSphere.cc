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

#include "TsSphere.hh"

#include "TsParameterManager.hh"
#include "TsSequenceManager.hh"

#include "TsParameterisation.hh"

#include "G4Step.hh"
#include "G4GeometryTolerance.hh"
#include "G4SystemOfUnits.hh"

TsSphere::TsSphere(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
							 TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name) :
TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name)
{
	fIsDividable = true;
	fCanCalculateSurfaceArea = true;
	fHasDifferentVolumePerDivision = true;

	fDivisionNames[0] = "R";
	fDivisionNames[1] = "Phi";
	fDivisionNames[2] = "Theta";
	fDivisionUnits[0] = "cm";
	fDivisionUnits[1] = "deg";
	fDivisionUnits[2] = "deg";
}


TsSphere::~TsSphere()
{
}


G4VPhysicalVolume* TsSphere::Construct()
{
	BeginConstruction();

	fPhiDivisionRotations.clear();
	fThetaAreaRatios.clear();

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

	if (fDivisionCounts[0] > 1 || fDivisionCounts[1] > 1 || fDivisionCounts[2] > 1)
		SetHasDividedCylinderOrSphere();

	if (fPm->ParameterExists(GetFullParmName("RMin")))
		fTotalRMin = fPm->GetDoubleParameter(GetFullParmName("RMin"),"Length");
	else
		fTotalRMin = 0.;

	G4double totalRMax = fPm->GetDoubleParameter(GetFullParmName("RMax"), "Length");

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

	if (fPm->ParameterExists(GetFullParmName("STheta")))
		fTotalSTheta = fPm->GetDoubleParameter(GetFullParmName("STheta"),"Angle");
	else
		fTotalSTheta = 0. * deg;

	G4double totalDTheta;
	if (fPm->ParameterExists(GetFullParmName("DTheta")))
		totalDTheta = fPm->GetDoubleParameter(GetFullParmName("DTheta"),"Angle");
	else
		totalDTheta = 180. * deg;

	if (fTotalRMin < 0.          ) OutOfRange( GetFullParmName("RMin"), "can not be negative");
	if (totalRMax < fTotalRMin   ) OutOfRange( GetFullParmName("RMax"), "can not be less than RMin");
	if (totalSPhi < 0.           ) OutOfRange( GetFullParmName("SPhi"), "can not be negative");
	if (totalSPhi > 360. * deg   ) OutOfRange( GetFullParmName("SPhi"), "can not be greater than 360 degrees");
	if (totalDPhi < 0.           ) OutOfRange( GetFullParmName("DPhi"), "can not be negative");
	if (totalDPhi > 360. * deg   ) OutOfRange( GetFullParmName("DPhi"), "can not be greater than 360 degrees");
	if (fTotalSTheta < 0.        ) OutOfRange( GetFullParmName("STheta"), "can not be negative");
	if (fTotalSTheta > 180. * deg) OutOfRange( GetFullParmName("STheta"), "can not be greater than 180 degrees");
	if (totalDTheta < 0.         ) OutOfRange( GetFullParmName("DTheta"), "can not be negative");
	if (totalDTheta > 180. * deg ) OutOfRange( GetFullParmName("DTheta"), "can not be greater than 180 degrees");

	fFullWidths[0] = totalRMax - fTotalRMin;
	fFullWidths[1] = totalDPhi;
	fFullWidths[2] = totalDTheta;

	G4Sphere* envelopeSolid = new G4Sphere(fName, fTotalRMin, totalRMax, totalSPhi, totalDPhi, fTotalSTheta, totalDTheta);
	fEnvelopeLog = CreateLogicalVolume(envelopeSolid);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

	G4int nDivisions = fDivisionCounts[0] * fDivisionCounts[1] * fDivisionCounts[2];
	if (nDivisions > 1) {
		G4String envelopeMaterialName = GetResolvedMaterialName();

		// Calculate divisions
		G4double deltaPhi = totalDPhi / fDivisionCounts[1];
		G4double deltaTheta = totalDTheta / fDivisionCounts[2];

		G4String divisionName;

		// Use Parameterisation for all three divisions
		divisionName = fName + "_Division";

		// If necessary, propagate MaxStepSize to divisions
		G4String ParmNameForDivisionMaxStepSize = "Ge/" + fName + "/" + divisionName + "/MaxStepSize";
		if ((!fPm->ParameterExists(ParmNameForDivisionMaxStepSize)) &&
			fPm->ParameterExists(GetFullParmName("MaxStepSize")))
			fPm->AddParameter("d:" + ParmNameForDivisionMaxStepSize, GetFullParmName("MaxStepSize") + + " " + fPm->GetUnitOfParameter(GetFullParmName("MaxStepSize")));

		G4VSolid* divisionSolid = new G4Sphere(divisionName, fTotalRMin, totalRMax, totalSPhi, deltaPhi, fTotalSTheta, deltaTheta);
		G4LogicalVolume* divsionLog = CreateLogicalVolume(divisionName, envelopeMaterialName, divisionSolid);
		CreatePhysicalVolume(divisionName, divsionLog, fEnvelopePhys, kUndefined, nDivisions, new TsParameterisation(this));
		divsionLog->SetVisAttributes(GetVisAttributes(""));

		fScoringVolume = divsionLog;
	}

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

	PreCalculateThetaRatios();

	// Instantiate children unless this was the World component.
	// The World's children are constructed in a separate call from the Geometry Manager.
	// This makes the World available to Geant4 when our other constructions ask Geant4 for parallel worlds.
	if (fParentVolume)
		InstantiateChildren();

  	return fEnvelopePhys;
}


G4int TsSphere::GetIndex(G4Step* aStep) {
	if (fDivisionCounts[0] * fDivisionCounts[1] * fDivisionCounts[2] == 1) {
		return 0;
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


G4int TsSphere::GetIndex(G4int iR, G4int iPhi, G4int iTheta) {
	if (fDivisionCounts[0] * fDivisionCounts[1] * fDivisionCounts[2] == 1) {
		return 0;
	} else {
		return iR * fDivisionCounts[1] * fDivisionCounts[2] + iPhi * fDivisionCounts[2] + iTheta;
	}
}


G4int TsSphere::GetBin(G4int index, G4int iBin) {
	G4int binR = int( index / ( fDivisionCounts[1] * fDivisionCounts[2] ) );
	if (iBin==0) return binR;

	G4int binPhi = int( ( index - binR * fDivisionCounts[1] * fDivisionCounts[2] ) / fDivisionCounts[2]);
	if (iBin==1) return binPhi;

	G4int binTheta = index - binR * fDivisionCounts[1] * fDivisionCounts[2] - binPhi * fDivisionCounts[2];
	if (iBin==2) return binTheta;

	return -1;
}


TsVGeometryComponent::SurfaceType TsSphere::GetSurfaceID(G4String surfaceName) {
	SurfaceType surfaceID;
	G4String surfaceNameLower = surfaceName;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(surfaceNameLower);
#else 
	surfaceNameLower.toLower();
#endif
	if (surfaceNameLower=="outercurvedsurface")
		surfaceID = OuterCurvedSurface;
	else if (surfaceNameLower=="innercurvedsurface")
		surfaceID = InnerCurvedSurface;
	else if (surfaceNameLower=="phiplussurface")
		surfaceID = PhiPlusSurface;
	else if (surfaceNameLower=="phiminussurface")
		surfaceID = PhiMinusSurface;
	else if (surfaceNameLower=="thetaplussurface")
		surfaceID = ThetaPlusSurface;
	else if (surfaceNameLower=="thetaminussurface")
		surfaceID = ThetaMinusSurface;
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


G4bool TsSphere::IsOnBoundary(G4ThreeVector localpos, G4VSolid* solid, SurfaceType surfaceID) {
	G4double kCarTolerance = G4GeometryTolerance::GetInstance()->GetSurfaceTolerance();

	switch (surfaceID) {

		case AnySurface:
			return true;

		case OuterCurvedSurface: {
			G4double localR2 = localpos.mag2();
			G4double radius = ((G4Sphere*)(solid))->GetOuterRadius();
			return (localR2 > (radius-kCarTolerance)*(radius-kCarTolerance)
					&& localR2 < (radius+kCarTolerance)*(radius+kCarTolerance));
		}

		case InnerCurvedSurface: {
			G4double localR2 = localpos.mag2();
			G4double radius = ((G4Sphere*)(solid))->GetInnerRadius();
			return (localR2 > (radius-kCarTolerance)*(radius-kCarTolerance)
					&& localR2 < (radius+kCarTolerance)*(radius+kCarTolerance));
		}

		case PhiPlusSurface: {
			G4double localPhi = localpos.phi();
			G4double sPhi = ((G4Sphere*)(solid))->GetStartPhiAngle();
			G4double dPhi = ((G4Sphere*)(solid))->GetDeltaPhiAngle();
			return (localPhi > (sPhi + dPhi - kCarTolerance)
					&& localPhi < (sPhi + dPhi + kCarTolerance));
		}

		case PhiMinusSurface: {
			G4double localPhi = localpos.phi();
			G4double sPhi = ((G4Sphere*)(solid))->GetStartPhiAngle();
			return (localPhi > (sPhi - kCarTolerance)
					&& localPhi < (sPhi + kCarTolerance));
		}

		case ThetaPlusSurface: {
			G4double localTheta = localpos.theta();
			G4double sTheta = ((G4Sphere*)(solid))->GetStartThetaAngle();
			G4double dTheta = ((G4Sphere*)(solid))->GetDeltaThetaAngle();
			return (localTheta > (sTheta + dTheta - kCarTolerance)
					&& localTheta < (sTheta + dTheta + kCarTolerance));
		}

		case ThetaMinusSurface: {
			G4double localTheta = localpos.theta();
			G4double sTheta = ((G4Sphere*)(solid))->GetStartThetaAngle();
			return (localTheta > (sTheta - kCarTolerance)
					&& localTheta < (sTheta + kCarTolerance));
		}

		default:
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "TsSphere::IsOnBoundary called for unknown surface of component: " << fName << G4endl;
			fPm->AbortSession(1);
			return false;
	}
}


G4double TsSphere::GetAreaOfSelectedSurface(G4VSolid* solid, SurfaceType surfaceID, G4int, G4int, G4int copyNo) {
	// Third index is the copy number. Other two indices are no used for the Sphere.
	// copyNo = iR * nBinsPhi * nBinsTheta + iPhi * nBinsTheta + iTheta
	G4int iR = int( copyNo / ( fDivisionCounts[1] * fDivisionCounts[2] ) );
	G4int iPhi = int( ( copyNo - iR * fDivisionCounts[1] * fDivisionCounts[2] ) / fDivisionCounts[2] );
	G4int iTheta = copyNo - iR * fDivisionCounts[1] * fDivisionCounts[2] - iPhi * fDivisionCounts[2];

	//G4cout << "In GetAreaOfSelectedSurface for iTheta: " << iTheta << " using fThetaAreaRatios[iTheta]: " << fThetaAreaRatios[iTheta] << G4endl;

	G4double r_inner     = ((G4Sphere*)(solid))->GetInnerRadius();
	G4double r_outer     = ((G4Sphere*)(solid))->GetOuterRadius();
	G4double delta_phi   = ((G4Sphere*)(solid))->GetDeltaPhiAngle();
	G4double delta_theta = ((G4Sphere*)(solid))->GetDeltaThetaAngle();
	G4double theta_start = ((G4Sphere*)(solid))->GetStartThetaAngle();
	G4double theta_end   = theta_start + delta_theta;

	switch (surfaceID) {
	case OuterCurvedSurface:
		return fThetaAreaRatios[iTheta] * 2. * delta_phi * r_outer*r_outer;

	case InnerCurvedSurface:
		return fThetaAreaRatios[iTheta] * 2. * delta_phi * r_inner*r_inner;

	case PhiPlusSurface:
	case PhiMinusSurface:
		return 0.5 * delta_theta * (r_outer*r_outer - r_inner*r_inner);

	case ThetaPlusSurface:
		return 0.5 * sin(theta_start) * delta_phi * (r_outer*r_outer - r_inner*r_inner);

	case ThetaMinusSurface:
		return 0.5 * sin(theta_end) * delta_phi * (r_outer*r_outer - r_inner*r_inner);

	case AnySurface:
		return (fThetaAreaRatios[iTheta] * 2. * delta_phi * r_outer*r_outer)
			 + (fThetaAreaRatios[iTheta] * 2. * delta_phi * r_inner*r_inner)
			 + (delta_theta * (r_outer*r_outer - r_inner*r_inner))
			 + (0.5 * sin(theta_start) * delta_phi * (r_outer*r_outer - r_inner*r_inner))
			 + (0.5 * sin(theta_end) * delta_phi * (r_outer*r_outer - r_inner*r_inner));

	default:
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "TsSphere::GetAreaOfSelectedSurface called for unknown surface of component: " << fName << G4endl;
		fPm->AbortSession(1);
		return 0.;
	}
}


void TsSphere::PreCalculateThetaRatios() {
	// Precalculate ratios of theta bin surface areas.
	// Do so by subtracting area of the spherical cap defined by smaller theta boundary
	// from area of the spherical cap defined by the larger theta boundary.
	// If odd number of bins, the middle bin is special. For this one, we calculate half the value then double it.
	//
	// Area of Spherical Cap = 2 Pi R (1 - Sin Theta)
	// Area of Sphere = 4 Pi R
	// Since we only need ratio of spherical cap over sphere, use:
	// Area Factor = (1/2) (1 - Sin Theta)
	//
	G4double halfWidthTheta = fFullWidths[2] / (2. * fDivisionCounts[2] );

	G4double theta;
	G4double areaFactorOfSmallerCap;
	G4double areaFactorOfLargerCap = .5;
	G4double areaFactorOfBin;

	G4bool isOdd;
	if (fDivisionCounts[2]%2 == 0) isOdd = false;
	else isOdd = true;

	G4int startTheta = 1;
	if (isOdd) startTheta = 0;

	G4int middleBin = fDivisionCounts[2] / 2;
	fThetaAreaRatios.resize(fDivisionCounts[2],0);

	for (G4int iTheta = startTheta; iTheta < fDivisionCounts[2]/2. + .5; iTheta++) {
		theta = fTotalSTheta + 2. * iTheta * halfWidthTheta;
		if (isOdd) theta += halfWidthTheta;

		areaFactorOfSmallerCap = (1./2.) * (1. - sin(theta) );
		areaFactorOfBin = areaFactorOfLargerCap - areaFactorOfSmallerCap;

		// If odd number of bins, double size of center bin since it actually crosses the equator.
		if (iTheta==0)
			areaFactorOfBin*=2.;

		/*G4cout << "TsSphere::PreCalculateThetaRatios Filling theta bin: " << iTheta << G4endl;
		G4cout << "Theta: " << theta << G4endl;
		G4cout << "areaFactorOfSmallerCap: " << areaFactorOfSmallerCap << G4endl;
		G4cout << "areaFactorOfLargerCap: " << areaFactorOfLargerCap << G4endl;
		G4cout << "areaFactorOfBin: " << areaFactorOfBin << G4endl;
		*/

		if (iTheta==0) {
			fThetaAreaRatios[middleBin] = areaFactorOfBin;
		} else {
			fThetaAreaRatios[middleBin - iTheta] = areaFactorOfBin;
			if (isOdd) fThetaAreaRatios[middleBin + iTheta] = areaFactorOfBin;
			else       fThetaAreaRatios[middleBin + iTheta - 1] = areaFactorOfBin;
		}

		areaFactorOfLargerCap = areaFactorOfSmallerCap;
	}
}


G4Material* TsSphere::ComputeMaterial(const G4int repNo, G4VPhysicalVolume* pvol, const G4VTouchable* parent) {
	if (parent==0)
		G4cerr << "TsSphere::ComputeMaterial called with parent touchable zero for repNo: " << repNo << ", pvol: " << pvol->GetName() << G4endl;

	//G4cout << "TsSphere::ComputeMaterial called with repNo: " << repNo << ", pvol: " << pvol << ", parent: " << parent << G4endl;
	if (parent==0 || !fHasVariableMaterial) return pvol->GetLogicalVolume()->GetMaterial();

	unsigned int matIndex = (*fCurrentMaterialIndex)[repNo];

	G4Material* material = (*fMaterialList)[ matIndex ];

	pvol->GetLogicalVolume()->SetVisAttributes(fPm->GetColor(GetDefaultMaterialColor(material->GetName())));

	return material;
}


void TsSphere::ComputeTransformation(const G4int copyNo, G4VPhysicalVolume* physVol) const
{
	// copyNo = iR * nBinsPhi * nBinsTheta + iPhi * nBinsTheta + iTheta
	G4int iR = int( copyNo / ( fDivisionCounts[1] * fDivisionCounts[2] ) );
	G4int iPhi = int( ( copyNo - iR * fDivisionCounts[1] * fDivisionCounts[2] ) / fDivisionCounts[2] );

	if (fPhiDivisionRotations.size()>0) physVol->SetRotation(fPhiDivisionRotations[iPhi]);
}


void TsSphere::ComputeDimensions(G4Sphere& sphere, const G4int copyNo, const G4VPhysicalVolume*) const
{
	// copyNo = iR * nBinsPhi * nBinsTheta + iPhi * nBinsTheta + iTheta
	G4int iR = int( copyNo / ( fDivisionCounts[1] * fDivisionCounts[2] ) );
	G4int iPhi = int( ( copyNo - iR * fDivisionCounts[1] * fDivisionCounts[2] ) / fDivisionCounts[2] );
	G4int iTheta = copyNo - iR * fDivisionCounts[1] * fDivisionCounts[2] - iPhi * fDivisionCounts[2];

#if GEANT4_VERSION_MAJOR >= 11
	sphere.SetInnerRadius( fTotalRMin + iR * fFullWidths[0] / fDivisionCounts[0] );
#else
	sphere.SetInsideRadius( fTotalRMin + iR * fFullWidths[0] / fDivisionCounts[0] );
#endif
	sphere.SetOuterRadius( fTotalRMin + (iR+1) * fFullWidths[0] / fDivisionCounts[0] );
	sphere.SetStartThetaAngle( fTotalSTheta + iTheta * fFullWidths[2] / fDivisionCounts[2] );
}


void TsSphere::CreateDefaults(TsParameterManager* pM, G4String& childName, G4String&) {
	G4String parameterName;
	G4String transValue;

	parameterName = "dc:Ge/" + childName + "/RMax";
	transValue = "80. cm";
	pM->AddParameter(parameterName, transValue);

	parameterName = "dc:Ge/" + childName + "/RMin";
	transValue = "0. cm";
	pM->AddParameter(parameterName, transValue);

	parameterName = "ic:Ge/" + childName + "/RBins";
	transValue = "1";
	pM->AddParameter(parameterName, transValue);

	parameterName = "ic:Ge/" + childName + "/PhiBins";
	transValue = "1";
	pM->AddParameter(parameterName, transValue);

	parameterName = "ic:Ge/" + childName + "/ThetaBins";
	transValue = "1";
	pM->AddParameter(parameterName, transValue);

	parameterName = "dc:Ge/" + childName + "/SPhi";
	transValue = "0. deg";
	pM->AddParameter(parameterName, transValue);

	parameterName = "dc:Ge/" + childName + "/DPhi";
	transValue = "360. deg";
	pM->AddParameter(parameterName, transValue);

	parameterName = "dc:Ge/" + childName + "/STheta";
	transValue = "0. deg";
	pM->AddParameter(parameterName, transValue);

	parameterName = "dc:Ge/" + childName + "/DTheta";
	transValue = "180. deg";
	pM->AddParameter(parameterName, transValue);
}
