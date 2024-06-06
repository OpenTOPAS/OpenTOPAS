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

#include "TsGeometrySampler.hh"
#include "TsParameterManager.hh"
#include "TsGeometryManager.hh"
#include "TsSplitConfigurator.hh"
#include "TsIStore.hh"
#include "G4VIStore.hh"

#include "G4VPhysicalVolume.hh"
#include "G4ImportanceConfigurator.hh"
#include "G4WeightWindowConfigurator.hh"
#include "G4WeightCutOffConfigurator.hh"
#include "G4TransportationManager.hh"

TsGeometrySampler::TsGeometrySampler(TsParameterManager* pm, TsGeometryManager* gm, G4String worldName, const G4String &particlename)
: fPm(pm), fGm(gm), fSplitConfigurator(0), fIStore(0), fParticleName(particlename), fWorldName(worldName),
fIsConfigured(false), fParaFlag(false)
{
	fWorld = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume();
}


TsGeometrySampler::~TsGeometrySampler() {
	ClearSampling();
}


void TsGeometrySampler::ClearSampling() {
	if (fSplitConfigurator) {
		delete fSplitConfigurator;
		fSplitConfigurator = 0;
	}
	
	fIStore = 0;
	fConfigurators.clear();
	fIsConfigured = false;
}


G4bool TsGeometrySampler::IsConfigured() const {
	G4bool isconf = false;
	if (fIsConfigured) {
		G4cout << "WARNING - TsGeometrySampler::IsConfigured()"
		<< "          Some initalization exists, use ClearSampling()"
		<< "          before a new initialization !" << G4endl;
		isconf = true;
	}
	
	return isconf;
}


void TsGeometrySampler::PrepareImportanceSampling(G4VIStore*,
												  const G4VImportanceAlgorithm  *) {
	return;
}


void TsGeometrySampler::PrepareWeightRoulett(G4double, G4double, G4double) {
	return;
}


void TsGeometrySampler::PrepareWeightWindow(G4VWeightWindowStore *,
											G4VWeightWindowAlgorithm *,
											G4PlaceOfAction ) {
	return;
}


void TsGeometrySampler::PrepareSplitSampling(TsIStore* iStore) {
	fIStore = iStore;
	fSplitConfigurator = new TsSplitConfigurator(fPm, fGm, fWorldName, fParticleName, *fIStore, fParaFlag);
	fSplitConfigurator->SetWorldName(fWorldName);
}


void TsGeometrySampler::Configure() {
	if (!IsConfigured()) {
		fIsConfigured = true;
		
		if (fSplitConfigurator)
			fConfigurators.push_back(fSplitConfigurator);
		
	}
	
#ifdef TOPAS_MT
	G4cout << " make sure AddProcess() is invoked for biasing!!! " << G4endl;
#else
	AddProcess();
#endif
}


void TsGeometrySampler::AddProcess() {
	G4VSamplerConfigurator *preConf = 0;
	G4int i = 0;
	for (G4Configurators::iterator it = fConfigurators.begin(); it != fConfigurators.end(); it++) {
		i++;
		G4VSamplerConfigurator *currConf =*it;
		currConf->Configure(preConf);
		preConf = *it;
	}
}


void TsGeometrySampler::SetParallel(G4bool para) {
	fParaFlag = para;
}


void TsGeometrySampler::SetWorld(const G4VPhysicalVolume* World) {
	fWorld = World;
}


void TsGeometrySampler::SetParticle(const G4String &particlename) {
	fParticleName = particlename;
}
