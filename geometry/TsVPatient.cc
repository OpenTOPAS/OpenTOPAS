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

#include "TsVPatient.hh"

#include "TsParameterManager.hh"
#include "TsExtensionManager.hh"
#include "TsMaterialManager.hh"
#include "TsGeometryManager.hh"

#include "TsTopasConfig.hh"
#include "TsImagingToMaterialSchneider.hh"
#include "TsImagingToMaterialXcat.hh"
#include "TsImagingToMaterialByTagNumber.hh"

#include "G4Box.hh"
#include "G4UIcommand.hh"
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"
#include "G4Material.hh"

TsVPatient::TsVPatient(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
					   TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name)
	: TsBox(pM, eM, mM, gM, parentComponent, parentVolume, name),
	  fRestrictVoxelsXMin(0), fRestrictVoxelsYMin(0), fRestrictVoxelsZMin(0), fRestrictVoxelsXMax(0), fRestrictVoxelsYMax(0), fRestrictVoxelsZMax(0),
	  fNeedToSetImage(true), fCurrentImageName(""), fNImageSections(0), fDataType("s"),
	  fUnrestrictedVoxelCountX(0), fUnrestrictedVoxelCountY(0), fUnrestrictedVoxelCountZ(0),
	  fVoxelCountX(0), fVoxelCountY(0), fVoxelCountZ(nullptr),
	  fFirstOriginalSliceThisSection(nullptr), fLastOriginalSliceThisSection(nullptr),
	  fVoxelSizeZ(nullptr), fTotalNumberOfSlices(0), fTotalNumberOfVoxels(0), fNEnergies(1),
	  fImagingToMaterial(nullptr),
	  fLastImageName(""), fLastNImageSections(0),
	  fLastVoxelCountX(0), fLastVoxelCountY(0), fLastVoxelCountZ(nullptr),
	  fLastFullWidthX(0), fLastFullWidthY(0), fLastVoxelSizeZ(nullptr),
	  fSlicesToShowX(nullptr), fNSlicesToShowX(0),
	  fSlicesToShowY(nullptr), fNSlicesToShowY(0),
	  fSlicesToShowZ(nullptr), fNSlicesToShowZ(0)
{
	fConstructParameterized = true;
}


TsVPatient::~TsVPatient()
{;}


void TsVPatient::UpdateForSpecificParameterChange(G4String parameter)
{
	if (parameter == GetFullParmNameLower("ShowSpecificSlicesX") || parameter == GetFullParmNameLower("ShowSpecificSlicesY") ||
		parameter == GetFullParmNameLower("ShowSpecificSlicesZ")) {
		GetSlicesToShow();
		fNeedToSetImage = true;
	} else {
		// For any other parameters, fall back to the base class Update method
		TsVGeometryComponent::UpdateForSpecificParameterChange(parameter);
	}
}


void TsVPatient::UpdateForNewRun(G4bool force) {
	if (!fIsCopy && fNeedToSetImage)
		SetImage();

	TsVGeometryComponent::UpdateForNewRun(force);
}


