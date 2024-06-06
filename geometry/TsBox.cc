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

#include "TsBox.hh"

#include "TsParameterManager.hh"
#include "TsMaterialManager.hh"
#include "TsGeometryManager.hh"
#include "TsSequenceManager.hh"

#include "TsParameterisation.hh"

#include "G4Box.hh"
#include "G4Step.hh"
#include "G4VTouchable.hh"
#include "G4GeometryTolerance.hh"

TsBox::TsBox(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
			 TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name) :
TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name),
fShowOnlyOutline(false), fConstructParameterized(false),
fShowSpecificSlicesX(0), fShowSpecificSlicesY(0), fShowSpecificSlicesZ(0)
{
	fIsDividable = true;
	fCanCalculateSurfaceArea = true;

	fDivisionNames[0] = "X";
	fDivisionNames[1] = "Y";
	fDivisionNames[2] = "Z";
	fDivisionUnits[0] = "cm";
	fDivisionUnits[1] = "cm";
	fDivisionUnits[2] = "cm";
}


TsBox::~TsBox()
{
}


G4VPhysicalVolume* TsBox::Construct()
{
	BeginConstruction();

	if (!fIsCopy) {
		for (G4int i = 0; i<3; i++) {
			if (fPm->ParameterExists(GetBinParmName(i))) {
				fDivisionCounts[i] = fPm->GetIntegerParameter(GetBinParmName(i));
				if (fDivisionCounts[i] <= 0.) OutOfRange( GetBinParmName(i), "must be larger than zero");
			}
		}
	}

	if (fPm->ParameterExists(GetFullParmName("ConstructParameterized")))
		fConstructParameterized = fPm->GetBooleanParameter(GetFullParmName("ConstructParameterized"));

	if (fPm->ParameterExists(GetFullParmName("VoxelMaterials"))) {
		G4int nDivisions = fDivisionCounts[0] * fDivisionCounts[1] * fDivisionCounts[2];
		if (fPm->GetVectorLength(GetFullParmName("VoxelMaterials")) != nDivisions) {
			G4cerr << "Topas is exiting due to a serious error in geometry setup." << G4endl;
			G4cerr << GetName() << " has " << nDivisions << " voxels," << G4endl;
			G4cerr << "but " << GetFullParmName("VoxelMaterials") << " has length of " << fPm->GetVectorLength(GetFullParmName("VoxelMaterials")) << G4endl;
			fPm->AbortSession(1);
		}
		fConstructParameterized = true;
	}

	G4double totalHLX = fPm->GetDoubleParameter(GetFullParmName("HLX"),"Length");
	G4double totalHLY = fPm->GetDoubleParameter(GetFullParmName("HLY"),"Length");
	G4double totalHLZ = fPm->GetDoubleParameter(GetFullParmName("HLZ"),"Length");

	if (totalHLX <= 0.) OutOfRange( GetFullParmName("HLX"), "must be larger than zero");
	if (totalHLY <= 0.) OutOfRange( GetFullParmName("HLY"), "must be larger than zero");
	if (totalHLZ <= 0.) OutOfRange( GetFullParmName("HLZ"), "must be larger than zero");

	fFullWidths[0] = 2. * totalHLX;
	fFullWidths[1] = 2. * totalHLY;
	fFullWidths[2] = 2. * totalHLZ;

	G4double voxelSizeX = fFullWidths[0] / fDivisionCounts[0];
	G4double voxelSizeY = fFullWidths[1] / fDivisionCounts[1];
	G4double voxelSizeZ = fFullWidths[2] / fDivisionCounts[2];
	fTransFirstVoxelCenterRelToComponentCenter = G4Point3D(-0.5*fFullWidths[0] + 0.5*voxelSizeX,
														   -0.5*fFullWidths[1] + 0.5*voxelSizeY,
														   -0.5*fFullWidths[2] + 0.5*voxelSizeZ);

	G4Box* envelopeSolid = new G4Box(fName, totalHLX, totalHLY, totalHLZ);
	fEnvelopeLog = CreateLogicalVolume(envelopeSolid);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

	G4int nDivisions = fDivisionCounts[0] * fDivisionCounts[1] * fDivisionCounts[2];
	if (nDivisions > 1) {
		if (nDivisions > fPm->GetIntegerParameter("Gr/SwitchOGLtoOGLIifVoxelCountExceeds"))
			SetTooComplexForOGLS();

		fShowOnlyOutline = nDivisions > fPm->GetIntegerParameter("Gr/ShowOnlyOutlineIfVoxelCountExceeds");
		if (!fShowOnlyOutline)
			fEnvelopeLog->SetVisAttributes(fPm->GetInvisible());

		ConstructVoxelStructure();
	}

	// Instantiate children unless this was the World component.
	// The World's children are constructed in a separate call from the Geometry Manager.
	// This makes the World available to Geant4 when our other constructions ask Geant4 for parallel worlds.
	if (fParentVolume)
		InstantiateChildren();

	return fEnvelopePhys;
}


