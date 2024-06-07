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

#include "TsParameterManager.hh"
#include "TsGeometryManager.hh"
#include "TsPhysicsManager.hh"
#include "TsVarianceManager.hh"

#include "TsGeometricalParticleSplit.hh"
#include "TsImportanceSampling.hh"
#include "TsWeightWindow.hh"
#include "TsUniformSplitting.hh"
#include "TsCrossSectionEnhancement.hh"
#include "TsForcedInteraction.hh"
#include "TsKillOtherParticles.hh"
#include "TsDirectionalRussianRoulette.hh"
#include "TsRangeRejection.hh"
#include "TsInelasticSplitManager.hh"
#include "TsAutomaticImportanceSamplingManager.hh"
#include "TsAutomaticImportanceSamplingParallelManager.hh"
#include "TsVBiasingProcess.hh"

#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4ClassificationOfNewTrack.hh"

#include <vector>

TsVarianceManager::TsVarianceManager(TsParameterManager *pM, TsGeometryManager *gM, TsPhysicsManager *phM)
	: fPm(pM), fGm(gM), fPhm(phM),
	  fUseKillOtherParticles(false), fIndexKillOtherParticles(-1),
	  fUseDirectionalRussianRoulette(false), fIndexDirectionalRussianRoulette(-1),
	  fUseRangeRejection(false), fIndexRangeRejection(-1)
{
	fGm->SetVarianceManager(this);
	fPhm->SetVarianceManager(this);
}


TsVarianceManager::~TsVarianceManager()
{}


void TsVarianceManager::Configure() {
	std::vector<G4String>* biasingProcessNames = new std::vector<G4String>;
	G4String prefix = "Vr";
	G4String suffix = "Type";
	fPm->GetParameterNamesBracketedBy(prefix, suffix, biasingProcessNames);
	G4int numberOfBiasingProcesses = biasingProcessNames->size();
	G4bool found;
	if ( numberOfBiasingProcesses > 0 ) {
		for ( int i = 0; i < numberOfBiasingProcesses; i++ ) {
			G4String aBiasingProcessName = (*biasingProcessNames)[i];
			G4String type = fPm->GetStringParameter(aBiasingProcessName);
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(type);
#else
			type.toLower();
#endif
			aBiasingProcessName = aBiasingProcessName.substr(0, aBiasingProcessName.length()-suffix.length()-1);
			aBiasingProcessName = aBiasingProcessName.substr(prefix.length()+1);
			
			found = false;
			for ( int j = i + 1; j < numberOfBiasingProcesses; j++ ) {
				G4String tempName = (*biasingProcessNames)[j];
#if GEANT4_VERSION_MAJOR >= 11
				G4StrUtil::to_lower(tempName);
#else
				tempName.toLower();
#endif
				if ( (*biasingProcessNames)[i] == tempName )
					found = true;
			}
			
			if (!found) {
				if ( type == "geometricalparticlesplit" ) {
					TsGeometricalParticleSplit* geomPartSplit = new TsGeometricalParticleSplit(aBiasingProcessName, fPm, fGm);
					geomPartSplit->SetBiasingProcess(geomPartSplit->GetGeometrySampler());
					fBiasingProcesses.push_back(geomPartSplit);
				} else if ( type == "importancesampling" ) {
					TsImportanceSampling* importanceSampling = new TsImportanceSampling(aBiasingProcessName, fPm, fGm);
					fBiasingProcesses.push_back(importanceSampling);
				} else if ( type == "weightwindow" ) {
					TsWeightWindow* weightWindow = new TsWeightWindow(aBiasingProcessName, fPm, fGm);
					fBiasingProcesses.push_back(weightWindow);
				}
				else if ( type == "uniformsplitting" || type == "secondarybiasing" ) {
					fBiasingProcesses.push_back(new TsUniformSplitting(aBiasingProcessName, fPm, fGm));
				} else if ( type == "crosssectionenhancement") {
					fBiasingProcesses.push_back(new TsCrossSectionEnhancement(aBiasingProcessName, fPm, fGm));
				} else if ( type == "forcedinteraction" ) {
					fBiasingProcesses.push_back(new TsForcedInteraction(aBiasingProcessName, fPm, fGm));
				} else if ( type == "killotherparticles" ) {
					fUseKillOtherParticles = true;
					fIndexKillOtherParticles = (G4int)fBiasingProcesses.size();
					fBiasingProcesses.push_back(new TsKillOtherParticles(aBiasingProcessName, fPm, fGm));
				} else if ( type == "directionalrussianroulette" ) {
					fUseDirectionalRussianRoulette = true;
					fIndexDirectionalRussianRoulette = (G4int)fBiasingProcesses.size();
					fBiasingProcesses.push_back(new TsDirectionalRussianRoulette(aBiasingProcessName, fPm, fGm));
				} else if ( type == "rangerejection" ) {
					fUseRangeRejection = true;
					fIndexRangeRejection = (G4int)fBiasingProcesses.size();
					fBiasingProcesses.push_back(new TsRangeRejection(aBiasingProcessName, fPm, fGm));
				} else if ( type == "inelasticsplitting" ) {
					fBiasingProcesses.push_back(new TsInelasticSplitManager(aBiasingProcessName, fPm, fGm));
				} else if ( type == "automaticimportancesampling") {
					fBiasingProcesses.push_back(new TsAutomaticImportanceSamplingManager(aBiasingProcessName, fPm, fGm));
				} else if ( type == "automaticimportancesamplingparallel") {
					fBiasingProcesses.push_back(new TsAutomaticImportanceSamplingParallelManager(aBiasingProcessName, fPm, fGm));
				}
				else {
					G4cerr << "Error, biasing technique " << type << " not found" << G4endl;
					fPm->AbortSession(1);
				}
			}
		}
	}
}


