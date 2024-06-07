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

#include "TsRangeRejection.hh"
#include "TsGeometryManager.hh"
#include "TsParameterManager.hh"

#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4VSolid.hh"
#include "G4EnergyLossTables.hh"
#include "G4NavigationHistory.hh"
#include "G4ClassificationOfNewTrack.hh"

#include "globals.hh"
#include <vector>

TsRangeRejection::TsRangeRejection(G4String name, TsParameterManager* pM, TsGeometryManager* gM)
	: TsVBiasingProcess(name, pM, gM), fPm(pM), fName(name),
	  fInvertFilter(false), fNumberOfRegions(0), fParticleName(nullptr)
{
}


TsRangeRejection::~TsRangeRejection()
{
}

void TsRangeRejection::ResolveParameters() {
	G4int sizeOfParticleName;
			
	if (fPm->ParameterExists(GetFullParmName("ParticlesNamed")) &&
		fPm->ParameterExists(GetFullParmName("ParticlesNotNamed"))) {
		G4cerr << "Topas is exiting due to error in variance reduction setup." << G4endl;
		G4cerr << "Parameter:" << G4endl;
		G4cerr << GetFullParmName("ParticlesNamed") << G4endl;
		G4cerr << "is incompatible with parameter:" << G4endl;
		G4cerr << GetFullParmName("ParticlesNotNamed") << G4endl;
		fPm->AbortSession(1);
	}
	
	fInvertFilter = false;
	if (fPm->ParameterExists(GetFullParmName("ParticlesNamed"))) {
		fParticleName = fPm->GetStringVector(GetFullParmName("ParticlesNamed"));
		sizeOfParticleName = fPm->GetVectorLength(GetFullParmName("ParticlesNamed"));
	} else {
		fParticleName = fPm->GetStringVector(GetFullParmName("ParticlesNotNamed"));
		sizeOfParticleName = fPm->GetVectorLength(GetFullParmName("ParticlesNotNamed"));
		fInvertFilter = true;
	}
	
	fParticleDefs.clear();
	fIsGenericIon.clear();
	fAtomicNumbers.clear();
	fAtomicMasses.clear();
	fCharges.clear();
			
	for ( G4int i = 0; i < sizeOfParticleName; i++ ) {
		TsParticleDefinition resolvedDef = fPm->GetParticleDefinition(fParticleName[i]);
		if (!resolvedDef.particleDefinition) {
			G4cerr << "Topas is exiting due to error in variance reduction setup." << G4endl;
			if (!fInvertFilter)
				G4cerr << GetFullParmName("ParticlesNamed") + " has unknown particle name: "
				<< fParticleName[i] << G4endl;
			else
				G4cerr << GetFullParmName("ParticlesNotNamed") + " has unknown particle name: "
				<< fParticleName[i] << G4endl;
			
			fPm->AbortSession(1);
		}
				
		fParticleDefs.push_back(resolvedDef.particleDefinition);
		fIsGenericIon.push_back(resolvedDef.isGenericIon);
		fAtomicNumbers.push_back(resolvedDef.ionZ);
		fAtomicMasses.push_back(resolvedDef.ionA);
		fCharges.push_back(resolvedDef.ionCharge);
	}
	
	if ( !fPm->ParameterExists(GetFullParmName("Regions"))) {
		G4cerr << "Topas is exiting due to error in variance reduction setup." << G4endl;
		G4cerr << GetFullParmName("Regions") + " has not been defined. " << G4endl;
		fPm->AbortSession(1);
	}
	
	G4String* regionsNamed = fPm->GetStringVector(GetFullParmName("Regions"));
	fNumberOfRegions = fPm->GetVectorLength(GetFullParmName("Regions"));
	fRegions.clear();

	for ( int i = 0; i < fNumberOfRegions; i++ ) {
		G4String aRegionName = regionsNamed[i];
		G4String lowerCaseName = aRegionName;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(lowerCaseName);
#else
		lowerCaseName.toLower();
#endif
		if ( lowerCaseName == "defaultregionfortheworld" ) {
			fRegions.push_back( G4RegionStore::GetInstance()->FindOrCreateRegion("DefaultRegionForTheWorld") );
		} else if ( G4RegionStore::GetInstance()->GetRegion(lowerCaseName) == nullptr ) {
			G4cerr << "Topas is exiting due to error in variance reduction setup." << G4endl;
			G4cerr << GetFullParmName("Regions") + ". Region " << aRegionName << " does not exists." << G4endl;
			fPm->AbortSession(1);
		} else {
			fRegions.push_back( G4RegionStore::GetInstance()->FindOrCreateRegion(lowerCaseName) );
		}
	}
}


