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

#include "TsImagingToMaterialXcat.hh"

#include "TsParameterManager.hh"

#include "TsVGeometryComponent.hh"

#include <fstream>

TsImagingToMaterialXcat::TsImagingToMaterialXcat(TsParameterManager* pM, TsVGeometryComponent* component,
												 std::vector<G4Material*>* materialList, G4String converterNameLower)
:TsVImagingToMaterial(pM, component, materialList), fHasMetaDataFile(false) {

	fNumberOfVoxelsFromMetaDataFile = new G4int[3];
	fVoxelSizeFromMetaDataFile = new G4double[3];

	// Create the map from float to material name
	fMaterials = new std::map<G4float, G4String>;

	if (fPm->ParameterExists(fComponent->GetFullParmName("MetaDataFile"))) {
		G4String fileName = fPm->GetStringParameter(fComponent->GetFullParmName("InputDirectory")) +
							fPm->GetStringParameter(fComponent->GetFullParmName("MetaDataFile"));
		
		fHasMetaDataFile = true;
		
		std::ifstream infile(fileName);
		if (!infile) {
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "Unable to open XCAT file: " << fileName << G4endl;
			fPm->AbortSession(1);
		} else {
			G4cout << "\nReading MetaDataFile: " << fileName << G4endl;
		}
		
		// Use the following lines from the log file:
		// pixel width =  0.0500 (cm/pixel)
		// slice width =  0.0500 (cm/pixel)
		// array_size = 86
		// starting slice number    = 50
		// ending slice number      = 210

		G4String pixelWidthString = "";
		G4String sliceWidthString = "";
		G4int arraySize = -1;
		G4int startingSlice = -1;
		G4int endingSlice = -1;
		
		G4bool inMaterialDefinitions = false;
		
		while(infile)
		{
			std::string line;
			safeGetline( infile, line );
			
			if (converterNameLower == "xcat_attenuation") {
				if (line == "Linear Attenuation Coefficients (1/pixel):") {
					G4cout << "Begin building map from Linear Attenuation Coefficients (1/pixel) to Material Names" << G4endl;
					inMaterialDefinitions = true;
				} else if (inMaterialDefinitions && line == "") {
					G4cout << "Finished building map from Linear Attenuation Coefficients (1/pixel) to Material Names" << G4endl;
					inMaterialDefinitions = false;
				}
			} else if (converterNameLower == "xcat_activity") {
				if (line == "Activity Ratios") {
					G4cout << "Begin building map from Activity Ratios to Material Names" << G4endl;
					inMaterialDefinitions = true;
				} else if (inMaterialDefinitions && line == "") {
					G4cout << "Finished building map from Activity Ratios to Material Names" << G4endl;
					inMaterialDefinitions = false;
				}
			}

			std::string::size_type delcharPos = line.find( "=" );
			if ( delcharPos < std::string::npos )
			{
				std::string key = line.substr( 0, delcharPos );
				line.replace( 0, delcharPos+1, "" );
				Trim(key);
				Trim(line);
				
				if (inMaterialDefinitions) {
					// Replace any spaces in key with underscores
					std::replace(key.begin(), key.end(), ' ', '_');
					G4String materialName = "XCAT_" + key;
					
					// Test whether the material has been defined
					fComponent->GetMaterial(materialName);
					
					G4String lineString = line;
					G4float materialValue = (G4float) G4UIcommand::ConvertToDouble(lineString);
					
					G4cout << "Inserting into material map: " << materialValue << ", " << materialName << G4endl;
					fMaterials->insert(std::pair<G4float,G4String>(materialValue, materialName));
				}
				
				if (key == "pixel width") {
					std::string::size_type spacePos = line.find( " " );
					pixelWidthString = line.substr(0, spacePos);
				} else if (key == "slice width") {
					std::string::size_type spacePos = line.find( " " );
					sliceWidthString = line.substr(0, spacePos);
				} else if (key == "array_size") {
					G4String lineString = line;
					arraySize = G4UIcommand::ConvertToInt(lineString);
				} else if (key == "starting slice number") {
					G4String lineString = line;
					startingSlice = G4UIcommand::ConvertToInt(lineString);
				} else if (key == "ending slice number") {
					G4String lineString = line;
					endingSlice = G4UIcommand::ConvertToInt(lineString);
				}
			}
		}
		
		if (pixelWidthString == "") {
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "MetaDataFile: " << fileName << " does not specify a pixel width." << G4endl;
			fPm->AbortSession(1);
		}
		G4double pixelWidth = G4UIcommand::ConvertToDouble(pixelWidthString) * cm;
		G4cout << "pixel width: " << pixelWidth << G4endl;
		fVoxelSizeFromMetaDataFile[0] = pixelWidth;
		fVoxelSizeFromMetaDataFile[1] = pixelWidth;

		if (sliceWidthString == "") {
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "MetaDataFile: " << fileName << " does not specify a slice width." << G4endl;
			fPm->AbortSession(1);
		}
		G4double sliceWidth = G4UIcommand::ConvertToDouble(sliceWidthString) * cm;
		G4cout << "slice width: " << sliceWidth << G4endl;
		fVoxelSizeFromMetaDataFile[2] = sliceWidth;

		if (arraySize == -1) {
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "MetaDataFile: " << fileName << " does not specify an array_size." << G4endl;
			fPm->AbortSession(1);
		}
		G4cout << "array_size: " << arraySize << G4endl;
		fNumberOfVoxelsFromMetaDataFile[0] = arraySize;
		fNumberOfVoxelsFromMetaDataFile[1] = arraySize;
		
		if (startingSlice == -1) {
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "MetaDataFile: " << fileName << " does not specify a starting slice number." << G4endl;
			fPm->AbortSession(1);
		}
		if (endingSlice == -1) {
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "MetaDataFile: " << fileName << " does not specify an ending slice number." << G4endl;
			fPm->AbortSession(1);
		}
		G4cout << "startingSlice: " << startingSlice << G4endl;
		G4cout << "endingSlice: " << endingSlice << G4endl;
		fNumberOfVoxelsFromMetaDataFile[2] = endingSlice - startingSlice + 1;
	} else {
		G4cout << "No MetaDataFile was provided, so getting XCAT Meta Data from Parameters instead." << G4endl;
		
		G4String prefix;
		if (converterNameLower == "xcat_attenuation")
			prefix = fComponent->GetFullParmName("AttenuationForMaterial_");
		else
			prefix = fComponent->GetFullParmName("ActivityForMaterial_");
		
		std::vector<G4String>* values = new std::vector<G4String>;
		fPm->GetParameterNamesStartingWith(prefix, values);
		G4int length = values->size();

		for (G4int iToken=0; iToken<length; iToken++) {
			G4String parameterName = (*values)[iToken];

			// Extract the material name from the part of the parameter name after the last underscore
			G4String materialName = parameterName.substr(prefix.length());

			// Test whether the material has been defined
			fComponent->GetMaterial(materialName);

			G4float materialValue = fPm->GetUnitlessParameter(parameterName);
			
			G4cout << "Inserting into material map: " << materialValue << ", " << materialName << G4endl;
			fMaterials->insert(std::pair<G4float,G4String>(materialValue, materialName));
		}
	}
}


