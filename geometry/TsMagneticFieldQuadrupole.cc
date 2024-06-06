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

#include "TsMagneticFieldQuadrupole.hh"

#include "TsParameterManager.hh"

#include "TsVGeometryComponent.hh"

#include "G4TransportationManager.hh"

TsMagneticFieldQuadrupole::TsMagneticFieldQuadrupole(TsParameterManager* pM, TsGeometryManager* gM,
													 TsVGeometryComponent* component):
TsVMagneticField(pM, gM, component) {
	ResolveParameters();
}

TsMagneticFieldQuadrupole::~TsMagneticFieldQuadrupole() {;}


void TsMagneticFieldQuadrupole::ResolveParameters() {
	fGradientX = fPm->GetDoubleParameter(fComponent->GetFullParmName("MagneticFieldGradientX"), "magnetic field gradient");
	fGradientY = fPm->GetDoubleParameter(fComponent->GetFullParmName("MagneticFieldGradientY"), "magnetic field gradient");

	TsVMagneticField::ResolveParameters();
}


void TsMagneticFieldQuadrupole::GetFieldValue(const G4double Point[3], G4double* Field) const{
	const G4ThreeVector localPoint = fNavigator->GetGlobalToLocalTransform().TransformPoint(G4ThreeVector(Point[0],Point[1],Point[2]));

	G4ThreeVector B_local = G4ThreeVector(fGradientX * localPoint.y(),
										  fGradientY * localPoint.x(),
										  0);

	G4ThreeVector B_global = G4ThreeVector(fComponent->GetRotRelToWorld()->inverse().rowX() * B_local,
										   fComponent->GetRotRelToWorld()->inverse().rowY() * B_local,
										   fComponent->GetRotRelToWorld()->inverse().rowZ() * B_local);

	Field[0] = B_global.x() ;
	Field[1] = B_global.y() ;
	Field[2] = B_global.z() ;
}
