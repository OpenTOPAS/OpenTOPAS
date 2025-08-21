//
// ********************************************************************
// *                                                                  *
// * This file was obtained from Topas MC Inc                         *
// * under the license agreement set forth at                         *
// * http://www.topasmc.org/registration                              *
// * Any use of this file constitutes full acceptance                 *
// * of this TOPAS MC license agreement.                              *
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
