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

#include "TsGeneratorIsotropic.hh"

#include "TsParameterManager.hh"

#include "G4PhysicalConstants.hh"
#include "Randomize.hh"

TsGeneratorIsotropic::TsGeneratorIsotropic(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName) :
TsVGenerator(pM, gM, pgM, sourceName)
{
	ResolveParameters();
}


TsGeneratorIsotropic::~TsGeneratorIsotropic()
{
}


void TsGeneratorIsotropic::ResolveParameters() {
	TsVGenerator::ResolveParameters();
}


void TsGeneratorIsotropic::GeneratePrimaries(G4Event* anEvent)
{
	if (CurrentSourceHasGeneratedEnough())
		return;

	TsPrimaryParticle p;

	G4double costheta = G4RandFlat::shoot( -1., 1);
	G4double sintheta = sqrt(1. - costheta*costheta);
	G4double phi = 2.* CLHEP::pi * G4UniformRand();
	G4double sinphi = sin(phi);
	G4double cosphi = cos(phi);
	G4double px = sintheta * cosphi;
	G4double py = sintheta * sinphi;
	G4double pz = costheta;
	G4double mag = std::sqrt((px*px) + (py*py) + (pz*pz));

	p.dCos1 = px / mag;
	p.dCos2 = py / mag;
	p.dCos3 = pz / mag;
	p.posX = 0.;
	p.posY = 0.;
	p.posZ = 0.;

	SetEnergy(p);
	SetParticleType(p);

	p.weight = 1.;
	p.isNewHistory = true;

	TransformPrimaryForComponent(&p);
	GenerateOnePrimary(anEvent, p);
	AddPrimariesToEvent(anEvent);
}
