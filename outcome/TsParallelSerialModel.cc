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

#include "TsParallelSerialModel.hh"

#include <vector>
#include <math.h>

TsParallelSerialModel::TsParallelSerialModel(TsParameterManager* pM, G4String parmName)
: TsVOutcomeModel(pM, parmName), fPm(pM), fModelName("ParallelSerial")
{
	fS = fPm->GetUnitlessParameter(GetFullParmName(fModelName, "s"));
	fGamma = fPm->GetUnitlessParameter(GetFullParmName(fModelName, "gamma"));
	fTD50 = fPm->GetUnitlessParameter(GetFullParmName(fModelName, "TD50"));
}


TsParallelSerialModel::~TsParallelSerialModel() {;}


G4double TsParallelSerialModel::Initialize(std::vector<G4double> dose, std::vector<G4double> volume) {
	G4double totalVolume = SumElementsOfVector(volume);
	G4double NTCP = 1.0;

	// Based on Kallman, Agren and Brahme "Tumor and normal tissue responses to fractionated non-uniform dose delivery"
	G4double e = 2.718281828459045;
	for ( int i = 0; i < int(dose.size()); i++ ) {
		G4double P_1_D = pow( 2.0, -exp( e * fGamma * ( 1.0 - dose[i]/fTD50)));
		G4double argument = 1.0 - pow(P_1_D, fS);
		NTCP *= pow( argument, volume[i]/totalVolume );
	}

	NTCP = 100 * pow(1 - NTCP, 1./fS);
	return NTCP;
}
