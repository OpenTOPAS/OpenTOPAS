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

#include "TsAutomaticImportanceSamplingProcess.hh"
#include "TsParameterManager.hh"

#include "Randomize.hh"

TsAutomaticImportanceSamplingProcess::TsAutomaticImportanceSamplingProcess(TsParameterManager*, G4String name)
: G4VBiasingOperation(name), fParticleChange(), fParticleChangeForNothing(), fName(name)
{}


TsAutomaticImportanceSamplingProcess::TsAutomaticImportanceSamplingProcess(G4String name)
: G4VBiasingOperation(name), fParticleChange(), fParticleChangeForNothing(), fName(name)
{}


TsAutomaticImportanceSamplingProcess::~TsAutomaticImportanceSamplingProcess()
{}


G4double TsAutomaticImportanceSamplingProcess::DistanceToApplyOperation(const G4Track*,
														 G4double,
														 G4ForceCondition* condition)
{
	*condition = Forced;
	return DBL_MAX;
}


G4VParticleChange* TsAutomaticImportanceSamplingProcess::
GenerateBiasingFinalState( const G4Track* aTrack, const G4Step* aStep)
{

	G4StepPoint* postStep = aStep->GetPostStepPoint();
	
	if ( (postStep->GetStepStatus() == fGeomBoundary ) && ( aTrack->GetCurrentStepNumber() != 1) ) {
		G4StepPoint* preStep = aStep->GetPreStepPoint();
		const G4VTouchable* preTouchable = preStep->GetTouchable();
		G4int iX = preTouchable->GetReplicaNumber(2);
		G4int iY = preTouchable->GetReplicaNumber(1);
		G4int iZ = preTouchable->GetReplicaNumber(0);
		G4int index = iX * fDivisionCounts[1] * fDivisionCounts[2] + iY * fDivisionCounts[2] + iZ;
		G4double preImp = fImportanceValues[index];
		
		const G4VTouchable* postTouchable = postStep->GetTouchable();
		iX = postTouchable->GetReplicaNumber(2);
		iY = postTouchable->GetReplicaNumber(1);
		iZ = postTouchable->GetReplicaNumber(0);
		index = iX * fDivisionCounts[1] * fDivisionCounts[2] + iY * fDivisionCounts[2] + iZ;
		G4double postImp = fImportanceValues[index];
				
		G4double weight = aTrack->GetWeight();
		fParticleChange.Initialize(*aTrack);
		
		if ( postImp > preImp ) {
			G4int numberOfSplit = G4int(postImp/preImp);
			G4double newWeight = weight/numberOfSplit;
			fParticleChange.ProposeParentWeight(newWeight);
			fParticleChange.SetNumberOfSecondaries( numberOfSplit-1 );

			for ( int i = 1; i < numberOfSplit; i++ ) {
				G4Track* newTrack = new G4Track(*aTrack);
				newTrack->SetWeight(newWeight);
				fParticleChange.AddSecondary(newTrack);
			}
			
			fParticleChange.SetSecondaryWeightByProcess(true);
			return &fParticleChange;
			
		} else {
			G4double survivingProbability = postImp/preImp;
			G4double rnd = G4UniformRand();
			if ( rnd > survivingProbability ) {
				fParticleChange.ProposeTrackStatus(fStopAndKill);
			} else {
				fParticleChange.ProposeParentWeight(weight/survivingProbability);
			}
			return &fParticleChange;
		}
	}
	
	fParticleChangeForNothing.Initialize(*aTrack);
	return &fParticleChangeForNothing;
}


void TsAutomaticImportanceSamplingProcess::SetImportanceValues(std::map<G4int, G4double> impValues) {
	fImportanceValues = impValues;
	G4cout << "[VRT] AutomaticImportanceSampling: setting importance map of "
	<< fImportanceValues.size() << " elements " << G4endl;
}


void TsAutomaticImportanceSamplingProcess::SetDivisionCounts(std::vector<G4int> division) {
	fDivisionCounts = division;
	G4cout << "[VRT] AutomaticImportanceSampling: divisions "
	<< "(" << fDivisionCounts[0] << ", " << fDivisionCounts[1] << ", " << fDivisionCounts[2] << ")" << G4endl;
}
