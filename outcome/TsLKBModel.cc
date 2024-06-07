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

#include "TsVOutcomeModel.hh"
#include "TsLKBModel.hh"

#include <math.h>
#include <vector>

TsLKBModel::TsLKBModel(TsParameterManager* pM, G4String parmName)
: TsVOutcomeModel(pM, parmName), fPm(pM), fModelName("LKB"), fScorerName(parmName)
{
	if ( fPm->ParameterExists( GetFullParmName(fModelName, "UsePresetParametersFromOrganNamed") ) ) {
		G4String organName = fPm->GetStringParameter( GetFullParmName(fModelName, "UsePresetParametersFromOrganNamed"));
		G4String valueParmName = "Sc/LKBValues/" + organName;
		if ( fPm->ParameterExists(valueParmName) ) {
			G4double* parameters = fPm->GetUnitlessVector(valueParmName);
			fN = parameters[0];
			fM = parameters[1];
			fTD50 = parameters[2];
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring for outcome setup." << G4endl;
			G4cerr << organName << " refers to an unknown organ in database" << G4endl;
			fPm->AbortSession(1);
		}
	} else {
		fN = fPm->GetUnitlessParameter( GetFullParmName(fModelName, "n"));
		fM = fPm->GetUnitlessParameter( GetFullParmName(fModelName, "m"));
		fTD50 = fPm->GetUnitlessParameter( GetFullParmName(fModelName, "TD50"));
	}
	return;
}


TsLKBModel::~TsLKBModel() {;}


G4double TsLKBModel::Initialize(std::vector<G4double> dose, std::vector<G4double> volume) {

	// As in AAPM report of TG166, mathematically equivalent than previous implementation
	// in Luxton et. al. 2008. But this implementation contains gEUD directly.
	G4double gEUD = CalculateEUD(dose, volume, 1.0/fN);

	G4double t = ( gEUD - fTD50 ) / ( fM * fTD50 );

	G4double NTCP = 100.0 * CalculateProbit(t);

	return NTCP;

}
