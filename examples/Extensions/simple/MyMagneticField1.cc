// Magnetic Field for MyMagneticField1
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
#include "MyMagneticField1.hh"

#include "TsParameterManager.hh"

#include "TsVGeometryComponent.hh"

#include "G4TransportationManager.hh"

MyMagneticField1::MyMagneticField1(TsParameterManager* pM, TsGeometryManager* gM,
											 TsVGeometryComponent* component):
TsVMagneticField(pM, gM, component) {
	ResolveParameters();
}

MyMagneticField1::~MyMagneticField1() {;}


// This is currently just a copy of our dipole field code.
// Adjust this to do whatever you want.

void MyMagneticField1::ResolveParameters() {
	G4ThreeVector direction;
	direction.setX(fPm->GetUnitlessParameter(fComponent->GetFullParmName("MagneticFieldDirectionX")));
	direction.setY(fPm->GetUnitlessParameter(fComponent->GetFullParmName("MagneticFieldDirectionY")));
	direction.setZ(fPm->GetUnitlessParameter(fComponent->GetFullParmName("MagneticFieldDirectionZ")));
	direction.unit();

	G4double strength = fPm->GetDoubleParameter(fComponent->GetFullParmName("MagneticFieldStrength"), "Magnetic flux density");
	fFieldValue.setX(strength * fComponent->GetEnvelopePhysicalVolume()->GetRotation()->colX() * direction);
	fFieldValue.setY(strength * fComponent->GetEnvelopePhysicalVolume()->GetRotation()->colY() * direction);
	fFieldValue.setZ(strength * fComponent->GetEnvelopePhysicalVolume()->GetRotation()->colZ() * direction);
}


void MyMagneticField1::GetFieldValue(const G4double Point[3], G4double* Field) const{
	// This point can be useful if your field depends on location within the component
	const G4ThreeVector localPoint = fNavigator->GetGlobalToLocalTransform().TransformPoint(G4ThreeVector(Point[0],Point[1],Point[2]) );

	Field[0] = fFieldValue.x();
	Field[1] = fFieldValue.y();
	Field[2] = fFieldValue.z();
}
