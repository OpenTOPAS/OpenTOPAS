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

#include "TsScorePET.hh"
#include "TsParameterManager.hh"
#include "TsMaterialManager.hh"
#include "TsGeometryManager.hh"

#include "G4Material.hh"
#include "G4ParticleDefinition.hh"
#include "G4Proton.hh"

TsScorePET::TsScorePET(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
					   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer),
fPETCrossSectionDataNames(0), fNumberOfCrossSectionData(0)
{
	SetUnit("");
	G4cout << "WARNING: TOPAS PET scorer currently only supported for proton beams." << G4endl;

	fPETCrossSectionDataNames = fPm->GetStringVector(GetFullParmName("PETCrossSectionDataNames"));
	fNumberOfCrossSectionData = fPm->GetVectorLength(GetFullParmName("PETCrossSectionDataNames"));

	fForceUseHighestEnergyBin = false;
	if (fPm->ParameterExists(GetFullParmName("ForceUseOfHighestEnergyCrossSection"))){
		fForceUseHighestEnergyBin = fPm->GetBooleanParameter(GetFullParmName("ForceUseOfHighestEnergyCrossSection"));
	}

	for (G4int i=0; i<fNumberOfCrossSectionData; i++){

		G4String elementSymbol = fPETCrossSectionDataNames[i] + "/ElementSymbol";
		fElementName.push_back(fPm->GetStringParameter(GetFullParmName(elementSymbol)));

		G4String binSize = fPETCrossSectionDataNames[i] + "/CrossSectionDataBinSize";
		G4double* tmp = fPm->GetDoubleVector(GetFullParmName(binSize), "Energy");
		for (G4int itmp=0; itmp<fPm->GetVectorLength(GetFullParmName(binSize)); itmp++)
			fCrossSectionDataBinSize.push_back(tmp[itmp]);

		G4String data = fPETCrossSectionDataNames[i] + "/CrossSectionData";
		std::vector<G4double> tmp1;
		G4double* tmpData = fPm->GetDoubleVector(GetFullParmName(data), "Surface");
		for (G4int itmp=0; itmp<fPm->GetVectorLength(GetFullParmName(data)); itmp++)
			tmp1.push_back(tmpData[itmp]);
		fCrossSectionData.push_back(tmp1);

		G4String abundanceFraction = fPETCrossSectionDataNames[i] + "/AbundanceFraction";
		fAbundanceFraction.push_back(fPm->GetUnitlessParameter(GetFullParmName(abundanceFraction)));

		G4String gramsPerMole = fPETCrossSectionDataNames[i] + "/GramsPerMole";
		fGramsPerMole.push_back(fPm->GetDoubleParameter(GetFullParmName(gramsPerMole), "Mass"));
	}
}


TsScorePET::~TsScorePET() {;}


G4bool TsScorePET::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	ResolveSolid(aStep);

	G4ParticleDefinition* particle = aStep->GetTrack()->GetDefinition();
	if ( particle == G4Proton::ProtonDefinition() ) {

		G4double volume = GetCubicVolume(aStep);
		G4double stepLength = aStep->GetStepLength();
		G4double kinEnergy = aStep->GetTrack()->GetKineticEnergy();

		G4Material* material = aStep->GetPreStepPoint()->GetMaterial();
		G4String materialName = aStep->GetPreStepPoint()->GetMaterial()->GetName();
		G4double density = aStep->GetPreStepPoint()->GetMaterial()->GetDensity();
		G4int numberOfElements = material->GetNumberOfElements();

		const G4double * massFractions = material->GetFractionVector();

		int hasDataFlag = 0;
		for (G4int iter=0; iter<fNumberOfCrossSectionData; iter++){

			for (G4int iElement=0; iElement<numberOfElements; iElement++)
			{
				G4String elementName = material->GetElement(iElement)->GetName();

				if (elementName == fElementName[iter]) {
					G4int crossSectionBin = (G4int)floor(kinEnergy/fCrossSectionDataBinSize[iter]);
					if (!fForceUseHighestEnergyBin && kinEnergy > fCrossSectionData[iter].size()*fCrossSectionDataBinSize[iter]) {
						G4cout << "TOPAS is exiting due to an error in PET scoring." << G4endl;
						G4cout << "The energy of a " << aStep->GetTrack()->GetDefinition()->GetParticleName() <<
							" was " <<  kinEnergy << " which is larger than the maximum energy in the cross section tables provided." << G4endl;
						G4cout << "Please adjust the cross section tables or set scorer to \"ForceUseOfHighestEnergyCrossSection\"" << G4endl;
						fPm->AbortSession(1);
					}
					G4double nAtoms = 6.02214179E23*density * massFractions[iElement]*(volume)/(fGramsPerMole[iter]);
					G4double petProduction = stepLength*nAtoms*fAbundanceFraction[iter]*fCrossSectionData[iter][crossSectionBin]/volume;
					petProduction *= aStep->GetPreStepPoint()->GetWeight();
					AccumulateHit(aStep, petProduction);
					hasDataFlag++;
				}
			}
		}
		if (hasDataFlag != 0) return true;

	}
	return false;
}

