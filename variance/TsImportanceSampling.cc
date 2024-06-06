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

#include "TsImportanceSampling.hh"
#include "TsGeometryManager.hh"
#include "TsParameterManager.hh"

#include "TsVGeometryComponent.hh"

#include "G4IStore.hh"
#include "G4GeometrySampler.hh"
#include "G4UImanager.hh"
#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"

#include "globals.hh"
#include <vector>

TsImportanceSampling::TsImportanceSampling(G4String name, TsParameterManager* pM, TsGeometryManager* gM)
: TsVBiasingProcess(name, pM, gM), fGm(gM), fPm(pM), fName(name), fIsParallel(false),
fGeometryAlreadyBuilt(false), fCellAlreadyBuilt(false), fVerbosity(0)
{
	ResolveParameters();
	SetGeometrySampler();
}


TsImportanceSampling::~TsImportanceSampling()
{;}


void TsImportanceSampling::ResolveParameters() {
	fType = fPm->GetStringParameter(GetFullParmName("Type"));
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(fType);
#else
	fType.toLower();
#endif
	
	fParticleName = fPm->GetStringVector(GetFullParmName("ParticleName"));
	fParticleNameLength = fPm->GetVectorLength(GetFullParmName("ParticleName"));
	
	fComponentName = fPm->GetStringParameter(GetFullParmName("Component"));
	fSubComponentNames = fPm->GetStringVector(GetFullParmName("SubComponents"));
	fSubCompSize = fPm->GetVectorLength(GetFullParmName("SubComponents"));
	
	
	G4int impSize = fPm->GetVectorLength(GetFullParmName("ImportanceValues"));
	
	if ( fSubCompSize != impSize ) {
		G4String message = "The number of elements must matches with " + GetFullParmName("SubComponents");
		Quit(GetFullParmName("ImportanceValues"), message);
	}
	
	fVerbosity = fPm->GetIntegerParameter("Ts/SequenceVerbosity");
}


void TsImportanceSampling::Initialize() {
	if (!fGeometryAlreadyBuilt)
		SetGeometry();
	
	SetImportanceSampling();
}


void TsImportanceSampling::SetGeometry() {
	G4VPhysicalVolume* physWorld = fGm->GetComponent(fComponentName)->GetParentVolume();
	G4VPhysicalVolume* physComponent = fGm->GetPhysicalVolume(fComponentName);
	
	fPhysVol.push_back(physWorld);
	fPhysVol.push_back(physComponent);
	
	for ( int i = 0; i < fSubCompSize; i++ )
		fPhysVol.push_back(fGm->GetPhysicalVolume(fSubComponentNames[i]));
	
	fGeometryAlreadyBuilt = true;
}


void TsImportanceSampling::SetImportanceSampling() {
	G4double* importances = fPm->GetUnitlessVector(GetFullParmName("ImportanceValues"));
	
	G4GeometryCell world(*(fPhysVol[0]), 0);
	G4GeometryCell component(*(fPhysVol[1]), 0);
	
#if GEANT4_VERSION_MAJOR >= 11
	G4IStore* aIstore = G4IStore::GetInstance(fPhysVol[0]->GetName());
#else 
	G4IStore* aIstore = G4IStore::GetInstance(*fPhysVol[0]->GetName());
#endif
	
	if ( !fCellAlreadyBuilt ) {
		aIstore->AddImportanceGeometryCell(1, world);
		aIstore->AddImportanceGeometryCell(1, component);
		
		for ( int i = 0; i < fSubCompSize; i++ ) {
			G4GeometryCell aSubComponent(*fPhysVol[i+2], 0);
			aIstore->AddImportanceGeometryCell( importances[i], aSubComponent);
			if ( fVerbosity > 1 ) {
				G4cout << "Adding importance value: " << importances[i]
				<< " to component: " << aSubComponent.GetPhysicalVolume().GetName() << G4endl;
			}
		}
	} else {
		aIstore->ChangeImportance(1, world);
		aIstore->ChangeImportance(1, component);
		
		for ( int i = 0; i < fSubCompSize; i++ ) {
			G4GeometryCell aSubComponent(*fPhysVol[i+2], 0);
			aIstore->ChangeImportance( importances[i], aSubComponent);
			if ( fVerbosity > 1 ) {
				G4cout << "Changing to importance value: " << importances[i]
				<< " for component: " << aSubComponent.GetPhysicalVolume().GetName() << G4endl;
			}
		}
	}
	fCellAlreadyBuilt = true;
}


void TsImportanceSampling::SetGeometrySampler() {
	G4String compName;
	if (fGm->GetComponent(fComponentName)->IsParallel()) {
		fIsParallel = true;
		compName = fComponentName;
		fWorldBiasingName = fComponentName;
	} else {
		compName = "World";
		fWorldBiasingName = "World";
	}
	
	G4IStore* iStore = G4IStore::GetInstance(compName);
	
	for ( int i = 0; i < fParticleNameLength; i++ ) {
		G4GeometrySampler* pgs = new G4GeometrySampler(compName, fParticleName[i]);
		if (fIsParallel) {
			pgs->SetParallel(true);
			pgs->SetWorld(iStore->GetParallelWorldVolumePointer());
		}
		fGeometrySampler.push_back(pgs);
	}
}


void TsImportanceSampling::AddBiasingProcess() {
	for ( size_t u = 0; u < fGeometrySampler.size(); u++ ) {
		if ( !G4Threading::IsWorkerThread() ) {
			fGeometrySampler[u]->PrepareImportanceSampling(G4IStore::GetInstance(fWorldBiasingName),0);
			fGeometrySampler[u]->Configure();
		}
		fGeometrySampler[u]->AddProcess();
	}
}


void TsImportanceSampling::Clear() {
	return;
}
