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

#include "TsScoringHub.hh"

#include "TsParameterManager.hh"
#include "TsExtensionManager.hh"

#include "TsScoreAmbientDoseEquivalent.hh"
#include "TsScoreDoseToMedium.hh"
#include "TsScoreDoseToMaterial.hh"
#include "TsScoreEnergyDeposit.hh"
#include "TsScoreProtonLET.hh"
#include "TsScoreProtonLET_Denominator.hh"
#include "TsScoreCharge.hh"
#include "TsScoreEffectiveCharge.hh"
#include "TsScoreFluence.hh"
#include "TsScoreKineticEnergyAtSurface.hh"
#include "TsScoreEnergyFluence.hh"
#include "TsScoreOriginCount.hh"
#include "TsScoreStepCount.hh"
#include "TsScorePET.hh"
#include "TsScoreSurfaceCurrent.hh"
#include "TsScoreSurfaceTrackCount.hh"
#include "TsScoreOpticalPhotonCount.hh"
#include "TsScorePhaseSpace.hh"
#include "TsScoreTrackLengthEstimator.hh"

TsScoringHub::TsScoringHub(TsParameterManager* pM):fPm(pM)
{
	pM->RegisterScorerQuantityName("ambientdoseequivalent");
	pM->RegisterScorerQuantityName("dosetomedium");
	pM->RegisterScorerQuantityName("dosetowater");
	pM->RegisterScorerQuantityName("dosetomaterial");
	pM->RegisterScorerQuantityName("energydeposit");
	pM->RegisterScorerQuantityName("protonlet");
	pM->RegisterScorerQuantityName("protonlet_denominator");
	pM->RegisterScorerQuantityName("charge");
	pM->RegisterScorerQuantityName("effectivecharge");
	pM->RegisterScorerQuantityName("kineticenergyatsurface");
	pM->RegisterScorerQuantityName("fluence");
	pM->RegisterScorerQuantityName("energyfluence");
	pM->RegisterScorerQuantityName("origincount");
	pM->RegisterScorerQuantityName("stepcount");
	pM->RegisterScorerQuantityName("pet");
	pM->RegisterScorerQuantityName("surfacecurrent");
	pM->RegisterScorerQuantityName("surfacetrackcount");
	pM->RegisterScorerQuantityName("opticalphotoncount");
	pM->RegisterScorerQuantityName("phasespace");
}


TsScoringHub::~TsScoringHub()
{
}


TsVScorer* TsScoringHub::InstantiateScorer(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
										   TsScoringManager* scM, G4String scorerName, G4String quantityName,
										   G4String outFileName, G4bool isSubScorer)
{
	G4String quantityNameLower = quantityName;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(quantityNameLower);
#else
	quantityNameLower.toLower();
#endif

	// First see if the user's extensions include this scorer
	TsVScorer* scorer = eM->InstantiateScorer(pM, mM, gM, scM, scorerName, quantityName, quantityNameLower, outFileName, isSubScorer);
	if (scorer) return scorer;

	if (quantityNameLower=="ambientdoseequivalent") {
	    return new TsScoreAmbientDoseEquivalent(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	} else if (quantityNameLower=="dosetomedium") {
		return new TsScoreDoseToMedium(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	} else if (quantityNameLower=="dosetowater" || quantityNameLower=="dosetomaterial") {
		return new TsScoreDoseToMaterial(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	} else if (quantityNameLower=="energydeposit") {
		return new TsScoreEnergyDeposit(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	} else if (quantityNameLower=="protonlet") {
		return new TsScoreProtonLET(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	} else if (quantityNameLower=="protonlet_denominator") {
		return new TsScoreProtonLET_Denominator(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	} else if (quantityNameLower=="charge") {
		return new TsScoreCharge(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	} else if (quantityNameLower=="effectivecharge") {
		return new TsScoreEffectiveCharge(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	} else if (quantityNameLower=="kineticenergyatsurface") {
		return new TsScoreKineticEnergyAtSurface(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	} else if (quantityNameLower=="fluence") {
		return new TsScoreFluence(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	} else if (quantityNameLower=="energyfluence") {
		return new TsScoreEnergyFluence(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	} else if (quantityNameLower=="origincount") {
		return new TsScoreOriginCount(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	} else if (quantityNameLower=="stepcount") {
		return new TsScoreStepCount(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	} else if (quantityNameLower=="pet") {
		return new TsScorePET(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	} else if (quantityNameLower=="surfacecurrent") {
		return new TsScoreSurfaceCurrent(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	} else if (quantityNameLower=="surfacetrackcount") {
		return new TsScoreSurfaceTrackCount(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	} else if (quantityNameLower=="opticalphotoncount") {
	    return new TsScoreOpticalPhotonCount(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	} else if (quantityNameLower=="phasespace") {
		return new TsScorePhaseSpace(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	} else if (quantityNameLower=="tracklengthestimator") {
		return new TsScoreTrackLengthEstimator(pM, mM, gM, scM, eM, scorerName, quantityName, outFileName, isSubScorer);
	}
	else return 0;
}


void TsScoringHub::AddScorerFromGUI(G4String& scorerName, G4String& componentName, G4String& quantityName) {
	G4String parameterName = "s:Sc/" + scorerName + "/Quantity";
	G4String transValue = "\"" + quantityName + "\"";
	fPm->AddParameter(parameterName, transValue);

	if ((quantityName == "surfacecurrent") ||
		(quantityName == "surfacetrackcount") ||
		(quantityName == "phasespace")) {
		parameterName = "sc:Sc/" + scorerName + "/Surface";
		transValue = "\"" + componentName + "/ZPlusSurface\"";
	} else {
		parameterName = "s:Sc/" + scorerName + "/Component";
		transValue = "\"" + componentName + "\"";
	}
	fPm->AddParameter(parameterName, transValue);

	parameterName = "bc:Sc/" + scorerName + "/OutputToConsole";
	transValue = "\"True\"";
	fPm->AddParameter(parameterName, transValue);

	parameterName = "sc:Sc/" + scorerName + "/IfOutputFileAlreadyExists";
	transValue = "\"Overwrite\"";
	fPm->AddParameter(parameterName, transValue);
	
	if (quantityName == "dosetomaterial") {
		parameterName = "sc:Sc/" + scorerName + "/Material";
		transValue = "\"G4_WATER\"";
		fPm->AddParameter(parameterName, transValue);
	}

	if ((quantityName == "opticalphotoncount") ||
		(quantityName == "phasespace")) {
		parameterName = "sc:Sc/" + scorerName + "/OutputType";
		transValue = "\"ASCII\"";
		fPm->AddParameter(parameterName, transValue);
	}
}
