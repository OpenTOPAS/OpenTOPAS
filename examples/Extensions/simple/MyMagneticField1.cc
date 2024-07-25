// Magnetic Field for MyMagneticField1
//
// ********************************************************************
// *                                                                  *
// * Copyright 2024 The TOPAS Collaboration                           *
// * Copyright 2022 The TOPAS Collaboration                           *
// *                                                                  *
// * Permission is hereby granted, free of charge, to any person      *
// * obtaining a copy of this software and associated documentation   *
// * files (the "Software"), to deal in the Software without          *
// * restriction, including without limitation the rights to use,     *
// * copy, modify, merge, publish, distribute, sublicense, and/or     *
// * sell copies of the Software, and to permit persons to whom the   *
// * Software is furnished to do so, subject to the following         *
// * conditions:                                                      *
// *                                                                  *
// * The above copyright notice and this permission notice shall be   *
// * included in all copies or substantial portions of the Software.  *
// *                                                                  *
// * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,  *
// * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES  *
// * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND         *
// * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT      *
// * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,     *
// * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
// * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR    *
// * OTHER DEALINGS IN THE SOFTWARE.                                  *
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
