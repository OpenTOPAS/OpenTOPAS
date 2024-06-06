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

#include "TsVGenerator.hh"

#include "TsParameterManager.hh"
#include "TsGeometryManager.hh"
#include "TsGeneratorManager.hh"
#include "TsVGeometryComponent.hh"
#include "TsSource.hh"

#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4IonTable.hh"
#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"

#include "Randomize.hh"

TsVGenerator::TsVGenerator(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName)
:fPm(pM), fBeamEnergyParameterExists(false),
fUseSpectrum(false), fSpectrumIsContinuous(false), fSpectrumNBins(0), fIsGenericIon(false), fIonCharge(0), fIsOpticalPhoton(false), fSetRandomPolarization(true),
fGm(gM), fSourceName(sourceName), fHistoriesGeneratedInRun(0),
fPgm(pgM), fIsExecutingSequence(false), fHadParameterChangeSinceLastRun(false), fFilter(0),
fTotalHistoriesGenerated(0), fParticlesGeneratedInRun(0), fParticlesSkippedInRun(0)
{
	fVerbosity = fPm->GetIntegerParameter("So/Verbosity");

	fPs = GetSource();
	fNumberOfHistoriesInRandomJob = fPs->GetNumberOfHistoriesInRandomJob();
	fProbabilityOfUsingAGivenRandomTime = fPs->GetProbabilityOfUsingAGivenRandomTime();

	fPgm->SetCurrentGenerator(this);
}


TsVGenerator::~TsVGenerator()
{
}


void TsVGenerator::UpdateForSpecificParameterChange(G4String parameter) {
	if (fVerbosity>0)
		G4cout << "TsVGenerator::UpdateForSpecificParameterChange called for parameter: " << parameter << G4endl;
	fHadParameterChangeSinceLastRun = true;
}


void TsVGenerator::UpdateForNewRun(G4bool rebuiltSomeComponents) {
	if (fVerbosity>0){
		G4cout << "TsVGenerator::UpdateForNewRun for source: " << GetName() << " called with fHadParameterChangeSinceLastRun: " <<
		fHadParameterChangeSinceLastRun << ", rebuiltSomeComponents: " << rebuiltSomeComponents << G4endl;
	    G4cout << "Particle source: " << GetName() << " generated " << fHistoriesGeneratedInRun << " in the previous run." << G4endl;
	}

	fPs = GetSource();
	fNumberOfHistoriesInRandomJob = fPs->GetNumberOfHistoriesInRandomJob();
	fProbabilityOfUsingAGivenRandomTime = fPs->GetProbabilityOfUsingAGivenRandomTime();

	fHistoriesGeneratedInRun = 0;
	fParticlesGeneratedInRun = 0;
	fParticlesSkippedInRun = 0;

	fComponent->MarkAsNeedToUpdatePlacement();

	if (fHadParameterChangeSinceLastRun) {
		ResolveParameters();
		fHadParameterChangeSinceLastRun = false;
	} else if (rebuiltSomeComponents)
		CacheGeometryPointers();
}


void TsVGenerator::ClearGenerator() {
}


