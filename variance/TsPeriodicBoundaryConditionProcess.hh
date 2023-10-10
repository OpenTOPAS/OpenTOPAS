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

#ifndef TsPeriodicBoundaryConditionProcess_hh
#define TsPeriodicBoundaryConditionProcess_hh 1

#include "G4VBiasingOperation.hh"
#include "G4ParticleChange.hh"
#include "G4ParticleChangeForNothing.hh"

#include <vector>

class TsParameterManager;
class G4Region;

class TsPeriodicBoundaryConditionProcess : public G4VBiasingOperation
{	
public:
	TsPeriodicBoundaryConditionProcess(TsParameterManager* pM, G4String name);
	TsPeriodicBoundaryConditionProcess(G4String name);
	virtual ~TsPeriodicBoundaryConditionProcess();
	
public:
	virtual const G4VBiasingInteractionLaw* ProvideOccurenceBiasingInteractionLaw(const G4BiasingProcessInterface*,
																				  G4ForceCondition& ) { return 0; };
	
	
	virtual G4VParticleChange*   ApplyFinalStateBiasing( const G4BiasingProcessInterface*,
														const G4Track*,
														const G4Step*,
														G4bool& ) {return 0;};
	
	virtual G4double           DistanceToApplyOperation( const G4Track*,
														G4double,
														G4ForceCondition*);
	
	
	virtual G4VParticleChange* GenerateBiasingFinalState( const G4Track*,
														 const G4Step*   );
	
	
	void SetRegions(G4String*);
	void SetNumberOfRegions(G4int);
	
private:
	G4ParticleChange fParticleChange;
	G4ParticleChangeForNothing fParticleChangeForNothing;
	G4String fName;
	G4String* fNamesOfRegions;
	G4int fNumberOfRegions;
	std::vector<G4Region*> fRegions;
	G4double kCarTolerance;
	G4int fMinParentID;
};
#endif
