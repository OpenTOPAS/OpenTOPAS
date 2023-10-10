//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * This file was obtained from Topas MC Inc under the license       *
// * agreement set forth at http://www.topasmc.org/registration       *
// * Any use of this file constitutes full acceptance of              *
// * this TOPAS MC license agreement.                                 *
// *                                                                  *
// ********************************************************************
//

#ifndef MyScoreDoseToWater_hh
#define MyScoreDoseToWater_hh

#include "TsVBinnedScorer.hh"

#include "G4EmCalculator.hh"

class G4Material;

class MyScoreDoseToWater : public TsVBinnedScorer
{
public:
	MyScoreDoseToWater(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
					   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer);

	virtual ~MyScoreDoseToWater();

	G4bool ProcessHits(G4Step*,G4TouchableHistory*);

private:
	G4EmCalculator fEmCalculator;
	G4Material* fWater;
	G4ParticleDefinition* fProtonSubstituteForNeutrals;
	G4double fSubstituteForNeutralsEnergy;
};
#endif