void TsVGenerator::ResolveParameters() {
	if (fVerbosity>0)
		G4cout << "TsVGenerator::ResolveParameters" << G4endl;

	G4bool isSpectrum = false;
	if (fPm->ParameterExists(GetFullParmName("BeamEnergySpectrumType"))) {
		G4String spectrumType = fPm->GetStringParameter(GetFullParmName("BeamEnergySpectrumType"));
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(spectrumType);
#else
		spectrumType.toLower();
#endif
		if (spectrumType != "none") isSpectrum = true;
	}

	if (isSpectrum) {
		fEnergy = 0.;
		fEnergySpread  = 0.;

		G4String spectrumType = fPm->GetStringParameter(GetFullParmName("BeamEnergySpectrumType"));
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(spectrumType);
#else
		spectrumType.toLower();
#endif

		if (spectrumType=="discrete" || spectrumType=="continuous") {
			fUseSpectrum = true;

			fSpectrumNBins = fPm->GetVectorLength(GetFullParmName("BeamEnergySpectrumValues"));
			if (fSpectrumNBins < 1) {
				G4cout << GetFullParmName("BeamEnergySpectrumValues") << " has wrong length." << G4endl;
				G4cout << "Must be greater than 0." << G4endl;
				fPm->AbortSession(1);
			}

			G4double* spectrumEnergies = fPm->GetDoubleVector(GetFullParmName("BeamEnergySpectrumValues"), "Energy");

			if (fPm->GetVectorLength(GetFullParmName("BeamEnergySpectrumWeights")) != fSpectrumNBins) {
				G4cout << GetFullParmName("BeamEnergySpectrumWeights") << " has wrong length." << G4endl;
				G4cout << "Length must match: " << GetFullParmName("BeamEnergySpectrumValues") << G4endl;
				fPm->AbortSession(1);
			}

			G4double* spectrumWeights = fPm->GetUnitlessVector(GetFullParmName("BeamEnergySpectrumWeights"));

			if (spectrumType=="continuous") {
				fSpectrumIsContinuous = true;

				// Add missing zero bins
				fSpectrumNBins++;
				fSpectrumEnergies = new G4double[fSpectrumNBins];
				fSpectrumWeights = new G4double[fSpectrumNBins];
				fSpectrumWeightSums = new G4double[fSpectrumNBins];
				fSpectrumEnergies[0] = 0.;
				fSpectrumWeights[0] = 0.;
				fSpectrumWeightSums[0] = 0.;
				for (int i = 1; i < fSpectrumNBins; i++) {
					fSpectrumEnergies[i] = spectrumEnergies[i-1];
					fSpectrumWeights[i] = spectrumWeights[i-1];
					fSpectrumWeightSums[i] = fSpectrumWeightSums[i-1] + 0.5 * (fSpectrumWeights[i] + fSpectrumWeights[i-1]) * (fSpectrumEnergies[i] - fSpectrumEnergies[i-1]);
				}

				fSpectrumSlopes = new G4double[fSpectrumNBins];
				for (int i = 0; i < fSpectrumNBins-1; i++)
					fSpectrumSlopes[i] = (fSpectrumWeights[i+1] - fSpectrumWeights[i]) / (fSpectrumEnergies[i+1] - fSpectrumEnergies[i]);
			} else {
				fSpectrumEnergies = spectrumEnergies;
				fSpectrumWeights = spectrumWeights;
				fSpectrumBinTops = new G4double[fSpectrumNBins];
				fSpectrumBinTops[0] = fSpectrumWeights[0];
				for (int i = 1; i < fSpectrumNBins; i++)
					fSpectrumBinTops[i] = fSpectrumBinTops[i-1] + fSpectrumWeights[i];

				if (fSpectrumBinTops[fSpectrumNBins-1] < .999 || fSpectrumBinTops[fSpectrumNBins-1] > 1.001) {
					G4cout << "Sum of weights in " << GetFullParmName("BeamEnergySpectrumWeights") <<
					" needs to be exactly 1 but instead is: " << fSpectrumBinTops[fSpectrumNBins-1] << G4endl;
					fPm->AbortSession(1);
				}
			}
		} else if (spectrumType != "none") {
			G4cout << "Invalid parameter: " << GetFullParmName("BeamEnergySpectrumType") << G4endl;
			G4cout << "Value must be one of \"None\", \"Discrete\" or \"Continuous\"" << G4endl;
			fPm->AbortSession(1);
		}
	} else {
		if (fPm->ParameterExists(GetFullParmName("BeamEnergy"))) {
			fBeamEnergyParameterExists = "true";
			fEnergy = fPm->GetDoubleParameter(GetFullParmName("BeamEnergy"), "Energy");

			if (fPm->ParameterExists(GetFullParmName("BeamEnergySpread")))
				fEnergySpread  = fPm->GetUnitlessParameter(GetFullParmName("BeamEnergySpread")) * fEnergy / 100.;
		}
	}


	TsParticleDefinition resolvedDef = fPm->GetParticleDefinition(fPm->GetStringParameter(GetFullParmName("BeamParticle")));

	if (!resolvedDef.particleDefinition)
		G4cout << "Unknown particle type read from parameter = " << GetFullParmName("BeamParticle") << G4endl;

	fParticleDefinition = resolvedDef.particleDefinition;

	if (resolvedDef.isGenericIon) {
		fIsGenericIon = true;

		if ( resolvedDef.ionZ == -1 || resolvedDef.ionA == -1) {
			G4cerr << "Topas is exiting. Particle source: " << fSourceName << G4endl;
			G4cerr <<" is attempting to use wild card in Generic Ion Z or A: " << fPm->GetStringParameter(GetFullParmName("BeamParticle")) << G4endl;
			fPm->AbortSession(1);
		}

		if (resolvedDef.ionCharge != 999)
			fIonCharge = resolvedDef.ionCharge;
	}

	fIsOpticalPhoton = resolvedDef.isOpticalPhoton;
	if (fIsOpticalPhoton) {
		if (fPm->ParameterExists(GetFullParmName("BeamPolarizationX"))||
			fPm->ParameterExists(GetFullParmName("BeamPolarizationY"))||
			fPm->ParameterExists(GetFullParmName("BeamPolarizationZ"))) {
			fPolX = fPm->GetUnitlessParameter(GetFullParmName("BeamPolarizationX"));
			fPolY = fPm->GetUnitlessParameter(GetFullParmName("BeamPolarizationY"));
			fPolZ = fPm->GetUnitlessParameter(GetFullParmName("BeamPolarizationZ"));
			fSetRandomPolarization = false;
		}
	}

	CacheGeometryPointers();
}


