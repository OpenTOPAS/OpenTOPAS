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

#include "TsDicomPatient.hh"
#include "TsParameterManager.hh"

#include "G4UIcommand.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"

#include "gdcmDirectory.h"
#include "gdcmImageReader.h"
#include "gdcmIPPSorter.h"
#include "gdcmScanner.h"
#include "gdcmAttribute.h"
#include "gdcmRescaler.h"

TsDicomPatient::TsDicomPatient(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM,
							   TsGeometryManager* gM, TsVGeometryComponent* parentComponent,
							   G4VPhysicalVolume* parentVolume, G4String& name)
:TsVPatient(pM, eM, mM, gM, parentComponent, parentVolume, name),
fFirstFileName(""), fCurrentImageNumber(0), fCreateDoseGrid(false),
fShowSliceSeparations(false) {
	if (fPm->ParameterExists(GetFullParmName("ShowSliceSeparations")))
		fShowSliceSeparations = fPm->GetBooleanParameter(GetFullParmName("ShowSliceSeparations"));
}


TsDicomPatient::~TsDicomPatient() {
}


void TsDicomPatient::UpdateForSpecificParameterChange(G4String parameter)
{
	if (parameter == GetFullParmNameLower("DicomDirectory")) {
        ReadImage();
		fNeedToSetImage = true;
	} else
		// For any other parameters, fall back to the base class Update method
		TsVPatient::UpdateForSpecificParameterChange(parameter);
}