G4VPhysicalVolume* TsVPatient::Construct() {
	BeginConstruction();

	GetSlicesToShow();

	GetRestrictVoxelParameters();

	if (fIsCopy) {
		// This is a parallel scoring copy
		fConstructParameterized = false;

		// Take dimensions from original component
		TsBox* originalComponent = dynamic_cast<TsBox*>(GetOriginalComponent());
		if (!originalComponent) {
			G4cerr << "TOPAS is exiting due to a serious error in the setup of " << GetName() << G4endl;
			G4cerr << "Error occurred when constructing copy component." << G4endl;
			fPm->AbortSession(1);
		}
		fFullWidths[0] = originalComponent->GetFullWidth(0);
		fFullWidths[1] = originalComponent->GetFullWidth(1);
		fFullWidths[2] = originalComponent->GetFullWidth(2);
		fTransFirstVoxelCenterRelToComponentCenter = originalComponent->GetTransFirstVoxelCenterRelToComponentCenter();

		// Set graphics options based on number of voxels
		G4int numberOfVoxelsInCopy = GetDivisionCount(0) * GetDivisionCount(1) * GetDivisionCount(2);
		fShowOnlyOutline = numberOfVoxelsInCopy > fPm->GetIntegerParameter("Gr/ShowOnlyOutlineIfVoxelCountExceeds");
		if (numberOfVoxelsInCopy > fPm->GetIntegerParameter("Gr/SwitchOGLtoOGLIifVoxelCountExceeds"))
			SetTooComplexForOGLS();

		SetOriginalComponent(originalComponent);
	} else {
		// This is not a parallel scoring copy
		G4String converterName = fPm->GetStringParameter(GetFullParmName("ImagingToMaterialConverter"));
		G4String converterNameLower = converterName;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(converterNameLower);
#else
		converterNameLower.toLower();
#endif
		fImagingToMaterial = fEm->InstantiateImagingToMaterial(fPm, this, fMaterialList, converterNameLower);

		if (!fImagingToMaterial) {
			if (converterNameLower == "schneider") {
				fImagingToMaterial = new TsImagingToMaterialSchneider(fPm, this, fMaterialList);
			} else if (converterNameLower == "xcat_attenuation" || converterNameLower == "xcat_activity") {
				fImagingToMaterial = new TsImagingToMaterialXcat(fPm, this, fMaterialList, converterNameLower);
			} else if (converterNameLower == "materialtagnumber") {
				fImagingToMaterial = new TsImagingToMaterialByTagNumber(fPm, this, fMaterialList, converterNameLower);
			} else {
				G4cout << GetFullParmName("ImagingToMaterialConverter") << " specifies an unknown conversion method: " << converterName << G4endl;
				fPm->AbortSession(1);
			}
		}

		if (fPm->ParameterExists(GetFullParmName("NumberOfEnergies")))
			fNEnergies = fPm->GetIntegerParameter(GetFullParmName("NumberOfEnergies"));

		// Read the image from either DICOM or XIO
		ReadImage();

		G4double transX, transY, transZ;

		if (fPm->ParameterExists(GetFullParmName("TransX")))
			transX = fPm->GetDoubleParameter(GetFullParmName("TransX"), "Length");
		else
			transX = 0.;

		if (fPm->ParameterExists(GetFullParmName("TransY")))
			transY = fPm->GetDoubleParameter(GetFullParmName("TransY"), "Length");
		else
			transY = 0.;

		if (fPm->ParameterExists(GetFullParmName("TransZ")))
			transZ = fPm->GetDoubleParameter(GetFullParmName("TransZ"), "Length");
		else
			transZ = 0.;

		G4cout << "    Iso Center [cm]:       ( " << transX/cm << ", " << transY/cm << ", " << transZ/cm <<  " )" << G4endl;

		G4cout << "    # of Voxels:           ( " << fVoxelCountX << ", " << fVoxelCountY << ", " <<
		fTotalNumberOfSlices << " )" << G4endl;

		G4cout << "    Voxel Dimensions X and Y [cm]: ( " << fFullWidths[0] / fVoxelCountX / cm << ", " <<
		fFullWidths[1] / fVoxelCountY / cm << " ) " << G4endl;

		G4cout << "    Number of Slice Thickness Sections: " << fNImageSections << G4endl;

		for (G4int i=0; i< fNImageSections; i++)
			G4cout << "      Slice thickness section " << i+1 << " has number of voxels: " << fVoxelCountZ[i] <<
			" of thickness: " << fVoxelSizeZ[i] / cm << " cm." << G4endl;

		G4double rotX, rotY, rotZ;

		if (fPm->ParameterExists(GetFullParmName("RotX")))
			rotX = fPm->GetDoubleParameter(GetFullParmName("RotX"), "Angle");
		else
			rotX = 0.;

		if (fPm->ParameterExists(GetFullParmName("RotY")))
			rotY = fPm->GetDoubleParameter(GetFullParmName("RotY"), "Angle");
		else
			rotY = 0.;

		if (fPm->ParameterExists(GetFullParmName("RotZ")))
			rotZ = fPm->GetDoubleParameter(GetFullParmName("RotZ"), "Angle");
		else
			rotZ = 0.;

		G4cout << "    Rotation [deg]:       ( " << rotX/deg << ", " << rotY/deg << ", "  << rotZ/deg <<  " )" << G4endl;

		// Since Geant4 does not allow us to add materials after the job has started,
		// and some 4D imaging cases may introduce new materials after the first image,
		// have options to force loading of materials in advance.
		if (fPm->ParameterExists(GetFullParmName("PreLoadAllMaterials")) && fPm->GetBooleanParameter(GetFullParmName("PreLoadAllMaterials"))) {
			G4cout << "\nPreloading all materials since " << GetFullParmName("PreLoadAllMaterials") << " has been set true." << G4endl;
			G4cout << "This parameter, which used to be necessary for 4DCT, is no longer recommended." << G4endl;
			G4cout << "Instead, you should use the parameter PreLoadMaterialsFrom." << G4endl;
			G4cout << "PreLoadAllMaterials loads every material defined in the HU conversion table." << G4endl;
			G4cout << "PreLoadMaterialsFrom loads only those materials that are actually used in one of the images." << G4endl;
			G4cout << "Thus PreLoadMaterialsFrom saves both memory and startup time." << G4endl;
			fImagingToMaterial->PreloadAllMaterials();
			G4cout << "Finished defining materials from imaging. Total number of materials used was: " << fMaterialList->size() << G4endl;
		} else if (fPm->ParameterExists(GetFullParmName("PreLoadMaterialsFrom"))) {
			G4cout << "\nPreloading materials from several images since " << GetFullParmName("PreLoadMaterialsFrom") << " has been set." << G4endl;
			G4String imageNameBeforeLoop = fCurrentImageName;
			G4String* imageNames = fPm->GetStringVector(GetFullParmName("PreLoadMaterialsFrom"));
			G4int nImages = fPm->GetVectorLength(GetFullParmName(("PreLoadMaterialsFrom")));
			for (G4int imageNum = 0; imageNum < nImages; imageNum++) {
				fCurrentImageName = imageNames[imageNum];
				SetImage();
			}
			fCurrentImageName = imageNameBeforeLoop;
		}

		// Store Material Pointers used for RTStruct evaluation
		for (G4int i = 0; i < (int)fMaterialByRTStructMaterialNames.size(); i++) {
			if (fMaterialByRTStructMaterialNames[i]!="") {
				G4Material* RTStructMaterial = GetMaterial(fMaterialByRTStructMaterialNames[i]);
				fMaterialByRTStructMaterials.push_back(RTStructMaterial);
				fMaterialList->push_back(RTStructMaterial);
			}
		}

		// Set graphics options based on number of voxels
		fShowOnlyOutline = fTotalNumberOfVoxels > fPm->GetIntegerParameter("Gr/ShowOnlyOutlineIfVoxelCountExceeds");
		if (fTotalNumberOfVoxels > fPm->GetIntegerParameter("Gr/SwitchOGLtoOGLIifVoxelCountExceeds"))
			SetTooComplexForOGLS();
	}

	// Create outer box.
	// For single slice thickness patient, this will contain the voxels.
	// For multiple slice thickness patient, this will contain a separate TsBox for each slice thickness section.
	// Alternatively, we may have been called to create a parallel scoring copy of the patient.
	// If parallel scoring copy is undivided, just create a G4Box.
	// If parallel scoring copy is divided, the G4Box will in turn contain scoring grid voxels (not real patient voxels).
	G4Box* envelopeBox = new G4Box("Envelope",fFullWidths[0]/2.,fFullWidths[1]/2.,fFullWidths[2]/2.);
	fEnvelopeLog = CreateLogicalVolume(envelopeBox);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);
	//G4cout << "Creating box: " << fEnvelopePhys->GetName() << "< widths: " << fFullWidths[0] << ", " << fFullWidths[1] << ", " << fFullWidths[2] << G4endl;

	if (!fIsCopy && !fShowOnlyOutline)
		fEnvelopeLog->SetVisAttributes(fPm->GetInvisible());

	if (fIsCopy && fDivisionCounts[0] == 1 && fDivisionCounts[1] == 1 && fDivisionCounts[2] == 1) {
		//G4cout << "Creating undivided parallel world scoring copy" << G4endl;
	} else if (fIsCopy || fNImageSections == 1) {
		//G4cout << "Creating divided parallel world scoring copy or single slice thickness patient" << G4endl;
		ConstructVoxelStructure();
	} else {
		//G4cout << "Creating multiple slice thickness patient" << G4endl;
		G4double sectionPosZ = - fFullWidths[2] / 2.;

		// Loop over image sectionss
		for (G4int i = 0; i < fNImageSections; i++) {
			sectionPosZ += 0.5 * fVoxelCountZ[i] * fVoxelSizeZ[i];

			// Create a TsBox in transient parameters
			G4String childName = fName + "/Section" + G4UIcommand::ConvertToString(i);

			G4String parameterName = "s:Ge/" + childName + "/Parent";
			G4String transValue = "\"" + fName + "\"";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "d:Ge/" + childName + "/HLX";
			transValue = G4UIcommand::ConvertToString(fFullWidths[0]/2.) + " mm";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "d:Ge/" + childName + "/HLY";
			transValue = G4UIcommand::ConvertToString(fFullWidths[1]/2.) + " mm";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "d:Ge/" + childName + "/HLZ";
			transValue = G4UIcommand::ConvertToString(fVoxelCountZ[i] * fVoxelSizeZ[i] / 2.) + " mm";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "d:Ge/" + childName + "/TransX";
			transValue = "0. mm";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "d:Ge/" + childName + "/TransY";
			transValue = "0. mm";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "d:Ge/" + childName + "/TransZ";
			transValue = G4UIcommand::ConvertToString(sectionPosZ) + " mm";
			fPm->AddParameter(parameterName, transValue);
			sectionPosZ += 0.5 * fVoxelCountZ[i] * fVoxelSizeZ[i];

			parameterName = "d:Ge/" + childName + "/RotX";
			transValue = "0. deg";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "d:Ge/" + childName + "/RotY";
			transValue = "0. deg";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "d:Ge/" + childName + "/RotZ";
			transValue = "0. deg";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "i:Ge/" + childName + "/XBins";
			transValue = G4UIcommand::ConvertToString(fVoxelCountX);
			fPm->AddParameter(parameterName, transValue);

			parameterName = "i:Ge/" + childName + "/YBins";
			transValue = G4UIcommand::ConvertToString(fVoxelCountY);
			fPm->AddParameter(parameterName, transValue);

			parameterName = "i:Ge/" + childName + "/ZBins";
			transValue = G4UIcommand::ConvertToString(fVoxelCountZ[i]);
			fPm->AddParameter(parameterName, transValue);

			parameterName = "s:Ge/" + childName + "/Material";
			transValue = "\"G4_WATER\"";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "s:Ge/" + childName + "/Type";
			transValue = "\"TsBox\"";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "b:Ge/" + childName + "/ConstructParameterized";
			transValue = "\"True\"";
			fPm->AddParameter(parameterName, transValue);

			// If necessary, propagate MaxStepSize to divisions
			if (fPm->ParameterExists(GetFullParmName("MaxStepSize"))) {
				parameterName = "d:Ge/" + childName + "/MaxStepSize";
				transValue = GetFullParmName("MaxStepSize") + " " + fPm->GetUnitOfParameter(GetFullParmName("MaxStepSize"));
				fPm->AddParameter(parameterName, transValue);
			}

			// Instantiate and construct the TsBox as a child of the overall patient G4Box
			TsVGeometryComponent* patientTsBoxPart = InstantiateChild(childName);
			patientTsBoxPart->Construct();
			fImageSections.push_back((TsBox*)patientTsBoxPart);
		}
	}

	InstantiateChildren();

	if (!fIsCopy)
		SetImage();

	return fEnvelopePhys;
}