TsImagingToMaterialXcat::~TsImagingToMaterialXcat() {
}


G4String TsImagingToMaterialXcat::GetDataType() {
	return "f";
}


G4int TsImagingToMaterialXcat::GetNumberOfZSections() {
	if (fHasMetaDataFile)
		return 1;
	else
		return TsVImagingToMaterial::GetNumberOfZSections();
}

	
G4int TsImagingToMaterialXcat::GetNumberOfVoxelsX() {
	if (fHasMetaDataFile)
		return fNumberOfVoxelsFromMetaDataFile[0];
	else
		return TsVImagingToMaterial::GetNumberOfVoxelsX();
}

	
G4int TsImagingToMaterialXcat::GetNumberOfVoxelsY() {
	if (fHasMetaDataFile)
		return fNumberOfVoxelsFromMetaDataFile[1];
	else
		return TsVImagingToMaterial::GetNumberOfVoxelsY();
}

	
G4int* TsImagingToMaterialXcat::GetNumberOfVoxelsZ() {
	if (fHasMetaDataFile) {
		G4int* numberOfVoxelsZ = new G4int[1];
		numberOfVoxelsZ[0] = fNumberOfVoxelsFromMetaDataFile[2];
		return numberOfVoxelsZ;
	} else
		return TsVImagingToMaterial::GetNumberOfVoxelsZ();
}


G4double TsImagingToMaterialXcat::GetVoxelSizeX() {
	if (fHasMetaDataFile)
		return fVoxelSizeFromMetaDataFile[0];
	else
		return TsVImagingToMaterial::GetVoxelSizeX();
}


G4double TsImagingToMaterialXcat::GetVoxelSizeY() {
	if (fHasMetaDataFile)
		return fVoxelSizeFromMetaDataFile[1];
	else
		return TsVImagingToMaterial::GetVoxelSizeY();
}


G4double* TsImagingToMaterialXcat::GetVoxelSizeZ() {
	if (fHasMetaDataFile) {
		G4double* voxelSizeZ = new G4double[1];
		voxelSizeZ[0] = fVoxelSizeFromMetaDataFile[2];
		return voxelSizeZ;
	} else
		return TsVImagingToMaterial::GetVoxelSizeZ();
}


void TsImagingToMaterialXcat::PreloadAllMaterials() {
}


unsigned short TsImagingToMaterialXcat::AssignMaterialFloat(std::vector< float >* ImagingValues, G4int) {

	// The XCAT method uses only one energy of imaging
	G4float imagingValue = (*ImagingValues)[0];

	// If the voxel value is not yet in fImagingToMaterialMap, define a new material and add its index to the map
	if (fImagingToMaterialMap.find(imagingValue) == fImagingToMaterialMap.end()) {

		G4String materialName = "";
	
		for (std::map<G4float, G4String>::iterator it = fMaterials->begin(); it != fMaterials->end(); ++it)
			if (fabs(it->first - imagingValue) < 0.001)
				materialName = it->second;
		
		if (materialName=="") {
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "No material name has been defined for the value: " << imagingValue << G4endl;
			if (fHasMetaDataFile)
				G4cerr << "This was expected to be in the Meta Data File: " <<
					fPm->GetStringParameter(fComponent->GetFullParmName("InputDirectory")) +
					fPm->GetStringParameter(fComponent->GetFullParmName("MetaDataFile")) << G4endl;
			else {
				G4cerr << "This should have been defined in parameters of the form" << G4endl;
				G4cerr << fComponent->GetFullParmName("AttenuationForMaterial_") <<
					" or " << fComponent->GetFullParmName("ActivityForMaterial_") << G4endl;
			}
			fPm->AbortSession(1);
		}
		
		fMaterialList->push_back(GetMaterial(materialName));
		G4int materialListIndex = fMaterialList->size() - 1;

		// Note in our map that this ImagingValue now has this material list index assigned to it
		fImagingToMaterialMap.insert(std::pair<G4float, G4int>(imagingValue, materialListIndex));
	}

	// Return the index of the material in fImagingToMaterialMap (either previously existed or just created above)
	std::map<G4float,G4int>::iterator it = fImagingToMaterialMap.find(imagingValue);
	return it->second;
}
