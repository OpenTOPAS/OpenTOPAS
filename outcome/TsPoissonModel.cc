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
#include "TsPoissonModel.hh"

#include <vector>
#include <math.h>

TsPoissonModel::TsPoissonModel(TsParameterManager* pM, G4String parName)
: TsVOutcomeModel(pM, parName), fPm(pM), fModelName("Poisson")
{
	if ( fPm->ParameterExists( GetFullParmName(fModelName, "UsePresetParametersFromOrganNamed") ) ) {
		G4String organName = fPm->GetStringParameter( GetFullParmName(fModelName, "UsePresetParametersFromOrganNamed"));
		G4String valueParmName = "Sc/PoissonValues/" + organName;
		if ( fPm->ParameterExists(valueParmName) ) {
			G4double* parameters = fPm->GetUnitlessVector(valueParmName);
			fTCD50 = parameters[0];
			fGamma50 = parameters[1];
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring for outcome setup." << G4endl;
			G4cerr << organName << " refers to an unknown organ in database" << G4endl;
			fPm->AbortSession(1);
		}
	} else {
		fTCD50 = fPm->GetUnitlessParameter( GetFullParmName(fModelName, "TCD50" ) );
		fGamma50 = fPm->GetUnitlessParameter( GetFullParmName(fModelName, "Gamma50" ));
	}
}


TsPoissonModel::~TsPoissonModel() {;}


G4double TsPoissonModel::Initialize(std::vector<G4double> dose, std::vector<G4double> volume) {
	// based on Warkentin. et. al. J. Applied Clinical Medical Physics, 5(1), 2004
	G4double argument = 0.0;
	G4double totalVolume = SumElementsOfVector(volume);

	for ( int i = 0; i < int(dose.size()); i++ ) {
		argument += (volume[i]/totalVolume) * exp( 2.0 * fGamma50 * (1.0 - dose[i]/fTCD50)/log(2.0));
	}

	G4double TCP = 100 * pow(0.5, argument);
	return TCP;
}
