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

#include "TsSplitConfigurator.hh"
#include "TsParameterManager.hh"
#include "TsGeometryManager.hh"
#include "TsSplitProcess.hh"
#include "TsIStore.hh"

#include "G4ProcessPlacer.hh"
#include "G4TransportationManager.hh"

TsSplitConfigurator::TsSplitConfigurator(TsParameterManager* pm, TsGeometryManager* gm,
										 G4String worldvolumeName, const G4String &particlename,
										 TsIStore &istore, G4bool para)
: fPm(pm), fGm(gm), fWorldName(worldvolumeName), fPlacer(particlename), fIStore(istore), fSplitProcess(0),
fParaFlag(para)
{
	fWorld = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume();
	
	if(fParaFlag)
		fWorld = G4TransportationManager::GetTransportationManager()->GetParallelWorld(fWorldName);
}


TsSplitConfigurator::~TsSplitConfigurator() {
	if (fSplitProcess) {
		fPlacer.RemoveProcess(fSplitProcess);
		delete fSplitProcess;
	}
}

void TsSplitConfigurator::Configure(G4VSamplerConfigurator*)
{
	G4cout << "TsSplitConfigurator:: entering importance configure, paraflag " << fParaFlag << G4endl;
	
	std::vector<G4String>* biasingProcessNames = new std::vector<G4String>;
	G4String prefix = "Vr";
	G4String suffix = "Type";
	fPm->GetParameterNamesBracketedBy(prefix, suffix, biasingProcessNames);
	G4int numberOfBiasingProcesses = biasingProcessNames->size();
	G4bool found = false;
	G4String name;
	if ( numberOfBiasingProcesses > 0 ) {
		for ( int i = 0; i < numberOfBiasingProcesses; i++ ) {
			G4String aBiasingProcessName = (*biasingProcessNames)[i];
			G4String type = fPm->GetStringParameter(aBiasingProcessName);
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(type);
#else
			type.toLower();
#endif
			if ( type == "geometricalparticlesplit") {
				aBiasingProcessName = aBiasingProcessName.substr(0, aBiasingProcessName.length()-suffix.length()-1);
				aBiasingProcessName = aBiasingProcessName.substr(prefix.length()+1);
				name = aBiasingProcessName;
				found = true;
				break;
			}
		}
	}
	
	if ( found )
		fSplitProcess = new TsSplitProcess(fPm, fGm, name, &fIStore);
	
	if (!fSplitProcess) {
		G4cerr << "Topas is exiting due to error in variance reduction." << G4endl;
		G4cerr << "GeometricalParticleSplit not found." << G4endl;
		G4cerr << "Failed allocation of TsSplitProcess." << G4endl;
		fPm->AbortSession(1);
	}
	
	if(fParaFlag)
		fSplitProcess->SetParallelWorld(fWorld->GetName());
	
	fPlacer.AddProcessAsSecondDoIt(fSplitProcess);
}


const G4VTrackTerminator *TsSplitConfigurator::GetTrackTerminator() const {
	return 0;
}


void TsSplitConfigurator::SetWorldName(G4String name) {
	fWorldName = name;
}
