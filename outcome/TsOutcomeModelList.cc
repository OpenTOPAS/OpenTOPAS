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
#include "TsExtensionManager.hh"

#include "TsOutcomeModelList.hh"
#include "TsLKBModel.hh"
#include "TsCriticalElementModel.hh"
#include "TsParallelSerialModel.hh"
#include "TsCriticalVolumeModel.hh"
#include "TsNiemerkoModel.hh"
#include "TsPoissonModel.hh"
#include "TsStavrevModel.hh"
#include "TsVOutcomeModel.hh"
#include "TsZaiderMinerbo.hh"

#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

#include <vector>

TsOutcomeModelList::TsOutcomeModelList(TsParameterManager* pm, TsExtensionManager* em, G4String scorerName, const G4String& unitName)
: fPm(pm), fUnitName(unitName), fScorerName(scorerName), fCorrectForDoseFractionation(false), fgEUDa_1(0)
{
	G4String parmPrefix = "Sc/" + fScorerName + "/";
	if ( fUnitName != "Gy") {
		G4cerr << "Topas is exiting due to a serious error in outcome scoring setup" << G4endl;
		G4cerr << "Expected the parameter named: " << parmPrefix + "Quantity" << " to have unit category: Dose" << G4endl;
		G4cerr << "But instead found unit category: " << fPm->GetUnitCategory(fUnitName) << G4endl;
		fPm->AbortSession(1);
	}
	G4String* modelNames = fPm->GetStringVector(parmPrefix + "OutcomeModelName");
	G4int lengthModelName = fPm->GetVectorLength(parmPrefix + "OutcomeModelName");
	G4String modelName;
	// Just for set all the parameters of the corresponding model
	for ( int i = 0; i < lengthModelName; i++ ) {
		modelName = modelNames[i];
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(modelName);
#else
		modelName.toLower();
#endif

		TsVOutcomeModel* model = em->InstantiateOutcomeModel(fPm, parmPrefix, modelName);
		if (model) {
			fMod.push_back(model);
			fModelName.push_back(modelName);
		} else if ( modelName == "lkb" ) {
			fMod.push_back(new TsLKBModel(fPm, parmPrefix));
			fModelName.push_back(modelName);
		} else if ( modelName == "criticalelement") {
			fMod.push_back(new TsCriticalElementModel(fPm, parmPrefix));
			fModelName.push_back(modelName);
		} else if ( modelName == "parallelserial" ) {
			fMod.push_back(new TsParallelSerialModel(fPm, parmPrefix));
			fModelName.push_back(modelName);
		} else if ( modelName == "criticalvolume" ) {
			fMod.push_back(new TsCriticalVolumeModel(fPm, parmPrefix));
			fModelName.push_back(modelName);
		} else if ( modelName == "niemerko" ) {
			fMod.push_back(new TsNiemerkoModel(fPm, parmPrefix));
			fModelName.push_back(modelName);
		} else if ( modelName == "poisson" ) {
			fMod.push_back(new TsPoissonModel(fPm, parmPrefix));
			fModelName.push_back(modelName);
		} else if ( modelName == "stavrev" ) {
			fMod.push_back(new TsStavrevModel(fPm, parmPrefix));
			fModelName.push_back(modelName);
		} else if ( modelName == "zaiderminerbo") {
			fMod.push_back(new TsZaiderMinerbo(fPm, parmPrefix));
			fModelName.push_back(modelName);
		} else {
			G4cerr << G4endl;
			G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
			G4cerr << parmPrefix + "ModelName" << " refers to an unknown model name: " << modelName << G4endl;
			fPm->AbortSession(1);
		}
	}

	// Correct for dose fractionation
	if ( fPm->ParameterExists( parmPrefix + "NumberOfFractions" ) ) {
		fCorrectForDoseFractionation = true;
		fNbOfFractions = fPm->GetIntegerParameter( parmPrefix + "NumberOfFractions" );
	}

	if ( fPm->ParameterExists( parmPrefix + "DosePerFraction") )
		fDosePerFraction = fPm->GetDoubleParameter( parmPrefix + "DosePerFraction" , "Dose" );
	else
		fDosePerFraction = 2.0*gray;

	if ( fPm->ParameterExists( parmPrefix + "AlphaOverBeta" ) )
		fAlphaOverBeta = fPm->GetDoubleParameter( parmPrefix + "AlphaOverBeta" , "Dose" );
	else
		fAlphaOverBeta = 3.0*gray;
}


TsOutcomeModelList::~TsOutcomeModelList() {;}


std::map<G4String, G4double> TsOutcomeModelList::CalculateOutcome(std::vector<G4double> dose,
																	  std::vector<G4double> volume,
																	  G4bool isDifDVH) {
	G4double value;
	for ( int i = 0; i < int(fMod.size()); i++ ) {
		value = Initialize(fMod[i], dose, volume, isDifDVH);
		fOutcome[ fModelName[i] ] = value;
	}
	return fOutcome;
}


G4double TsOutcomeModelList::Initialize(TsVOutcomeModel* model, std::vector<G4double> dose,
											std::vector<G4double> volume, G4bool isDifDVH) {

	if ( !isDifDVH ) {
		std::vector< std::vector<G4double> > DoseVolume = model->CumulativeToDifferentialDVH(dose, volume);
		dose = DoseVolume[0];
		volume = DoseVolume[1];
	}

	if ( fCorrectForDoseFractionation ) {
		std::vector<G4double> correctedDose = model->DoseToBED(dose, fAlphaOverBeta/gray, fNbOfFractions, fDosePerFraction/gray);
		dose = correctedDose;
	}

	fgEUDa_1 = model->CalculateEUD(dose, volume, 1.0);

	return model->Initialize(dose, volume);
}


void TsOutcomeModelList::Print(G4String modelSource) {
	G4cout << "" << G4endl;
	G4cout << "- Outcome models for scorer: " << fScorerName << G4endl;
	G4cout << "  from " << modelSource << G4endl;

	if ( fCorrectForDoseFractionation )
		G4cout << "   * Corrected for dose fractionation" << G4endl;

	G4cout << "   * gEUD (a=1): " << fgEUDa_1 << "(Gy)" << G4endl;

	std::map<G4String, G4double>::iterator it;
	for (it = fOutcome.begin(); it != fOutcome.end(); it++) {
		G4cout << "  Model: " << it->first << G4endl;
		G4cout << "    Probability: " << it->second << " %" << G4endl;
	}
}
