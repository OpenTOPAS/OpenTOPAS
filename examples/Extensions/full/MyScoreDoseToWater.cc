// Scorer for DoseToWater
//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * This file was obtained from Topas MC Inc under the license       *
// * agreement set forth at http://www.topasmc.org/registration       *
// * Any use of this file constitutes full acceptance of              *
// * this TOPAS MC license agreement.                                 *
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

