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

#include "TsSourceDistributed.hh"

#include "TsParameterManager.hh"
#include "TsVGeometryComponent.hh"

#include "G4TransportationManager.hh"
#include "G4Navigator.hh"
#include "G4VisExtent.hh"
#include "G4VPhysicalVolume.hh"
#include "Randomize.hh"

TsSourceDistributed::TsSourceDistributed(TsParameterManager* pM, TsSourceManager* psM, G4String sourceName)
: TsSource(pM, psM, sourceName), fNumberOfSourcePoints(0), fPreviousNumberOfSourcePoints(-1),
fRedistributePointsOnNewRun(false)
{
	ResolveParameters();
}

TsSourceDistributed::~TsSourceDistributed()
{}


void TsSourceDistributed::UpdateForNewRun(G4bool rebuiltSomeComponents) {
	TsSource::UpdateForNewRun(rebuiltSomeComponents);

	if (fRedistributePointsOnNewRun || fNumberOfSourcePoints != fPreviousNumberOfSourcePoints)
		PrepareSampledPoints();
}


void TsSourceDistributed::ResolveParameters()
{
	TsSource::ResolveParameters();
	
	if (fPm->ParameterExists(GetFullParmName("RedistributePointsOnNewRun")))
		fRedistributePointsOnNewRun = fPm->GetBooleanParameter(GetFullParmName("RedistributePointsOnNewRun"));

	fNumberOfSourcePoints = fPm->GetIntegerParameter(GetFullParmName("NumberOfSourcePoints"));
	if (fNumberOfSourcePoints < 1) {
		G4cerr << "TOPAS is exiting due to a serious error in the distributed source: " << GetName() << G4endl;
		G4cerr << "NumberOfSourcePoints must be greater than zero." << G4endl;
		fPm->AbortSession(1);
	}

	G4String dist;
	if (fPm->ParameterExists(GetFullParmName("PointDistribution")))
		dist = fPm->GetStringParameter(GetFullParmName("PointDistribution"));
	else
		dist = "flat";

#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(dist);
#else
	dist.toLower();
#endif
	if (dist == "flat") {
		fPointDistribution = FLAT;
	} else if (dist == "gaussian") {
		fPointDistribution = GAUSSIAN;
		fPointDistributionSigma = fPm->GetDoubleParameter(GetFullParmName("PointDistributionSigma"), "Length");
		if (fPointDistributionSigma <= 0.) {
			G4cout << GetFullParmName("PointDistributionSigma") << " must be greater than zero." << G4endl;
			fPm->AbortSession(1);
		}
	} else {
		G4cout << "Particle source \"" << fSourceName << "\" has unknown PointDistribution \""
		<< fPm->GetStringParameter(GetFullParmName("PointDistribution")) << "\"" << G4endl;
		G4cout << "Accepted values are Flat and Gaussian." << G4endl;
		fPm->AbortSession(1);
	}
}


void TsSourceDistributed::PrepareSampledPoints()
{
	fPreviousNumberOfSourcePoints = fNumberOfSourcePoints;
	fSampledPoints.clear();

	G4int maxNumberOfPointsToSample = 1000000;
	if (fPm->ParameterExists(GetFullParmName("MaxNumberOfPointsToSample")))
		maxNumberOfPointsToSample = fPm->GetIntegerParameter(GetFullParmName("MaxNumberOfPointsToSample"));

	G4bool recursivelyIncludeChildren = false;
	if (fPm->ParameterExists(GetFullParmName("RecursivelyIncludeChildren")))
		recursivelyIncludeChildren = fPm->GetBooleanParameter(GetFullParmName("RecursivelyIncludeChildren"));

	G4VisExtent myExtent = fComponent->GetExtent();
	G4double xMin = myExtent.GetXmin();
	G4double xMax = myExtent.GetXmax();
	G4double yMin = myExtent.GetYmin();
	G4double yMax = myExtent.GetYmax();
	G4double zMin = myExtent.GetZmin();
	G4double zMax = myExtent.GetZmax();

	G4TransportationManager* transportationManager = G4TransportationManager::GetTransportationManager();
	G4Navigator* navigator = transportationManager->GetNavigator(transportationManager->GetParallelWorld(fComponent->GetWorldName()));

	G4double testX;
	G4double testY;
	G4double testZ;

	G4VPhysicalVolume* foundVolume;

	std::vector<G4VPhysicalVolume*> volumes = fComponent->GetAllPhysicalVolumes(recursivelyIncludeChildren);

	for (G4int iPoint=0; iPoint < fNumberOfSourcePoints; iPoint++) {
		G4int counter = 0;
		G4bool foundPoint = false;
		while (!foundPoint) {
			//G4cout << "looking for point: " << counter << G4endl;
			// Randomly sample a point in a big cube
			if (fPointDistribution == FLAT) {
				testX = G4RandFlat::shoot(xMin, xMax);
				testY = G4RandFlat::shoot(yMin, yMax);
				testZ = G4RandFlat::shoot(zMin, zMax);
			} else {
				testX = G4RandGauss::shoot(0., fPointDistributionSigma);
				testY = G4RandGauss::shoot(0., fPointDistributionSigma);
				testZ = G4RandGauss::shoot(0., fPointDistributionSigma);
			}
		
			// Check whether they are inside any of the component's volumes
			foundVolume = navigator->LocateGlobalPointAndSetup(G4ThreeVector(testX, testY, testZ));
			//G4cout << "foundVolume: " << foundVolume->GetName() << G4endl;
			
			// Protect against case where extent has extended outside of world
			if (foundVolume) {
				if (foundVolume->GetName()!="World") {
					for ( size_t t = 0; !foundPoint && t < volumes.size(); t++ )
							foundPoint = foundVolume == volumes[t];
				}

				if (counter++ > maxNumberOfPointsToSample) {
					G4cerr << "TOPAS is exiting due to a serious error in the distributed source: " << GetName() << G4endl;
					G4cerr << "In " << maxNumberOfPointsToSample << " attempts, we have never found a suitable starting position." << G4endl;
					fPm->AbortSession(1);
				}
			}
		}
		fSampledPoints.push_back(new G4Point3D(testX, testY, testZ));
	}
}