void TsDicomPatient::ReadImage() {
	fCurrentImageName = fPm->GetStringParameter(GetFullParmName("DicomDirectory"));

	// If the current image name is not yet in fImageNames, read the image files
	if (fImageNames.find(fCurrentImageName) == fImageNames.end()) {
		fIsInNamedStructure.resize(fCurrentImageNumber+1);

		G4cout << "TOPAS will read DICOM directory: " << fCurrentImageName << G4endl;

		fRawImageShort.clear();
		fRawImageShort.resize(fNEnergies);

		// Need to initialize these to avoid compiler warning,
		// even though they will get set properly before first use
		G4double voxelSizeX = 0.;
		G4double voxelSizeY = 0.;
		G4double currentSliceSeparation = 0.;
		G4double previousSliceSeparation = 0.;
		G4double dicomSliceThicknessAttributeValue = 0.;

		// Set up file scanner
		gdcm::Scanner scanner;

		gdcm::Tag const modalityTag = gdcm::Tag(0x08, 0x60);
		scanner.AddTag(modalityTag);

		gdcm::Tag const acquisitionTag = gdcm::Tag(0x20, 0x12);
		scanner.AddTag(acquisitionTag);

		std::vector<G4double> slicePositions;

		if (fNEnergies > 1)
		G4cout << "TOPAS will look for separate subdirectories for " << fNEnergies << " different imaging energies" << G4endl;

		// Loop over multiple imaging energies
		for (G4int iEnergy = 0; iEnergy < fNEnergies; iEnergy++) {

			G4String subDirectoryName;
			if (fNEnergies == 1)
				subDirectoryName = fCurrentImageName;
			else {
				subDirectoryName = fCurrentImageName + "/" + G4UIcommand::ConvertToString(iEnergy+1);
				G4cout << "Reading energy-specific subdirectory: " << subDirectoryName << G4endl;
			}

			gdcm::Directory dir;
			if (!dir.Load(subDirectoryName)) {
				G4cout << "Unable to read the specified DICOM directory." << G4endl;
				G4cout << "It may be that you just forgot to unzip the relevant DICOM dataset." << G4endl;
				fPm->AbortSession(1);
			}

			// Finds all files in the directory, including those that are not of desired imaging modality
			std::vector<std::string> allFilenames = dir.GetFilenames();

			if (allFilenames.size() == 0) {
				G4cout << "Found no files in the specified DICOM directory." << G4endl;
				G4cout << "It may be that you just forgot to unzip the relevant DICOM dataset." << G4endl;
				fPm->AbortSession(1);
			}

			if (fVerbosity>0) {
				G4cout << "Found " << allFilenames.size() << " files in the DICOM directory:" << G4endl;
				for (G4int iFile = 0; iFile < (G4int)allFilenames.size(); iFile++)
					G4cout << allFilenames[iFile] << G4endl;
			}

            scanner.Scan(allFilenames);

			// Use scanner to clean up list of file names so that it is only the image files
			std::vector<std::string> unsortedImageFilenames;
			std::vector<std::string> unsortedImageFilenamesOneModality;

			G4String* modalityTags;
			G4int nModalityTags;

			if (fPm->ParameterExists(GetFullParmName("DicomModalityTags"))) {
				modalityTags = fPm->GetStringVector(GetFullParmName("DicomModalityTags"));
				nModalityTags = fPm->GetVectorLength(GetFullParmName("DicomModalityTags"));
			} else {
				modalityTags = new G4String[1];
				modalityTags[0] = "CT";
				nModalityTags = 1;
			}

			for (G4int iModalityTag = 0; iModalityTag < nModalityTags; iModalityTag++) {
				unsortedImageFilenamesOneModality = scanner.GetAllFilenamesFromTagToValue(modalityTag, modalityTags[iModalityTag]);
				unsortedImageFilenames.insert(unsortedImageFilenames.end(), unsortedImageFilenamesOneModality.begin(),unsortedImageFilenamesOneModality.end());
			}

			if (unsortedImageFilenames.size() == 0) {
				G4cout << "The specified DICOM directory did not contain any files of the desired modalities" << G4endl;
                G4cout << "that were indicated in " << GetFullParmName("DicomModalityTags") << G4endl;
				G4cout << "These were: " << G4endl;
				for (G4int iModalityTag = 0; iModalityTag < nModalityTags; iModalityTag++)
					G4cout << "  " << modalityTags[iModalityTag] << G4endl;
				fPm->AbortSession(1);
			}

			if (fVerbosity>0) {
				G4cout << "Found " << unsortedImageFilenames.size() << " imaging files of the appropriate modalities in the DICOM directory:" << G4endl;
				for (G4int iFile = 0; iFile < (G4int)unsortedImageFilenames.size(); iFile++)
					G4cout << unsortedImageFilenames[iFile] << G4endl;
			}

			// Use sorter to sort list of imaging files in order of Z slice irrespective of file name
			std::vector<std::string> sortedImageFilenames;
			gdcm::IPPSorter sorter;
			sorter.SetComputeZSpacing(false);
			sorter.Sort(unsortedImageFilenames);
			sortedImageFilenames = sorter.GetFilenames();

			if (sortedImageFilenames.size() == 0) {
				G4cout << "Failed to sort files in the specified DICOM directory." << G4endl;
				fPm->AbortSession(1);
			}

			if (fVerbosity>0) {
				G4cout << "Sorted files in the DICOM directory:" << G4endl;
				for (G4int iFile = 0; iFile < (G4int)sortedImageFilenames.size(); iFile++)
					G4cout << sortedImageFilenames[iFile] << G4endl;
			}

			// Read slice files to get position and dimensions
			slicePositions.clear();
			gdcm::ImageReader* reader;
			gdcm::Image* image;
			gdcm::DataSet* dataSet;

			G4bool DumpImagingValues = (fPm->ParameterExists(GetFullParmName("DumpImagingValues")) &&
								   fPm->GetBooleanParameter(GetFullParmName("DumpImagingValues")));

			fNImageSections = 1;
			fVoxelCountZ = new G4int[9];
			fFirstOriginalSliceThisSection = new G4int[9];
			fLastOriginalSliceThisSection = new G4int[9];
			fVoxelSizeZ = new G4double[9];
			G4bool firstSlice = true;
			fUnrestrictedVoxelCountX = 0;
			fUnrestrictedVoxelCountY = 0;
			G4int countThisSection = 1;

			if (fRestrictVoxelsZMin > (G4int)sortedImageFilenames.size()) {
				G4cerr << "Error: RestrictVoxelsZMin = " << fRestrictVoxelsZMin << " is greater than number of slices in this image: "
				<< (G4int)sortedImageFilenames.size() << G4endl;
				fPm->AbortSession(1);
			}

			fUnrestrictedVoxelCountZ = (G4int)sortedImageFilenames.size();
			G4int iSlice;
			for (iSlice=0; iSlice < fUnrestrictedVoxelCountZ; iSlice++) {
				if (((iSlice+1 >= fRestrictVoxelsZMin) && (iSlice+1 <= fRestrictVoxelsZMax))) {

				if (fVerbosity>0)
					G4cout << "Reading file: " << sortedImageFilenames[iSlice] << G4endl;

				reader = new gdcm::ImageReader();
				reader->SetFileName(sortedImageFilenames[iSlice].c_str());
				if (!(reader->Read())) {
					G4cout << "Failed on attempt to read data from DICOM file: " << sortedImageFilenames[iSlice] << G4endl;
					fPm->AbortSession(1);
				}

				dataSet = &reader->GetFile().GetDataSet();

				gdcm::Attribute<0x0020,0x0032> atPos;
				atPos.SetFromDataElement( dataSet->GetDataElement( atPos.GetTag() ) );
				slicePositions.push_back(atPos.GetValues()[2]);

				gdcm::Attribute<0x0018,0x0050> atThick;
				atThick.SetFromDataElement( dataSet->GetDataElement( atThick.GetTag() ) );
				dicomSliceThicknessAttributeValue = atThick.GetValue();

				image = &reader->GetImage();

				// Enforce that all slices need same voxel size and count
				// except that there can be different slice thickness sections
				// Can't assume first slice that passed restrictVoxels test will be slice number zero
				if (firstSlice) {
					firstSlice = false;
					fFirstOriginalSliceThisSection[fNImageSections-1] = iSlice + 1;
					fFirstFileName = sortedImageFilenames[0];

					fUnrestrictedVoxelCountX = image->GetDimensions()[0];
					fVoxelCountX = fmin(fUnrestrictedVoxelCountX, fRestrictVoxelsXMax) - fRestrictVoxelsXMin + 1;

					if (fRestrictVoxelsXMin > fUnrestrictedVoxelCountX) {
						G4cerr << "Error: RestrictVoxelsXMin = " << fRestrictVoxelsXMin << " is greater than number of X voxels in this image: "
						<< fUnrestrictedVoxelCountX << G4endl;
						fPm->AbortSession(1);
					}

					if (fVoxelCountX < 1) {
						G4cerr << "Error: No voxels in this image are in range RestrictVoxelsXMin = " << fRestrictVoxelsXMin <<
						" and RestrictVoxelsXMax = " << fRestrictVoxelsXMax << G4endl;
						fPm->AbortSession(1);
					}

					fUnrestrictedVoxelCountY = image->GetDimensions()[1];
					fVoxelCountY = fmin(fUnrestrictedVoxelCountY, fRestrictVoxelsYMax) - fRestrictVoxelsYMin + 1;

					if (fRestrictVoxelsYMin > fUnrestrictedVoxelCountY) {
						G4cerr << "Error: RestrictVoxelsYMin = " << fRestrictVoxelsYMin << " is greater than number of Y voxels in this image: "
						<< fUnrestrictedVoxelCountY << G4endl;
						fPm->AbortSession(1);
					}

					if (fVoxelCountY < 1) {
						G4cerr << "Error: No voxels in this image are in range RestrictVoxelsYMin = " << fRestrictVoxelsYMin <<
						" and RestrictVoxelsYMax = " << fRestrictVoxelsYMax << G4endl;
						fPm->AbortSession(1);
					}

					voxelSizeX = image->GetSpacing()[0];
					voxelSizeY = image->GetSpacing()[1];
					fTransFirstVoxelCenterRelToDicom = G4Point3D(atPos.GetValues()[0], atPos.GetValues()[1], atPos.GetValues()[2]);
					fTransFirstVoxelCenterRelToDicom.setX(fTransFirstVoxelCenterRelToDicom.x() + (fRestrictVoxelsXMin-1) * voxelSizeX);
					fTransFirstVoxelCenterRelToDicom.setY(fTransFirstVoxelCenterRelToDicom.y() + (fRestrictVoxelsYMin-1) * voxelSizeY);

					gdcm::Attribute<0x0020,0x0052> atFrameOfReferenceUID;
					atFrameOfReferenceUID.SetFromDataElement( dataSet->GetDataElement( atFrameOfReferenceUID.GetTag() ) );
					fFrameOfReferenceUID = atFrameOfReferenceUID.GetValue();
				} else {
					countThisSection++;

					if (fUnrestrictedVoxelCountX != (int)(image->GetDimensions()[0])) {
						G4cout << "DICOM Slices have inconsistent voxelCountX" << G4endl;
						fPm->AbortSession(1);
					}
					if (fUnrestrictedVoxelCountY != (int)(image->GetDimensions()[1])) {
						G4cout << "DICOM Slices have inconsistent voxelCountY" << G4endl;
						fPm->AbortSession(1);
					}
					if (voxelSizeX != image->GetSpacing()[0]) {
						G4cout << "DICOM Slices have inconsistent X thickness" << G4endl;
						fPm->AbortSession(1);
					}
					if (voxelSizeY != image->GetSpacing()[1]) {
						G4cout << "DICOM Slices have inconsistent Y thickness" << G4endl;
						fPm->AbortSession(1);
					}

					currentSliceSeparation = std::fabs( slicePositions[iSlice-fRestrictVoxelsZMin+1] - slicePositions[iSlice-fRestrictVoxelsZMin]);

					if (fShowSliceSeparations)
						G4cout << "SliceSeparation: " << currentSliceSeparation << G4endl;

					if ((previousSliceSeparation != 0.) &&
						(std::fabs(currentSliceSeparation - previousSliceSeparation) > 0.002)) {
						// This slice is the first slice of a new slice thickness section
						// Save information on section just completed
						fVoxelSizeZ[fNImageSections-1] = currentSliceSeparation;
						fVoxelCountZ[fNImageSections-1] = countThisSection - 1;
						fFirstOriginalSliceThisSection[fNImageSections] = iSlice;
						fLastOriginalSliceThisSection[fNImageSections-1] = iSlice - 1;

						// Increment to next section and reset count to zero
						fNImageSections++;
						if (fNImageSections > 9) {
							G4cout << "DICOM has too many different slice thickness sections." << G4endl;
							G4cout << "Limit is 9" << G4endl;
							if (!fShowSliceSeparations)
								G4cout << "To see slice separations, rerun with " << GetFullParmName("ShowSliceSeparations") << " set to True" << G4endl;
							fPm->AbortSession(1);
						}
						countThisSection = 1;
					}
					previousSliceSeparation = currentSliceSeparation;
				}

				// Read the pixels
				gdcm::PixelFormat pixel_format = image->GetPixelFormat();

				G4int buffer_length = image->GetBufferLength();
				char* buffer_in   = new char[buffer_length];
				char* buffer_out  = new char[buffer_length];
				image->GetBuffer(buffer_in);

				G4double slope = image->GetSlope();
				G4double intercept = image->GetIntercept();
				gdcm::Rescaler rescaler = gdcm::Rescaler();
				rescaler.SetIntercept(intercept);
				rescaler.SetSlope(slope);
				rescaler.SetPixelFormat(pixel_format);
				rescaler.SetMinMaxForPixelType(((gdcm::PixelFormat)gdcm::PixelFormat::INT16).GetMin(),
											   ((gdcm::PixelFormat)gdcm::PixelFormat::INT16).GetMax());
				rescaler.SetTargetPixelType((gdcm::PixelFormat)gdcm::PixelFormat::INT16);
				rescaler.SetUseTargetPixelType(true);
				rescaler.Rescale(buffer_out, buffer_in, buffer_length);
				short* ImagingValue= (short *) buffer_out;

				G4int iVoxel = 0;
				for (G4int iY=1; iY<=fUnrestrictedVoxelCountY; iY++) {
					for (G4int iX=1; iX<=fUnrestrictedVoxelCountX; iX++) {
						if (((iX >= fRestrictVoxelsXMin) && (iX <= fRestrictVoxelsXMax) &&
							 (iY >= fRestrictVoxelsYMin) && (iY <= fRestrictVoxelsYMax))) {
							fRawImageShort[iEnergy].push_back((signed short) ImagingValue[iVoxel]);
							if (DumpImagingValues) G4cout << "Pixel: " << iVoxel << " has value: " << (signed short) ImagingValue[iVoxel] << G4endl;
						}
						iVoxel++;
					}
				}
				delete[] buffer_out;
				delete[] buffer_in;
				delete reader;
				}
			}

			// If there is only one slice, can't use slice separation to get thickness. Instead use DICOM thickness attribute.
			if (currentSliceSeparation == 0.) {
				G4cout << "Since there is only one slice, we cannot get slice thickness by checking slice separation." << G4endl;
				G4cout << "Will instead use the DICOM's slice thickness attribute (not always the most reliable choice)." << G4endl;
				currentSliceSeparation = dicomSliceThicknessAttributeValue;
			}

			// Store information for last slice thickness section
			fVoxelSizeZ[fNImageSections-1] = currentSliceSeparation;
			fVoxelCountZ[fNImageSections-1] = countThisSection;
			fLastOriginalSliceThisSection[fNImageSections-1] = iSlice;
		}

		// Calculate remaining dimensions
		fFullWidths[0] = fVoxelCountX * voxelSizeX;
		fFullWidths[1] = fVoxelCountY * voxelSizeY;
		SetUpZSlices();

		// Define changeable parameters so DICOM frame of reference can be used in parameter files
		G4String transName, transValue;
		transName  = "dc:Ge/" + fName + "/DicomOriginX";
		transValue = G4UIcommand::ConvertToString((fTransFirstVoxelCenterRelToDicom.x() - fTransFirstVoxelCenterRelToComponentCenter.x()) / mm) + " mm";
		fPm->AddParameter(transName, transValue);
		transName  = "dc:Ge/" + fName + "/DicomOriginY";
		transValue = G4UIcommand::ConvertToString((fTransFirstVoxelCenterRelToDicom.y() - fTransFirstVoxelCenterRelToComponentCenter.y()) / mm) + " mm";
		fPm->AddParameter(transName, transValue);
		transName  = "dc:Ge/" + fName + "/DicomOriginZ";
		transValue = G4UIcommand::ConvertToString((fTransFirstVoxelCenterRelToDicom.z() - fTransFirstVoxelCenterRelToComponentCenter.z()) / mm) + " mm";
		fPm->AddParameter(transName, transValue);

		// See if there is any need to load structure set information.
		// Need structure information if coloring by structure.
		G4String parmName = GetFullParmName("ColorByRTStructNames");
		if (fPm->ParameterExists(parmName)) {
			G4String* structureNames = fPm->GetStringVector(parmName);
			G4int length = fPm->GetVectorLength(parmName);

			G4String colorParmName = GetFullParmName("ColorByRTStructColors");
			if (!fPm->ParameterExists(colorParmName)) {
				G4cout << "Missing parameter: " << colorParmName << G4endl;
				fPm->AbortSession(1);
			}

			if (fPm->GetVectorLength(colorParmName) != length) {
				G4cout << "Wrong length for parameter: " << colorParmName << G4endl;
				fPm->AbortSession(1);
			}

			G4String* structureColors = fPm->GetStringVector(colorParmName);

			for (G4int i = 0; i < length; i++) {
				// Loop over all elements already in the vector to avoid repetitions
				G4bool found = false;
				for (G4int j = 0; j < (int)fStructureNames.size() && !found; j++)
					if (structureNames[i] == fStructureNames[j])
						found = true;
				if (!found) {
					fStructureNames.push_back(structureNames[i]);

					G4VisAttributes* visAtt	= new G4VisAttributes();
					visAtt->SetColor( fPm->GetColor(structureColors[i])->GetColor() );
					fStructureColors.push_back(visAtt);
				}
			}
		}

		// Need structure information if setting material by structure.
		// Remember that fStructureNames will already contain some names if had color by structure.
		parmName = GetFullParmName("MaterialByRTStructNames");
		if (fPm->ParameterExists(parmName)) {
			G4String* structureNames = fPm->GetStringVector(parmName);
			G4int length = fPm->GetVectorLength(parmName);

			G4String materialParmName = GetFullParmName("MaterialByRTStructMaterials");
			if (!fPm->ParameterExists(materialParmName)) {
				G4cout << "Missing parameter: " << materialParmName << G4endl;
				fPm->AbortSession(1);
			}

			if (fPm->GetVectorLength(materialParmName) != length) {
				G4cout << "Wrong length for parameter: " << materialParmName << G4endl;
				fPm->AbortSession(1);
			}

			G4String* structureMaterials = fPm->GetStringVector(materialParmName);

			for (G4int iToken=0; iToken<length; iToken++) {
				fMaterialByRTStructMaterialNames.push_back(structureMaterials[iToken]);

				G4bool found = false;
				for (G4int i = 0; i < (int)fStructureNames.size() && !found; i++)
					if (structureNames[iToken] == fStructureNames[i]) {
						found = true;
						fMaterialByRTStructNamesIndexIntoIsInNamedStructure.push_back(i);
					}

				if (!found) {
					fStructureNames.push_back(structureNames[iToken]);
					fMaterialByRTStructNamesIndexIntoIsInNamedStructure.push_back(fStructureNames.size()-1);
				}
			}
		}

		// Need structure information if filtering by structure.
		// Make sure to do this only after the previous block where we load
		// structure names for Coloring by Structure. This is important so that
		// indexing of structureColors will always match indexing of structureNames
		// even when, due to filtering, there may be more structureNames than structureColors.
		std::vector<G4String>* values = new std::vector<G4String>;
		G4String prefix = "Sc";
		G4String suffix = "/OnlyIncludeIfInRTStructure";
		fPm->GetParameterNamesBracketedBy(prefix, suffix, values);
		G4int length = values->size();
		for (G4int iToken=0; iToken<length; iToken++) {
			parmName = (*values)[iToken];
			G4String* structureNames = fPm->GetStringVector(parmName);
			G4int structureNamesLength = fPm->GetVectorLength(parmName);
			for (G4int i = 0; i < structureNamesLength; i++) {
				// Loop over all elements already in the vector to avoid repetitions
				G4bool found = false;
				for (G4int j = 0; j < (int)fStructureNames.size() && !found; j++)
					if (structureNames[i] == fStructureNames[j])
						found = true;
				if (!found) {
					fStructureNames.push_back(structureNames[i]);
					fMaterialByRTStructMaterialNames.push_back("");
				}
			}
		}
		suffix = "/OnlyIncludeIfNotInRTStructure";
		fPm->GetParameterNamesBracketedBy(prefix, suffix, values);
		length = values->size();
		for (G4int iToken=0; iToken<length; iToken++) {
			parmName = (*values)[iToken];
			G4String* structureNames = fPm->GetStringVector(parmName);
			G4int structureNamesLength = fPm->GetVectorLength(parmName);
			for (G4int i = 0; i < structureNamesLength; i++) {
				// Loop over all elements already in the vector to avoid repetitions
				G4bool found = false;
				for (G4int j = 0; j < (int)fStructureNames.size() && !found; j++)
					if (structureNames[i] == fStructureNames[j])
						found = true;
				if (!found)
					fStructureNames.push_back(structureNames[i]);
			}
		}

		if (fStructureNames.size() > 0)
		{
			G4cout << "The following structures are needed for graphics, material overrides or score filtering:" << G4endl;
			for (G4int i=0; i < (int)fStructureNames.size(); i++)
				G4cout << fStructureNames[i] << G4endl;

			fIsInNamedStructure[fCurrentImageNumber].resize(fStructureNames.size());

			// Fake the structures
			if (fPm->ParameterExists(GetFullParmName("FakeStructures")) &&
				fPm->GetBooleanParameter(GetFullParmName("FakeStructures")))
			{
				G4cout << "Will fake structures as being all voxels in lower X and Y quadrant." << G4endl;
				G4int copyNo;
				for (G4int i=0; i < (int)fStructureNames.size(); i++) {
					fIsInNamedStructure[fCurrentImageNumber][i].resize(fTotalNumberOfVoxels);

					for (G4int iSlice=0; iSlice < fVoxelCountZ[0]; iSlice++) {
						for (G4int iY=0; iY < fVoxelCountY; iY++) {
							for (G4int iX=0; iX < fVoxelCountX; iX++) {
								copyNo = iY * fVoxelCountX * fVoxelCountZ[0] + iX * fVoxelCountZ[0] + iSlice;
								fIsInNamedStructure[fCurrentImageNumber][i][copyNo] = iY < fVoxelCountY /2 && iX < fVoxelCountX / 2;
							}
						}
					}
				}
			} else {

				// Use scanner to see if there are any structure set files
				std::vector<std::string> structure_set_filenames =
				scanner.GetAllFilenamesFromTagToValue(modalityTag, "RTSTRUCT");
				if (structure_set_filenames.size() == 0) {
					G4cout << "No files of modality RTSTRUCT in the specified DICOM directory." << G4endl;
					fPm->AbortSession(1);
				}
				if (fVerbosity>0)
					G4cout << "Found RTSTRUCT file: " << structure_set_filenames[0] << G4endl;

				// Read structure set file
				gdcm::Reader RTreader;
				RTreader.SetFileName( structure_set_filenames[0].c_str() );
				if (!RTreader.Read())
				{
					G4cout << "Failed on attempt to read data from RTSTRUCT file: " << structure_set_filenames[0] << G4endl;
					fPm->AbortSession(1);
				}
				const gdcm::DataSet& structureSetDataSet = RTreader.GetFile().GetDataSet();

				// Get region of interest sequence data
				gdcm::Tag regionOfInterestSequenceTag(0x3006,0x0020);
				if( !structureSetDataSet.FindDataElement( regionOfInterestSequenceTag ) )
				{
					G4cout << "Problem locating regionOfInterestSequenceTag - Is this a valid RTSTRUCT file?" << G4endl;
					fPm->AbortSession(1);
				}
				const gdcm::DataElement &regionOfInterestSequenceData = structureSetDataSet.GetDataElement( regionOfInterestSequenceTag );
				gdcm::SmartPointer<gdcm::SequenceOfItems> regionOfInterestSequenceOfItems = regionOfInterestSequenceData.GetValueAsSQ();
				if( !regionOfInterestSequenceOfItems || !regionOfInterestSequenceOfItems->GetNumberOfItems() )
				{
					G4cout << "No regions of interest found in RTSTRUCT file: " << structure_set_filenames[0] << G4endl;
					fPm->AbortSession(1);
				}

				// Get contour sequence data
				gdcm::Tag contourSequenceTag(0x3006,0x0039);
				if( !structureSetDataSet.FindDataElement( contourSequenceTag ) )
				{
					G4cout << "Problem locating contourSequenceTag - Is this a valid RTSTRUCT file?" << G4endl;
					fPm->AbortSession(1);
				}
				const gdcm::DataElement &contourSequenceData = structureSetDataSet.GetDataElement( contourSequenceTag );
				gdcm::SmartPointer<gdcm::SequenceOfItems> contourSequenceOfItems = contourSequenceData.GetValueAsSQ();
				if( !contourSequenceOfItems || !contourSequenceOfItems->GetNumberOfItems() )
				{
					G4cout << "No contours found in RTSTRUCT file: " << structure_set_filenames[0] << G4endl;
					fPm->AbortSession(1);
				}
				G4int numberOfStructures = contourSequenceOfItems->GetNumberOfItems();
				std::vector<G4String> allStructureNames;

				// Cross-check Frame of Reference UID
				gdcm::Attribute<0x0020,0x0052> atFrameOfReferenceUID;
				atFrameOfReferenceUID.SetFromDataElement( structureSetDataSet.GetDataElement( atFrameOfReferenceUID.GetTag()));
				G4String rtstructFrameOfReferenceUID = atFrameOfReferenceUID.GetValue();
				if ( ( !fPm->ParameterExists(GetFullParmName("IgnoreInconsistentFrameOfReferenceUID")) ||
					   !fPm->GetBooleanParameter(GetFullParmName("IgnoreInconsistentFrameOfReferenceUID")) ) &&
					 rtstructFrameOfReferenceUID != fFrameOfReferenceUID ) {
					G4cerr << "Patient image and RTSTRUCT files have inconsistent Frame of Reference UID" << G4endl;
					G4cerr << "Image: " << fFrameOfReferenceUID << G4endl << G4endl;
					G4cerr << "RTSTRUCT: " << rtstructFrameOfReferenceUID << G4endl;
					G4cerr << "To disable this check, set b:" << GetFullParmName("IgnoreInconsistentFrameOfReferenceUID") << " = \"True\"" << G4endl;
					fPm->AbortSession(1);
				}

				// Loop through contoured structures
				for(G4int iContourStructure = 0; iContourStructure < numberOfStructures; ++iContourStructure)
				{
					const gdcm::Item & item = contourSequenceOfItems->GetItem(iContourStructure+1); // Item start at #1
					gdcm::Attribute<0x3006,0x0084> roinumber;
					const gdcm::DataSet& nestedds = item.GetNestedDataSet();
					roinumber.SetFromDataElement( nestedds.GetDataElement( roinumber.GetTag() ) );

					// find structure_set_roi_sequence corresponding to roi_contour_sequence (by comparing id numbers)
					unsigned int spd = 0;
					gdcm::Item & sitem = regionOfInterestSequenceOfItems->GetItem(spd+1);
					gdcm::DataSet& snestedds = sitem.GetNestedDataSet();
					gdcm::Attribute<0x3006,0x0022> sroinumber;
					do {
						sitem = regionOfInterestSequenceOfItems->GetItem(spd+1);
						snestedds = sitem.GetNestedDataSet();
						sroinumber.SetFromDataElement( snestedds.GetDataElement( sroinumber.GetTag() ) );
						spd++;
					} while ( sroinumber.GetValue()  != roinumber.GetValue() );

					gdcm::Tag stcsq(0x3006,0x0026);
					if( !snestedds.FindDataElement( stcsq ) )
					{
						G4cout << "Did not find sttsq data el " << stcsq << "   continuing..." << G4endl;
						continue;
					}
					const gdcm::DataElement &sde = snestedds.GetDataElement( stcsq );

					//(3006,002a) IS [255\192\96]                              # 10,3 ROI Display Color
					gdcm::Tag troidc(0x3006,0x002a);
					gdcm::Attribute<0x3006,0x002a> color = {};
					if( nestedds.FindDataElement( troidc) )
					{
						const gdcm::DataElement &decolor = nestedds.GetDataElement( troidc );
						color.SetFromDataElement( decolor );
					}
					//(3006,0040) SQ (Sequence with explicit length #=8)      # 4326, 1 ContourSequence
					gdcm::Tag tcsq(0x3006,0x0040);
					if( !nestedds.FindDataElement( tcsq ) )
					{
						continue;
					}
					const gdcm::DataElement& csq = nestedds.GetDataElement( tcsq );

					gdcm::SmartPointer<gdcm::SequenceOfItems> sqi2 = csq.GetValueAsSQ();
					if( !sqi2 || !sqi2->GetNumberOfItems() )
					{
						G4cout << "csq: " << csq << G4endl;
						G4cout << "sqi2: " << *sqi2 << G4endl;
						G4cout << "Did not find sqi2 or no. items == 0   " <<  sqi2->GetNumberOfItems() << "   continuing..." << G4endl;
						continue;
					}
					unsigned int nitems = sqi2->GetNumberOfItems();
					std::string structureName(sde.GetByteValue()->GetPointer(), sde.GetByteValue()->GetLength());
					G4cout << "Found a structure named: " << structureName << G4endl;

					// Trim the structure name and replace any spaces with underscores
					static const char whitespace[] = " \n\t\v\r\f";
					structureName.erase( 0, structureName.find_first_not_of(whitespace) );
					structureName.erase( structureName.find_last_not_of(whitespace) + 1U );
					std::replace(structureName.begin(), structureName.end(), ' ', '_');
					allStructureNames.push_back(structureName);

					// See if we need this structure
					G4int structureIndex;
					for (structureIndex = 0; structureIndex < (int)fStructureNames.size(); structureIndex++)
						if (structureName == fStructureNames[structureIndex])
							break;

					if (structureIndex < (int)fStructureNames.size()) {
						G4cout << "  Now processing " << nitems << " contours for this structure." << G4endl;

						// Allow room to store one boolean per voxel and preset all to false.
						fIsInNamedStructure[fCurrentImageNumber][structureIndex].resize(fTotalNumberOfVoxels);

						for (G4int iVoxel = 0; iVoxel < fTotalNumberOfVoxels; iVoxel++)
							fIsInNamedStructure[fCurrentImageNumber][structureIndex].push_back(false);

						//now loop through each item for this structure (eg one prostate region on a single slice is an item)
						G4int firstSliceInStructure = 100000;
						G4int lastSliceInStructure = 0;
						for(unsigned int i = 0; i < nitems; ++i)
						{
							const gdcm::Item & item2 = sqi2->GetItem(i+1); // Item start at #1
							const gdcm::DataSet& nestedds2 = item2.GetNestedDataSet();
							// (3006,0050) DS [43.57636\65.52504\-10.0\46.043102\62.564945\-10.0\49.126537\60.714... # 398,48 ContourData
							gdcm::Tag tcontourdata(0x3006,0x0050);
							const gdcm::DataElement & contourdata = nestedds2.GetDataElement( tcontourdata );
							gdcm::Attribute<0x3006,0x0050> at;
							at.SetFromDataElement( contourdata );
							const double* pts = at.GetValues();
							G4int npts = at.GetNumberOfValues() / 3;
							G4int copyNo;
							//G4cout << "Number of points coming: " << npts << G4endl;

							if (npts < 3) {
								G4cout << "  Ignoring a contour since it has fewer than 3 points" << G4endl;
							} else {
								// Find the slice number
								G4bool found = false;
								for (G4int iSlice=0; iSlice < fVoxelCountZ[0] && !found; iSlice++) {
									if (pts[2] == slicePositions[iSlice]) {
										found = true;

										if (iSlice < firstSliceInStructure)
											firstSliceInStructure = iSlice;
										if (iSlice > lastSliceInStructure)
											lastSliceInStructure = iSlice;

										// Create new set of x and y points for closed polygon (so one more than npts)
										double* xVtx = new double[npts+1];
										double* yVtx = new double[npts+1];
										G4int jPt=0;
										for (G4int iPt=0; iPt < npts; iPt++, jPt+=3) {
											xVtx[iPt] = pts[jPt]   + fTransFirstVoxelCenterRelToComponentCenter.x() - fTransFirstVoxelCenterRelToDicom.x();
											yVtx[iPt] = pts[jPt+1] + fTransFirstVoxelCenterRelToComponentCenter.y() - fTransFirstVoxelCenterRelToDicom.y();
										}
										xVtx[npts] = xVtx[0];
										yVtx[npts] = yVtx[0];

										// For every voxel in this slice, set whether it is inside or outside the contour.
										// Algorithm is from: http://geomalgorithms.com/a03-_inclusion.html
										for (G4int iY=0; iY < fVoxelCountY; iY++) {
											G4double pixelY = iY * voxelSizeY - fFullWidths[1] / 2.;

											for (G4int iX=0; iX < fVoxelCountX; iX++) {
												G4double pixelX = iX * voxelSizeX - fFullWidths[0] / 2.;

												// loop through all edges of the polygon
												G4int wn=0;
												for (G4int iPt=0; iPt < npts; iPt++) {
													if (yVtx[iPt] <= pixelY) {          // start y <= pixelY
														if (yVtx[iPt+1]  > pixelY)      // an upward crossing
															if ((xVtx[iPt+1] - xVtx[iPt]) * (pixelY - yVtx[iPt]) - (pixelX -  xVtx[iPt]) * (yVtx[iPt+1] - yVtx[iPt]) > 0)  // P left of edge
																++wn;            // have  a valid up intersect

													} else {                        // start y > pixelY (no test needed)
														if (yVtx[iPt+1]  <= pixelY)     // a downward crossing
															if ((xVtx[iPt+1] - xVtx[iPt]) * (pixelY - yVtx[iPt]) - (pixelX -  xVtx[iPt]) * (yVtx[iPt+1] - yVtx[iPt]) < 0)  // P right of edge
																--wn;            // have  a valid down intersect
													}
												}
												if (wn !=0) {
													copyNo = iX * fVoxelCountY * fVoxelCountZ[0] + iY * fVoxelCountZ[0] + iSlice;
													fIsInNamedStructure[fCurrentImageNumber][structureIndex][copyNo] = true;
												}
											}
										}
									}
								}
							}
						}

						G4cout << "  Structure contours extend from slice: " << firstSliceInStructure << " to slice: " << lastSliceInStructure << G4endl;
					}
				}

				// Check whether we are missing any required structures
				for (G4int i=0; i < (int)fStructureNames.size(); i++) {
					G4bool found = false;
					for (G4int j=0; j < (int)allStructureNames.size() && !found; j++) {
						//G4cout << "fStructureNames[i] " << fStructureNames[i] << ", allStructureNames[j]: " << allStructureNames[j] << G4endl;
						//G4cout << "fStructureNames[i].size " << fStructureNames[i].size() << ", allStructureNames[j].size: " << allStructureNames[j].size() << G4endl;
						if (fStructureNames[i]==allStructureNames[j])
							found = true;
					}
					if (!found) {
						G4cout << "Required structure " << fStructureNames[i] << " not found in RTSTRUCT file: " << structure_set_filenames[0] << G4endl;
						fPm->AbortSession(1);
					}
				}
			}
		}
		fCurrentImageNumber++;
		
		if (fPm->ParameterExists(GetFullParmName("CloneRTDoseGridFrom")) || 
			fPm->ParameterExists(GetFullParmName("CloneRTDoseGridSize"))){
				CreateDoseGrid();
		}
			
	}
}


