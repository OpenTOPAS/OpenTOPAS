//
// ********************************************************************
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// *               Medical Physics Group                              *
// *                  Laval University                                *
// *             Contact - fberumenm@gmail.com                        *
// *      https://doi.org/10.1016/j.brachy.2020.12.007                *
// *                                                                  *
// ********************************************************************
//

#include "TsScoreTrackLengthEstimator.hh"

#include "G4ElementVector.hh"
#include "G4Material.hh"

#include <fstream>

TsScoreTrackLengthEstimator::TsScoreTrackLengthEstimator(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
							   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("Gy");

	G4String muEnFileName = fPm->GetStringParameter(GetFullParmName("InputFile"));
	std::ifstream inputFile(muEnFileName);

	if ( !inputFile ) {
		G4cerr << "TOPAS is exiting due a serious error in scoring." << G4endl;
		G4cerr << GetFullParmName("InputFile") << G4endl;
		G4cerr << "Refers to an input file that could not be found or could not be opened:" << G4endl;
		G4cerr << muEnFileName << G4endl;
		fPm->AbortSession(1);
	}
	
	G4int i = 0, n, N;
	G4double muEnergy, dummy, muEnValue;

	while (inputFile >> n >> N) {
	  i = 0;
	  while (i < N) {
		inputFile >> muEnergy >> dummy >> muEnValue;
		muEnergy *= MeV;
		muEnValue *= cm*cm/g;
		fEnergyValuesPerAtomicNumber[n].push_back(muEnergy);
		fEnergyAbsorptionCoefficientsPerAtomicNumber[n].push_back(muEnValue);
		i++;
	  }
	}
	
	inputFile.close();
}


TsScoreTrackLengthEstimator::~TsScoreTrackLengthEstimator() {;}


G4double TsScoreTrackLengthEstimator::GetEnergyAbsorptionCoeffForMaterial(G4Material* material,
																		  G4double ekin) {
	const G4ElementVector *elementVector = material->GetElementVector();
	const G4double *weightFractions = material->GetFractionVector();
	G4int nbOfElements = material->GetNumberOfElements();
	G4double muEn = 0.0;

	for ( int i = 0; i < nbOfElements; i++ ){
		G4int Z = G4int((*elementVector)[i]->GetZ());
		muEn += weightFractions[i] * LinLogLogInterpolate(ekin, Z); 
	}

	return muEn;
}


G4double TsScoreTrackLengthEstimator::LinLogLogInterpolate(G4double x, G4int index) {
	G4double value = 0.0;
	if ( x < fEnergyValuesPerAtomicNumber[index].front() ) {
		return value;
	} else if ( x > fEnergyValuesPerAtomicNumber[index].back() ) {
		return fEnergyAbsorptionCoefficientsPerAtomicNumber[index].back();
	} else {
		G4int bin = G4int(upper_bound(fEnergyValuesPerAtomicNumber[index].begin(),
									  fEnergyValuesPerAtomicNumber[index].end(), x)-
						  fEnergyValuesPerAtomicNumber[index].begin())-1;
		G4double e1 = fEnergyValuesPerAtomicNumber[index][bin];
		G4double e2 = fEnergyValuesPerAtomicNumber[index][bin+1];
		G4double d1 = fEnergyAbsorptionCoefficientsPerAtomicNumber[index][bin];
		G4double d2 = fEnergyAbsorptionCoefficientsPerAtomicNumber[index][bin+1];

		if ( d1 > 0 && d2 > 0 )
			value = std::pow(10., (std::log10(d1) * std::log10(e2 / x) +
								   std::log10(d2) * std::log10(x / e1)) / std::log10(e2 / e1));
		else
			value = (d1 * std::log10(e2 / x) + d2 * std::log10(x / e1)) / std::log10(e2 / e1);

	}
	return value;
}


G4bool TsScoreTrackLengthEstimator::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}
	
	G4double stepLength = aStep->GetStepLength();

	if ( stepLength > 0 ) {

		ResolveSolid(aStep);
		
		G4double kineticEnergy = aStep->GetPreStepPoint()->GetKineticEnergy();
		G4double quantity = GetEnergyAbsorptionCoeffForMaterial(aStep->GetPreStepPoint()->GetMaterial(),
																kineticEnergy) * kineticEnergy;
		G4double volume = fSolid->GetCubicVolume();
		quantity *= stepLength * aStep->GetPreStepPoint()->GetWeight() / volume;
		
		AccumulateHit(aStep, quantity);
		
		return true;
	}
	return false;
}