void TsVPatient::GetSlicesToShow() {
	if (fPm->ParameterExists(GetFullParmName("ShowSpecificSlicesX"))) {
		fSlicesToShowX = fPm->GetIntegerVector(GetFullParmName("ShowSpecificSlicesX"));
		fNSlicesToShowX = fPm->GetVectorLength(GetFullParmName("ShowSpecificSlicesX"));
	}

	if (fPm->ParameterExists(GetFullParmName("ShowSpecificSlicesY"))) {
		fSlicesToShowY = fPm->GetIntegerVector(GetFullParmName("ShowSpecificSlicesY"));
		fNSlicesToShowY = fPm->GetVectorLength(GetFullParmName("ShowSpecificSlicesY"));
	}

	if (fPm->ParameterExists(GetFullParmName("ShowSpecificSlicesZ"))) {
		fSlicesToShowZ = fPm->GetIntegerVector(GetFullParmName("ShowSpecificSlicesZ"));
		fNSlicesToShowZ = fPm->GetVectorLength(GetFullParmName("ShowSpecificSlicesZ"));
	}
}


void TsVPatient::GetRestrictVoxelParameters() {
	if (fPm->ParameterExists(GetFullParmName("RestrictVoxelsXMin")))
		fRestrictVoxelsXMin = fPm->GetIntegerParameter(GetFullParmName("RestrictVoxelsXMin"));
	else
		fRestrictVoxelsXMin = 1;

	if (fPm->ParameterExists(GetFullParmName("RestrictVoxelsYMin")))
		fRestrictVoxelsYMin = fPm->GetIntegerParameter(GetFullParmName("RestrictVoxelsYMin"));
	else
		fRestrictVoxelsYMin = 1;

	if (fPm->ParameterExists(GetFullParmName("RestrictVoxelsZMin")))
		fRestrictVoxelsZMin = fPm->GetIntegerParameter(GetFullParmName("RestrictVoxelsZMin"));
	else
		fRestrictVoxelsZMin = 1;

	if (fPm->ParameterExists(GetFullParmName("RestrictVoxelsXMax"))) {
		fRestrictVoxelsXMax = fPm->GetIntegerParameter(GetFullParmName("RestrictVoxelsXMax"));
		if (fRestrictVoxelsXMax < 1) {
			G4cerr << "Error: RestrictVoxelsXMax can not be less than 1."  << G4endl;
			fPm->AbortSession(1);
		}
	} else
		fRestrictVoxelsXMax = 1000000;

	if (fPm->ParameterExists(GetFullParmName("RestrictVoxelsYMax"))) {
		fRestrictVoxelsYMax = fPm->GetIntegerParameter(GetFullParmName("RestrictVoxelsYMax"));
		if (fRestrictVoxelsYMax < 1) {
			G4cerr << "Error: RestrictVoxelsYMax can not be less than 1."  << G4endl;
			fPm->AbortSession(1);
		}
	} else
		fRestrictVoxelsYMax = 1000000;

	if (fPm->ParameterExists(GetFullParmName("RestrictVoxelsZMax"))) {
		fRestrictVoxelsZMax = fPm->GetIntegerParameter(GetFullParmName("RestrictVoxelsZMax"));
		if (fRestrictVoxelsZMax < 1) {
			G4cerr << "Error: RestrictVoxelsZMax can not be less than 1."  << G4endl;
			fPm->AbortSession(1);
		}
	} else
		fRestrictVoxelsZMax = 1000000;

	if (fRestrictVoxelsXMax < fRestrictVoxelsXMin) {
		G4cerr << "Error: RestrictVoxelsXMax can not be less than RestrictVoxelsXMin."  << G4endl;
		fPm->AbortSession(1);
	}

	if (fRestrictVoxelsYMax < fRestrictVoxelsYMin) {
		G4cerr << "Error: RestrictVoxelsYMax can not be less than RestrictVoxelsYMin."  << G4endl;
		fPm->AbortSession(1);
	}

	if (fRestrictVoxelsZMax < fRestrictVoxelsZMin) {
		G4cerr << "Error: RestrictVoxelsZMax can not be less than RestrictVoxelsZMin."  << G4endl;
		fPm->AbortSession(1);
	}
}


