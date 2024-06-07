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

#include "TsImagingToMaterialByTagNumber.hh"

#include "TsParameterManager.hh"

#include "G4Material.hh"

TsImagingToMaterialByTagNumber::TsImagingToMaterialByTagNumber(TsParameterManager* pM, TsVGeometryComponent* component,
												 std::vector<G4Material*>* materialList, G4String)
:TsVImagingToMaterial(pM, component, materialList) {
	// Create map from material tag number to material list index
	fMapTagNumberToMaterialListIndex = new std::map<G4int, unsigned short>;

	G4int numberOfMaterials = fPm->GetVectorLength(GetFullParmName("MaterialTagNumbers"));

	if (fPm->GetVectorLength(GetFullParmName("MaterialNames")) != numberOfMaterials) {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "The length of the vector paremeter " << GetFullParmName("MaterialTagNumbers") << G4endl;
		G4cerr << "does not match the length of the vector paremeter " << GetFullParmName("MaterialNames") << G4endl;
		fPm->AbortSession(1);
	}

	G4int* materialTagNumbers = fPm->GetIntegerVector(GetFullParmName("MaterialTagNumbers"));
	G4String* materialNames = fPm->GetStringVector(GetFullParmName("MaterialNames"));

	for (unsigned short iMaterial = 0; iMaterial < numberOfMaterials; iMaterial++) {
		//G4cout << "Evaluating material tag: " << materialTagNumbers[iMaterial] <<
		//", material name: " << materialNames[iMaterial] << G4endl;

		G4Material* materialPointer = GetMaterial(materialNames[iMaterial]);
		
		//G4cout << "Found material: " << materialPointer->GetName() << G4endl;

		// Insert the material into the materials list
		fMaterialList->push_back(materialPointer);

		// Note index into materials list for this material tag number
		fMapTagNumberToMaterialListIndex->insert(std::pair<G4int, unsigned short>(materialTagNumbers[iMaterial], iMaterial));
	}
}


TsImagingToMaterialByTagNumber::~TsImagingToMaterialByTagNumber() {
}


G4String TsImagingToMaterialByTagNumber::GetDataType() {
	return "s";
}


void TsImagingToMaterialByTagNumber::PreloadAllMaterials() {
	// Not bothering to implement since not worrying about 4DCT
}


unsigned short TsImagingToMaterialByTagNumber::AssignMaterial(std::vector< signed short >* imagingValues, G4int) {

	//G4cout << "AssignMaterial called for material tag: " << (*imagingValues)[0] << G4endl;
	std::map<G4int, unsigned short>::iterator it = fMapTagNumberToMaterialListIndex->find((*imagingValues)[0]);
	
	if(it == fMapTagNumberToMaterialListIndex->end()) {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "The ImageCube contains the material tag number: " << (*imagingValues)[0] << G4endl;
		G4cerr << "However this tag number does not appear in " << GetFullParmName("MaterialTagNumbers") << G4endl;
		fPm->AbortSession(1);
	}

	unsigned short materialListIndex = it->second;
	//G4cout << "Found materialListIndex: " << materialListIndex << G4endl;
	
	return materialListIndex;
}
