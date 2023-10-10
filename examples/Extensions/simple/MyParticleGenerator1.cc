// Particle Generator for MyParticleGenerator1
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

#include "MyParticleGenerator1.hh"

#include "TsParameterManager.hh"

MyParticleGenerator1::MyParticleGenerator1(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName) :
TsVGenerator(pM, gM, pgM, sourceName), fSplitNumber(1)
{
	ResolveParameters();
}


MyParticleGenerator1::~MyParticleGenerator1()
{
}


// This is just a silly demonstration particle source.
// It creates one or more identical primary particles, based on the parameter SplitNumber.
// The rest of the parameters are already provided by the base class's ResolveParameters.
void MyParticleGenerator1::ResolveParameters() {
	TsVGenerator::ResolveParameters();
	
	fSplitNumber = fPm->GetIntegerParameter(GetFullParmName("SplitNumber"));
}


void MyParticleGenerator1::GeneratePrimaries(G4Event* anEvent)
{
	if (CurrentSourceHasGeneratedEnough())
		return;
	
	for (int iSplit = 0; iSplit < fSplitNumber; iSplit++) {
		TsPrimaryParticle p;

		p.posX = 0.;
		p.posY = 0.;
		p.posZ = 0.;
		
		p.dCos1 = 0.;
		p.dCos2 = 0.;
		p.dCos3 = 1.;
		
		p.kEnergy = fEnergy / fSplitNumber;
		p.weight = 1. / fSplitNumber;
		
		p.particleDefinition = fParticleDefinition;
		
		p.isNewHistory = iSplit == 0;
		
		p.isOpticalPhoton = fIsOpticalPhoton;
		p.isGenericIon = fIsGenericIon;
		p.ionCharge = fIonCharge;
	
		GenerateOnePrimary(anEvent, p);
	}
	
	AddPrimariesToEvent(anEvent);
}
