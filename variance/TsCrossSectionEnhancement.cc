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

#include "TsCrossSectionEnhancement.hh"
#include "TsGeometryManager.hh"
#include "TsParameterManager.hh"

#include "G4UImanager.hh"
#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"
#include "globals.hh"

#include <vector>

TsCrossSectionEnhancement::TsCrossSectionEnhancement(G4String name, TsParameterManager* pM, TsGeometryManager* gM)
: TsVBiasingProcess(name, pM, gM), fPm(pM), fName(name)
{
	ResolveParameters();
}


TsCrossSectionEnhancement::~TsCrossSectionEnhancement()
{;}


void TsCrossSectionEnhancement::ResolveParameters() {
	G4String* processes = fPm->GetStringVector(GetFullParmName("ProcessesNamed"));
	G4double* factors = fPm->GetUnitlessVector(GetFullParmName("EnhancementFactor"));
	G4int nbOfProcesses = fPm->GetVectorLength(GetFullParmName("ProcessesNamed"));
	
	if ( nbOfProcesses != fPm->GetVectorLength(GetFullParmName("EnhancementFactor")))
		Quit(GetFullParmName("ProcessesNamed"),
			 "size does not match with size of parameter named: " +
			 GetFullParmName("EnhancementFactor"));
	
	for ( int i = 0; i < nbOfProcesses; i++) {
		fProcesses.push_back(processes[i]);
		fFactors.push_back(factors[i]);
	}
}


void TsCrossSectionEnhancement::Initialize() {
	SetCrossSectionEnhancement();
}


void TsCrossSectionEnhancement::SetCrossSectionEnhancement() {
	for ( size_t u = 0; u < fProcesses.size(); u++) {
		G4UImanager::GetUIpointer()->ApplyCommand("/process/em/setBiasingFactor " +
												  fProcesses[u] + " " +
												  G4UIcommand::ConvertToString(fFactors[u]) +
												  " true ");
		G4cout << "Cross-section enhancement of process " << fProcesses[u]
		       << " by a factor of " << fFactors[u] << G4endl;
	}
}


void TsCrossSectionEnhancement::Clear() {
	fProcesses.clear();
	fFactors.clear();
}
