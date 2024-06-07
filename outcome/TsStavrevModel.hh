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

#ifndef TsStavrevModel_hh
#define TsStavrevModel_hh

#include "TsVOutcomeModel.hh"

#include <vector>

class TsParameterManager;

class TsStavrevModel : public TsVOutcomeModel
{
public:
	TsStavrevModel(TsParameterManager* pM, G4String parName);
	~TsStavrevModel();

	G4double Initialize(std::vector<G4double> dose, std::vector<G4double> volume);
	G4double Function(G4double x);
	G4double mu_d(G4double x);
	G4double P_FSU(G4double x, G4double dose);

private:
	TsParameterManager* fPm;
	G4String fModelName;
	std::vector<G4double> fDose;
	std::vector<G4double> fVolume;
	G4double fN0;
	G4double fMu_cr;
	G4double fSigma_cr;
	G4double fAlpha;
	G4double fSigmaAlpha;
};
#endif
