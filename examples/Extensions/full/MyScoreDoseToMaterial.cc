// Scorer for DoseToMaterial
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

#include "MyScoreDoseToMaterial.hh"
#include "TsParameterManager.hh"

#include "G4Material.hh"
#include "G4ParticleDefinition.hh"
#include "G4Proton.hh"

MyScoreDoseToMaterial::MyScoreDoseToMaterial(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
											 G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer),
fEmCalculator()
{
	SetUnit("Gy");

	if (fPm->ParameterExists(GetFullParmName("Material"))) {
		G4String materialName = fPm->GetStringParameter(GetFullParmName("Material"));
		fReferenceMaterial = GetMaterial(materialName);

		if (!fReferenceMaterial) {
			G4cout << "Topas is exiting due to error in scoring setup." << G4endl;
			G4cout << "Unknown material, " << materialName << ", specified in: " << GetFullParmName("Material") << G4endl;
			exit(1);
		}
	} else {
		G4cout << "Topas is exiting due to error in scoring setup." << G4endl;
		G4cout << "Scorer " << GetName() << " has quantity DoseToMaterial" << G4endl;
		G4cout << "But you have not specified what material." << G4endl;
		G4cout << "Need a parameter: " << GetFullParmName("Material") << G4endl;
		exit(1);
	}

	fProtonSubstituteForNeutrals = G4Proton::ProtonDefinition();

	if (fPm->ParameterExists(GetFullParmName("SubstituteEnergyForNeutralScaling")))
		fSubstituteForNeutralsEnergy =  fPm->GetDoubleParameter(GetFullParmName("SubstituteEnergyForNeutralScaling"), "Energy");
	else
		fSubstituteForNeutralsEnergy =  100*MeV;
}


MyScoreDoseToMaterial::~MyScoreDoseToMaterial() {;}


G4bool MyScoreDoseToMaterial::ProcessHits(G4Step* aStep,G4TouchableHistory*)
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
			// convert to Dose to Material for charged particles from EM tables:
			G4double energy = aStep->GetPreStepPoint()->GetKineticEnergy();
			G4double materialStoppingPower = fEmCalculator.ComputeTotalDEDX(energy, particle,
																			aStep->GetPreStepPoint()->GetMaterial());
			G4double referenceMaterialStoppingPower = fEmCalculator.ComputeTotalDEDX(energy, particle, fReferenceMaterial);
			dose *= ( density / fReferenceMaterial->GetDensity() ) *  ( referenceMaterialStoppingPower / materialStoppingPower );
		} else {
			// convert to Dose to Material for neutral particles from EM tables, assuming scaling factor from protons (dE/dx not defined for neutrals):
			G4double materialStoppingPower = fEmCalculator.ComputeTotalDEDX(fSubstituteForNeutralsEnergy, fProtonSubstituteForNeutrals,
																			aStep->GetPreStepPoint()->GetMaterial());
			G4double referenceMaterialStoppingPower = fEmCalculator.ComputeTotalDEDX(fSubstituteForNeutralsEnergy, fProtonSubstituteForNeutrals, fReferenceMaterial);
			dose *= ( density / fReferenceMaterial->GetDensity() ) *  ( referenceMaterialStoppingPower / materialStoppingPower );
		}
		AccumulateHit(aStep, dose);
		return true;
	}
	return false;
}