void TsBox::ConstructVoxelStructure() {

	G4String envelopeMaterialName = GetResolvedMaterialName();

	// Calculate divisions
	G4double deltaX = fFullWidths[0] / fDivisionCounts[0] / 2.;
	G4double deltaY = fFullWidths[1] / fDivisionCounts[1] / 2.;
	G4double deltaZ = fFullWidths[2] / fDivisionCounts[2] / 2.;

	G4String divisionName;

	// Use Replica for X division
	divisionName = fName + "_X_Division";
	G4VSolid* XDivisionSolid = new G4Box(divisionName, deltaX, fFullWidths[1]/2., fFullWidths[2]/2.);
	G4LogicalVolume* XDivisionLog = CreateLogicalVolume(divisionName, envelopeMaterialName, XDivisionSolid);
	G4VPhysicalVolume* XDivisionPhys = CreatePhysicalVolume(divisionName,XDivisionLog,fEnvelopePhys,kXAxis,fDivisionCounts[0],deltaX*2.);
	XDivisionLog->SetVisAttributes(fPm->GetInvisible());

	// Use Replica for Y division
	divisionName = fName + "_Y_Division";
	G4VSolid* YDivisionSolid = new G4Box(divisionName, deltaX, deltaY, fFullWidths[2]/2.);
	G4LogicalVolume* YDivisionLog = CreateLogicalVolume(divisionName, envelopeMaterialName, YDivisionSolid);
	G4VPhysicalVolume* YDivisionPhys = CreatePhysicalVolume(divisionName,YDivisionLog,XDivisionPhys,kYAxis,fDivisionCounts[1],deltaY*2.);
	YDivisionLog->SetVisAttributes(fPm->GetInvisible());

	// Use either Replica or Parameterization for Z division
	divisionName = fName + "_Z_Division";

	// If necessary, propagate MaxStepSize to divisions
	G4String ParmNameForDivisionMaxStepSize = "Ge/" + fName + "/" + divisionName + "/MaxStepSize";
	if ((!fPm->ParameterExists(ParmNameForDivisionMaxStepSize)) &&
		fPm->ParameterExists(GetFullParmName("MaxStepSize")))
		fPm->AddParameter("d:" + ParmNameForDivisionMaxStepSize, GetFullParmName("MaxStepSize") + + " " + fPm->GetUnitOfParameter(GetFullParmName("MaxStepSize")));

	G4VSolid* ZDivisionSolid = new G4Box(divisionName,deltaX, deltaY, deltaZ);
	G4LogicalVolume* ZDivisionLog = CreateLogicalVolume(divisionName, envelopeMaterialName, ZDivisionSolid);

	// Since parameterization is not reliable in parallel worlds, only parameterize if really necessary.
	if (fConstructParameterized)
		CreatePhysicalVolume(divisionName, ZDivisionLog, YDivisionPhys, kZAxis, fDivisionCounts[2], new TsParameterisation(this));
	else
		CreatePhysicalVolume(divisionName,ZDivisionLog,YDivisionPhys,kZAxis,fDivisionCounts[2],deltaZ*2.);

	if (fShowOnlyOutline)
		ZDivisionLog->SetVisAttributes(fPm->GetInvisible());
	else
		ZDivisionLog->SetVisAttributes(GetVisAttributes(""));

	fScoringVolume = ZDivisionLog;
}


G4int TsBox::GetIndex(G4Step* aStep) {
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

		G4int iX = touchable->GetReplicaNumber(2);
		G4int iY = touchable->GetReplicaNumber(1);
		G4int iZ = touchable->GetReplicaNumber(0);

		if (iX < 0 || iX >= fDivisionCounts[0]) {
			fPm->GetSequenceManager()->NoteIndexError(aStep->GetTotalEnergyDeposit(),
													  GetNameWithCopyId(), "X", iX, fDivisionCounts[0] - 1);
			return -1;
		}

		if (iY < 0 || iY >= fDivisionCounts[1]) {
			fPm->GetSequenceManager()->NoteIndexError(aStep->GetTotalEnergyDeposit(),
													  GetNameWithCopyId(), "Y", iY, fDivisionCounts[1] - 1);
			return -1;
		}

		if (iZ < 0 || iZ >= fDivisionCounts[2]) {
			fPm->GetSequenceManager()->NoteIndexError(aStep->GetTotalEnergyDeposit(),
													  GetNameWithCopyId(), "Z", iZ, fDivisionCounts[2] - 1);
			return -1;
		}

		return iX * fDivisionCounts[1] * fDivisionCounts[2] + iY * fDivisionCounts[2] + iZ;
	}
}


