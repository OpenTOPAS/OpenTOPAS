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

#include "TsXiOPatient.hh"

#include "TsParameterManager.hh"

#include "G4UIcommand.hh"

#include <fstream>

TsXiOPatient::TsXiOPatient(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
						   TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name)
:TsVPatient(pM, eM, mM, gM, parentComponent, parentVolume, name) {
}


TsXiOPatient::~TsXiOPatient() {
}


void TsXiOPatient::UpdateForSpecificParameterChange(G4String parameter)
{
	if (parameter == GetFullParmNameLower("InputDirectory") || parameter == GetFullParmNameLower("InputFile")) {
		ReadImage();
		fNeedToSetImage = true;
	} else
		// For any other parameters, fall back to the base class Update method
		TsVPatient::UpdateForSpecificParameterChange(parameter);
}


void TsXiOPatient::ReadImage() {

	fCurrentImageName = fPm->GetStringParameter(GetFullParmName("InputDirectory")) +
	fPm->GetStringParameter(GetFullParmName("InputFile"));

	// If the current image name is not yet in fImageNames, read the image file
	if (fImageNames.find(fCurrentImageName) == fImageNames.end()) {

		G4int dotpos = fCurrentImageName.find_last_of(".");

		if (fNEnergies == 1)
			G4cout << "TOPAS will read XiO file: " << fCurrentImageName << G4endl;
		else {
			G4cout << "TOPAS will read " << fNEnergies << " energy-specific XiO files with names:" << G4endl;
			for (G4int iEnergy=0; iEnergy < fNEnergies; iEnergy++)
				G4cout << "  " << fCurrentImageName.substr(0, dotpos) << "_" << iEnergy+1 << fCurrentImageName.substr(dotpos) << G4endl;
		}

		fRawImageShort.clear();
		fRawImageShort.resize(fNEnergies);

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
				G4cerr << "Unable to read XiO File: " << fullImageName << G4endl;
				fPm->AbortSession(1);
			}

			// Calculate overall patient size in X and Y
			fVoxelCountX = fPm->GetIntegerParameter(GetFullParmName("NumberOfVoxelsX"));
			fFullWidths[0] = fVoxelCountX * fPm->GetDoubleParameter(GetFullParmName("VoxelSizeX"), "Length");
			fVoxelCountY = fPm->GetIntegerParameter(GetFullParmName("NumberOfVoxelsY"));
			fFullWidths[1] = fVoxelCountY * fPm->GetDoubleParameter(GetFullParmName("VoxelSizeY"), "Length");

			// Number of voxels and overall patient size in Z may involve multiple slice thickness sections
			fNImageSections = fPm->GetVectorLength(GetFullParmName("NumberOfVoxelsZ"));
			if (fPm->GetVectorLength(GetFullParmName("VoxelSizeZ")) != fNImageSections) {
				G4cout << "ERROR: NumberVoxelsZ needs to have same number of entries as VoxelSizeZ." << G4endl;
				fPm->AbortSession(1);
			}
			fVoxelCountZ = fPm->GetIntegerVector(GetFullParmName("NumberOfVoxelsZ"));
			fVoxelSizeZ = fPm->GetDoubleVector(GetFullParmName("VoxelSizeZ"), "Length");

			// Z width and total number of slices depends on whether we are only loading one slice or all slices
			SetUpZSlices();

			// Read the pixels into fRawImage
			char* imageBuffer;
			imageBuffer = new char[fTotalNumberOfVoxels*sizeof(signed short)];
			patientIn.read(imageBuffer, fTotalNumberOfVoxels*sizeof(signed short));
			if ( patientIn.fail() ) {
				G4cout << "TOPAS experienced a serious error while loading patient: Requested Number of Voxels larger than available in image cube."  << G4endl;
				fPm->AbortSession(1);
			}
			patientIn.peek();
			if ( !patientIn.eof() ) {
				G4cout << "TOPAS experienced a serious error while loading patient: image cube larger than indicated by requested Number of Voxels."  << G4endl;
				fPm->AbortSession(1);
			}
			patientIn.close();

			signed short* ImagingValue= (signed short *) imageBuffer;
			for (G4int i=0; i<fTotalNumberOfVoxels; i++) {
				//G4cout << "Pixel: " << i << " has value: " << ImagingValue[i] << G4endl;
				fRawImageShort[iEnergy].push_back((signed short) ImagingValue[i]);
			}

			delete imageBuffer;
		}
	}
}
