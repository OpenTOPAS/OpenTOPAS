//
// ********************************************************************
// *                                                                  *
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

#ifndef TsVGenerator_hh
#define TsVGenerator_hh

#include "TsPrimaryParticle.hh"

#include <vector>

class TsParameterManager;
class TsGeometryManager;

class TsGeneratorManager;

class TsVFilter;
class TsVGeometryComponent;
class TsSource;

class G4Event;
class G4ParticleDefinition;
class G4PrimaryVertex;

class TsVGenerator
{
public:
	TsVGenerator(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName);
	~TsVGenerator();

	// Gives full parameter name given just last part
	G4String GetFullParmName(const char* parmName);

	// Handle time-dependent parameter changes
	void UpdateForSpecificParameterChange(G4String parameter);

	// Called for each event. Define TsPrimaryParticles in above structure
	// then call GenerateOnePrimary and AddPrimariesToEvent
	virtual void GeneratePrimaries(G4Event*) = 0;

	// Get name of this particle generator
	G4String GetName();

	// Get pointer to corresponding TsSource
	TsSource* GetSource();

protected:
	// Resolve any parameters needed by the concrete generator
	virtual void ResolveParameters();

	// Cache any geometry pointers that are to be used by generators.
	// This will be called automatically when relevant geometry has updated.
	virtual void CacheGeometryPointers();

	// Call at start of GeneratePrimaries, returning immediately if this returns true
	G4bool CurrentSourceHasGeneratedEnough();

	// Call at end of GeneratePrimaries
	void GenerateOnePrimary(G4Event*, TsPrimaryParticle);
	void AddPrimariesToEvent(G4Event*);

	// Pointer to parameter manager
	TsParameterManager* fPm;

	// These are values of parameters already resolved by the base class.
	// It is up to you whether you actually use these in your TsPrimaryParticle.
	// SetEnergy() allows the user to set kEnergy according to the same scheme as a "Beam" source.
	void SetEnergy(TsPrimaryParticle &p) const;
	G4double fEnergy;
	G4double fEnergySpread;
	G4bool fBeamEnergyParameterExists;
	G4bool fUseSpectrum;
	G4bool fSpectrumIsContinuous;
	G4int fSpectrumNBins;
	G4double* fSpectrumEnergies;
	G4double* fSpectrumWeights;
	G4double* fSpectrumBinTops;
	G4double* fSpectrumWeightSums;
	G4double* fSpectrumSlopes;

	// These are values of parameters already resolved by the base class.
	// It is up to you whether you actually use these in your TsPrimaryParticle.
	// SetParticleType() allows the user to set {particleDefinition, isOpticalPhoton, isGenericIon, ionCharge}
	// according to the same scheme as a "Beam" source.
	void SetParticleType(TsPrimaryParticle &p) const;
	G4ParticleDefinition* fParticleDefinition;
	G4bool fIsGenericIon;
	G4int fIonCharge;
	G4bool fIsOpticalPhoton;
	G4bool fSetRandomPolarization;
	G4double fPolX;
	G4double fPolY;
	G4double fPolZ;

	// User classes should not access any methods or data beyond this point

public:
	virtual void UpdateForNewRun(G4bool rebuiltSomeComponents);
	virtual void ClearGenerator();

	void SetFilter(TsVFilter* filter);
	TsVFilter* GetFilter();

	void SetIsExecutingSequence(G4bool isExecutingSequence);

	void Finalize();

protected:
	TsGeometryManager* fGm;
	TsSource* fPs;

	G4String fSourceName;

	G4int fHistoriesGeneratedInRun;
	TsVGeometryComponent* fComponent;

	void TransformPrimaryForComponent(TsPrimaryParticle* p);

private:
	TsGeneratorManager* fPgm;

	G4int fVerbosity;

	G4bool fIsExecutingSequence;
	G4bool fHadParameterChangeSinceLastRun;

	TsVFilter* fFilter;
	G4double fProbabilityOfUsingAGivenRandomTime;
	G4long fNumberOfHistoriesInRandomJob;
	G4long fTotalHistoriesGenerated;
	G4long fParticlesGeneratedInRun;
	G4long fParticlesSkippedInRun;
	std::vector<G4PrimaryVertex*> fPrimaries;
};

#endif
