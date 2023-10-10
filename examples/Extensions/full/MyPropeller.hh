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

#ifndef MyPropeller_hh
#define MyPropeller_hh

#include "TsVGeometryComponent.hh"

#include "G4VPVParameterisation.hh"

class MyPropeller : public TsVGeometryComponent
{
public:
	MyPropeller(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, 
				TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name);
	~MyPropeller();
	
	G4VPhysicalVolume* Construct();
};

#endif
