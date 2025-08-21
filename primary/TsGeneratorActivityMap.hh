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

#ifndef TsGeneratorActivityMap_hh
#define TsGeneratorActivityMap_hh

#include "TsVGenerator.hh"
#include "TsSourceActivityMap.hh"
#include "G4SystemOfUnits.hh"

class TsGeneratorActivityMap : public TsVGenerator
{
public:
	TsGeneratorActivityMap(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName);
	~TsGeneratorActivityMap();

	void GeneratePrimaries(G4Event* );

	G4Point3D SampleVoxel();

private:
	TsSourceActivityMap* fSource;

	bool isDigitOrDot(const char c);
	G4String invertRadionuclideName(G4String radionuclideName);
};

#endif
