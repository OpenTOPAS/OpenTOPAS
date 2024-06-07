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

#include "TsParameterisation.hh"

#include "TsVGeometryComponent.hh"

#include "G4VVolumeMaterialScanner.hh"

TsParameterisation::TsParameterisation(TsVGeometryComponent* component):G4VPVParameterisation(), G4VVolumeMaterialScanner(),
fComponent(component)
{}

TsParameterisation::~TsParameterisation() {}

G4Material* TsParameterisation::ComputeMaterial(const G4int repNo, G4VPhysicalVolume* pvol, const G4VTouchable* parent)
{
	return fComponent->ComputeMaterial(repNo, pvol, parent);
}


G4VVolumeMaterialScanner* TsParameterisation::GetMaterialScanner()
{
   return this;
}


G4int TsParameterisation::GetNumberOfMaterials() const
{
	return fComponent->GetNumberOfMaterials();
}


G4Material* TsParameterisation::GetMaterial(G4int i) const
{
	return fComponent->GetMaterialInVoxel(i);
}


void TsParameterisation::ComputeTransformation(const G4int copyNo, G4VPhysicalVolume* pvol) const
{
	fComponent->ComputeTransformation(copyNo, pvol);
}



void TsParameterisation::ComputeDimensions(G4Tubs& tubs, const G4int copyNo, const G4VPhysicalVolume* pvol) const
{
	fComponent->ComputeDimensions(tubs, copyNo, pvol);
}


void TsParameterisation::ComputeDimensions(G4Sphere& sphere, const G4int copyNo, const G4VPhysicalVolume* pvol) const
{
	fComponent->ComputeDimensions(sphere, copyNo, pvol);
}

G4bool TsParameterisation::IsNested() const
{
	return true;
}
