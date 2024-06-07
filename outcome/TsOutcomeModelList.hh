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

#ifndef TsOutcomeModelList_hh
#define TsOutcomeModelList_hh

#include <vector>
#include <map>
#include "globals.hh"

class TsParameterManager;
class TsExtensionManager;
class TsVOutcomeModel;

class TsOutcomeModelList
{
public:
	TsOutcomeModelList(TsParameterManager* pm, TsExtensionManager* em, G4String scorerName, const G4String& unitName);
	~TsOutcomeModelList();

	G4double Initialize(TsVOutcomeModel* mod, std::vector<G4double> dose, std::vector<G4double> volume, G4bool isDifDVH);
	std::map<G4String, G4double> CalculateOutcome(std::vector<G4double> dose, std::vector<G4double> volume, G4bool isDifDVH);

	void Print(G4String modelSource);

private:
	TsParameterManager* fPm;

	std::vector<TsVOutcomeModel*> fMod;
	std::vector<G4String> fModelName;
	std::map<G4String, G4double> fOutcome;
	G4String fUnitName;
	G4String fScorerName;
	G4bool fCorrectForDoseFractionation;
	G4int fNbOfFractions;
	G4double fDosePerFraction;
	G4double fAlphaOverBeta;
	G4double fgEUDa_1;
};
#endif
