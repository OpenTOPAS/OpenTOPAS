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

#include "TsScoreDoseToMaterial.hh"
#include "TsParameterManager.hh"
#include "TsMaterialManager.hh"
#include "TsGeometryManager.hh"
#include "TsSequenceManager.hh"

#include "G4Material.hh"
#include "G4ParticleDefinition.hh"
#include "G4Proton.hh"
#include "G4Electron.hh"

TsScoreDoseToMaterial::TsScoreDoseToMaterial(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
											 G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer),
fEmCalculator(), fSubstituteForNeutralsEnergy(0),
fProtonStoppingPowerRatio(0), fElectronStoppingPowerRatio(0), fOtherStoppingPowerRatio(0), fMaxProtonEnergyForRatio(0), fMaxElectronEnergyForRatio(0)
{
	SetUnit("Gy");

	fOutputWeightingFactor = 1.0;
	if (fPm->ParameterExists(GetFullParmName("OutputWeightingFactor"))){
		fOutputWeightingFactor = fPm->GetUnitlessParameter(GetFullParmName("OutputWeightingFactor"));
	}

	G4String quantityNameLower = quantity;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(quantityNameLower);
#else
	quantityNameLower.toLower();
#endif
	if (quantityNameLower == "dosetowater") {
		fReferenceMaterial = GetMaterial("G4_WATER");
	} else {
		G4String materialName = fPm->GetStringParameter(GetFullParmName("Material"));
		fReferenceMaterial = GetMaterial(materialName);
		if (!fReferenceMaterial) {
			G4cout << "Topas is exiting due to error in scoring setup." << G4endl;
			G4cout << "Unknown material, " << materialName << ", specified in: " << GetFullParmName("Material") << G4endl;
			fPm->AbortSession(1);
		}
	}

	fProtonSubstituteForNeutrals = G4Proton::ProtonDefinition();

	if (fPm->ParameterExists(GetFullParmName("SubstituteEnergyForNeutralScaling")))
		fSubstituteForNeutralsEnergy = fPm->GetDoubleParameter(GetFullParmName("SubstituteEnergyForNeutralScaling"), "Energy");
	else
		fSubstituteForNeutralsEnergy = 100*MeV;


	fPreCalculateStoppingPowerRatios = fPm->ParameterExists(GetFullParmName("PreCalculateStoppingPowerRatios")) && fPm->GetBooleanParameter(GetFullParmName("PreCalculateStoppingPowerRatios"));
	if (fPreCalculateStoppingPowerRatios) {

		if (fPm->ParameterExists(GetFullParmName("ProtonEnergyBinSize")))
			fProtonEnergyBinSize = fPm->GetDoubleParameter(GetFullParmName("ProtonEnergyBinSize"), "Energy");
		else
			fProtonEnergyBinSize = 1*MeV;

		if (fPm->ParameterExists(GetFullParmName("MinProtonEnergyForStoppingPowerRatio")))
			fMinProtonEnergyForRatio = fPm->GetDoubleParameter(GetFullParmName("MinProtonEnergyForStoppingPowerRatio"), "Energy");
		else
			fMinProtonEnergyForRatio = 1*MeV;

		if (fPm->ParameterExists(GetFullParmName("MaxProtonEnergyForStoppingPowerRatio")))
			fMaxProtonEnergyForRatio = fPm->GetDoubleParameter(GetFullParmName("MaxProtonEnergyForStoppingPowerRatio"), "Energy");
		else
			fMaxProtonEnergyForRatio = 500*MeV;

		fProtonEnergyNBins = (int)((fMaxProtonEnergyForRatio - fMinProtonEnergyForRatio) / fProtonEnergyBinSize + 0.5);

		if (fPm->ParameterExists(GetFullParmName("ElectronEnergyBinSize")))
			fElectronEnergyBinSize = fPm->GetDoubleParameter(GetFullParmName("ElectronEnergyBinSize"), "Energy");
		else
			fElectronEnergyBinSize = 1*keV;

		if (fPm->ParameterExists(GetFullParmName("MinElectronEnergyForStoppingPowerRatio")))
			fMinElectronEnergyForRatio = fPm->GetDoubleParameter(GetFullParmName("MinElectronEnergyForStoppingPowerRatio"), "Energy");
		else
			fMinElectronEnergyForRatio = 1*keV;

		if (fPm->ParameterExists(GetFullParmName("MaxElectronEnergyForStoppingPowerRatio")))
			fMaxElectronEnergyForRatio = fPm->GetDoubleParameter(GetFullParmName("MaxElectronEnergyForStoppingPowerRatio"), "Energy");
		else
			fMaxElectronEnergyForRatio = 1000*keV;

		fElectronEnergyNBins = (int)((fMaxElectronEnergyForRatio - fMinElectronEnergyForRatio) / fElectronEnergyBinSize + 0.5);
	}
}


