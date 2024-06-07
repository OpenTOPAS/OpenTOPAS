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

#include "TsAutomaticImportanceSamplingParallelProcess.hh"
#include "TsParameterManager.hh"

#include "G4BiasingProcessInterface.hh"
#include "G4ParallelGeometriesLimiterProcess.hh"
#include "G4BiasingProcessSharedData.hh"
#include "G4ProcessManager.hh"
#include "Randomize.hh"

TsAutomaticImportanceSamplingParallelProcess::TsAutomaticImportanceSamplingParallelProcess(TsParameterManager*, G4String name)
	: G4VBiasingOperation(name), fName(name), fParallelWorldIndex(-1), fBiasingSharedData(nullptr)
{
}

TsAutomaticImportanceSamplingParallelProcess::TsAutomaticImportanceSamplingParallelProcess(G4String name)
	: G4VBiasingOperation(name), fName(name), fParallelWorldIndex(0), fBiasingSharedData(nullptr)
{
}


TsAutomaticImportanceSamplingParallelProcess::~TsAutomaticImportanceSamplingParallelProcess()
{}


G4double TsAutomaticImportanceSamplingParallelProcess::DistanceToApplyOperation(const G4Track*,
														 G4double,
														 G4ForceCondition* condition)
{
	auto biasingLimiterProcess = fBiasingSharedData->GetParallelGeometriesLimiterProcess();
	fPreStepTouchableHistory = biasingLimiterProcess->GetNavigator(fParallelWorldIndex)->CreateTouchableHistoryHandle();
	
	*condition = Forced;
	return DBL_MAX;
}


G4VParticleChange* TsAutomaticImportanceSamplingParallelProcess::
GenerateBiasingFinalState( const G4Track* aTrack, const G4Step* )
{
	G4bool isLimiting = fBiasingSharedData->GetParallelGeometriesLimiterProcess()->GetIsLimiting(fParallelWorldIndex);
	if ( !isLimiting ) {
		fParticleChangeForNothing.Initialize(*aTrack);
		return &fParticleChangeForNothing;
	}
	
	auto biasingLimiterProcess = fBiasingSharedData->GetParallelGeometriesLimiterProcess();
	fPostStepTouchableHistory  = biasingLimiterProcess->GetNavigator( fParallelWorldIndex )->CreateTouchableHistoryHandle();
	
 	G4int iX = fPreStepTouchableHistory->GetReplicaNumber(2);
	G4int iY = fPreStepTouchableHistory->GetReplicaNumber(1);
	G4int iZ = fPreStepTouchableHistory->GetReplicaNumber(0);
	G4int index1 = iX * fDivisionCounts[1] * fDivisionCounts[2] + iY * fDivisionCounts[2] + iZ;
		
	iX = fPostStepTouchableHistory->GetReplicaNumber(2);
	iY = fPostStepTouchableHistory->GetReplicaNumber(1);
	iZ = fPostStepTouchableHistory->GetReplicaNumber(0);
	G4int index2 = iX * fDivisionCounts[1] * fDivisionCounts[2] + iY * fDivisionCounts[2] + iZ;
	
	if ( index1 == index2 ) {
		fParticleChangeForNothing.Initialize(*aTrack);
		return &fParticleChangeForNothing;
	}
	
	G4double preImp = fImportanceValues[index1];
	G4double postImp = fImportanceValues[index2];
	G4double weight = aTrack->GetWeight();
	
	if ( postImp > preImp ) {
		G4int numberOfSplit = G4int(postImp/preImp);
		weight /= numberOfSplit;

		fParticleChange.Initialize(*aTrack);
		fParticleChange.ProposeParentWeight(weight);
		fParticleChange.SetNumberOfSecondaries(numberOfSplit-1);
		
		for ( int i = 1; i < numberOfSplit; i++ ) {
			G4Track* newTrack = new G4Track(*aTrack);
			newTrack->SetWeight(weight);
			fParticleChange.AddSecondary(newTrack);
		}
			
		fParticleChange.SetSecondaryWeightByProcess(true);
		return &fParticleChange;
		
	} else {
		fParticleChange.Initialize(*aTrack);
		G4double survivingProbability = postImp/preImp;
		G4double rnd = G4UniformRand();
		if ( rnd > survivingProbability ) {
			fParticleChange.ProposeTrackStatus(fStopAndKill);
		} else {
			weight /= survivingProbability;
			fParticleChange.ProposeParentWeight(weight);
			fParticleChange.SetSecondaryWeightByProcess(true);
		}
		return &fParticleChange;
	}
}


void TsAutomaticImportanceSamplingParallelProcess::SetParallelWorldIndex(G4int parallelWorldIndex ) {
	fParallelWorldIndex = parallelWorldIndex;
}


G4int TsAutomaticImportanceSamplingParallelProcess::GetParallelWorldIndex() const {
	return fParallelWorldIndex;
}


void TsAutomaticImportanceSamplingParallelProcess::SetBiasingSharedData(const G4BiasingProcessSharedData* sharedData ) {
	fBiasingSharedData = sharedData;
}


void TsAutomaticImportanceSamplingParallelProcess::SetImportanceValues(std::map<G4int, G4double> impValues) {
	fImportanceValues = impValues;
	G4cout << "[VRT] AutomaticImportanceSampling: setting importance map of "
	<< fImportanceValues.size() << " elements " << G4endl;
}


void TsAutomaticImportanceSamplingParallelProcess::SetDivisionCounts(std::vector<G4int> division) {
	fDivisionCounts = division;
	G4cout << "[VRT] AutomaticImportanceSampling: divisions "
	<< "(" << fDivisionCounts[0] << ", " << fDivisionCounts[1] << ", " << fDivisionCounts[2] << ")" << G4endl;
}
