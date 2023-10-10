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

#ifndef TsPeriodicBoundaryConditionOperator_hh
#define TsPeriodicBoundaryConditionOperator_hh 1

#include "G4VBiasingOperator.hh"
#include "G4LogicalVolume.hh"

class TsParameterManager;
class G4VBiasingOperation;

class TsPeriodicBoundaryConditionOperator : public G4VBiasingOperator
{
public:
	TsPeriodicBoundaryConditionOperator(TsParameterManager* pM, G4String name);
	virtual ~TsPeriodicBoundaryConditionOperator();
	
	void      StartRun();
	G4bool IsApplicable(G4LogicalVolume*);
	
private:
	virtual G4VBiasingOperation*
	ProposeNonPhysicsBiasingOperation(const G4Track*,
									  const G4BiasingProcessInterface*);
	
	virtual G4VBiasingOperation*
	ProposeOccurenceBiasingOperation (const G4Track*,
									  const G4BiasingProcessInterface*)
	{ return 0; }
	
	virtual G4VBiasingOperation*
	ProposeFinalStateBiasingOperation(const G4Track*,
									  const G4BiasingProcessInterface*)
	{ return 0; }
	
private:
	TsParameterManager*           fPm;
	G4VBiasingOperation* fKillingOperation;
	
	G4String* fNamesOfRegions;
	G4int     fNumberOfRegions;
	G4String* fAcceptedLogicalVolumeNames;
	G4int     fNbOfAcceptedLogicalVolumeNames;
};
#endif
