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

#ifndef MyComponent1_hh
#define MyComponent1_hh

#include "TsVGeometryComponent.hh"

class MyComponent1 : public TsVGeometryComponent
{    
public:
	MyComponent1(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
				  TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name);
	~MyComponent1();
	
	G4VPhysicalVolume* Construct();
};

#endif
