//
// ********************************************************************
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * TOPAS collaboration.                                             *
// * Use or redistribution of this code is not permitted without the  *
// * explicit approval of the TOPAS collaboration.                    *
// * Contact: Joseph Perl, perl@slac.stanford.edu                     *
// *                                                                  *
// ********************************************************************
//

#include "TsPeriodicBoundaryConditionManager.hh"
#include "TsGeometryManager.hh"
#include "TsParameterManager.hh"

#include "G4UImanager.hh"
#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"
#include "G4GenericBiasingPhysics.hh"

#include "globals.hh"
#include <vector>

TsPeriodicBoundaryConditionManager::TsPeriodicBoundaryConditionManager(G4String name, TsParameterManager* pM, TsGeometryManager* gM)
: TsVBiasingProcess(name, pM, gM), fPm(pM), fName(name)
{
	ResolveParameters();
}


TsPeriodicBoundaryConditionManager::~TsPeriodicBoundaryConditionManager()
{;}


void TsPeriodicBoundaryConditionManager::ResolveParameters() {
    fParticlesToBias = fPm->GetStringVector(GetFullParmName("ParticlesNamed"));
    fNbOfParticles = fPm->GetVectorLength(GetFullParmName("ParticlesNamed"));
    fType = fPm->GetStringParameter(GetFullParmName("Type"));
	
}


void TsPeriodicBoundaryConditionManager::Initialize() {
	SetGenericBiasing();
}


void TsPeriodicBoundaryConditionManager::SetGenericBiasing() {
	return;
}


G4GenericBiasingPhysics* TsPeriodicBoundaryConditionManager::GetGenericBiasingPhysics() {
	G4GenericBiasingPhysics* biasingPhysics = new G4GenericBiasingPhysics(fName);
	for ( int i = 0; i < fNbOfParticles; i++ )
		biasingPhysics->NonPhysicsBias(fParticlesToBias[i]);	

    return biasingPhysics;
}


void TsPeriodicBoundaryConditionManager::Clear() {
	return;
}
