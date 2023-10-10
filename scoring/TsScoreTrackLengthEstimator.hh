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

#ifndef TsScoreTrackLengthEstimator_hh
#define TsScoreTrackLengthEstimator_hh

#include "TsVBinnedScorer.hh"

class TsScoreTrackLengthEstimator : public TsVBinnedScorer
{
public:
	TsScoreTrackLengthEstimator(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
				   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer);

	virtual ~TsScoreTrackLengthEstimator();

	G4bool ProcessHits(G4Step*,G4TouchableHistory*);

private:
	G4double GetEnergyAbsorptionCoeffForMaterial(G4Material* material, G4double ekin);
	G4double LinLogLogInterpolate(G4double, G4int);

private:
	std::map<G4int, std::vector<G4double> > fEnergyValuesPerAtomicNumber;
	std::map<G4int, std::vector<G4double> > fEnergyAbsorptionCoefficientsPerAtomicNumber;
};
#endif
