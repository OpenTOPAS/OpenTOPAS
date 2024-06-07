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
#include "TsCriticalElementModel.hh"

#include "G4PhysicalConstants.hh"

#include <vector>
#include <math.h>

TsCriticalElementModel::TsCriticalElementModel(TsParameterManager* pM, G4String parmName)
: TsVOutcomeModel(pM, parmName), fPm(pM), fModelName("CriticalElement")
{
	if ( fPm->ParameterExists( GetFullParmName(fModelName, "UsePresetParametersFromOrganNamed") ) ) {
		G4String organName = fPm->GetStringParameter( GetFullParmName(fModelName, "UsePresetParametersFromOrganNamed"));
		G4String valueParmName = "Sc/CriticalElementValues/" + organName;
		if ( fPm->ParameterExists(valueParmName) ) {
			G4double* parameters = fPm->GetUnitlessVector(valueParmName);
			fGamma = parameters[0];
			fTD50 = parameters[1];
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring for outcome setup." << G4endl;
			G4cerr << organName << " refers to an unknown organ in database" << G4endl;
			fPm->AbortSession(1);
		}
	} else {
		fGamma = fPm->GetUnitlessParameter(GetFullParmName(fModelName, "Gamma"));
		fTD50 = fPm->GetUnitlessParameter(GetFullParmName(fModelName, "TD50"));
	}
}


TsCriticalElementModel::~TsCriticalElementModel() {;}


G4double TsCriticalElementModel::Initialize(std::vector<G4double> dose, std::vector<G4double> volume) {
	G4double totalVolume = SumElementsOfVector(volume);
	G4double NTCP = 1.0;

	// Based on Niemerko and Goitein 1991. Radiation and Oncology, 20, 166-176.
	G4double e = 2.718281828459045;
	for ( int i = 0; i < int(dose.size()); i++ ) {
		G4double P_1_D = pow( 2.0, -exp( e * fGamma * ( 1.0 - dose[i]/fTD50)));
		NTCP *= pow( 1.0 - P_1_D, volume[i]/totalVolume );

	}

	NTCP = 100 * (1 - NTCP);
	return NTCP;
}
