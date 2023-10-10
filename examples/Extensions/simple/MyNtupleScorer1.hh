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

#ifndef MyNtupleScorer1_hh
#define MyNtupleScorer1_hh

#include "TsVNtupleScorer.hh"

class MyNtupleScorer1 : public TsVNtupleScorer
{
public:
	MyNtupleScorer1(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
				G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer);

	virtual ~MyNtupleScorer1();

	G4bool ProcessHits(G4Step*,G4TouchableHistory*);

protected:
	// Output variables
	G4double fEnergy;
	G4float fWeight;
	G4int fParticleType;
	G4bool fIsNewHistory;

	G4int fRunID;
	G4int fEventID;
	G4int fPrevRunID;
	G4int fPrevEventID;
};
#endif
