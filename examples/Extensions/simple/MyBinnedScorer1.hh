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

#ifndef MyBinnedScorer1_hh
#define MyBinnedScorer1_hh

#include "TsVBinnedScorer.hh"

class MyBinnedScorer1 : public TsVBinnedScorer
{
public:
	MyBinnedScorer1(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
				G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer);

	virtual ~MyBinnedScorer1();

	G4bool ProcessHits(G4Step*,G4TouchableHistory*);
};
#endif
