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
#include "TsElectroMagneticFieldUniform.hh"

#include "TsParameterManager.hh"

#include "TsVGeometryComponent.hh"

#include "G4TransportationManager.hh"

TsElectroMagneticFieldUniform::TsElectroMagneticFieldUniform(TsParameterManager* pM, TsGeometryManager* gM,
											 TsVGeometryComponent* component):
TsVElectroMagneticField(pM, gM, component) {
	ResolveParameters();
}

TsElectroMagneticFieldUniform::~TsElectroMagneticFieldUniform() {;}


void TsElectroMagneticFieldUniform::ResolveParameters() {
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


void TsElectroMagneticFieldUniform::GetFieldValue(const G4double[4], G4double *fieldBandE) const{
	fieldBandE[0]= fMagneticFieldValue.x();
	fieldBandE[1]= fMagneticFieldValue.y();
	fieldBandE[2]= fMagneticFieldValue.z();
	fieldBandE[3]= fElectricFieldValue.x();
	fieldBandE[4]= fElectricFieldValue.y();
	fieldBandE[5]= fElectricFieldValue.z();
}