G4VPhysicalVolume* TsDicomPatient::Construct() {
	fEnvelopePhys = TsVPatient::Construct();

	if (fCreateDoseGrid) {
		TsVGeometryComponent* doseGridComponent = InstantiateChild(fName + "/RTDoseGrid");
		doseGridComponent->Construct();
		doseGridComponent->SetOriginalComponent(this);
	}

	return fEnvelopePhys;
}

void TsDicomPatient::CreateDoseGrid() {

	fCreateDoseGrid = true;
	G4int    nXYZ[3];
	G4double dXYZ[3];
	G4double lXYZ[3]; //set by RTDOSE or user input

	//Vector between RTDOSE to its mother component's center
	//This vector is set from RTDOSE or (0,0,0) by default. 
	//i.e., users have no control for positioning RTDOSE
	G4Point3D transDoseGridCenterRelToComponentCenter ; 

	bool IsFromRTDose = false;
	if(fPm->ParameterExists(GetFullParmName("CloneRTDoseGridFrom"))){
		IsFromRTDose = true;
		G4String rtDoseFile = fPm->GetStringParameter(GetFullParmName("CloneRTDoseGridFrom"));
		gdcm::ImageReader reader;
    	reader.SetFileName(rtDoseFile.c_str());	

		if(!reader.Read()){
			G4cerr << "Topas is exiting because of a problem in DICOM setup." << G4endl;
			G4cerr << "Unable to read file: " << rtDoseFile << G4endl;
			G4cerr << "specified in parameter: " << GetFullParmName("CloneRTDoseGridFrom") << G4endl;
			fPm->AbortSession(1);
		}
		const gdcm::Image &image = reader.GetImage();

		nXYZ[0] = image.GetDimensions()[0]; 
		nXYZ[1] = image.GetDimensions()[1];
    	nXYZ[2] = image.GetDimensions()[2];

	    dXYZ[0] = image.GetSpacing()[0]*mm;
	    dXYZ[1] = image.GetSpacing()[1]*mm;
	    dXYZ[2] = image.GetSpacing()[2]*mm;

    	lXYZ[0] = nXYZ[0]*dXYZ[0];
    	lXYZ[1] = nXYZ[1]*dXYZ[1];
    	lXYZ[2] = nXYZ[2]*dXYZ[2];

		gdcm::File &file = reader.GetFile();
    	gdcm::DataSet &ds = file.GetDataSet();

    	gdcm::Attribute<0x3004,0x0002> gDoseUnits             ;
    	gdcm::Attribute<0x3004,0x000e> gDoseGridScaling       ;
    	gdcm::Attribute<0x0028,0x0009> gFrameIncrementPointer ;
    	gdcm::Attribute<0x3004,0x000c> gGridFrameOffsetVector ;

    	gDoseUnits.SetFromDataElement( ds.GetDataElement( gDoseUnits.GetTag()));
    	gDoseGridScaling.SetFromDataElement( ds.GetDataElement( gDoseGridScaling.GetTag()));

    	gFrameIncrementPointer.SetFromDataElement( ds.GetDataElement( gFrameIncrementPointer.GetTag()));

    	G4double DoseScaling = gDoseGridScaling.GetValue();
    	G4String DoseUnit    = gDoseUnits.GetValue();

    	// Cross-check Frame of Reference UID
    	gdcm::Attribute<0x0020,0x0052> atFrameOfReferenceUID;
    	atFrameOfReferenceUID.SetFromDataElement( ds.GetDataElement( atFrameOfReferenceUID.GetTag()));
    	G4String rtdoseFrameOfReferenceUID = atFrameOfReferenceUID.GetValue();
    	if ( (!fPm->ParameterExists(GetFullParmName("IgnoreInconsistentFrameOfReferenceUID")) ||
    		  !fPm->GetBooleanParameter(GetFullParmName("IgnoreInconsistentFrameOfReferenceUID"))) &&
    			rtdoseFrameOfReferenceUID != fFrameOfReferenceUID ) {
    		G4cerr << "Patient image and RTDOSE grid have inconsistent Frame of Reference UID" << G4endl;
    		G4cerr << "Image: " << fFrameOfReferenceUID << G4endl << G4endl;
    		G4cerr << "RTDOSE: " << rtdoseFrameOfReferenceUID << G4endl;
    		G4cerr << "To disable this check, set b:" << GetFullParmName("IgnoreInconsistentFrameOfReferenceUID") << " = \"True\"" << G4endl;
    		fPm->AbortSession(1);
    	}

		//Calculates vector between first pixel of RTDOSE to DICOM origin
    	G4Point3D transDoseGridFirstVoxelCenterRelToDicom          ; 
		transDoseGridFirstVoxelCenterRelToDicom.setX(image.GetOrigin()[0]*mm);
		transDoseGridFirstVoxelCenterRelToDicom.setY(image.GetOrigin()[1]*mm);
		transDoseGridFirstVoxelCenterRelToDicom.setZ(image.GetOrigin()[2]*mm);

		//Calculates vector between first pixel of RTDOSE to RTDOSE center
		G4Point3D transDoseGridFirstVoxelCenterRelToDoseGridCenter ; 
    	if (gFrameIncrementPointer.GetValue() == gdcm::Tag(0x3004,0x000c)){
        	gGridFrameOffsetVector.SetFromDataElement( ds.GetDataElement( gGridFrameOffsetVector.GetTag()));
        	const G4double* offset_vector = gGridFrameOffsetVector.GetValues();
        	const G4int     offset_size   = gGridFrameOffsetVector.GetNumberOfValues();
        	G4double thickness = std::abs(offset_vector[1] - offset_vector[0]);
        	for( int i = 1 ; i < offset_size - 2 ; ++i){
				G4double new_thickness = std::abs(offset_vector[i+1] - offset_vector[i]);
            	if ( std::abs(new_thickness-thickness)/thickness > 0.02 ){
                	G4cerr<<i+1<<"-th GridFrameOffset is greater than 2% of first slice thickness."<<G4endl;
                	fPm->AbortSession(1);
            	}
        	}
        	transDoseGridFirstVoxelCenterRelToDoseGridCenter.setX(-0.5*dXYZ[0] * (nXYZ[0]-1));
			transDoseGridFirstVoxelCenterRelToDoseGridCenter.setY(-0.5*dXYZ[1] * (nXYZ[1]-1));
			transDoseGridFirstVoxelCenterRelToDoseGridCenter.setZ(-0.5*offset_vector[offset_size-1]);
		}else{
        	G4cerr << "FrameIncrementPointer in RTDOSE does not point GridFrameOffsetVector!" << G4endl;
        	fPm->AbortSession(1);
    	}

		transDoseGridCenterRelToComponentCenter = 
			(transDoseGridFirstVoxelCenterRelToDicom - transDoseGridFirstVoxelCenterRelToDoseGridCenter)
			- (fTransFirstVoxelCenterRelToDicom - fTransFirstVoxelCenterRelToComponentCenter);
		
		G4cout << "RTDOSE file is found: "<< rtDoseFile << G4endl;
    	G4cout << "  # of Voxels: ("<< nXYZ[0] <<", " << nXYZ[1] << ", "<<nXYZ[2]<<") "<<G4endl;
    	G4cout << "  Voxel Dimensions X,Y,Z [mm]: ("<< dXYZ[0]/mm <<", " << dXYZ[1]/mm << ", "<<dXYZ[2]/mm<<")"<<G4endl;
    	G4cout << "  ImagePositionPatient (mm): "<< transDoseGridFirstVoxelCenterRelToDicom <<G4endl;
    	G4cout << "  DoseGrid center (mm):" << (transDoseGridFirstVoxelCenterRelToDicom - transDoseGridFirstVoxelCenterRelToDoseGridCenter) << G4endl;
    	G4cout << "  Image size (bytes): "<< image.GetBufferLength()<<G4endl;
    	G4cout << "  DoseUnit:"<< DoseUnit<<G4endl;
    	G4cout << "  DoseScaling:"<< DoseScaling<<G4endl;

	}
	
	// If DoseGridSize is specified, DICOM values are override
	if(fPm->ParameterExists(GetFullParmName("CloneRTDoseGridSize"))){
		G4double* pixel_size = fPm->GetDoubleVector(GetFullParmName("CloneRTDoseGridSize"),"Length");
		dXYZ[0] = pixel_size[0];
		dXYZ[1] = pixel_size[1];
		dXYZ[2] = pixel_size[2];
		
		//Resize RTDOSE grid
		if (!IsFromRTDose){
			//If RTDOSE is not given
			//we extend RTDOSE to cover CT but with different voxel size
			nXYZ[0] = std::ceil(fFullWidths[0]/dXYZ[0]);
			nXYZ[1] = std::ceil(fFullWidths[1]/dXYZ[1]);
			nXYZ[2] = std::ceil(fFullWidths[2]/dXYZ[2]);
		}else{
			//If RTDOSE is given but pixel size is adjusted,
			//we change the RTDOSE size accordingly.
			nXYZ[0] = std::ceil(lXYZ[0]/dXYZ[0]);
			nXYZ[1] = std::ceil(lXYZ[1]/dXYZ[1]);
			nXYZ[2] = std::ceil(lXYZ[2]/dXYZ[2]);
		}
		lXYZ[0] = dXYZ[0]* nXYZ[0];
		lXYZ[1] = dXYZ[1]* nXYZ[1];
		lXYZ[2] = dXYZ[2]* nXYZ[2];
		G4cout<<"Dose grid size is re-defined as "<< dXYZ[0] <<", " << dXYZ[1] <<", " << dXYZ[2] << G4endl;
	}

    G4String gridName = fName + "/RTDoseGrid";
	G4String parameterName;
	G4String transValue;

    //Following shouldn't be uncommented.
    //with this parameter, the grid doesn't work properly for dose scoring.
	//parameterName = "s:Ge/" + gridName + "/Parent";
    //transValue = "\"" + fName+ "\"";
	//G4cout<<"fName: "<<fName << G4endl;
    //fPm->AddParameter(parameterName, transValue);

    parameterName = "d:Ge/" + gridName + "/HLX";
    transValue = G4UIcommand::ConvertToString(lXYZ[0]/2.) + " mm";
    fPm->AddParameter(parameterName, transValue);

    parameterName = "d:Ge/" + gridName + "/HLY";
    transValue = G4UIcommand::ConvertToString(lXYZ[1]/2.) + " mm";
    fPm->AddParameter(parameterName, transValue);

    parameterName = "d:Ge/" + gridName + "/HLZ";
    transValue = G4UIcommand::ConvertToString(lXYZ[2]/ 2.) + " mm";
    fPm->AddParameter(parameterName, transValue);

    parameterName = "d:Ge/" + gridName + "/TransX";
    transValue = G4UIcommand::ConvertToString(transDoseGridCenterRelToComponentCenter.x()/mm) + " mm ";
    fPm->AddParameter(parameterName, transValue);

	parameterName = "d:Ge/" + gridName + "/TransY";
    transValue = G4UIcommand::ConvertToString(transDoseGridCenterRelToComponentCenter.y()/mm) + " mm ";
    fPm->AddParameter(parameterName, transValue);

	parameterName = "d:Ge/" + gridName + "/TransZ";
    transValue = G4UIcommand::ConvertToString(transDoseGridCenterRelToComponentCenter.z()/mm) + " mm ";
    fPm->AddParameter(parameterName, transValue);

    parameterName = "d:Ge/" + gridName + "/RotX";
    transValue = "0. deg";
    fPm->AddParameter(parameterName, transValue);

    parameterName = "d:Ge/" + gridName + "/RotY";
    transValue = "0. deg";
    fPm->AddParameter(parameterName, transValue);

    parameterName = "d:Ge/" + gridName + "/RotZ";
    transValue = "0. deg";
    fPm->AddParameter(parameterName, transValue);

    parameterName = "i:Ge/" + gridName + "/XBins";
    transValue = G4UIcommand::ConvertToString(nXYZ[0]);
    fPm->AddParameter(parameterName, transValue);

    parameterName = "i:Ge/" + gridName + "/YBins";
    transValue = G4UIcommand::ConvertToString(nXYZ[1]);
    fPm->AddParameter(parameterName, transValue);

    parameterName = "i:Ge/" + gridName + "/ZBins";
    transValue = G4UIcommand::ConvertToString(nXYZ[2]);
    fPm->AddParameter(parameterName, transValue);

    parameterName = "s:Ge/" + gridName + "/Type";
    transValue = "\"TsBox\"";
    fPm->AddParameter(parameterName, transValue);

    parameterName = "b:Ge/" + gridName + "/IsParallel";
    transValue = "\"True\"";
    fPm->AddParameter(parameterName, transValue);
}


G4String TsDicomPatient::GetFirstFileName() {
	return fFirstFileName;
}


G4Point3D TsDicomPatient::GetTransFirstVoxelCenterRelToDicom() {
	return fTransFirstVoxelCenterRelToDicom;
}
