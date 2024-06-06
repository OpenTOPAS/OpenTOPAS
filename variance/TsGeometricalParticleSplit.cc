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

#include "TsGeometricalParticleSplit.hh"
#include "TsGeometryManager.hh"
#include "TsParameterManager.hh"

#include "TsVGeometryComponent.hh"
#include "TsSplitProcess.hh"
#include "TsIStore.hh"
#include "TsGeometrySampler.hh"
#include "TsTopasConfig.hh"

#include "G4UImanager.hh"
#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"

#include "globals.hh"
#include <vector>

TsGeometricalParticleSplit::TsGeometricalParticleSplit(G4String name, TsParameterManager* pM, TsGeometryManager* gM)
: TsVBiasingProcess(name, pM, gM), fGm(gM), fPm(pM), fName(name), fVerbosity(0)
{
	ResolveParameters();
}


TsGeometricalParticleSplit::~TsGeometricalParticleSplit()
{;}


void TsGeometricalParticleSplit::ResolveParameters() {
	fType = fPm->GetStringParameter(GetFullParmName("Type"));
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(fType);
#else
	fType.toLower();
#endif
	
	if ( fType == "geometricalparticlesplit" ) {
		fParticleName = fPm->GetStringVector(GetFullParmName("ParticleName"));
		fParticleNameLength = fPm->GetVectorLength(GetFullParmName("ParticleName"));
		
		fComponentName = fPm->GetStringParameter(GetFullParmName("Component"));
		fSubComponentNames = fPm->GetStringVector(GetFullParmName("SubComponents"));
		fSubCompSize = fPm->GetVectorLength(GetFullParmName("SubComponents"));
		
		G4String splitAxis = fPm->GetStringParameter(GetFullParmName("SplitAxis"));
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(splitAxis);
#else
		splitAxis.toLower();
#endif
		
		G4int splSize = fPm->GetVectorLength(GetFullParmName("SplitNumber"));
		G4int symSize = fPm->GetVectorLength(GetFullParmName("Symmetric"));
		G4int rrlSize = fPm->GetVectorLength(GetFullParmName("RussianRoulette"));
		
		G4String message;
		if ( fSubCompSize != splSize ) {
			message = "Vector must matches with " + GetFullParmName("SubComponents");
			Quit(GetFullParmName("SplitNumber"), message);
		}
		
		if ( fSubCompSize != symSize ) {
			message = "Vector must matches with " + GetFullParmName("SubComponents");
			Quit(GetFullParmName("Symmetry"), message);
		}
		
		if ( fSubCompSize != rrlSize ) {
			message = "The number of elements must matches with " +	GetFullParmName("SubComponents");
			Quit(GetFullParmName("RussianRoulette"), message);
		}
		
		if ( splitAxis != "zaxis" && splitAxis != "yaxis" && splitAxis != "xaxis" )
			Quit(GetFullParmName("SplitAxis"), "Refers to an unknown split axis");
		
		fVerbosity = fPm->GetIntegerParameter("Ts/SequenceVerbosity");
	}
}


void TsGeometricalParticleSplit::Initialize() {
	SetGeometry();
	SetGeometricalParticleSplit(); 
}


void TsGeometricalParticleSplit::SetGeometry()
{
	if (!fGm->GetComponent(fComponentName)->IsParallel())
		Quit(GetFullParmName("Component"), "Geometry component for variance reduction is not a parallel component");
	
	G4VPhysicalVolume* physWorld = fGm->GetComponent(fComponentName)->GetParentVolume();
	G4VPhysicalVolume* physComponent = fGm->GetPhysicalVolume(fComponentName);
	
	fPhysVol.push_back(physWorld);
	fPhysVol.push_back(physComponent);
	
	for ( int i = 0; i < fSubCompSize; i++ )
		fPhysVol.push_back(fGm->GetPhysicalVolume(fSubComponentNames[i]));
}


