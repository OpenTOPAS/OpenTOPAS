// Imaging to Material Converter for MyImagingToMaterialConverter1
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

#include "MyImagingToMaterialConverter.hh"

#include "TsParameterManager.hh"

MyImagingToMaterialConverter::MyImagingToMaterialConverter(TsParameterManager* pM,
														   TsVGeometryComponent* component, std::vector<G4Material*>* materialList)
:TsVImagingToMaterial(pM, component, materialList), fFirst(true) {

	// In this toy conversion method, all imaging values will be converter to the same material
	fMaterialName = fPm->GetStringParameter(GetFullParmName("ToyMaterialName"));
	fDensity = fPm->GetDoubleParameter(GetFullParmName("ToyMaterialDensity"), "Volumic Mass");
	fMeanExcitationEnergy = fPm->GetDoubleParameter(GetFullParmName("ToyMaterialMeanExcitationEnergy"), "Energy");
	fColorName = fPm->GetStringParameter(GetFullParmName("ToyMaterialColorName"));
	fNumberOfElements = fPm->GetVectorLength(GetFullParmName("ToyMaterialElementNameVector"));
	fElementNames = fPm->GetStringVector(GetFullParmName("ToyMaterialElementNameVector"));
	fElementFractions = fPm->GetUnitlessVector(GetFullParmName("ToyMaterialElementWeightVector"));;
}


MyImagingToMaterialConverter::~MyImagingToMaterialConverter() {
}


void MyImagingToMaterialConverter::PreloadAllMaterials() {
	// Not bothering to implement since not worrying about 4DCT
}


// Toy method. Converts everything to whatever one material was specified in the parameters
// that were read in the constructor above.
unsigned short MyImagingToMaterialConverter::AssignMaterial(std::vector< signed short >* imagingValues, G4int timeSliceIndex) {
	
	G4cout << "AssignMaterial called for imagingValues: ";
	for (G4int iEnergy = 0; iEnergy < (int)(*imagingValues).size(); iEnergy++)
		G4cout << (*imagingValues)[iEnergy];
	G4cout << " and time slice: " << timeSliceIndex << G4endl;

	if (fFirst) {
		fMaterialListIndex = DefineNewMaterial(fMaterialName, fDensity, fMeanExcitationEnergy, fColorName,
											   fNumberOfElements, fElementNames, fElementFractions);
		fFirst = false;
	}
	
	return fMaterialListIndex;
}