TsScoreDoseToMaterial::~TsScoreDoseToMaterial() {;}


void TsScoreDoseToMaterial::UpdateForSpecificParameterChange(G4String parameter)
{
	G4String parameterLower = parameter;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(parameterLower);
#else
	parameterLower.toLower();
#endif

	if (parameterLower == GetFullParmNameLower("Material")) {
		if (fVerbosity>0)
			G4cout << "TsScoreDoseToMaterial::UpdateForParameterChange for parameter: " << parameter << G4endl;

		G4String materialName = fPm->GetStringParameter(GetFullParmName("Material"));
		fReferenceMaterial = GetMaterial(materialName);
		if (!fReferenceMaterial) {
			G4cout << "Topas is exiting due to error in scoring setup." << G4endl;
			G4cout << "Unknown material, " << materialName << ", specified in: " << GetFullParmName("Material") << G4endl;
			fPm->AbortSession(1);
		}
		fHadParameterChangeSinceLastRun = true;
	} 
	else if (parameterLower == GetFullParmNameLower("OutputWeightingFactor")) {
		if (fVerbosity>0)
			G4cout << "TsScoreDoseToMaterial::UpdateForParameterChange for parameter: " << parameter << G4endl;

		fOutputWeightingFactor = fPm->GetUnitlessParameter(GetFullParmName("OutputWeightingFactor"));
        fHadParameterChangeSinceLastRun = true;
	} else {
		TsVScorer::UpdateForSpecificParameterChange(parameter);
	}
}


