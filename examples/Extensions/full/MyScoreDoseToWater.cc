// Scorer for DoseToWater
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

#include "MyScoreDoseToWater.hh"
#include "TsParameterManager.hh"

#include "G4Material.hh"
#include "G4ParticleDefinition.hh"
#include "G4Proton.hh"

MyScoreDoseToWater::MyScoreDoseToWater(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
									   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer),
fEmCalculator()
{
	SetUnit("Gy");

	fWater = GetMaterial("G4_WATER");

	fProtonSubstituteForNeutrals = G4Proton::ProtonDefinition();

	if (fPm->ParameterExists(GetFullParmName("SubstituteEnergyForNeutralScaling")))
		fSubstituteForNeutralsEnergy =  fPm->GetDoubleParameter(GetFullParmName("SubstituteEnergyForNeutralScaling"), "Energy");
	else
		fSubstituteForNeutralsEnergy =  100*MeV;
}


MyScoreDoseToWater::~MyScoreDoseToWater() {;}


G4bool MyScoreDoseToWater::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	G4double edep = aStep->GetTotalEnergyDeposit();
	if ( edep > 0. ) {
		G4double density = aStep->GetPreStepPoint()->GetMaterial()->GetDensity();

		ResolveSolid(aStep);

		G4double dose = edep / ( density * fSolid->GetCubicVolume() );
		dose *= aStep->GetPreStepPoint()->GetWeight();

		G4ParticleDefinition* particle = aStep->GetTrack()->GetDefinition();
		if ( particle->GetPDGCharge() != 0 ) {
			// convert to Dose to Water for charged particles from EM tables:
			G4double energy = aStep->GetPreStepPoint()->GetKineticEnergy();
			G4double materialStoppingPower = fEmCalculator.ComputeTotalDEDX(energy, particle,
																			aStep->GetPreStepPoint()->GetMaterial());
			G4double waterStoppingPower = fEmCalculator.ComputeTotalDEDX(energy, particle, fWater);
			dose *= ( density / fWater->GetDensity() ) *  ( waterStoppingPower / materialStoppingPower );
		} else {
			// convert to Dose to Water for neutral particles from EM tables, assuming scaling factor from protons (dE/dx not defined for neutrals):
			G4double materialStoppingPower = fEmCalculator.ComputeTotalDEDX(fSubstituteForNeutralsEnergy, fProtonSubstituteForNeutrals,
																			aStep->GetPreStepPoint()->GetMaterial());
			G4double waterStoppingPower = fEmCalculator.ComputeTotalDEDX(fSubstituteForNeutralsEnergy, fProtonSubstituteForNeutrals, fWater);
			dose *= ( density / fWater->GetDensity() ) *  ( waterStoppingPower / materialStoppingPower );
		}
		AccumulateHit(aStep, dose);
		return true;
	}
	return false;
}