void TsVGenerator::CacheGeometryPointers() {
	if (fVerbosity>0)
		G4cout << "TsVGenerator::CacheGeometryPointers" << G4endl;

	// Find the Geometry Component relative to which the start position and momentum direction should be set.
	G4String componentName = fPm->GetStringParameter(GetFullParmName("Component"));

	fComponent = fGm->GetComponent(componentName);
	if (!fComponent) {
		G4cerr << "Topas is exiting. Particle source: " << fSourceName << G4endl;
		G4cerr <<" has been placed relative to unknown Geometry Component: " << componentName << G4endl;
		fPm->AbortSession(1);
	}
}


void TsVGenerator::SetEnergy(TsPrimaryParticle &p) const
{
	if (fUseSpectrum) {
		if (fSpectrumIsContinuous) {
			G4double aRandom = G4UniformRand() * fSpectrumWeightSums[fSpectrumNBins - 1];
			G4int j = fSpectrumNBins - 2;
			while ((fSpectrumWeightSums[j] > aRandom) && (j > 0)) j--;
			p.kEnergy = fSpectrumEnergies[j];

			if (fSpectrumSlopes[j] != 0.) {
				G4double b = fSpectrumWeights[j] / fSpectrumSlopes[j];
				G4double c = 2 * (aRandom - fSpectrumWeightSums[j]) / fSpectrumSlopes[j];
				G4double delta = b * b + c;

				G4int sign = 1;
				if (fSpectrumSlopes[j] < 0.) sign = -1;

				p.kEnergy += sign * sqrt(delta) - b;
			} else if (fSpectrumWeights[j] > 0.) {
				p.kEnergy += (aRandom - fSpectrumWeightSums[j]) / fSpectrumWeights[j];
			}
		} else {
			G4double aRandom = G4UniformRand();
			G4int j = fSpectrumNBins - 1;
			while ((fSpectrumBinTops[j] >= aRandom) && (j >= 0)) j--;
			p.kEnergy = fSpectrumEnergies[j+1];
		}
	} else {
		if (fBeamEnergyParameterExists) {
			if (fEnergySpread == 0.)
				p.kEnergy = fEnergy;
			else
				p.kEnergy = G4RandGauss::shoot(fEnergy, fEnergySpread);
		} else {
			G4cerr << "Topas is exiting. Particle source: " << fSourceName << G4endl;
			G4cerr << "is missing the required parameter: BeamEnergy" << G4endl;
			fPm->AbortSession(1);
		}
	}
}


void TsVGenerator::SetParticleType(TsPrimaryParticle &p) const
{
	p.particleDefinition = fParticleDefinition;
	p.isOpticalPhoton = fIsOpticalPhoton;
	p.isGenericIon = fIsGenericIon;
	p.ionCharge = fIonCharge;
}


void TsVGenerator::TransformPrimaryForComponent(TsPrimaryParticle* p) {
	// Set the particle direction to include rotation of the geometry component
	G4ThreeVector* direction = new G4ThreeVector(p->dCos1,p->dCos2,p->dCos3);
	G4RotationMatrix* invMatrix = new G4RotationMatrix(fComponent->GetRotRelToWorld()->inverse());
	*direction = *invMatrix * *direction;

	// Set the vertex position to include position of the geometry component
	G4Point3D* local = new G4Point3D(p->posX,p->posY,p->posZ);
	G4Point3D* center = new G4Point3D(*(fComponent->GetTransRelToWorld()));
	*center = G4Translate3D(*invMatrix * *local) * (*center);

	p->posX = center->x();
	p->posY = center->y();
	p->posZ = center->z();
	p->dCos1 = direction->x();
	p->dCos2 = direction->y();
	p->dCos3 = direction->z();

	delete direction;
	delete invMatrix;
	delete local;
	delete center;
}