G4bool TsScoreDoseToMaterial::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	// Pre-calculate stopping power ratios on first call
	if (fPreCalculateStoppingPowerRatios && fProtonStoppingPowerRatio.size()==0) {
		G4MaterialTable theTable = *(G4Material::GetMaterialTable());
		for (unsigned int i=0; i<theTable.size(); i++) {

			std::vector<G4double> protonVec;
			for (G4double energy=fMinProtonEnergyForRatio; energy<=fMaxProtonEnergyForRatio; energy+=fProtonEnergyBinSize) {
				protonVec.push_back( theTable[i]->GetDensity() / fReferenceMaterial->GetDensity() *
									 fEmCalculator.ComputeTotalDEDX(energy, G4Proton::ProtonDefinition(), fReferenceMaterial)
									/ fEmCalculator.ComputeTotalDEDX(energy, G4Proton::ProtonDefinition(), theTable[i]) );
			}
			fProtonStoppingPowerRatio.push_back(protonVec);

			std::vector<G4double> electronVec;
			for (G4double energy=fMinElectronEnergyForRatio; energy<=fMaxElectronEnergyForRatio; energy+=fElectronEnergyBinSize) {
				electronVec.push_back( theTable[i]->GetDensity() / fReferenceMaterial->GetDensity() *
									   fEmCalculator.ComputeTotalDEDX(energy, G4Electron::ElectronDefinition(), fReferenceMaterial)
									  / fEmCalculator.ComputeTotalDEDX(energy, G4Electron::ElectronDefinition(), theTable[i]) );
			}
			fElectronStoppingPowerRatio.push_back(electronVec);

			fOtherStoppingPowerRatio.push_back( theTable[i]->GetDensity() / fReferenceMaterial->GetDensity() *
											    fEmCalculator.ComputeTotalDEDX(fSubstituteForNeutralsEnergy, fProtonSubstituteForNeutrals, fReferenceMaterial)
											   / fEmCalculator.ComputeTotalDEDX(fSubstituteForNeutralsEnergy, fProtonSubstituteForNeutrals, theTable[i]));
		}
	}

	G4double edep = aStep->GetTotalEnergyDeposit();
	if ( edep > 0. ) {
		ResolveSolid(aStep);

		G4double density = aStep->GetPreStepPoint()->GetMaterial()->GetDensity();
		G4double dose = edep / (density * GetCubicVolume(aStep));
		dose *= aStep->GetPreStepPoint()->GetWeight() * fOutputWeightingFactor;

		G4ParticleDefinition* particle = aStep->GetTrack()->GetDefinition();
		G4double energy = aStep->GetPreStepPoint()->GetKineticEnergy();

		if (fPreCalculateStoppingPowerRatios) {
			G4int materialIndex = aStep->GetPreStepPoint()->GetMaterial()->GetIndex();
			G4int energyIndex;

			if ( particle == G4Proton::ProtonDefinition() ) {
				// convert to Dose to reference material for protons from EM tables:
				if (energy < fMinProtonEnergyForRatio) energyIndex = 0;
				else if (energy >= fMaxProtonEnergyForRatio) energyIndex = fProtonEnergyNBins - 1;
				else energyIndex = (int)((energy - fMinProtonEnergyForRatio) / fProtonEnergyBinSize + 0.5);
				dose *= fProtonStoppingPowerRatio[materialIndex][energyIndex];
			} else if ( particle == G4Electron::ElectronDefinition() ) {
				// convert to Dose to reference material for electrons from EM tables:
				if (energy < fMinElectronEnergyForRatio) energyIndex = 0;
				else if (energy >= fMaxElectronEnergyForRatio) energyIndex = fElectronEnergyNBins - 1;
				else energyIndex = (int)((energy - fMinElectronEnergyForRatio) / fElectronEnergyBinSize + 0.5);
				dose *= fElectronStoppingPowerRatio[materialIndex][energyIndex];
			} else {
				// convert to Dose to reference material for neutral particles from EM tables, assuming scaling factor from protons (dE/dx not defined for neutrals):
				dose *= fOtherStoppingPowerRatio[materialIndex];
			}
		} else {
			if ( particle->GetPDGCharge() != 0 ) {
				// convert to Dose to Material for charged particles from EM tables:
				G4double materialStoppingPower = fEmCalculator.ComputeTotalDEDX(energy, particle,
																				aStep->GetPreStepPoint()->GetMaterial());
				if (materialStoppingPower == 0.) {
					fPm->GetSequenceManager()->NoteUnscoredHit(energy, GetName());
					return false;
				}
				G4double referenceMaterialStoppingPower = fEmCalculator.ComputeTotalDEDX(energy, particle, fReferenceMaterial);
				dose *= ( density / fReferenceMaterial->GetDensity() ) *  ( referenceMaterialStoppingPower / materialStoppingPower );
			} else {
				// convert to Dose to Material for neutral particles from EM tables, assuming scaling factor from protons (dE/dx not defined for neutrals):
				G4double materialStoppingPower = fEmCalculator.ComputeTotalDEDX(fSubstituteForNeutralsEnergy, fProtonSubstituteForNeutrals,
																				aStep->GetPreStepPoint()->GetMaterial());
				if (materialStoppingPower == 0.) {
					fPm->GetSequenceManager()->NoteUnscoredHit(energy, GetName());
					return false;
				}
				G4double referenceMaterialStoppingPower = fEmCalculator.ComputeTotalDEDX(fSubstituteForNeutralsEnergy, fProtonSubstituteForNeutrals, fReferenceMaterial);
				dose *= ( density / fReferenceMaterial->GetDensity() ) *  ( referenceMaterialStoppingPower / materialStoppingPower );
			}
		}

		AccumulateHit(aStep, dose);
		return true;
	}
	return false;
}
