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

#include "TsGeneratorVolumetric.hh"

#include "TsParameterManager.hh"
#include "TsVGeometryComponent.hh"
#include "TsGeometryManager.hh"

#include "G4TransportationManager.hh"
#include "G4Navigator.hh"
#include "G4VisExtent.hh"
#include "Randomize.hh"

TsGeneratorVolumetric::TsGeneratorVolumetric(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName) :
TsVGenerator(pM, gM, pgM, sourceName), fNeedToCalculateExtent(true)
{
	fRecursivelyIncludeChildren = false;
	if (fPm->ParameterExists(GetFullParmName("RecursivelyIncludeChildren")))
		fRecursivelyIncludeChildren = fPm->GetBooleanParameter(GetFullParmName("RecursivelyIncludeChildren"));
	
	ResolveParameters();
}


TsGeneratorVolumetric::~TsGeneratorVolumetric()
{
}


void TsGeneratorVolumetric::ResolveParameters() {
	TsVGenerator::ResolveParameters();
	
	fActiveMaterial = fComponent->GetMaterial(fPm->GetStringParameter(GetFullParmName("ActiveMaterial")));
	
	fMaxNumberOfPointsToSample = 1000000;
	if (fPm->ParameterExists(GetFullParmName("MaxNumberOfPointsToSample")))
		fMaxNumberOfPointsToSample = fPm->GetIntegerParameter(GetFullParmName("MaxNumberOfPointsToSample"));

	fVolumes = fComponent->GetAllPhysicalVolumes(fRecursivelyIncludeChildren);

	fNeedToCalculateExtent = true;
}


void TsGeneratorVolumetric::UpdateForNewRun(G4bool rebuiltSomeComponents) {
	TsVGenerator::UpdateForNewRun(rebuiltSomeComponents);
	ResolveParameters();

	// Needed so that voxelizer and navigator can correctly evaluate positions in whole new world
	fGm->UpdateWorldForNewRun();
}


void TsGeneratorVolumetric::CalculateExtent() {
	G4VisExtent myExtent = fComponent->GetExtent();
	fXMin = myExtent.GetXmin();
	fXMax = myExtent.GetXmax();
	fYMin = myExtent.GetYmin();
	fYMax = myExtent.GetYmax();
	fZMin = myExtent.GetZmin();
	fZMax = myExtent.GetZmax();

	fNeedToCalculateExtent = false;

	G4TransportationManager* transportationManager = G4TransportationManager::GetTransportationManager();
	fNavigator = transportationManager->GetNavigator(transportationManager->GetParallelWorld(fComponent->GetWorldName()));
}


void TsGeneratorVolumetric::GeneratePrimaries(G4Event* anEvent)
{
	if (CurrentSourceHasGeneratedEnough())
		return;

	if (fNeedToCalculateExtent)
		CalculateExtent();

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

	G4double testX;
	G4double testY;
	G4double testZ;
	
	G4VPhysicalVolume* foundVolume;
	G4bool foundPointInComponent = false;

	// Uncomment to make all sampled points be accepted.
	// This allows one to see the full extent by visualizing the starting points of all primaries.
	/*foundPointInComponent = true;
	testX = G4RandFlat::shoot(fXMin, fXMax);
	testY = G4RandFlat::shoot(fYMin, fYMax);
	testZ = G4RandFlat::shoot(fZMin, fZMax);*/

	G4int counter = 0;
	while (!foundPointInComponent) {
		//G4cout << "looking for point: " << counter << G4endl;
		// Randomly sample a point in a big cube
		testX = G4RandFlat::shoot(fXMin, fXMax);
		testY = G4RandFlat::shoot(fYMin, fYMax);
		testZ = G4RandFlat::shoot(fZMin, fZMax);
	
		// Check whether they are inside any of the component's volumes
		foundVolume = fNavigator->LocateGlobalPointAndSetup(G4ThreeVector(testX, testY, testZ));
		//G4cout << "foundVolume: " << foundVolume->GetName() << G4endl;
		
		// Protect against case where extent has extended outside of world
		if (foundVolume) {
			if (foundVolume->GetName()!="World") {
				for ( size_t t = 0; !foundPointInComponent && t < fVolumes.size(); t++ )
					if (fVolumes[t]->GetLogicalVolume()->GetMaterial() == fActiveMaterial)
						foundPointInComponent = foundVolume == fVolumes[t];
			}

			if (counter++ > fMaxNumberOfPointsToSample) {
				G4cerr << "TOPAS is exiting due to a serious error in the volumetric source: " << GetName() << G4endl;
				G4cerr << "In " << fMaxNumberOfPointsToSample << " attempts, we have never found a suitable starting position." << G4endl;
				G4cerr << "Check that the ActiveMaterial is actually present somewhere in the source component." << G4endl;
				fPm->AbortSession(1);
			}
		}
	}
	
	p.posX = testX;
	p.posY = testY;
	p.posZ = testZ;

	SetEnergy(p);
	SetParticleType(p);

	p.weight = 1.;
	p.isNewHistory = true;

	GenerateOnePrimary(anEvent, p);
	AddPrimariesToEvent(anEvent);
}
