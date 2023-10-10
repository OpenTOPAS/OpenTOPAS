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

#ifndef MyMultiWireChamber_hh
#define MyMultiWireChamber_hh

#include "TsVGeometryComponent.hh"

class MyMultiWireChamber : public TsVGeometryComponent
{
public:
	MyMultiWireChamber(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
					   TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name);
	~MyMultiWireChamber();

	G4VPhysicalVolume* Construct();
};

#endif
