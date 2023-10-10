// ElectroMagnetic Field for MyElectroMagneticField1
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
#include "MyElectroMagneticField1.hh"

#include "TsParameterManager.hh"

#include "TsVGeometryComponent.hh"

#include "G4TransportationManager.hh"

MyElectroMagneticField1::MyElectroMagneticField1(TsParameterManager* pM, TsGeometryManager* gM,
											 TsVGeometryComponent* component):
TsVElectroMagneticField(pM, gM, component) {
	ResolveParameters();
}

MyElectroMagneticField1::~MyElectroMagneticField1() {;}


// This is currently just a copy of our Uniform field code.
// Adjust this to do whatever you want.

void MyElectroMagneticField1::ResolveParameters() {
	G4ThreeVector bDirection;
	bDirection.setX(fPm->GetUnitlessParameter(fComponent->GetFullParmName("MagneticFieldDirectionX")));
	bDirection.setY(fPm->GetUnitlessParameter(fComponent->GetFullParmName("MagneticFieldDirectionY")));
	bDirection.setZ(fPm->GetUnitlessParameter(fComponent->GetFullParmName("MagneticFieldDirectionZ")));
	bDirection.unit();
	
	G4double bStrength = fPm->GetDoubleParameter(fComponent->GetFullParmName("MagneticFieldStrength"), "Magnetic flux density");
	fMagneticFieldValue.setX(bStrength * fComponent->GetEnvelopePhysicalVolume()->GetRotation()->colX() * bDirection);
	fMagneticFieldValue.setY(bStrength * fComponent->GetEnvelopePhysicalVolume()->GetRotation()->colY() * bDirection);
	fMagneticFieldValue.setZ(bStrength * fComponent->GetEnvelopePhysicalVolume()->GetRotation()->colZ() * bDirection);
	
	G4ThreeVector eDirection;
	eDirection.setX(fPm->GetUnitlessParameter(fComponent->GetFullParmName("ElectricFieldDirectionX")));
	eDirection.setY(fPm->GetUnitlessParameter(fComponent->GetFullParmName("ElectricFieldDirectionY")));
	eDirection.setZ(fPm->GetUnitlessParameter(fComponent->GetFullParmName("ElectricFieldDirectionZ")));
	eDirection.unit();
	
	G4double eStrength = fPm->GetDoubleParameter(fComponent->GetFullParmName("ElectricFieldStrength"), "electric field strength");
	fElectricFieldValue.setX(eStrength * fComponent->GetEnvelopePhysicalVolume()->GetRotation()->colX() * eDirection);
	fElectricFieldValue.setY(eStrength * fComponent->GetEnvelopePhysicalVolume()->GetRotation()->colY() * eDirection);
	fElectricFieldValue.setZ(eStrength * fComponent->GetEnvelopePhysicalVolume()->GetRotation()->colZ() * eDirection);
	
	TsVElectroMagneticField::ResolveParameters();
}


void MyElectroMagneticField1::GetFieldValue(const G4double[4], G4double *fieldBandE) const{
	fieldBandE[0]= fMagneticFieldValue.x();
	fieldBandE[1]= fMagneticFieldValue.y();
	fieldBandE[2]= fMagneticFieldValue.z();
	fieldBandE[3]= fElectricFieldValue.x();
	fieldBandE[4]= fElectricFieldValue.y();
	fieldBandE[5]= fElectricFieldValue.z();
}
