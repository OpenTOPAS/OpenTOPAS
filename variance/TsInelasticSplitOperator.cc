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

#include "TsParameterManager.hh"

#include "TsInelasticSplitOperator.hh"
#include "TsInelasticSplitProcess.hh"

#include "G4BiasingProcessInterface.hh"
#include "G4GenericMessenger.hh"

TsInelasticSplitOperator::TsInelasticSplitOperator(TsParameterManager* pM, G4String name)
	: G4VBiasingOperator(name), fPm(pM),
	  fSplittingFactor(1), fBiasPrimaryOnly(true), fBiasOnlyOnce(true), fNInteractions(0)
{
 	fSplittingOperation = new TsInelasticSplitProcess(fPm, name);
	fSplittingFactor = fPm->GetIntegerParameter("Vr/" + name + "/SplitNumber");
	fBiasPrimaryOnly = fPm->GetBooleanParameter("Vr/" + name + "/OnlyBiasPrimaryParticle");
	fBiasOnlyOnce = fPm->GetBooleanParameter("Vr/" + name + "/BiasParticleOnlyOnce");
	fAcceptedLogicalVolumeNames = fPm->GetStringVector("Vr/" + name + "/ApplyBiasingInVolumesNamed");
	fNbOfAcceptedLogicalVolumeNames = fPm->GetVectorLength("Vr/" + name + "/ApplyBiasingInVolumesNamed");
					
	if (fPm->ParameterExists("Vr/" + name + "/ApplyRussianRouletteToParticlesDifferentThan") ) {
		G4String* particleNames = fPm->GetStringVector("Vr/" + name + "/ApplyRussianRouletteToParticlesDifferentThan");
		G4int length = fPm->GetVectorLength("Vr/" + name + "/ApplyRussianRouletteToParticlesDifferentThan");
		for (G4int j = 0; j < length; j++) {
			TsParticleDefinition resolvedDef = fPm->GetParticleDefinition(particleNames[j]);
			if (!resolvedDef.particleDefinition && ! resolvedDef.isGenericIon) {
				G4cerr << "Topas is exiting due to a serious error in variance reduction setup." << G4endl;
				G4cerr << particleNames[j] << " used for Russian Roulette refers to an unknown particle type." << G4endl;
				fPm->AbortSession(1);
			}
			fParticleDefs.push_back(resolvedDef.particleDefinition);
		}
	}
}


TsInelasticSplitOperator::~TsInelasticSplitOperator()
{}


void TsInelasticSplitOperator::StartRun() {
	fSplittingOperation->SetSplittingFactor ( fSplittingFactor );
	G4cout << GetName() << " : starting run with split factor = " << fSplittingFactor;
	fSplittingOperation->SetRussianRouletteForParticles( fParticleDefs );
	
	if ( fBiasPrimaryOnly )
		G4cout << ", biasing only primaries ";
	else
		G4cout << ", biasing primary and secondary tracks ";
	
	if ( fBiasOnlyOnce )
		G4cout << ", biasing only once per track ";
	else
		G4cout << ", biasing several times per track ";
	
	G4cout << " . " << G4endl;
}


void TsInelasticSplitOperator::StartTracking( const G4Track* ) {
	fNInteractions = 0;
}


G4VBiasingOperation* TsInelasticSplitOperator::ProposeFinalStateBiasingOperation(const G4Track* track,
																				 const G4BiasingProcessInterface* ) {
	if ( fBiasPrimaryOnly && ( track->GetParentID() !=0 ) )
		return 0;
	
	if ( fBiasOnlyOnce && ( fNInteractions > 0 ) )
		return 0;
	
	fNInteractions++;
	return fSplittingOperation;
}


G4bool TsInelasticSplitOperator::IsApplicable(G4LogicalVolume* logicalVolume) {
	G4bool applicable = false;
	for ( int i = 0; i < fNbOfAcceptedLogicalVolumeNames; i++ )
		if ( logicalVolume->GetName() == fAcceptedLogicalVolumeNames[i] )
			applicable = true;
	return applicable;
}
