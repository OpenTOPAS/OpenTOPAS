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

#ifndef TsDicomActivityMap_hh
#define TsDicomActivityMap_hh

#include "TsBox.hh"
#include "globals.hh"

class TsDicomActivityMap : public TsBox
{
public:
	TsDicomActivityMap(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
			TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name);
	~TsDicomActivityMap();

	G4VPhysicalVolume* Construct();
	void UpdateForSpecificParameterChange(G4String parameter);
	void ReadImage();

	inline std::vector<G4Point3D> GetSourcePositions()		{ return fSourcePositions; }
	inline std::vector<G4double> GetSourceCounts()			{ return fSourceCounts; }
	inline std::vector<G4Point3D> GetSourcePosRelToDicom()	{ return fSourcePosRelToDicom; }
	inline G4double GetVoxelSizeX()							{ return fVoxelSizeX; }
	inline G4double GetVoxelSizeY()							{ return fVoxelSizeY; }
	inline G4double GetVoxelSizeZ()							{ return fVoxelSizeZ; }

private:
	G4String fDicomDirectory;

	G4int fCountThreshold;

	G4int fVoxelCountX;
	G4int fVoxelCountY;
	G4int fNumberOfSlices;

	G4double fVoxelSizeX;
	G4double fVoxelSizeY;
	G4double fVoxelSizeZ;

	G4Point3D fTransFirstVoxelCenterRelToDicom;

	std::vector<short> fRawImageShort;

	std::vector<G4Point3D> fSourcePositions;
	std::vector<G4double> fSourceCounts;
	std::vector<G4Point3D> fSourcePosRelToDicom;
};

#endif
