// Particle Source for MyParticleSource1
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

#include "MyParticleSource1.hh"

#include "TsParameterManager.hh"

MyParticleSource1::MyParticleSource1(TsParameterManager* pM, TsSourceManager* psM, G4String sourceName) :
TsSource(pM, psM, sourceName)
{
	ResolveParameters();
}


MyParticleSource1::~MyParticleSource1()
{
}


void MyParticleSource1::ResolveParameters() {
	TsSource::ResolveParameters();
}