void TsVarianceManager::Initialize() {
	for ( auto biasingProcess : fBiasingProcesses )
		biasingProcess->Initialize();
}


void TsVarianceManager::AddBiasingProcess() {
	for ( auto biasingProcess : fBiasingProcesses )
		biasingProcess->AddBiasingProcess();
}


TsVBiasingProcess* TsVarianceManager::GetBiasingProcessFromList(G4int index) {
	return fBiasingProcesses[index];
}


void TsVarianceManager::Clear() {
	for ( auto biasingProcess : fBiasingProcesses )
		biasingProcess->Clear();
}


void TsVarianceManager::UpdateForNewRun(G4bool) {
	Clear();
	Initialize();
}


G4bool TsVarianceManager::BiasingProcessExists(G4String type, G4int& index) {
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(type);
#else
	type.toLower();
#endif
	G4int ind = 0;
	for ( auto biasingProcess : fBiasingProcesses ) {
		G4String aName = biasingProcess->GetTypeName();
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(aName);
#else
		aName.toLower();
#endif
		if ( aName == type ) {
			index = ind;
			return true;
		}
		ind++;
	}
	return false;
}


G4ClassificationOfNewTrack TsVarianceManager::ApplyKillOtherParticles(const G4Track* aTrack) {
	return ((TsKillOtherParticles*)fBiasingProcesses[fIndexKillOtherParticles])->Apply(aTrack);
}


G4ClassificationOfNewTrack TsVarianceManager::ApplyDirectionalRussianRoulette(const G4Track* aTrack) {
	return ((TsDirectionalRussianRoulette*)fBiasingProcesses[fIndexDirectionalRussianRoulette])->Apply(aTrack);
}


G4ClassificationOfNewTrack TsVarianceManager::ApplyRangeRejection(const G4Track* aTrack) {
	return ((TsRangeRejection*)fBiasingProcesses[fIndexRangeRejection])->Apply(aTrack);
}
