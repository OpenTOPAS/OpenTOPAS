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

#include "TsImagingToMaterialSchneider.hh"

#include "TsParameterManager.hh"

#include <fstream>

TsImagingToMaterialSchneider::TsImagingToMaterialSchneider(TsParameterManager* pM,
														   TsVGeometryComponent* component, std::vector<G4Material*>* materialList)
:TsVImagingToMaterial(pM, component, materialList), fHaveMeanExcitationEnergies(false),
fWarnedAboutUnderflow (false), fWarnedAboutOverflow(false), fUseVariableDensityMaterials(false) {

	// Read the Density Correction vector
	G4String densityCorrectionParmName = GetFullParmName("DensityCorrection");
	fCorrections = fPm->GetDoubleVector(densityCorrectionParmName, "Volumic Mass");
	fNCorrections = fPm->GetVectorLength(densityCorrectionParmName);

	// Read the vector that delimits blocks of imaging values that have different correction factors
	G4String correctionSectionsParmName = GetFullParmName("SchneiderHounsfieldUnitSections");
	fNCorrectionSections = fPm->GetVectorLength(correctionSectionsParmName);
	fCorrectionSections = fPm->GetIntegerVector(correctionSectionsParmName);

	// The overall range of the Correction Sections must match the size of the Density Correction vector
	if (fCorrectionSections[fNCorrectionSections-1] - fCorrectionSections[0] != fNCorrections) {
		G4cout << "Range of " << correctionSectionsParmName << " does not match size of " << densityCorrectionParmName << G4endl;
		fPm->AbortSession(1);
	}

	// The Correction Section values must be monotonically increasing
	G4int lastValue = -99999;
	for (G4int i = 0; i < fNCorrectionSections-1; i++) {
		if (fCorrectionSections[i] < lastValue) {
			G4cout << "Values in " << correctionSectionsParmName << " are not monotonically increasing." << G4endl;
			fPm->AbortSession(1);
		}
		lastValue = fCorrectionSections[i];
	}

	// Read the correction factors for the different blocks of imaging values
	G4String densityOffsetParmName = GetFullParmName("SchneiderDensityOffset");
	if (fPm->GetVectorLength(densityOffsetParmName) != fNCorrectionSections-1) {
		G4cout << densityOffsetParmName << " has wrong size for given " << correctionSectionsParmName << G4endl;
		fPm->AbortSession(1);
	}
	fOffsets = fPm->GetUnitlessVector(densityOffsetParmName);

	G4String densityFactorParmName = GetFullParmName("SchneiderDensityFactor");
	if (fPm->GetVectorLength(densityFactorParmName) != fNCorrectionSections-1) {
		G4cout << densityFactorParmName << " has wrong size for given " << correctionSectionsParmName << G4endl;
		fPm->AbortSession(1);
	}
	fFactors = fPm->GetUnitlessVector(densityFactorParmName);

	G4String densityFactorOffsetParmName = GetFullParmName("SchneiderDensityFactorOffset");
	if (fPm->GetVectorLength(densityFactorOffsetParmName) != fNCorrectionSections-1) {
		G4cout << densityFactorOffsetParmName << " has wrong size for given " << correctionSectionsParmName << G4endl;
		fPm->AbortSession(1);
	}
	fFactorOffsets = fPm->GetUnitlessVector(densityFactorOffsetParmName);

	if (fPm->ParameterExists(GetFullParmName("MinImagingValue")))
		fMinImagingValue = fPm->GetIntegerParameter(GetFullParmName(("MinImagingValue")));
	else
		fMinImagingValue = -1000;

	// Read the vector of elements used in material compositions
	G4String elementsParmName = GetFullParmName("SchneiderElements");
	fNElements = fPm->GetVectorLength(elementsParmName);
	fElementNames = fPm->GetStringVector(elementsParmName);

	// Read the vector that delimits blocks of imaging values that have different material compositions and colors
	G4String materialSectionsParmName = GetFullParmName("SchneiderHUToMaterialSections");
	fNMaterialSections = fPm->GetVectorLength(materialSectionsParmName);
	fMaterialSections = fPm->GetIntegerVector(materialSectionsParmName);

	// The Materials Section values must be monotonically increasing
	lastValue = -99999;
	for (G4int i = 0; i < fNMaterialSections-1; i++) {
		if (fMaterialSections[i] < lastValue) {
			G4cout << "Values in " << materialSectionsParmName << " are not monotonically increasing." << G4endl;
			fPm->AbortSession(1);
		}
		lastValue = fMaterialSections[i];
	}

	// Range of Material Sections needs to match range of Correction Sections
	if (fMaterialSections[0] != fCorrectionSections[0]) {
		G4cout << materialSectionsParmName << " different first value than " << correctionSectionsParmName << G4endl;
		fPm->AbortSession(1);
	}
	if (fMaterialSections[fNMaterialSections-1] != fCorrectionSections[fNCorrectionSections-1]) {
		G4cout << materialSectionsParmName << " different last value than " << correctionSectionsParmName << G4endl;
		fPm->AbortSession(1);
	}

	// Read the materials information for the different blocks of imaging values
	G4String temp;
	for (G4int i = 1; i < fNMaterialSections; i++) {
		temp =  "SchneiderMaterialsWeight" + G4UIcommand::ConvertToString(i);
		G4String weightsParmName = GetFullParmName(temp);
		G4double* tmpWeight = fPm->GetUnitlessVector(weightsParmName);
		fWeights.push_back(tmpWeight);

		G4String cName = "PatientTissue" + G4UIcommand::ConvertToString(i);
		fColorNames.push_back(cName);
	}

	if (fPm->ParameterExists(GetFullParmName("SchneiderMaterialMeanExcitationEnergy"))) {
		fHaveMeanExcitationEnergies = true;
		fMeanExcitationEnergies = fPm->GetDoubleVector(GetFullParmName("SchneiderMaterialMeanExcitationEnergy"), "Energy");
	}

	if (fPm->ParameterExists(GetFullParmName("SchneiderUseVariableDensityMaterials")) &&
		fPm->GetBooleanParameter(GetFullParmName("SchneiderUseVariableDensityMaterials"))) {
		fUseVariableDensityMaterials = true;
		fBaseMaterialNames = new G4String[fNMaterialSections];
	}
}