void TsGeometricalParticleSplit::SetGeometricalParticleSplit() {
	G4int* splitValues = fPm->GetIntegerVector(GetFullParmName("SplitNumber"));
	G4bool* symmetryValues = fPm->GetBooleanVector(GetFullParmName("Symmetric"));
	G4bool* russianValues = fPm->GetBooleanVector(GetFullParmName("RussianRoulette"));
	
	std::vector<G4int> symmetryToInteger;
	std::vector<G4int> russianToInteger;
	
	for ( int i = 0; i < fSubCompSize; i++ ) {
		if ( symmetryValues[i] )
			symmetryToInteger.push_back(1);
		else
			symmetryToInteger.push_back(0);
		
		if ( russianValues[i] )
			russianToInteger.push_back(1);
		else
			russianToInteger.push_back(0);
	}
	
	G4GeometryCell world(*(fPhysVol[0]), 0);
	G4GeometryCell component(*(fPhysVol[1]), 0);
	
	TsIStore* aIstore = TsIStore::GetInstance(fComponentName);
	aIstore->AddPropertyValueToGeometryCell(1, world, -1);
	aIstore->AddPropertyValueToGeometryCell(0, world, 0);
	aIstore->AddPropertyValueToGeometryCell(0, world, 1);
	aIstore->AddPropertyValueToGeometryCell(1, component, -1);
	aIstore->AddPropertyValueToGeometryCell(0, component, 0);
	aIstore->AddPropertyValueToGeometryCell(0, component, 1);
	
	if ( fVerbosity > 1 ) {
		G4cout << " Adding cell: " << 1
		<< ".\t Name: " << component.GetPhysicalVolume().GetName()
		<< ".\t Number of split: " << 1
		<< ".\t Symmetric: " << 0
		<< ".\t Russian roulette: " << 1 << G4endl;
	}
	
	G4int nbOfSplit = 1;
	for ( int j = 0; j < fSubCompSize; j++ ) {
		G4GeometryCell aSubComponent(*(fPhysVol[ j + 2 ]), 0);
		nbOfSplit *= splitValues[j];
		if ( fVerbosity ) {
			G4cout << " Adding cell: " << j + 2
			<< ".\t Name: " << aSubComponent.GetPhysicalVolume().GetName()
			<< ".\t Number of split: " << splitValues[j]
			<< ".\t Symmetric: " << symmetryToInteger[j]
			<< ".\t Russian roulette: " << russianToInteger[j] << G4endl;
		}
		
		aIstore->AddPropertyValueToGeometryCell(nbOfSplit, aSubComponent, -1);
		aIstore->AddPropertyValueToGeometryCell(symmetryToInteger[j], aSubComponent, 0);
		aIstore->AddPropertyValueToGeometryCell(russianToInteger[j], aSubComponent, 1);
	}
	
	return;
}


std::vector<TsGeometrySampler*> TsGeometricalParticleSplit::GetGeometrySampler() {
	TsIStore* iStore = TsIStore::GetInstance(fComponentName);
	std::vector<TsGeometrySampler*> tgs;
	for ( int i = 0; i < fParticleNameLength; i++ ) {
		TsGeometrySampler* pgs = new TsGeometrySampler(fPm, fGm, fComponentName, fParticleName[i]);
		pgs->SetParallel(true);
		pgs->SetWorld(iStore->GetParallelWorldVolumePointer());
		tgs.push_back(pgs);
	}
	return tgs;
}


void TsGeometricalParticleSplit::SetBiasingProcess(std::vector<TsGeometrySampler*> tgs) {
	for ( size_t u = 0; u < tgs.size(); u++ ) {
		if ( !G4Threading::IsWorkerThread() ) {
			tgs[u]->PrepareImportanceSampling(TsIStore::GetInstance(fComponentName),0);
			tgs[u]->Configure();
		}
		tgs[u]->AddProcess();
	}
}
	
	
void TsGeometricalParticleSplit::AddBiasingProcess() {
	return;
}


void TsGeometricalParticleSplit::Clear() {
	for ( size_t t = 0; t < fGeometrySampler.size(); t++ )
		fGeometrySampler[t]->ClearSampling();
	return;
}
