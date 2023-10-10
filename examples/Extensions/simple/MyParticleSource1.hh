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

#ifndef MyParticleSource1_hh
#define MyParticleSource1_hh

#include "TsSource.hh"

class MyParticleSource1 : public TsSource
{
public:
	MyParticleSource1(TsParameterManager* pM, TsSourceManager* psM, G4String sourceName);
	~MyParticleSource1();

	void ResolveParameters();
};
#endif