// Z width and total number of slices depends on whether we are only loading one slice or all slices
void TsVPatient::SetUpZSlices() {
	fFullWidths[2] = 0.;
	fTotalNumberOfSlices = 0;
	for (G4int i = 0; i < fNImageSections; i++) {
		fFullWidths[2] += fVoxelCountZ[i] * fVoxelSizeZ[i];
		fTotalNumberOfSlices += fVoxelCountZ[i];
	}

	G4double voxelSizeX = fFullWidths[0] / fVoxelCountX;
	G4double voxelSizeY = fFullWidths[1] / fVoxelCountY;
	fTransFirstVoxelCenterRelToComponentCenter = G4Point3D(-0.5*fFullWidths[0] + 0.5*voxelSizeX,
														   -0.5*fFullWidths[1] + 0.5*voxelSizeY,
														   -0.5*fFullWidths[2] + 0.5*fVoxelSizeZ[0]);

	fTotalNumberOfVoxels = fVoxelCountX * fVoxelCountY * fTotalNumberOfSlices;;

	if (fNImageSections == 1) {
		fDivisionCounts[0] = fVoxelCountX;
		fDivisionCounts[1] = fVoxelCountY;
		fDivisionCounts[2] = fVoxelCountZ[0];
	} else {
		// For multiple slice thickness case, we will initially construct a single box sized to fit all slices
		fDivisionCounts[0] = 1;
		fDivisionCounts[1] = 1;
		fDivisionCounts[2] = 1;
	}
}


