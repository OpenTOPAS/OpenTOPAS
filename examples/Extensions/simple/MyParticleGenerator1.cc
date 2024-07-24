// Particle Generator for MyParticleGenerator1
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