void TsVGenerator::GenerateOnePrimary(G4Event*, TsPrimaryParticle p)
{
	// For now, time is always zero, but will later have option to get this from phase space
	G4double particle_time = 0.0;

	// Create a primary vertex
	G4PrimaryVertex* vertex = new G4PrimaryVertex(G4ThreeVector(p.posX,p.posY,p.posZ), particle_time);

	// Omit particles that came from phase space but have unknown definition
	// (will have already written relevant warning message during phase space reading).
	if (p.particleDefinition) {
		G4double mass = p.particleDefinition->GetPDGMass();
		G4double energy = p.kEnergy + mass;
		G4double pmom = sqrt(energy*energy-mass*mass);
		G4double px = pmom * p.dCos1;
		G4double py = pmom * p.dCos2;
		G4double pz = pmom * p.dCos3;

		G4PrimaryParticle* particle = new G4PrimaryParticle(p.particleDefinition,px,py,pz);
		particle->SetWeight(p.weight);

		if (p.isGenericIon)
			particle->SetCharge(p.ionCharge*eplus);

		if ( p.isOpticalPhoton ) {
			if ( fSetRandomPolarization ) {
				G4double tanTheta = sqrt(p.dCos1*p.dCos1 + p.dCos2*p.dCos2)/p.dCos3;
				G4double polX = p.dCos1/tanTheta;
				G4double polY = p.dCos2/tanTheta;
				G4double polZ = -tanTheta;
				G4ThreeVector polarization(polX, polY, polZ);
				G4ThreeVector orthogonalVector = G4ThreeVector(p.dCos1,p.dCos2,p.dCos3).cross(polarization);
				G4double phi = CLHEP::twopi*G4UniformRand();
				G4double sinPhi = sin(phi);
				G4double cosPhi = cos(phi);
				polarization = cosPhi * polarization + sinPhi * orthogonalVector;
				polarization = polarization.unit();
				fPolX = polarization.x();
				fPolY = polarization.y();
				fPolY = polarization.z();
			}
			
			particle->SetPolarization(fPolX, fPolY, fPolZ);
		}
		
		// Tell vertex to use this particle
		vertex->SetPrimary(particle);

		// Add vertex to the primary vector.
		// We have to add them later, after we have the full set, so that we
		// can get the correct order.
		fPrimaries.push_back(vertex);

		// Register the primary to this particular source
		fPgm->RegisterPrimary(this);
	} else {
		fParticlesSkippedInRun++;
	}
}


void TsVGenerator::SetFilter(TsVFilter* filter) {
	fFilter = filter;
}


TsVFilter* TsVGenerator::GetFilter() {
	return fFilter;
}


void TsVGenerator::SetIsExecutingSequence(G4bool isExecutingSequence) {
	fIsExecutingSequence = isExecutingSequence;
}


G4String TsVGenerator::GetFullParmName(const char* parm) {
	G4String fullName = "So/"+fSourceName+"/"+parm;
	return fullName;
}


G4bool TsVGenerator::CurrentSourceHasGeneratedEnough()
{
	// Do not limit histories that are initiated directly from Geant4 line.
	if (fIsExecutingSequence) {
		if (fPm->IsRandomMode()) {
			// Return if this source has already generated enough histories for the job.
			if ( fTotalHistoriesGenerated >= fNumberOfHistoriesInRandomJob )
				return true;

			// If we are using a probability function to determine which events to use,
			// randomly skip some fraction of events.
			if (fProbabilityOfUsingAGivenRandomTime >= 0. &&
				G4RandFlat::shoot( 0., 1. ) > fProbabilityOfUsingAGivenRandomTime)
				return true;
		}
	}

	return false;
}


void TsVGenerator::AddPrimariesToEvent(G4Event* anEvent) {
	// We couldn't add the primaries to the event in the above loop because
	// we need to add them in the opposite order they were generated in.
	// So now we loop through the vector of primaries and add them to the event.
	while (!fPrimaries.empty()) {
		anEvent->AddPrimaryVertex(fPrimaries.back());
		fPrimaries.pop_back();
	    fParticlesGeneratedInRun++;
	}
	fHistoriesGeneratedInRun++;
	fTotalHistoriesGenerated++;
}


G4String TsVGenerator::GetName() {
	return fSourceName;
}


TsSource* TsVGenerator::GetSource() {
	return fPgm->GetSource(fSourceName);
}


void TsVGenerator::Finalize() {
	fPs->NoteNumberOfHistoriesGenerated(fHistoriesGeneratedInRun);
	fPs->NoteNumberOfParticlesGenerated(fParticlesGeneratedInRun);
	fPs->NoteNumberOfParticlesSkipped(fParticlesSkippedInRun);
}