void TsRangeRejection::Initialize() {
	Clear();
	ResolveParameters();
}


G4ClassificationOfNewTrack TsRangeRejection::Apply(const G4Track* aTrack) {
	if (!ApplyToThisTrack(aTrack))
		return fUrgent;
	
	G4VPhysicalVolume* physicalVolume = aTrack->GetVolume();

	if (!physicalVolume )
		return fUrgent;
	
	G4LogicalVolume* logicalVolume = physicalVolume->GetLogicalVolume();
	
	if (!ApplyToThisRegion(logicalVolume->GetRegion()))
		return fUrgent;
	
	G4ParticleDefinition* particleDefinition = aTrack->GetDefinition();
	G4double kineticEnergy = aTrack->GetKineticEnergy();
	
	const G4MaterialCutsCouple* materialCutsCouple = logicalVolume->GetMaterialCutsCouple();
	G4double particleRange = G4EnergyLossTables::GetRange(particleDefinition,
														  kineticEnergy,
														  materialCutsCouple);
	
	G4VSolid* solid = logicalVolume->GetSolid();
	G4ThreeVector position = aTrack->GetPosition();
	G4ThreeVector direction = aTrack->GetMomentumDirection();
	const G4VTouchable* touchable = aTrack->GetTouchable();
	G4ThreeVector localPosition = touchable->GetHistory()->GetTopTransform().TransformPoint(position);
	G4ThreeVector localDirection = touchable->GetHistory()->GetTopTransform().TransformAxis(direction);
	
	G4double distanceToOut = solid->DistanceToOut(localPosition, localDirection);
	
	if ( distanceToOut > particleRange )
		return fKill;
	
	return fUrgent;
}

		
G4bool TsRangeRejection::ApplyToThisTrack(const G4Track* aTrack) {
	G4ParticleDefinition* particleDef = aTrack->GetDefinition();
	G4int charge =  (G4int)(aTrack->GetDynamicParticle()->GetCharge());
		
	for ( size_t i = 0; i < fParticleDefs.size(); i++) {
		if (fIsGenericIon[i]) {
			if (((fAtomicNumbers[i]==particleDef->GetAtomicNumber()) || (fAtomicNumbers[i] == -1 )) &&
				((fAtomicMasses[i] ==particleDef->GetAtomicMass())   || (fAtomicMasses[i]  == -1 )) &&
				((fCharges[i]      ==charge)                         || (fCharges[i]       == 999))) {
				if (fInvertFilter) return false;
				else return true;
			}
		} else {
			if (fParticleDefs[i]==particleDef) {
				if (fInvertFilter) return false;
					else return true;
			}
		}
	}
		
	if (fInvertFilter) return true;
	else return false;
}
		
		
G4bool TsRangeRejection::ApplyToThisRegion(G4Region* region) {
 	for ( int i = 0; i < fNumberOfRegions; i++ )
		if ( fRegions[i] == region )
			return true;
	return false;
}

		
void TsRangeRejection::Clear() {
	fParticleDefs.clear();
	fIsGenericIon.clear();
	fAtomicNumbers.clear();
	fAtomicMasses.clear();
	fCharges.clear();
	fRegions.clear();
	return;
}

