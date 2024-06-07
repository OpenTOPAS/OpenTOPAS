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

#ifndef MyImagingToMaterialConverter_hh
#define MyImagingToMaterialConverter_hh

#include "TsVImagingToMaterial.hh"

class MyImagingToMaterialConverter : public TsVImagingToMaterial
{
public:
	MyImagingToMaterialConverter(TsParameterManager* pM,
								 TsVGeometryComponent* component, std::vector<G4Material*>* materialList);
	~MyImagingToMaterialConverter();
	
	void PreloadAllMaterials();
	
	unsigned short AssignMaterial(std::vector< signed short >* HUValues, G4int timeSliceIndex);

private:
	G4bool fFirst;
	
	G4String fMaterialName;
	G4double fDensity;
	G4double fMeanExcitationEnergy;
	G4String fColorName;
	G4int fNumberOfElements;
	G4String* fElementNames;
	G4double* fElementFractions;
	
	G4int fMaterialListIndex;
};

#endif
