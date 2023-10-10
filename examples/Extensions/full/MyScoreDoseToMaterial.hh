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

#ifndef MyScoreDoseToMaterial_hh
#define MyScoreDoseToMaterial_hh

#include "TsVBinnedScorer.hh"

#include "G4EmCalculator.hh"

class G4Material;

class MyScoreDoseToMaterial : public TsVBinnedScorer
{
public:
	MyScoreDoseToMaterial(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
					   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer);

	virtual ~MyScoreDoseToMaterial();

	G4bool ProcessHits(G4Step*,G4TouchableHistory*);

private:
	G4EmCalculator fEmCalculator;
	G4Material* fReferenceMaterial;
	G4ParticleDefinition* fProtonSubstituteForNeutrals;
	G4double fSubstituteForNeutralsEnergy;
};
#endif
