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

#include "TsGeneratorEnvironment.hh"

#include "TsParameterManager.hh"
#include "TsVGeometryComponent.hh"
#include "TsGeometryManager.hh"

#include "G4TransportationManager.hh"
#include "G4Navigator.hh"
#include "G4VisExtent.hh"
#include "G4Scene.hh"
#include "Randomize.hh"
#include "G4RandomDirection.hh"
#include "G4UnitsTable.hh"

#include <limits>

TsGeneratorEnvironment::TsGeneratorEnvironment(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName) :
TsVGenerator(pM, gM, pgM, sourceName), fNeedToCalculateExtent(true)
{
	fRecursivelyIncludeChildren = false;
	if (fPm->ParameterExists(GetFullParmName("RecursivelyIncludeChildren")))
		fRecursivelyIncludeChildren = fPm->GetBooleanParameter(GetFullParmName("RecursivelyIncludeChildren"));
	
	ResolveParameters();
}


TsGeneratorEnvironment::~TsGeneratorEnvironment()
{
}


void TsGeneratorEnvironment::ResolveParameters() {
	TsVGenerator::ResolveParameters();
		
	fMaxNumberOfPointsToSample = 1000000;
	if (fPm->ParameterExists(GetFullParmName("MaxNumberOfPointsToSample")))
		fMaxNumberOfPointsToSample = fPm->GetIntegerParameter(GetFullParmName("MaxNumberOfPointsToSample"));

	fVolumes = fComponent->GetAllPhysicalVolumes(fRecursivelyIncludeChildren);

	fNeedToCalculateExtent = true;
}


void TsGeneratorEnvironment::UpdateForNewRun(G4bool rebuiltSomeComponents) {
	TsVGenerator::UpdateForNewRun(rebuiltSomeComponents);
	ResolveParameters();

	// Needed so that voxelizer and navigator can correctly evaluate positions in whole new world
	fGm->UpdateWorldForNewRun();
}


void TsGeneratorEnvironment::CalculateExtent()
{
	const G4VisExtent& myExtent = fComponent->GetExtent();
	fRadius = myExtent.GetExtentRadius();
	fCentre = myExtent.GetExtentCentre();
	fNeedToCalculateExtent = false;

	G4TransportationManager* transportationManager = G4TransportationManager::GetTransportationManager();
	fNavigator = transportationManager->GetNavigator(transportationManager->GetParallelWorld(fComponent->GetWorldName()));

	// Check that the environment cavity does not extend outside the world volume.
	// It can haapen that the world is big enough to enclose the component(s) but
	// not big enough to enclose the cavity, which is a sphere around the components.

	// If we are going to do this on one thread only (no need to check for every thread)
	// we will need to use a mutex lock.

	static G4Mutex mutex = G4MUTEX_INITIALIZER;
	G4AutoLock al(&mutex);
	static G4bool tested = false;
	if (!tested) {
		tested = true;

		// Get the world (the initial value of the iterator points to the mass world).
		G4VPhysicalVolume* world = *(transportationManager->GetWorldsIterator());
		const auto& worldExtent = world->GetLogicalVolume()->GetSolid()->GetExtent();

		// We need the limits of the sphere
		G4double xmin = fCentre.x() - fRadius;
		G4double xmax = fCentre.x() + fRadius;
		G4double ymin = fCentre.y() - fRadius;
		G4double ymax = fCentre.y() + fRadius;
		G4double zmin = fCentre.z() - fRadius;
		G4double zmax = fCentre.z() + fRadius;

		// The cavity extends outside the world by this much
		G4double out = -std::numeric_limits<G4double>::max();
		G4double dout;  // Working variable
		dout = worldExtent.GetXmin() - xmin; if (dout > out) out = dout;
		dout = xmax - worldExtent.GetXmax(); if (dout > out) out = dout;
		dout = worldExtent.GetYmin() - ymin; if (dout > out) out = dout;
		dout = ymax - worldExtent.GetYmax(); if (dout > out) out = dout;
		dout = worldExtent.GetZmin() - zmin; if (dout > out) out = dout;
		dout = zmax - worldExtent.GetZmax(); if (dout > out) out = dout;

		if (out >= 0.) {
			G4cerr << "TOPAS is exiting due to a serious error in the environment source: " << GetName() << G4endl;
			G4cerr << "The environment source extends outside the world by at least " << G4BestUnit(out,"Length") << G4endl;
			G4cerr << "Make your world bigger" << G4endl;
			fPm->AbortSession(1);
		} else {
			G4cout <<
			"TsGeneratorEnvironment::CalculateExtent: The environment source is"
			"\ncomfortably inside the world by " << G4BestUnit(-out,"Length")
			<< G4endl;
		}
	}
}


void TsGeneratorEnvironment::GeneratePrimaries(G4Event* anEvent)
{
	if (CurrentSourceHasGeneratedEnough())
		return;

	if (fNeedToCalculateExtent)
		CalculateExtent();

	// Generate a point on the "radiation cavity" surface
	auto d1 = G4RandomDirection();
	auto x = fRadius * d1.x() + fCentre.x();
	auto y = fRadius * d1.y() + fCentre.y();
	auto z = fRadius * d1.z() + fCentre.z();

	// Generate an emission angle according to Lambert's cosine law.
	// Implies cos^2 has uniform distribution.
	auto costh2 = G4UniformRand();
	auto costh = std::sqrt(costh2);
	auto sinth = std::sqrt(1. - costh2);
	auto phi = CLHEP::twopi * G4UniformRand();
	auto sinphi = std::sin(phi);
	auto cosphi = std::cos(phi);
	auto d2 = G4ThreeVector(sinth*cosphi,sinth*sinphi,costh);
	d2.rotateUz(-d1);  // Rotate to inward direction

	TsPrimaryParticle p;

	p.posX = x;
	p.posY = y;
	p.posZ = z;
	p.dCos1 = d2.x();
	p.dCos2 = d2.y();
	p.dCos3 = d2.z();

	SetEnergy(p);
	SetParticleType(p);

	p.weight = 1.;
	p.isNewHistory = true;

	GenerateOnePrimary(anEvent, p);
	AddPrimariesToEvent(anEvent);
}