void TsVPatient::SetImage() {
	// Clear previous image if not caching them
	if (!fPm->GetBooleanParameter("Ge/CacheMaterialMapForEachTimeSlice")) {
		fImageNames.clear();
		fLastImageName = "";
	}

	if (fImageNames.find(fCurrentImageName) == fImageNames.end()) {
		G4cout << "\nCreating material map from image: " << fCurrentImageName << G4endl;
		G4int imageIndex = fImageNames.size();
		fImageNames.insert(std::pair<G4String, G4int>(fCurrentImageName, imageIndex));

		// Expand material index vector to accomodate this image
		fMaterialIndexVector.resize(imageIndex+1);
		fMaterialIndexVector[imageIndex].resize(fNImageSections);

		// Check that new image matches voxel count and size of last image
		if ((fLastImageName != "") && ((fNImageSections != fLastNImageSections) ||
									   (fVoxelCountX != fLastVoxelCountX) || (fFullWidths[0] != fLastFullWidthX) ||
									   (fVoxelCountY != fLastVoxelCountY) || (fFullWidths[1] != fLastFullWidthY))) {
			G4cout << "Image " << fCurrentImageName << " has different dimensions than previously loaded image " << fLastImageName << G4endl;
			fPm->AbortSession(1);
		}
		fLastNImageSections = fNImageSections;
		fLastVoxelCountX = fVoxelCountX;
		fLastVoxelCountY = fVoxelCountY;
		fLastFullWidthX = fFullWidths[0];
		fLastFullWidthY = fFullWidths[1];

		if (fLastImageName == "") {
			fLastVoxelCountZ = new G4int[fNImageSections];
			fLastVoxelSizeZ = new G4double[fNImageSections];
		}

		// Loop over image sections, registering materials
		G4int indexStart = 0;
		for (G4int sliceIndex = 0; sliceIndex < fNImageSections; sliceIndex++) {
			//G4cout << "Loading slice: " << sliceIndex << G4endl;

			// Check that new image matches image sections of last image
			if ((fLastImageName != "") && ((fVoxelCountZ[sliceIndex] != fLastVoxelCountZ[sliceIndex]) ||
										   (fVoxelSizeZ[sliceIndex] != fLastVoxelSizeZ[sliceIndex]))) {
				G4cout << "Image " << fCurrentImageName << " has different dimensions than previously loaded image " << fLastImageName << G4endl;
				fPm->AbortSession(1);
			}
			fLastVoxelCountZ[sliceIndex] = fVoxelCountZ[sliceIndex];
			fLastVoxelSizeZ[sliceIndex]	= fVoxelSizeZ[sliceIndex];

			G4int numberOfVoxels = fVoxelCountX * fVoxelCountY * fVoxelCountZ[sliceIndex];
			fMaterialIndexVector[imageIndex][sliceIndex] = new std::vector<unsigned short>;
			fMaterialIndexVector[imageIndex][sliceIndex]->resize(numberOfVoxels);

			for (G4int j = indexStart; j < indexStart + numberOfVoxels; j++) {
				// Convert from indexing of fRawImage to indexing for TsBox
				G4int jNew = j - indexStart;
				G4int iZ = int( jNew / (fVoxelCountX * fVoxelCountY));
				G4int iY = int( (jNew - iZ * fVoxelCountX * fVoxelCountY) / fVoxelCountX);
				G4int iX = jNew - iZ * fVoxelCountX * fVoxelCountY - iY * fVoxelCountX;
				G4int newIndex = iX * fVoxelCountY * fVoxelCountZ[sliceIndex] + iY * fVoxelCountZ[sliceIndex] + iZ;
				//G4cout << "Will store result for imageIndex: " << imageIndex << ", sliceIndex: " << sliceIndex << ", newIndex: " << newIndex << G4endl;

				if (fDataType == "s") {
					std::vector< signed short >* ImagingValues = new std::vector< signed short >;
					for (G4int i = 0; i < fNEnergies; i++)
						ImagingValues->push_back(fRawImageShort[i][j]);
					(*fMaterialIndexVector[imageIndex][sliceIndex])[newIndex] = fImagingToMaterial->AssignMaterial(ImagingValues, imageIndex);
					delete ImagingValues;
					//G4cout << "For j: " << j << ", fRawImage: " << fRawImage[0][j] << ", gave back: " << (*fMaterialIndexVector[imageIndex][sliceIndex])[newIndex] << G4endl;
				} else if (fDataType == "i") {
					std::vector< int >* ImagingValues = new std::vector< int >;
					for (G4int i = 0; i < fNEnergies; i++)
						ImagingValues->push_back(fRawImageInt[i][j]);
					(*fMaterialIndexVector[imageIndex][sliceIndex])[newIndex] = fImagingToMaterial->AssignMaterialInt(ImagingValues, imageIndex);
					delete ImagingValues;
					//G4cout << "For j: " << j << ", fRawImage: " << fRawImage[0][j] << ", gave back: " << (*fMaterialIndexVector[imageIndex][sliceIndex])[newIndex] << G4endl;
				} else if (fDataType == "f") {
					std::vector< float >* ImagingValues = new std::vector< float >;
					for (G4int i = 0; i < fNEnergies; i++)
						ImagingValues->push_back(fRawImageFloat[i][j]);
					(*fMaterialIndexVector[imageIndex][sliceIndex])[newIndex] = fImagingToMaterial->AssignMaterialFloat(ImagingValues, imageIndex);
					delete ImagingValues;
					//G4cout << "For j: " << j << ", fRawImage: " << fRawImage[0][j] << ", gave back: " << (*fMaterialIndexVector[imageIndex][sliceIndex])[newIndex] << G4endl;
				}
			}
			indexStart += numberOfVoxels;
		}
		G4cout << "Finished defining materials from imaging. Total number of materials used was: " << fMaterialList->size() << G4endl;
	} else {
		G4cout << "\nReusing cached material map from image: " << fCurrentImageName << G4endl;
	}

	// Get the index of the image in fImages (either previously existed or just created above)
	std::map<G4String,G4int>::iterator it = fImageNames.find(fCurrentImageName);
	fImageIndex = it->second;

	// Set flags to control showing of specific slices
	G4bool* showSpecificSlicesX = 0;
	if (fNSlicesToShowX != 0) {
		if (fSlicesToShowX[0] == -1)
			fSlicesToShowX[0] = fVoxelCountX / 2 + fRestrictVoxelsXMin -1;
		else if (fSlicesToShowX[0] == -2) {
			fSlicesToShowX = new G4int[3];
			fSlicesToShowX[0] = fRestrictVoxelsXMin;
			fSlicesToShowX[1] = fVoxelCountX/2 + fRestrictVoxelsXMin;
			fSlicesToShowX[2] = fVoxelCountX + fRestrictVoxelsXMin -1;
			fNSlicesToShowX = 3;
		}
		showSpecificSlicesX = new G4bool[fVoxelCountX];
		for (G4int i = 0; i < fVoxelCountX; i++)
			showSpecificSlicesX[i] = false;

		for (G4int i = 0; i < fNSlicesToShowX; i++) {
			if (fSlicesToShowX[i] > 0 && fSlicesToShowX[i] <= fUnrestrictedVoxelCountX) {
				if (fSlicesToShowX[i] >= fRestrictVoxelsXMin && fSlicesToShowX[i] <= fRestrictVoxelsXMax)
					showSpecificSlicesX[fSlicesToShowX[i] - fRestrictVoxelsXMin] = true;
			} else {
				G4cerr << "Error: ShowSpecificSlicesX specifies voxel number: " << i << G4endl;
				G4cerr << "But X voxel range is from 1 to " << fUnrestrictedVoxelCountX << G4endl;
				fPm->AbortSession(1);
			}
		}
	}

	G4bool* showSpecificSlicesY = 0;
	if (fNSlicesToShowY != 0) {
		if (fSlicesToShowY[0] == -1)
			fSlicesToShowY[0] = fVoxelCountY / 2 + fRestrictVoxelsYMin -1;
		else if (fSlicesToShowY[0] == -2) {
			fSlicesToShowY = new G4int[3];
			fSlicesToShowY[0] = fRestrictVoxelsYMin;
			fSlicesToShowY[1] = fVoxelCountY/2 + fRestrictVoxelsYMin;
			fSlicesToShowY[2] = fVoxelCountY + fRestrictVoxelsYMin -1;
			fNSlicesToShowY = 3;
		}
		showSpecificSlicesY = new G4bool[fVoxelCountY];
		for (G4int i = 0; i < fVoxelCountY; i++)
			showSpecificSlicesY[i] = false;

		for (G4int i = 0; i < fNSlicesToShowY; i++) {
			if (fSlicesToShowY[i] > 0 && fSlicesToShowY[i] <= fUnrestrictedVoxelCountY) {
				if (fSlicesToShowY[i] >= fRestrictVoxelsYMin && fSlicesToShowY[i] <= fRestrictVoxelsYMax)
					showSpecificSlicesY[fSlicesToShowY[i] - fRestrictVoxelsYMin] = true;
			} else {
				G4cerr << "Error: ShowSpecificSlicesY specifies voxel number: " << i << G4endl;
				G4cerr << "But Y voxel range is from 1 to " << fUnrestrictedVoxelCountY << G4endl;
				fPm->AbortSession(1);
			}
		}
	}

	G4bool* showSpecificSlicesZ = 0;
	if (fNSlicesToShowZ != 0) {
		if (fSlicesToShowZ[0] == -1)
			fSlicesToShowZ[0] = fTotalNumberOfSlices / 2 + fRestrictVoxelsZMin -1;
		else if (fSlicesToShowZ[0] == -2) {
			fSlicesToShowZ = new G4int[3];
			fSlicesToShowZ[0] = fRestrictVoxelsZMin;
			fSlicesToShowZ[1] = fTotalNumberOfSlices/2 + fRestrictVoxelsZMin;
			fSlicesToShowZ[2] = fTotalNumberOfSlices + fRestrictVoxelsZMin -1;
			fNSlicesToShowZ = 3;
		}
	}

	// Loop over image sections, setting material index and material list
	TsBox* boxToSetUp;
	for (G4int sliceIndex = 0; sliceIndex < fNImageSections; sliceIndex++) {
		if (fNImageSections == 1)
			boxToSetUp = this;
		else
			boxToSetUp = fImageSections[sliceIndex];

		boxToSetUp->SetMaterialIndex(fMaterialIndexVector[fImageIndex][sliceIndex]);
		boxToSetUp->SetMaterialList(fMaterialList);

		boxToSetUp->SetShowSpecificSlicesX(showSpecificSlicesX);
		boxToSetUp->SetShowSpecificSlicesY(showSpecificSlicesY);

		if (fPm->ParameterExists(GetFullParmName("ShowSpecificSlicesZ"))) {
			showSpecificSlicesZ = new G4bool[fVoxelCountZ[sliceIndex]];
			for (G4int i = 0; i < fVoxelCountZ[sliceIndex]; i++)
				showSpecificSlicesZ[i] = false;

			for (G4int i = 0; i < fNSlicesToShowZ; i++) {
				if (fSlicesToShowZ[i] > 0 && fSlicesToShowZ[i] <= fUnrestrictedVoxelCountZ) {
					if (fSlicesToShowZ[i] >= fFirstOriginalSliceThisSection[sliceIndex] &&
						fSlicesToShowZ[i] <= fLastOriginalSliceThisSection[sliceIndex])
						showSpecificSlicesZ[fSlicesToShowZ[i] - fFirstOriginalSliceThisSection[sliceIndex]] = true;
				} else {
					G4cerr << "Error: ShowSpecificSlicesZ specifies voxel number: " << i << G4endl;
					G4cerr << "But Z voxel range is from 1 to " << fTotalNumberOfSlices << G4endl;
					fPm->AbortSession(1);
				}
			}
			boxToSetUp->SetShowSpecificSlicesZ(showSpecificSlicesZ);
		}
	}

	fLastImageName = fCurrentImageName;
	fNeedToSetImage = false;
}
