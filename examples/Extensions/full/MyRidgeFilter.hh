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

#ifndef MyRidgeFilter_hh
#define MyRidgeFilter_hh

#include "TsVGeometryComponent.hh"

class MyRidgeFilter : public TsVGeometryComponent {
public:
	MyRidgeFilter(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
				TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name);
	~MyRidgeFilter();

	G4VPhysicalVolume* Construct();
};

#endif
