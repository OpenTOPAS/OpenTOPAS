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
