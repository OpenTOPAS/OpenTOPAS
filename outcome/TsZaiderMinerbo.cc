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
#include "TsZaiderMinerbo.hh"

#include <vector>
#include <math.h>

TsZaiderMinerbo::TsZaiderMinerbo(TsParameterManager* pM, G4String parName)
: TsVOutcomeModel(pM, parName), fPm(pM), fModelName("ZaiderMinerbo")
{
	fAlpha = fPm->GetUnitlessParameter( GetFullParmName(fModelName, "Alpha" ) );
	fBeta = fPm->GetUnitlessParameter( GetFullParmName(fModelName, "Beta" ));
	fLambda = fPm->GetUnitlessParameter( GetFullParmName(fModelName, "Lambda"));
	fNbOfFractions = fPm->GetIntegerParameter( GetFullParmName(fModelName, "NumberOfFractions"));
	fTimek = fPm->GetUnitlessVector( GetFullParmName(fModelName, "TimeOfUntilAFraction"));
}


TsZaiderMinerbo::~TsZaiderMinerbo() {;}


G4double TsZaiderMinerbo::Initialize(std::vector<G4double> dose, std::vector<G4double> volume) {
	// based on Warkentin. et. al. J. Applied Clinical Medical Physics, 5(1), 2004
	G4double argument = 1.0;
	G4int N = int(volume.size());
	G4double totalTime = 0.0;

	for ( int i = 0; i < fNbOfFractions; i++)
		totalTime += fTimek[i];

	for ( int i = 0; i < int(dose.size()); i++ ) {
		double upper = LQPredictionAfterFraction(fNbOfFractions, dose[i]) * exp(fLambda * totalTime);
		double sum = 0.0;
		for ( int j = 0; j < fNbOfFractions; j++)
			sum += (1.0/LQPredictionAfterFraction(j, dose[i]));
		double lower = 1.0 - LQPredictionAfterFraction(fNbOfFractions, dose[i]) * exp(fLambda * totalTime) * sum;
		upper = pow(1.0 - (upper/lower),N);
		argument *= upper;
	}

	G4double TCP = 100 * argument;
	return TCP;
}


G4double TsZaiderMinerbo::LQPredictionAfterFraction(G4int fraction, G4double dose) {
	return exp( -fAlpha * dose * fraction/fNbOfFractions - fBeta * pow(fraction * dose /fNbOfFractions,2)/fraction);
}
