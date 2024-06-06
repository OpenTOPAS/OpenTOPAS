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

#include "TsImageCube.hh"

#include "TsParameterManager.hh"

#include "TsVImagingToMaterial.hh"

#include "G4UIcommand.hh"

#include <fstream>

TsImageCube::TsImageCube(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
						   TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name)
:TsVPatient(pM, eM, mM, gM, parentComponent, parentVolume, name) {
}


TsImageCube::~TsImageCube() {
}


void TsImageCube::UpdateForSpecificParameterChange(G4String parameter)
{
	if (parameter == GetFullParmNameLower("InputDirectory") || parameter == GetFullParmNameLower("InputFile")) {
		ReadImage();
		fNeedToSetImage = true;
	} else
		// For any other parameters, fall back to the base class Update method
		TsVPatient::UpdateForSpecificParameterChange(parameter);
}


void TsImageCube::ReadImage() {
	fDataType = fImagingToMaterial->GetDataType();
	fUnrestrictedVoxelCountX = fImagingToMaterial->GetNumberOfVoxelsX();
	fVoxelCountX = fmin(fUnrestrictedVoxelCountX, fRestrictVoxelsXMax) - fRestrictVoxelsXMin + 1;

	if (fVoxelCountX < 1) {
		G4cerr << "Error: No voxels in this image are in range RestrictVoxelsXMin = " << fRestrictVoxelsXMin <<
		" and RestrictVoxelsXMax = " << fRestrictVoxelsXMax << G4endl;
		fPm->AbortSession(1);
	}

	fFullWidths[0] = fVoxelCountX * fImagingToMaterial->GetVoxelSizeX();

	fUnrestrictedVoxelCountY = fImagingToMaterial->GetNumberOfVoxelsY();
	fVoxelCountY = fmin(fUnrestrictedVoxelCountY, fRestrictVoxelsYMax) - fRestrictVoxelsYMin + 1;

	if (fVoxelCountY < 1) {
		G4cerr << "Error: No voxels in this image are in range RestrictVoxelsYMin = " << fRestrictVoxelsYMin <<
		" and RestrictVoxelsYMax = " << fRestrictVoxelsYMax << G4endl;
		fPm->AbortSession(1);
	}

	fFullWidths[1] = fVoxelCountY * fImagingToMaterial->GetVoxelSizeY();

	// Number of voxels and overall patient size in Z may involve multiple slice thickness sections
	G4int nImageSections = fImagingToMaterial->GetNumberOfZSections();
	G4int* voxelCountZ = fImagingToMaterial->GetNumberOfVoxelsZ();
	G4double* voxelSizeZ = fImagingToMaterial->GetVoxelSizeZ();

	// Need to adjust number of image sections, voxelCountZ and voxelSizeZ to account for fRestrictVoxelsZ.
	// First get total number of slices and see how many slice thickness sections will survive the restrictions.
	fNImageSections = 0;
	G4bool foundKeptSliceThisSection;
	G4int iTotalSlice = 0;
	for (G4int iSection = 0; iSection < nImageSections; iSection++) {
		foundKeptSliceThisSection = false;
		for (G4int iSlice = 0; iSlice < voxelCountZ[iSection]; iSlice++) {
			iTotalSlice++;
			if (!foundKeptSliceThisSection && iTotalSlice >= fRestrictVoxelsZMin && iTotalSlice <= fRestrictVoxelsZMax) {
				foundKeptSliceThisSection = true;
				fNImageSections++;
			}
		}
	}
	fUnrestrictedVoxelCountZ = iTotalSlice;

	// Work out size per slice.
	G4double* sizePerSlice = new G4double[fUnrestrictedVoxelCountZ];
	iTotalSlice = 0;
	for (G4int iSection = 0; iSection < nImageSections; iSection++)
		for (G4int iSlice = 0; iSlice < voxelCountZ[iSection]; iSlice++)
			sizePerSlice[iTotalSlice++] = voxelSizeZ[iSection];

	// Work out new slice thickness sections after resrictions.
	fVoxelCountZ = new G4int[fNImageSections];
	fFirstOriginalSliceThisSection = new G4int[fNImageSections];
	fLastOriginalSliceThisSection = new G4int[fNImageSections];
	fVoxelSizeZ = new G4double[fNImageSections];
	fNImageSections = 0;
	iTotalSlice = 0;
	for (G4int iSection = 0; iSection < nImageSections; iSection++) {
		foundKeptSliceThisSection = false;
		for (G4int iSlice = 0; iSlice < voxelCountZ[iSection]; iSlice++) {
			iTotalSlice++;
			if (iTotalSlice >= fRestrictVoxelsZMin && iTotalSlice <= fRestrictVoxelsZMax) {
				if (foundKeptSliceThisSection) {
					fVoxelCountZ[fNImageSections-1] = fVoxelCountZ[fNImageSections-1] + 1;
					fLastOriginalSliceThisSection[fNImageSections-1] = iTotalSlice;
				} else {
					foundKeptSliceThisSection = true;
					fNImageSections++;
					fVoxelCountZ[fNImageSections-1] = 1;
					fVoxelSizeZ[fNImageSections-1] = sizePerSlice[iTotalSlice-1];
					fFirstOriginalSliceThisSection[fNImageSections-1] = iTotalSlice;
					fLastOriginalSliceThisSection[fNImageSections-1] = iTotalSlice;
				}
			}
		}
	}

	if (fNImageSections ==0) {
		G4cerr << "Error: No voxels in this image are in range RestrictVoxelsZMin = " << fRestrictVoxelsZMin <<
		" and RestrictVoxelsZMax = " << fRestrictVoxelsZMax << G4endl;
		fPm->AbortSession(1);
	}

	fCurrentImageName = fPm->GetStringParameter(GetFullParmName("InputDirectory")) +
	fPm->GetStringParameter(GetFullParmName("InputFile"));

	// If the current image name is not yet in fImageNames, read the image file
	if (fImageNames.find(fCurrentImageName) == fImageNames.end()) {

		G4int dotpos = fCurrentImageName.find_last_of(".");

		if (fNEnergies == 1)
			G4cout << "\nTOPAS will read image file: " << fCurrentImageName << G4endl;
		else {
			G4cout << "\nTOPAS will read " << fNEnergies << " energy-specific image files with names:" << G4endl;
			for (G4int iEnergy=0; iEnergy < fNEnergies; iEnergy++)
				G4cout << "  " << fCurrentImageName.substr(0, dotpos) << "_" << iEnergy+1 << fCurrentImageName.substr(dotpos) << G4endl;
		}

		if (fDataType == "s") {
			G4cout << "Image data type is Short" << G4endl;
			fRawImageShort.clear();
			fRawImageShort.resize(fNEnergies);
		} else if (fDataType == "i") {
			G4cout << "Image data type is Int" << G4endl;
			fRawImageInt.clear();
			fRawImageInt.resize(fNEnergies);
		} else if (fDataType == "f") {
			G4cout << "Image data type is Float" << G4endl;
			fRawImageFloat.clear();
			fRawImageFloat.resize(fNEnergies);
		}

		// Loop over multiple imaging energies
		for (G4int iEnergy = 0; iEnergy < fNEnergies; iEnergy++) {

			G4String fullImageName;
			if (fNEnergies == 1)
				fullImageName = fCurrentImageName;
			else
				fullImageName = fCurrentImageName.substr(0, dotpos) + "_" + G4UIcommand::ConvertToString(iEnergy+1)
				+ fCurrentImageName.substr(dotpos);

			// Open image
			std::ifstream patientIn;
			patientIn.open(fullImageName, std::ios::in & std::ios::binary);
			if (!patientIn) {
				G4cerr << "Unable to read image File: " << fullImageName << G4endl;
				fPm->AbortSession(1);
			}

			// Z width and total number of slices depends on whether we are only loading one slice or all slices
			SetUpZSlices();

			// Read the pixels into fRawImage
			char* imageBuffer;
			G4int elementSize = 0;
			
			if (fDataType == "s") elementSize = sizeof(signed short);
			else if (fDataType == "i") elementSize = sizeof(int);
			else if (fDataType == "f") elementSize = sizeof(float);

			G4int totalNumberOfVoxels = fUnrestrictedVoxelCountX * fUnrestrictedVoxelCountY * fUnrestrictedVoxelCountZ;
			imageBuffer = new char[totalNumberOfVoxels*elementSize];
			patientIn.read(imageBuffer, totalNumberOfVoxels*elementSize);
			if ( patientIn.fail() ) {
				G4cerr << "TOPAS experienced a serious error while loading image cube." << G4endl;
				G4cerr << "Size of the data file does not match what we calculate from data type size times number of voxels."  << G4endl;
				G4cerr << "Check that the number of voxels and datatype parameters make sense for this data file." << G4endl;
				fPm->AbortSession(1);
			}
			patientIn.peek();
			if ( !patientIn.eof() ) {
				G4cerr << "TOPAS experienced a serious error while loading image cube." << G4endl;
				G4cerr << "Size of the data file does not match what we calculate from data type size times number of voxels."  << G4endl;
				G4cerr << "Check that the number of voxels and datatype parameters make sense for this data file." << G4endl;
				fPm->AbortSession(1);
			}
			patientIn.close();

			G4int iVoxel = 0;
			if (fDataType == "s") {
				signed short* ImagingValue= (signed short *) imageBuffer;
				for (G4int iZ=1; iZ<=fUnrestrictedVoxelCountZ; iZ++) {
					for (G4int iY=1; iY<=fUnrestrictedVoxelCountY; iY++) {
						for (G4int iX=1; iX<=fUnrestrictedVoxelCountX; iX++) {
							if ((iX >= fRestrictVoxelsXMin) && (iX <= fRestrictVoxelsXMax) &&
								(iY >= fRestrictVoxelsYMin) && (iY <= fRestrictVoxelsYMax) &&
								(iZ >= fRestrictVoxelsZMin) && (iZ <= fRestrictVoxelsZMax)) {
								//G4cout << "Pixel: " << iVoxel << " has signed short value: " << ImagingValue[iVoxel] << G4endl;
								fRawImageShort[iEnergy].push_back((signed short) ImagingValue[iVoxel]);
							}
							iVoxel++;
						}
					}
				}
			} else if (fDataType == "i") {
				int* ImagingValue= (int *) imageBuffer;
				for (G4int iZ=1; iZ<=fUnrestrictedVoxelCountZ; iZ++) {
					for (G4int iY=1; iY<=fUnrestrictedVoxelCountY; iY++) {
						for (G4int iX=1; iX<=fUnrestrictedVoxelCountX; iX++) {
							if ((iX >= fRestrictVoxelsXMin) && (iX <= fRestrictVoxelsXMax) &&
								(iY >= fRestrictVoxelsYMin) && (iY <= fRestrictVoxelsYMax) &&
								(iZ >= fRestrictVoxelsZMin) && (iZ <= fRestrictVoxelsZMax)) {
								//G4cout << "Pixel: " << iVoxel << " has int value: " << ImagingValue[iVoxel] << G4endl;
								fRawImageInt[iEnergy].push_back((int) ImagingValue[iVoxel]);
							}
							iVoxel++;
						}
					}
				}
			} else if (fDataType == "f") {
				float* ImagingValue= (float *) imageBuffer;
				for (G4int iZ=1; iZ<=fUnrestrictedVoxelCountZ; iZ++) {
					for (G4int iY=1; iY<=fUnrestrictedVoxelCountY; iY++) {
						for (G4int iX=1; iX<=fUnrestrictedVoxelCountX; iX++) {
							if ((iX >= fRestrictVoxelsXMin) && (iX <= fRestrictVoxelsXMax) &&
								(iY >= fRestrictVoxelsYMin) && (iY <= fRestrictVoxelsYMax) &&
								(iZ >= fRestrictVoxelsZMin) && (iZ <= fRestrictVoxelsZMax)) {
								//G4cout << "Pixel: " << iVoxel << " has float value: " << ImagingValue[iVoxel] << G4endl;
								fRawImageFloat[iEnergy].push_back((float) ImagingValue[iVoxel]);
							}
							iVoxel++;
						}
					}
				}
			}
			delete imageBuffer;
		}
	}
}
