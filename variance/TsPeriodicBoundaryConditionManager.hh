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

#ifndef TsPeriodicBoundaryConditionManager_hh
#define TsPeriodicBoundaryConditionManager_hh

#include "TsVBiasingProcess.hh"

#include <vector>

class TsParameterManager;
class TsGeometryManager;
class G4GenericBiasingPhysics;

class TsPeriodicBoundaryConditionManager : public TsVBiasingProcess
{
public:
	TsPeriodicBoundaryConditionManager(G4String name, TsParameterManager*, TsGeometryManager*);
	~TsPeriodicBoundaryConditionManager();

	void ResolveParameters();
	void Initialize();
	void Clear();
	void SetGenericBiasing(); 
	void AddBiasingProcess() { return; };
	
	G4GenericBiasingPhysics* GetGenericBiasingPhysics();
	G4String GetTypeName() { return fType;};
	
private:
	TsParameterManager* fPm;

	G4String fName;
	G4String fType;
		
	G4int fNbOfParticles;
	G4String* fParticlesToBias;
};
#endif