TsImagingToMaterialSchneider::~TsImagingToMaterialSchneider() {
}


void TsImagingToMaterialSchneider::PreloadAllMaterials() {
	std::vector< signed short >* imagingValues = new std::vector< signed short >;
	for (G4int i = 0; i < fNCorrections; i++) {
		imagingValues->push_back(fMaterialSections[0]+i);
		AssignMaterial(imagingValues, 0);
		imagingValues->clear();
	}
}


unsigned short TsImagingToMaterialSchneider::AssignMaterial(std::vector< signed short >* imagingValues, G4int timeSliceIndex) {
	// The Schneider method uses only one energy of imaging
	signed short imagingValue = (*imagingValues)[0];

	// Enforce minimum value
	if (imagingValue < fCorrectionSections[0]) {
		if (!fWarnedAboutUnderflow) {
			G4cout << "Found Imaging Value: " << imagingValue << " less than minimum value in conversion table: " << fCorrectionSections[0] << G4endl;
			G4cout << "Resetting value to minimum value from the table: " << fCorrectionSections[0] << G4endl;
			G4cout << "This message is only written once per patient file. There may also be other underflow values. " << G4endl;
			fWarnedAboutUnderflow = true;
		}
		imagingValue = fCorrectionSections[0];
	}

	// Enforce maximum value
	if (imagingValue >= fCorrectionSections[fNCorrectionSections-1]) {
		if (!fWarnedAboutOverflow) {
			G4cout << "Found Imaging Value: " << imagingValue << " greater than maximum value in conversion table: " << fCorrectionSections[fNCorrectionSections-1] - 1 << G4endl;
			G4cout << "Resetting value to maximum value from the table: " << fCorrectionSections[fNCorrectionSections-1] - 1 << G4endl;
			G4cout << "This message is only written once per patient file. There may also be other overflow values. " << G4endl;
			fWarnedAboutOverflow = true;
		}
		imagingValue = fCorrectionSections[fNCorrectionSections-1] - 1;
	}

	// If the voxel value is not yet in fImagingToMaterialMap, define a new material and add its index to the map
	if (fImagingToMaterialMap.find(imagingValue) == fImagingToMaterialMap.end()) {
		G4String materialName;
		G4double density = 0.;
		G4double meanExcitationEnergy = 0.;
		G4String colorName;
		G4int numberOfElementsInCurrentMaterial = 0;
		G4String* elementNames = new G4String[fNElements];
		G4double* elementFractions = new G4double[fNElements];

		// For 4D imaging, new materials may not be defined after the first time slice
		if (timeSliceIndex > 0) {
			G4cout << "Image contains at least one material not found in first image." << G4endl;
			G4cout << "Since Geant4 physics does not allow adding new materials after the first run," << G4endl;
			G4cout << "you must instead tell TOPAS to load the full materials table in advance." << G4endl;
			G4cout << "Add the following parameter to your setup:" << G4endl;
			G4cout << "b:" << GetFullParmName("PreLoadAllMaterials") << " = \"True\"" << G4endl;
			fPm->AbortSession(1);
		}

		// Name to be assigned to this new material
		// Extra logic to avoid having minus sign in parameter name since minus sign is a reserved character there.
		if (imagingValue < 0)
			materialName = "PatientTissueFromHUNegative" + (G4UIcommand::ConvertToString(imagingValue)).substr(1);
		else
			materialName = "PatientTissueFromHU" + G4UIcommand::ConvertToString(imagingValue);

		// Find which correction section contains this imaging value
		G4bool found = false;
		for (G4int k = 0; k < fNCorrectionSections && !found; k++) {
			if (imagingValue >= fCorrectionSections[k] && imagingValue < fCorrectionSections[k+1]) {
				found = true;
				density = ( fOffsets[k] + ( fFactors[k] * ( fFactorOffsets[k] + imagingValue) ) ) * fCorrections[imagingValue - fMinImagingValue];
			}
		}

		if (!found) {
			G4cout << "Image contains imaging value " << imagingValue << " that does not fit into any of the defined unit sections." << G4endl;
			fPm->AbortSession(1);
		}

		// Find which material section contains this imaging value
		found = false;
		G4int k;
		for (k = 0; k < fNMaterialSections-1 && !found; k++) {
			if (imagingValue >= fMaterialSections[k] && imagingValue < fMaterialSections[k+1]) {
				found = true;

				// Count number of elements used in this material
				G4int nElementsUsed = 0;
				for (G4int iElem = 0; iElem < fNElements; iElem++)
					if (fWeights[k][iElem] != 0)
						nElementsUsed++;

				// Create the element name and weight vectors for this material
				for (G4int iElem = 0; iElem < fNElements; iElem++)
					if (fWeights[k][iElem] != 0) {
						elementNames[numberOfElementsInCurrentMaterial] = fElementNames[iElem];
						elementFractions[numberOfElementsInCurrentMaterial] = fWeights[k][iElem];
						numberOfElementsInCurrentMaterial++;
					}

				if (fHaveMeanExcitationEnergies)
					meanExcitationEnergy = fMeanExcitationEnergies[k];

				colorName = fColorNames[k];
			}
		}

		if (!found) {
			G4cout << "Image contains imaging value " << imagingValue << " that does not fit into any of the defined material sections." << G4endl;
			fPm->AbortSession(1);
		}

		G4int materialListIndex;

		if (fUseVariableDensityMaterials) {
		 	if (fBaseMaterialNames[k] == "") {
				materialListIndex = DefineNewMaterial(materialName, density, meanExcitationEnergy,
														colorName, numberOfElementsInCurrentMaterial,
														elementNames, elementFractions);
				fBaseMaterialNames[k] = materialName;
			} else {
				materialListIndex = DefineNewMaterial(materialName, density, fBaseMaterialNames[k], colorName);
			}
		} else {
			materialListIndex = DefineNewMaterial(materialName, density, meanExcitationEnergy,
													colorName, numberOfElementsInCurrentMaterial,
													elementNames, elementFractions);
		}

		delete[] elementNames;
		delete[] elementFractions;

		// Note in our map that this imagingValue now has this material list index assigned to it
		fImagingToMaterialMap.insert(std::pair<G4int, G4int>(imagingValue, materialListIndex));
	}

	// Return the index of the material in fImagingToMaterialMap (either previously existed or just created above)
	std::map<G4int,G4int>::iterator it = fImagingToMaterialMap.find(imagingValue);
	return it->second;
}
