// Scorer for ProtonLET
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

#include "MyScoreProtonLET.hh"

#include "TsParameterManager.hh"

#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4EmCalculator.hh"
#include "G4UIcommand.hh"

MyScoreProtonLET::MyScoreProtonLET(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
								   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("MeV/mm/(g/cm3)");

	fProtonDefinition = G4ParticleTable::GetParticleTable()->FindParticle("proton");
	fElectronDefinition = G4ParticleTable::GetParticleTable()->FindParticle("e-");

	fStepCount = 0;


	// Dose-averaged or fluence-averaged LET definition
	G4String weightType = "dose";
	if (fPm->ParameterExists(GetFullParmName("WeightBy")))
		weightType = fPm->GetStringParameter(GetFullParmName("WeightBy"));
	weightType.toLower();

	if (weightType == "dose") {
		fDoseWeighted = true;
	} else if (weightType == "fluence" || weightType == "track") {
		fDoseWeighted = false;
	} else {
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << GetFullParmName("WeightBy") << " refers to an unknown weighting: " << weightType << G4endl;
		exit(1);
	}


	// LET computation technique (step-by-step or prestep stopping power lookup)
	fPreStepLookup = false;
	if (fPm->ParameterExists(GetFullParmName("UsePreStepLookup")))
		fPreStepLookup = fPm->GetBooleanParameter(GetFullParmName("UsePreStepLookup"));


	// A dE/dx upper cutoff is used to fix spikes when dose-averaged LET is computed without the lookup table
	if (fDoseWeighted && !fPreStepLookup) {
		fMaxScoredLET = 100 * MeV/mm/(g/cm3);  // default: 100 keV/um in water

		if (fPm->ParameterExists(GetFullParmName("MaxScoredLET"))) {
			fMaxScoredLET = fPm->GetDoubleParameter(GetFullParmName("MaxScoredLET"), "force per density");
		} else {  // make available to subscorer
			G4String transValue = G4UIcommand::ConvertToString(fMaxScoredLET / (MeV/mm/(g/cm3))) + " MeV/mm/(g/cm3)";
			fPm->AddParameter("d:" + GetFullParmName("MaxScoredLET"), transValue);
		}
	} else {
		fMaxScoredLET = 0;

		if (fPm->ParameterExists(GetFullParmName("MaxScoredLET"))) {
			G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
			G4cerr << GetFullParmName("MaxScoredLET") << " is only available for dose-weighted LET scorers." << G4endl;
			exit(1);
		}
	}


	// Neglect secondary electrons in low density materials where the mean path length between discrete processes is
	// longer than the voxel width. Otherwise the LET is too high.
	if (fDoseWeighted && !fPreStepLookup) {
		fNeglectSecondariesBelowDensity = 0.1 * g/cm3;

		if (fPm->ParameterExists(GetFullParmName("NeglectSecondariesBelowDensity"))) {
			fNeglectSecondariesBelowDensity = fPm->GetDoubleParameter(GetFullParmName("NeglectSecondariesBelowDensity"), "Volumic Mass");
		} else {  // make available to subscorer
			G4String transValue = G4UIcommand::ConvertToString(fNeglectSecondariesBelowDensity / (g/cm3)) + " g/cm3";
			fPm->AddParameter("d:" + GetFullParmName("NeglectSecondariesBelowDensity"), transValue);
		}
	} else {
		fNeglectSecondariesBelowDensity = 0;

		if (fPm->ParameterExists(GetFullParmName("NeglectSecondariesBelowDensity"))) {
			G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
			G4cerr << GetFullParmName("NeglectSecondariesBelowDensity") << " is only available for dose-weighted LET scorers." << G4endl;
			exit(1);
		}
	}


	// Use fluence-weighted LET for low density materials in order to avoid rare spikes (a.k.a. pretty plot mode)
	// Disabled by default
	fUseFluenceWeightedBelowDensity = 0;
	if (fPm->ParameterExists(GetFullParmName("UseFluenceWeightedBelowDensity"))) {
		if (!fDoseWeighted) {
			G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
			G4cerr << GetFullParmName("UseFluenceWeightedBelowDensity") << " is only available for dose-weighted LET scorers." << G4endl;
			exit(1);
		}
		fUseFluenceWeightedBelowDensity = fPm->GetDoubleParameter(GetFullParmName("UseFluenceWeightedBelowDensity"), "Volumic Mass");
	}

	// Instantiate subscorer needed for denominator
	InstantiateSubScorer("ProtonLET_Denominator", outFileName, "Denominator");
}


MyScoreProtonLET::~MyScoreProtonLET() {;}


G4bool MyScoreProtonLET::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	if (aStep->GetTrack()->GetParticleDefinition()==fProtonDefinition) {

		G4double stepLength = aStep->GetStepLength();
		if (stepLength <= 0.)
			return false;

		G4double eDep = aStep->GetTotalEnergyDeposit();
		G4double density = aStep->GetPreStepPoint()->GetMaterial()->GetDensity();

		G4bool isStepFluenceWeighted = !fDoseWeighted || density < fUseFluenceWeightedBelowDensity;

		// Add energy deposited by secondary electrons (unless NeglectSecondariesBelowDensity is used)
		if (isStepFluenceWeighted || density > fNeglectSecondariesBelowDensity) {
			const G4TrackVector* secondary = aStep->GetSecondary();
			if (!secondary) {
				secondary = aStep->GetTrack()->GetStep()->GetSecondary();  // parallel worlds
			}
			if (secondary) {
				G4int diff;
				if (fStepCount == 0) diff = 0;
				else diff = secondary->size() - fStepCount;

				fStepCount = (*secondary).size();

				for (unsigned int i=(*secondary).size()-diff; i<(*secondary).size(); i++)
					if ((*secondary)[i]->GetParticleDefinition() == fElectronDefinition)
						eDep += (*secondary)[i]->GetKineticEnergy();
			}
		}

		// Compute LET
		G4double dEdx = 0;
		if (fPreStepLookup) {
			G4EmCalculator emCal;
			G4double preStepKE = aStep->GetPreStepPoint()->GetKineticEnergy();
			dEdx = emCal.ComputeElectronicDEDX(preStepKE, fProtonDefinition, aStep->GetPreStepPoint()->GetMaterial());
		} else {
			dEdx = eDep / stepLength;
		}

		// Compute weight (must be unitless in order to use a single denominator sub-scorer)
		G4double weight = 1.0;
		if (isStepFluenceWeighted)
			weight *= (stepLength/mm);
		else
			weight *= (eDep/MeV);

		// If dose-weighted and not using PreStepLookup, only score LET if below MaxScoredLET
		// Also must check if fluence-weighted mode has been enabled by a low density voxel
		if (isStepFluenceWeighted || fMaxScoredLET <= 0 || dEdx / density < fMaxScoredLET) {
			AccumulateHit(aStep, weight * dEdx / density);
			return true;
		}
	}
	return false;
}


G4int MyScoreProtonLET::CombineSubScorers()
{
	G4int counter = 0;

	TsVBinnedScorer* denomScorer = dynamic_cast<TsVBinnedScorer*>(GetSubScorer("Denominator"));
	for (G4int index=0; index < fNDivisions; index++) {
		if (denomScorer->fFirstMomentMap[index]==0.) {
			fFirstMomentMap[index] = 0;
			counter++;
		} else {
			fFirstMomentMap[index] = fFirstMomentMap[index] / denomScorer->fFirstMomentMap[index];
		}
	}

	return counter;
}