G4int TsBox::GetIndex(G4int iX, G4int iY, G4int iZ) {
	if (fDivisionCounts[0] * fDivisionCounts[1] * fDivisionCounts[2] == 1) {
		return 0;
	} else {
		return iX * fDivisionCounts[1] * fDivisionCounts[2] + iY * fDivisionCounts[2] + iZ;
	}
}


G4int TsBox::GetBin(G4int index, G4int iBin) {
	G4int binX = int( index / ( fDivisionCounts[1] * fDivisionCounts[2] ) );
	if (iBin==0) return binX;

	G4int binY = int( ( index - binX * fDivisionCounts[1] * fDivisionCounts[2] ) / fDivisionCounts[2]);
	if (iBin==1) return binY;

	G4int binZ = index - binX * fDivisionCounts[1] * fDivisionCounts[2] - binY * fDivisionCounts[2];
	if (iBin==2) return binZ;

	return -1;
}


TsVGeometryComponent::SurfaceType TsBox::GetSurfaceID(G4String surfaceName) {
	SurfaceType surfaceID;
	G4String surfaceNameLower = surfaceName;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(surfaceNameLower);
#else
	surfaceNameLower.toLower();
#endif
	if (surfaceNameLower=="xplussurface")
		surfaceID = XPlusSurface;
	else if (surfaceNameLower=="xminussurface")
		surfaceID = XMinusSurface;
	else if (surfaceNameLower=="yplussurface")
		surfaceID = YPlusSurface;
	else if (surfaceNameLower=="yminussurface")
		surfaceID = YMinusSurface;
	else if (surfaceNameLower=="zplussurface")
		surfaceID = ZPlusSurface;
	else if (surfaceNameLower=="zminussurface")
		surfaceID = ZMinusSurface;
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


G4bool TsBox::IsOnBoundary(G4ThreeVector localpos, G4VSolid* solid, SurfaceType surfaceID) {
	G4double kCarTolerance = G4GeometryTolerance::GetInstance()->GetSurfaceTolerance();

	switch(surfaceID) {

		case AnySurface:
			return true;

		case XPlusSurface:
			return (std::fabs(localpos.x() - ((G4Box*)(solid))->GetXHalfLength()) < kCarTolerance);

		case XMinusSurface:
			return (std::fabs(localpos.x() + ((G4Box*)(solid))->GetXHalfLength()) < kCarTolerance);

		case YPlusSurface:
			return (std::fabs(localpos.y() - ((G4Box*)(solid))->GetYHalfLength()) < kCarTolerance);

		case YMinusSurface:
			return (std::fabs(localpos.y() + ((G4Box*)(solid))->GetYHalfLength()) < kCarTolerance);

		case ZPlusSurface:
			return (std::fabs(localpos.z() - ((G4Box*)(solid))->GetZHalfLength()) < kCarTolerance);

		case ZMinusSurface:
			return (std::fabs(localpos.z() + ((G4Box*)(solid))->GetZHalfLength()) < kCarTolerance);

		default:
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "TsBox::IsOnBoundary called for unknown surface of component: " << fName << G4endl;
			fPm->AbortSession(1);
			return false;
	}
}


G4double TsBox::GetAreaOfSelectedSurface(G4VSolid* solid, SurfaceType surfaceID, G4int, G4int, G4int) {

	G4double delta_x = 2. * ((G4Box*)(solid))->GetXHalfLength();
	G4double delta_y = 2. * ((G4Box*)(solid))->GetYHalfLength();
	G4double delta_z = 2. * ((G4Box*)(solid))->GetZHalfLength();

	switch(surfaceID) {
		case XPlusSurface:
		case XMinusSurface:
			return delta_y * delta_z;

		case YPlusSurface:
		case YMinusSurface:
			return delta_x * delta_z;

		case ZPlusSurface:
		case ZMinusSurface:
			return delta_x * delta_y;

		case AnySurface:
			return (2. * (delta_y * delta_z) + (delta_x * delta_z) + (delta_x * delta_y));

		default:
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "TsBox::GetAreaOfSelectedSurface called for unknown surface of component: " << fName << G4endl;
			fPm->AbortSession(1);
			return 0.;
	}
}


G4Material* TsBox::ComputeMaterial(const G4int repNo, G4VPhysicalVolume* pvol, const G4VTouchable* parent) {
	if (parent==0)
		G4cerr << "TsBox::ComputeMaterial called with parent touchable zero for repNo: " << repNo << ", pvol: " << pvol->GetName() << G4endl;

	//G4cout << "TsBox::ComputeMaterial called with repNo: " << repNo << ", pvol: " << pvol->GetName() << ", parent: " << parent << G4endl;
	if (parent==0 || !fHasVariableMaterial) return pvol->GetLogicalVolume()->GetMaterial();

	G4int iX = parent->GetReplicaNumber(1);
	G4int iY = parent->GetReplicaNumber(0);
	G4int iZ = repNo;
	G4int copyNo = iX * fDivisionCounts[1] * fDivisionCounts[2] + iY * fDivisionCounts[2] + iZ;
	unsigned int matIndex = (*fCurrentMaterialIndex)[copyNo];

	G4Material* material = (*fMaterialList)[ matIndex ];

	// Override material if set by structure
	G4bool found = false;
	for (G4int i = 0; i < (int)fMaterialByRTStructMaterials.size() && !found; i++) {
		if (fMaterialByRTStructMaterials[i]) {
			if (fIsInNamedStructure[fImageIndex][fMaterialByRTStructNamesIndexIntoIsInNamedStructure[i]][copyNo]) {
				found = true;
				material = fMaterialByRTStructMaterials[i];
			}
		}
	}

	if (!fShowOnlyOutline) {
		if ((fShowSpecificSlicesX && !fShowSpecificSlicesX[iX]) ||
			(fShowSpecificSlicesY && !fShowSpecificSlicesY[iY]) ||
			(fShowSpecificSlicesZ && !fShowSpecificSlicesZ[iZ]))
			pvol->GetLogicalVolume()->SetVisAttributes(fPm->GetInvisible());
		else {
			pvol->GetLogicalVolume()->SetVisAttributes(fPm->GetColor(GetDefaultMaterialColor(material->GetName())));

	        // Override color if set by structure
	        for (G4int i = 0; i < (int)fStructureColors.size(); i++)
	            if (fStructureColors[i])
	                if (fIsInNamedStructure[fImageIndex][i][copyNo])
	                    pvol->GetLogicalVolume()->SetVisAttributes(fStructureColors[i]);
	    }
	}

	return material;
}


void TsBox::ComputeTransformation(const G4int copyNo, G4VPhysicalVolume* pvol) const {
	// copyNo = iZ
	G4double transZ = (copyNo + 0.5) * fFullWidths[2] / fDivisionCounts[2] - fFullWidths[2] / 2;
	G4ThreeVector position(0.,0., transZ);
	pvol->SetTranslation(position);
}


void TsBox::SetShowSpecificSlicesX(G4bool* showSpecificSlices) {
	fShowSpecificSlicesX = showSpecificSlices;
}


void TsBox::SetShowSpecificSlicesY(G4bool* showSpecificSlices) {
	fShowSpecificSlicesY = showSpecificSlices;
}


void TsBox::SetShowSpecificSlicesZ(G4bool* showSpecificSlices) {
	fShowSpecificSlicesZ = showSpecificSlices;
}


void TsBox::CreateDefaults(TsParameterManager* pM, G4String& childName, G4String&) {
	G4String parameterName;
	G4String transValue;

	parameterName = "dc:Ge/" + childName + "/HLX";
	transValue = "80. cm";
	pM->AddParameter(parameterName, transValue);

	parameterName = "dc:Ge/" + childName + "/HLY";
	transValue = "60. cm";
	pM->AddParameter(parameterName, transValue);

	parameterName = "dc:Ge/" + childName + "/HLZ";
	transValue = "20. cm";
	pM->AddParameter(parameterName, transValue);

	parameterName = "ic:Ge/" + childName + "/XBins";
	transValue = "1";
	pM->AddParameter(parameterName, transValue);

	parameterName = "ic:Ge/" + childName + "/YBins";
	transValue = "1";
	pM->AddParameter(parameterName, transValue);

	parameterName = "ic:Ge/" + childName + "/ZBins";
	transValue = "1";
	pM->AddParameter(parameterName, transValue);
}
