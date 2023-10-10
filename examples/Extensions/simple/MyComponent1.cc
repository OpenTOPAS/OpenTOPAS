// Component for MyComponent1
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

#include "MyComponent1.hh"

#include "TsParameterManager.hh"

#include "G4Box.hh"

MyComponent1::MyComponent1(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
			 TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name) :
TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name)
{
}


MyComponent1::~MyComponent1()
{
}


G4VPhysicalVolume* MyComponent1::Construct()
{
	BeginConstruction();

	G4double totalHLX = fPm->GetDoubleParameter(GetFullParmName("HLX"), "Length");
	G4double totalHLY = fPm->GetDoubleParameter(GetFullParmName("HLY"), "Length");
	G4double totalHLZ = fPm->GetDoubleParameter(GetFullParmName("HLZ"), "Length");

	G4Box* envelopeSolid = new G4Box(fName, totalHLX, totalHLY, totalHLZ);
	fEnvelopeLog = CreateLogicalVolume(envelopeSolid);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);
	
	InstantiateChildren(fEnvelopePhys);
	
	return fEnvelopePhys;
}
