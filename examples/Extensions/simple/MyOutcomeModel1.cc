// Outcome Model for MyOutcomeModel1
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
#include "MyOutcomeModel1.hh"

#include "G4PhysicalConstants.hh"

#include <vector>
#include <math.h>

MyOutcomeModel1::MyOutcomeModel1(TsParameterManager* pM, G4String parmName)
: TsVOutcomeModel(pM, parmName), fPm(pM), fModelName("CriticalElement")
{
	ResolveParameters();
}


MyOutcomeModel1::~MyOutcomeModel1() {;}


void MyOutcomeModel1::ResolveParameters() {
	fM = fPm->GetUnitlessParameter(GetFullParmName(fModelName, "m"));
	fTD50 = fPm->GetUnitlessParameter(GetFullParmName(fModelName, "TD50"));
	fFunction = fPm->GetStringParameter(GetFullParmName(fModelName, "Function"));
	fFunction.toLower();

	if ( fFunction != "probit" && fFunction != "logistic" ) {
	    G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
	    G4cerr << GetFullParmName(fModelName, "Function") << " refers to an unknown outcome function: "
	           << fPm->GetStringParameter(GetFullParmName(fModelName, "Function")) << G4endl;
	    exit(1);
	}
}


G4double MyOutcomeModel1::Initialize(std::vector<G4double> dose, std::vector<G4double> volume) {
	G4double totalVolume = SumElementsOfVector(volume);
	G4double NTCP = 1.0;
	G4double P_1_D;

	// Based on Niemerko and Goitein 1991. Radiation and Oncology, 20, 166-176.
	if ( fFunction == "probit" ) {
		G4double t;
		for ( int i = 0; i < int(dose.size()); i++ ) {
			t = 1.0/fM * ( dose[i]/fTD50 - 1 );
			P_1_D = CalculateProbit(t);
			NTCP *= pow( 1 - P_1_D, volume[t]/totalVolume );
		}
	} else {
		for ( int i = 0; i < int(dose.size()); i++ ) {
			P_1_D = CalculateLogistic( fTD50/dose[i], 4.0/(sqrt(2.0*pi) * fM) );
			NTCP *= pow( 1 - P_1_D, volume[i]/totalVolume );
		}
	}

	NTCP = 100 * (1 - NTCP);
	return NTCP;
}
