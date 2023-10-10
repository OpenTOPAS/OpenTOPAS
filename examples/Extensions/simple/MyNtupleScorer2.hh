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

#ifndef MyNtupleScorer2_hh
#define MyNtupleScorer2_hh

#include "TsVNtupleScorer.hh"

class MyNtupleScorer2 : public TsVNtupleScorer
{
public:
    MyNtupleScorer2(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
                G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer);
    
    virtual ~MyNtupleScorer2();

    G4bool ProcessHits(G4Step*,G4TouchableHistory*);

private:
	// Output variables
	G4float fPosX;
	G4float fPosY;
	G4float fPosZ;
	G4float fCosX;
	G4float fCosY;
	G4bool fCosZIsNegative;
	G4float fEnergy;
	G4float fWeight;
	G4int fPType;
	G4String fOriginProcessName;
	G4String fLastVolumeName;
};
#endif
