//
// ********************************************************************
// *                                                                  *
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

#ifndef TsVOutcomeModel_hh
#define TsVOutcomeModel_hh

#include <vector>

class TsParameterManager;

class TsVOutcomeModel
{
public:
	TsVOutcomeModel(TsParameterManager* pm, G4String parmName);
	virtual ~TsVOutcomeModel();

	virtual G4double Initialize(std::vector<G4double> dose, std::vector<G4double> volume);

	std::vector< std::vector<G4double> > CumulativeToDifferentialDVH(std::vector<G4double> dose,
																	 std::vector<G4double> volume);
	std::vector< std::vector<G4double> > DifferentialToCumulativeDVH(std::vector<G4double> dose,
																	 std::vector<G4double> volume);

	G4double CalculateBED(G4double dose, G4double alphaOverBeta, int nbOfFractions, G4double dosePerFraction);
	G4double CalculateEUD(std::vector<G4double> dose, std::vector<G4double> volume, G4double exponent);
	G4double CalculateLogistic(G4double t, G4double exponent);
	G4double CalculateLogLogistic(G4double exponent);
	G4double CalculateProbit(G4double t);

	G4String GetFullParmName(G4String modelName, const char* parmName);

	G4double GetMaxOfAVector(std::vector<G4double> v);
	G4double LogNormalFunction(G4double x, G4double mu, G4double sigma);
	G4double NormalFunction(G4double x, G4double mu, G4double sigma);
	G4double SumElementsOfVector(std::vector<G4double> v);

	std::vector<G4double> DoseToBED(std::vector<G4double> dose, G4double alphaOverBeta, int nbOfFractions, G4double dosePerFraction);

private:
	G4String fParmPrefix;
};
#endif
