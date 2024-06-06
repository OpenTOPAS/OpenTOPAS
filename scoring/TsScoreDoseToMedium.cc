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

#include "TsScoreDoseToMedium.hh"

TsScoreDoseToMedium::TsScoreDoseToMedium(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
								 G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("Gy");

	fOutputWeightingFactor = 1.0;
	if (fPm->ParameterExists(GetFullParmName("OutputWeightingFactor"))){
		fOutputWeightingFactor = fPm->GetUnitlessParameter(GetFullParmName("OutputWeightingFactor"));
	}
	
}


TsScoreDoseToMedium::~TsScoreDoseToMedium() {;}


void TsScoreDoseToMedium::UpdateForSpecificParameterChange(G4String parameter)
{		

	G4String parameterLower = parameter;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(parameterLower);
#else
	parameterLower.toLower();
#endif

	if (parameterLower == GetFullParmNameLower("OutputWeightingFactor")) {
		fOutputWeightingFactor = fPm->GetUnitlessParameter(GetFullParmName("OutputWeightingFactor"));
        fHadParameterChangeSinceLastRun = true;
    } else {
        TsVScorer::UpdateForSpecificParameterChange(parameter);
    }
}


G4bool TsScoreDoseToMedium::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	G4double edep = aStep->GetTotalEnergyDeposit();
	if ( edep > 0. ) {
		G4double density = aStep->GetPreStepPoint()->GetMaterial()->GetDensity();

		ResolveSolid(aStep);

		G4double dose = edep / ( density * GetCubicVolume(aStep));
		dose *= aStep->GetPreStepPoint()->GetWeight() * fOutputWeightingFactor;

		AccumulateHit(aStep, dose);

		return true;
	}
	return false;
}
