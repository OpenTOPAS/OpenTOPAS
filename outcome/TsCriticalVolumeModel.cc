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

#include "TsCriticalVolumeModel.hh"
#include "G4PhysicalConstants.hh"

#include <vector>

TsCriticalVolumeModel::TsCriticalVolumeModel(TsParameterManager* pM, G4String parName)
: TsVOutcomeModel(pM, parName), fPm(pM), fModelName("CriticalVolume")
{
	if ( fPm->ParameterExists( GetFullParmName(fModelName, "UsePresetParametersFromOrganNamed") ) ) {
		G4String organName = fPm->GetStringParameter( GetFullParmName(fModelName, "UsePresetParametersFromOrganNamed"));
		G4String valueParmName = "Sc/CriticalVolumeValues/" + organName;
		if ( fPm->ParameterExists(valueParmName) ) {
			G4double* parameters = fPm->GetUnitlessVector(valueParmName);
			fMucr = parameters[0];
			fSigma = parameters[2];
			fTD50 = parameters[4];
			fGamma50 = parameters[6];
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring for outcome setup." << G4endl;
			G4cerr << organName << " refers to an unknown organ in database" << G4endl;
			fPm->AbortSession(1);
		}
	} else {
		fMucr = fPm->GetUnitlessParameter(GetFullParmName(fModelName, "mu_cr"));
		fSigma = fPm->GetUnitlessParameter(GetFullParmName(fModelName, "sigma_mu_cr"));
		fTD50 = fPm->GetUnitlessParameter(GetFullParmName(fModelName, "D50FSU"));
		fGamma50 = fPm->GetUnitlessParameter(GetFullParmName(fModelName, "Gamma50FSU"));
	}
}


TsCriticalVolumeModel::~TsCriticalVolumeModel() {;}


G4double TsCriticalVolumeModel::Initialize(std::vector<G4double> dose, std::vector<G4double> volume) {
	G4double mud = 0.0;
	G4double argument;
	G4double totalVolume = SumElementsOfVector(volume);

	fSigma /= -(fMucr*log(fMucr));
	// Extension of Niemerko and Goitein paper 1992, Int J. Radiation Oncology Biol. Phys. 25, 135-145
	// Based on Warkentin et. al. Journal of Applied Clinical Physics, 5(1), 50-63. (2004); and
	// Stavrev et. al. Phys. Med. Biol. 46, 1501-1518, 2001.
	for ( int i = 0; i < int(dose.size()); i++ ) {
		argument = sqrt(2*pi) * fGamma50 * log(dose[i]/fTD50);
		mud += (volume[i]/totalVolume) * CalculateProbit(argument);
	}

	G4double NTCP = CalculateProbit((-log(-log(mud)) + log(-log(fMucr)))/fSigma);
	return NTCP*100;
}
