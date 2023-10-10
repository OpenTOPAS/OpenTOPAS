//
// ********************************************************************
// *                                                                  *
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

#ifndef TsVImagingToMaterial_hh
#define TsVImagingToMaterial_hh

#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"

#include <vector>

class TsParameterManager;

class TsVGeometryComponent;

class G4Material;

class TsVImagingToMaterial
{
public:
	TsVImagingToMaterial(TsParameterManager* pM,
						 TsVGeometryComponent* component, std::vector<G4Material*>* materialList);
	virtual ~TsVImagingToMaterial();

	// Geant4 does not support adding materials after the first history.
	// If you are doing 4D imaging, it may be that some materials not present in the first time slice
	// may appear in subsequent time slices.
	// In such cases, you need a way to load all materials before we start the first image.
	// If you do not need to support 4D imaging, you can leave this method empty.
	virtual void PreloadAllMaterials() = 0;

	// This method is called for each voxel in the image.
	// For single energy imaging, ImagingValues will contain only a single value.
	// For multi-energy imaging, ImagingValues will contain one value for each energy.
	// imageIndex is for 4D imaging. We may need to test whether it is 0, meaning first time slice, or not zero.
	// This method returns material information for this voxel by giving the appropriate index into the materialList.
	virtual unsigned short AssignMaterial(std::vector< signed short >* ImagingValues, G4int timeSliceIndex);
	virtual unsigned short AssignMaterialInt(std::vector< int >* ImagingValues, G4int timeSliceIndex);
	virtual unsigned short AssignMaterialFloat(std::vector< float >* ImagingValues, G4int timeSliceIndex);

	virtual G4String GetDataType();
	
	virtual G4int GetNumberOfZSections();
	
	virtual G4int GetNumberOfVoxelsX();
	virtual G4int GetNumberOfVoxelsY();
	virtual G4int* GetNumberOfVoxelsZ();
	
	virtual G4double GetVoxelSizeX();
	virtual G4double GetVoxelSizeY();
	virtual G4double* GetVoxelSizeZ();

protected:
	// Given a partial parameter name, such as "Phantom",
	// return the full parameter name for this component, such as "Ge/Phantom/DensityCorrection"
	G4String GetFullParmName(G4String);

	// Get a material that is already defined in the parameter files
	G4Material* GetMaterial(G4String name);
	G4Material* GetMaterial(const char* name);

	// Define a new material on the fly, returning its index into the material list
	G4int DefineNewMaterial(G4String materialName, G4double density, G4double meanExcitationEnergy, G4String colorName,
							G4int numberOfElements, G4String* elementNames, G4double* elementFractions,
							G4int relativeDensityBins = 0, G4double relativeDensityMin = 0., G4double relativeDensityMax = 100.);

	// Define a new material on the fly using a base material
	G4int DefineNewMaterial(G4String materialName, G4double density, G4String baseMaterialName, G4String colorName);

	void Trim(std::string& s);
	std::istream& safeGetline(std::istream& is, std::string& t);

	// Pointer to parameter manager
	TsParameterManager* fPm;

	TsVGeometryComponent* fComponent;

	std::vector<G4Material*>* fMaterialList;
};
#endif
