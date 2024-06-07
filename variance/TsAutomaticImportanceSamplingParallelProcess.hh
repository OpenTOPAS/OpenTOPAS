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

#ifndef TsAutomaticImportanceSamplingParallelProcess_hh
#define TsAutomaticImportanceSamplingParallelProcess_hh 1

#include "G4VBiasingOperation.hh"
#include "G4ParticleChange.hh"
#include "G4ParticleChangeForNothing.hh"
#include "G4TouchableHistoryHandle.hh"

#include <vector>

class TsParameterManager;
class G4BiasingProcessSharedData;

class TsAutomaticImportanceSamplingParallelProcess : public G4VBiasingOperation
{	
public:
	TsAutomaticImportanceSamplingParallelProcess(TsParameterManager* pM, G4String name);
	TsAutomaticImportanceSamplingParallelProcess(G4String name);
	virtual ~TsAutomaticImportanceSamplingParallelProcess();
	
public:
	virtual const G4VBiasingInteractionLaw* ProvideOccurenceBiasingInteractionLaw(const G4BiasingProcessInterface*,
																				  G4ForceCondition& ) final { return 0; };
	
	
	virtual G4VParticleChange*   ApplyFinalStateBiasing( const G4BiasingProcessInterface*,
														const G4Track*,
														const G4Step*,
														G4bool& ) final {return 0;};
	
	virtual G4double           DistanceToApplyOperation( const G4Track*,
														G4double,
														G4ForceCondition*) final;
	
	
	virtual G4VParticleChange* GenerateBiasingFinalState( const G4Track*,
														 const G4Step*   ) final;
	
	void  SetParallelWorldIndex( G4int parallelWorldIndex );
	G4int GetParallelWorldIndex() const;
	void SetBiasingSharedData( const G4BiasingProcessSharedData* sharedData );
	
	void SetImportanceValues(std::map<G4int, G4double> );
	void SetDivisionCounts(std::vector<G4int> );
	
private:
	G4ParticleChange fParticleChange;
	G4ParticleChangeForNothing fParticleChangeForNothing;
	G4String fName;
	G4int                                   fParallelWorldIndex;
	const G4BiasingProcessSharedData*        fBiasingSharedData;
	G4TouchableHistoryHandle           fPreStepTouchableHistory;
	G4TouchableHistoryHandle          fPostStepTouchableHistory;
	std::map<G4int, G4double> fImportanceValues;
	std::vector<G4int> fDivisionCounts;
};
#endif
