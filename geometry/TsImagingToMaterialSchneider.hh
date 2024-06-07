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

#ifndef TsImagingToMaterialSchneider_hh
#define TsImagingToMaterialSchneider_hh

#include "TsVImagingToMaterial.hh"

#include <map>

class TsImagingToMaterialSchneider : public TsVImagingToMaterial
{
public:
	TsImagingToMaterialSchneider(TsParameterManager* pM,
								 TsVGeometryComponent* component, std::vector<G4Material*>* materialList);
	~TsImagingToMaterialSchneider();

	void PreloadAllMaterials();

	unsigned short AssignMaterial(std::vector< signed short >* ImagingValues, G4int timeSliceIndex);

private:
	G4double* fCorrections;
	G4int fNCorrections;
	G4int fNCorrectionSections;
	G4int* fCorrectionSections;
	G4int fMinImagingValue;
	G4double* fOffsets;
	G4double* fFactors;
	G4double* fFactorOffsets;
	G4int fNElements;
	G4String* fElementNames;
	G4int fNMaterialSections;
	G4int* fMaterialSections;
	std::vector<G4double*> fWeights;
	std::vector<G4String> fColorNames;

	G4bool fHaveMeanExcitationEnergies;
	G4double* fMeanExcitationEnergies;

	G4bool fWarnedAboutUnderflow;
	G4bool fWarnedAboutOverflow;

	G4bool fUseVariableDensityMaterials;
	G4String* fBaseMaterialNames;

	std::map<G4int, G4int> fImagingToMaterialMap;
};

#endif
