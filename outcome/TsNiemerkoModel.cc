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

#include "TsNiemerkoModel.hh"

#include <vector>
#include <math.h>

TsNiemerkoModel::TsNiemerkoModel(TsParameterManager* pM, G4String parName)
: TsVOutcomeModel(pM, parName), fPm(pM), fModelName("Niemerko")
{
	fA = fPm->GetUnitlessParameter( GetFullParmName( fModelName, "a" ) );
	fGamma50 = fPm->GetUnitlessParameter( GetFullParmName( fModelName, "Gamma50" ) );
	fMode = fPm->GetStringParameter( GetFullParmName( fModelName, "Mode" ) );
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(fMode);
#else
	fMode.toLower();
#endif

	if ( fMode == "ntcp" ) {
		G4cout << "Using Niemerko model in " << fMode << " mode" << G4endl;
		f50 = fPm->GetUnitlessParameter( GetFullParmName( fModelName, "TD50" ) );
	} else if (fMode == "tcp") {
		G4cout << "Using Niemerko model in " << fMode << " mode" << G4endl;
		f50 = fPm->GetUnitlessParameter( GetFullParmName( fModelName, "TCD50" ) );
	} else {
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << GetFullParmName(fModelName, "Mode") << " refers to an unknown mode: "
			   << fPm->GetStringParameter(GetFullParmName(fModelName, "Mode")) << G4endl;
		fPm->AbortSession(1);
	}
}


TsNiemerkoModel::~TsNiemerkoModel() {;}


G4double TsNiemerkoModel::Initialize(std::vector<G4double> dose, std::vector<G4double> volume) {
	G4double EUD = CalculateEUD(dose, volume, fA);
	G4double tcpOrNtcp = 100.0 / ( 1.0 + pow( f50/EUD, 4 * fGamma50) );
	return tcpOrNtcp;
}
