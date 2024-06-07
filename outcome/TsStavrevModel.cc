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
#include "TsStavrevModel.hh"

#include "G4Integrator.hh"

#include <vector>
#include <math.h>

TsStavrevModel::TsStavrevModel(TsParameterManager* pM, G4String parName)
: TsVOutcomeModel(pM, parName), fPm(pM), fModelName("Stavrev")
{
	fN0 = fPm->GetUnitlessParameter( GetFullParmName(fModelName, "N0"));
	fMu_cr = fPm->GetUnitlessParameter( GetFullParmName(fModelName, "mu_cr"));
	fSigma_cr = fPm->GetUnitlessParameter( GetFullParmName(fModelName, "sigma_cr"));
	fAlpha = fPm->GetUnitlessParameter( GetFullParmName(fModelName, "alpha"));
	fSigmaAlpha = fPm->GetUnitlessParameter( GetFullParmName(fModelName, "sigma_alpha"));
}


TsStavrevModel::~TsStavrevModel() {;}


G4double TsStavrevModel::Initialize(std::vector<G4double> dose, std::vector<G4double> volume) {
	fVolume = volume;
	fDose = dose;
	G4Integrator<TsStavrevModel, G4double(TsStavrevModel::*)(G4double)> integral;
	G4double ntcpOrTcp = integral.Legendre(this, &TsStavrevModel::Function, 0.0, 1.0, 1000);

	return 100 * ntcpOrTcp;
}


G4double TsStavrevModel::Function(G4double x) {
	// Here the substitution x -> x/(1.0 - x) was performed to allow integration [0,infty)
	double sigmaLogAlpha = sqrt( log( 1.0 + fSigmaAlpha*fSigmaAlpha/(log(fAlpha)*log(fAlpha))));

	double numerator = -log(-log(mu_d(x/(1.0-x)))) + log(-log(fMu_cr));
	double denominator = fSigma_cr/(-fMu_cr*log(fMu_cr));

	double phi = CalculateProbit( numerator/denominator );
	double argument = phi * LogNormalFunction(x/(1.0-x), fAlpha, sigmaLogAlpha)/pow(1.0-x,2.0);
	return argument;
}


G4double TsStavrevModel::mu_d(G4double x) {
	G4double totalVolume = SumElementsOfVector(fVolume);
	G4double mud = 0.0;

	for ( int i = 0; i < int(fDose.size()); i++ )
		mud += (fVolume[i]/totalVolume) * P_FSU(x, fDose[i]);

	return mud;
}


G4double TsStavrevModel::P_FSU(G4double x, G4double dose) {
	double pfsu = exp( -fN0 * exp( -x * dose ) );
	return pfsu;
}
