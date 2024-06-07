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

#include "G4Integrator.hh"
#include "G4PhysicalConstants.hh"

#include <iostream>
#include <vector>

TsVOutcomeModel::TsVOutcomeModel(TsParameterManager*, G4String parmPrefix)
:fParmPrefix(parmPrefix)
{;}


TsVOutcomeModel::~TsVOutcomeModel() {;}


G4double TsVOutcomeModel::Initialize(std::vector<G4double>, std::vector<G4double>) {
	return 0;
}


std::vector< std::vector<G4double> > TsVOutcomeModel::CumulativeToDifferentialDVH(std::vector<G4double> dose,
																				  std::vector<G4double> volume) {
	for ( int i = 1; i < int(dose.size()); i++ )
		volume[i-1] -= volume[i];

	dose.pop_back();
	volume.pop_back();
	std::vector< std::vector<G4double> > output;
	output.push_back(dose);
	output.push_back(volume);
	return output;
}


std::vector< std::vector<G4double> > TsVOutcomeModel::DifferentialToCumulativeDVH(std::vector<G4double> dose,
																				  std::vector<G4double> volume) {
	G4int n = G4int( dose.size() );
	G4double delta = dose[1]-dose[0];
	G4double* cummSum = new G4double[n];
	G4double* cummSumInv = new G4double[n];

	cummSum[0] = volume[0];
	for ( int i = 0; i < n; i++ )
		cummSum[i+1] = cummSum[i] + volume[i+1];

	for ( int i = 0; i < n; i++ )
		cummSumInv[i] = cummSum[n-1] - cummSum[i];

	std::vector<G4double> cummDose;
	std::vector<G4double> cummVolume;

	cummDose.push_back(0.0);
	cummVolume.push_back(1.0);
	for ( int i = 0; i < n; i++ ) {
		cummDose.push_back( dose[i] );
		cummVolume.push_back( cummSumInv[i-1]/cummSum[n-1] );
	}

	cummDose.push_back(dose[n-1]+delta);
	cummVolume.push_back(cummSumInv[n-1]/cummSum[n-1]);
	std::vector< std::vector<G4double> > output;
	output.push_back(cummDose);
	output.push_back(cummVolume);
	return output;
}


G4double TsVOutcomeModel::CalculateBED(G4double dose, G4double alphaOverBeta,
									   int nbOfFractions, G4double dosePerFraction){
	return dose * ( alphaOverBeta + dose/nbOfFractions ) / ( alphaOverBeta + dosePerFraction );
}


G4double TsVOutcomeModel::CalculateEUD(std::vector<G4double> dose, std::vector<G4double> volume,
									   G4double exponent) {
	G4double totalVolume = SumElementsOfVector(volume);

	G4double EUD = 0.0;
	for ( int i = 0; i < int(dose.size()); i++ )
		EUD += pow( dose[i], exponent ) * ( volume[i] /totalVolume );

	EUD = pow( EUD, 1.0/exponent);
	return EUD;
}


G4double TsVOutcomeModel::CalculateLogistic(G4double t, G4double exponent) {
	return 1.0 / ( 1.0 + pow( t, exponent ) );
}


G4double TsVOutcomeModel::CalculateLogLogistic(G4double t) {
	return exp( t ) / ( 1.0 + exp( t ) );
}


G4double TsVOutcomeModel::CalculateProbit(G4double t) {
	return 0.5 * ( 1.0 + erf(t / sqrt(2.0)));
}


std::vector<G4double> TsVOutcomeModel::DoseToBED(std::vector<G4double> dose, G4double alphaOverBeta,
												 int nbOfFractions, G4double dosePerFraction){
	for ( int i = 0; i < int(dose.size()); i++ )
		dose[i] = CalculateBED(dose[i], alphaOverBeta, nbOfFractions, dosePerFraction);

	return dose;
}


G4double TsVOutcomeModel::NormalFunction(G4double x, G4double mu, G4double sigma) {
	return 1.0/(sigma * sqrt(twopi)) * exp( -(x - mu)*(x - mu) / (2.0 * sigma*sigma));
}


G4double TsVOutcomeModel::LogNormalFunction(double x, double mu, double sigma_ln) {
	return NormalFunction(log(x), log(mu), sigma_ln)/x;
}


G4String TsVOutcomeModel::GetFullParmName(G4String modelName, const char* suffix) {
	return fParmPrefix + modelName + "/" + suffix;
}


G4double TsVOutcomeModel::GetMaxOfAVector(std::vector<G4double> vec) {
	G4double maximum = 0.0;
	for ( int i = 0; i < int(vec.size()); i++ )
		if ( maximum < vec[i] )
			maximum = vec[i];

	return maximum;
}


G4double TsVOutcomeModel::SumElementsOfVector(std::vector<G4double> vec) {
	G4double sum = 0.0;
	for ( int i = 0; i < int(vec.size()); i++ )
		sum += vec[i];

	return sum;
}
