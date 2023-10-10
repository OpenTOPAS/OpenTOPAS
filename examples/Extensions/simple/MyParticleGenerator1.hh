//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * This file was obtained from Topas MC Inc under the license       *
// * agreement set forth at http://www.topasmc.org/registration       *
// * Any use of this file constitutes full acceptance of              *
// * this TOPAS MC license agreement.                                 *
// *                                                                  *
// ********************************************************************
//

#ifndef MyParticleGenerator1_hh
#define MyParticleGenerator1_hh

#include "TsVGenerator.hh"

class MyParticleGenerator1 : public TsVGenerator
{
public:
	MyParticleGenerator1(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName);
	~MyParticleGenerator1();

	void ResolveParameters();
	
	void GeneratePrimaries(G4Event* );
	
private:
	G4int fSplitNumber;
};
#endif
