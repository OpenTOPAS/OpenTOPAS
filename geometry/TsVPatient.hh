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

#ifndef TsVPatient_hh
#define TsVPatient_hh

#include "TsBox.hh"

#include <map>

class TsVImagingToMaterial;

class TsVPatient : public TsBox
{
public:
	TsVPatient(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM,
							TsGeometryManager* gM, TsVGeometryComponent* parentComponent,
							G4VPhysicalVolume* parentVolume, G4String& name);
	virtual ~TsVPatient();

	virtual G4VPhysicalVolume* Construct();
	void UpdateForSpecificParameterChange(G4String parameter);
	void UpdateForNewRun(G4bool force);

protected:
	void SetImage();
	void SetUpZSlices();
	virtual void ReadImage() = 0;

	G4int fRestrictVoxelsXMin;
	G4int fRestrictVoxelsYMin;
	G4int fRestrictVoxelsZMin;
	G4int fRestrictVoxelsXMax;
	G4int fRestrictVoxelsYMax;
	G4int fRestrictVoxelsZMax;

	G4bool fNeedToSetImage;
	G4String fCurrentImageName;
	std::map<G4String, G4int> fImageNames;
	G4int fNImageSections;

	G4String fDataType;

	G4int fUnrestrictedVoxelCountX;
	G4int fUnrestrictedVoxelCountY;
	G4int fUnrestrictedVoxelCountZ;

	G4int fVoxelCountX;
	G4int fVoxelCountY;
	G4int* fVoxelCountZ;
	G4int* fFirstOriginalSliceThisSection;
	G4int* fLastOriginalSliceThisSection;
	G4double* fVoxelSizeZ;

	G4int fTotalNumberOfSlices;
	G4int fTotalNumberOfVoxels;

	// Outer vector supports multi-energy imaging
	G4int fNEnergies;
	std::vector< std::vector<signed short> > fRawImageShort;
	std::vector< std::vector<int> > fRawImageInt;
	std::vector< std::vector<float> > fRawImageFloat;

	TsVImagingToMaterial* fImagingToMaterial;

private:
	void GetSlicesToShow();
	void GetRestrictVoxelParameters();

	std::vector<TsBox*> fImageSections;

	G4String fLastImageName;
	G4int fLastNImageSections;
	G4int fLastVoxelCountX;
	G4int fLastVoxelCountY;
	G4int* fLastVoxelCountZ;
	G4double fLastFullWidthX;
	G4double fLastFullWidthY;
	G4double* fLastVoxelSizeZ;

	G4int* fSlicesToShowX;
	G4int fNSlicesToShowX;
	G4int* fSlicesToShowY;
	G4int fNSlicesToShowY;
	G4int* fSlicesToShowZ;
	G4int fNSlicesToShowZ;

	std::vector< std::vector< std::vector<unsigned short>* > > fMaterialIndexVector;
};

#endif
