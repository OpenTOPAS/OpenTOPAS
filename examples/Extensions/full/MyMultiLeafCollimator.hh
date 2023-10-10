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

#ifndef MyMultiLeafCollimator_hh
#define MyMultiLeafCollimator_hh

#include "TsVGeometryComponent.hh"

class MyMultiLeafCollimator : public TsVGeometryComponent
{
public:
	MyMultiLeafCollimator(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
							TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name);
	~MyMultiLeafCollimator();

	G4VPhysicalVolume* Construct();
	void UpdateForSpecificParameterChange(G4String parameter);

private:
	 G4double  LeafHalfLength;
	 G4double  MaximumLeafOpen;
	 G4double* XPlusLeavesOpen;
	 G4double* XMinusLeavesOpen;
	 std::vector<G4VPhysicalVolume*> XPlusLeaves;
	 std::vector<G4VPhysicalVolume*> XMinusLeaves;
	 G4int NbOfLeavesPerSide;
};

#endif
