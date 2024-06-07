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

#include "TsPixelatedBox.hh"

#include "TsParameterManager.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4UIcommand.hh"

TsPixelatedBox::TsPixelatedBox(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
             TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name) :
TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name)
{
    ResolveParameters();
}


TsPixelatedBox::~TsPixelatedBox()
{;}


void TsPixelatedBox::ResolveParameters() {
    fLZ = fPm->GetDoubleParameter(GetFullParmName("PixelSizeZ"),"Length");
    fLX = fPm->GetDoubleParameter(GetFullParmName("PixelSizeX"),"Length");
    fLY = fPm->GetDoubleParameter(GetFullParmName("PixelSizeY"),"Length");
    fPitchX = fPm->GetDoubleParameter(GetFullParmName("PitchX"), "Length");
    fPitchY = fPm->GetDoubleParameter(GetFullParmName("PitchY"), "Length");
    fnX = fPm->GetIntegerParameter(GetFullParmName("NumberOfPixelsX"));
    fnY = fPm->GetIntegerParameter(GetFullParmName("NumberOfPixelsY"));
    
    fSeparationX = fPitchX - fLX;
    fSeparationY = fPitchY - fLY;
    if ( fSeparationX < 0 || fSeparationY < 0 ) {
        G4cerr << "TOPAS is exiting due to an error geometry setup." << G4endl;
        G4cerr << "TsPixelatedBox Component name: " << fName << G4endl;
        G4cerr << "Pitch must be larger than the pixel size." << G4endl;
        fPm->AbortSession(1);
    }
}


G4VPhysicalVolume* TsPixelatedBox::Construct()
{
    BeginConstruction();
    
    G4double arrayLengthX = fnX * fLX + (fnX + 1 ) * fSeparationX;
    G4double arrayLengthY = fnY * fLY + (fnY + 1 ) * fSeparationY;
    
    G4Box* envelopeSolid = new G4Box(fName, 0.5 * arrayLengthX, 0.5 * arrayLengthY, 0.5 * fLZ);
    
    fEnvelopeLog = CreateLogicalVolume(envelopeSolid);
    fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);
    
    G4Box* pixel = new G4Box(fName + "_pixel", 0.5 * fLX, 0.5 * fLY, 0.5 * fLZ);
    G4LogicalVolume* pixelLog = CreateLogicalVolume("Pixel", pixel);
    
    G4int index = 1;
    for ( int i = 1; i <= fnX; i++ ) {
        G4double x = -0.5 * (fnX-1) * fPitchX + (i - 1) * fPitchX;
        for ( int j = 1; j <= fnY; j++ ) {
            G4double y = -0.5 * (fnY-1) * fPitchY + (j - 1) * fPitchY;
            G4String volumeName = fName + "_pixel";
            CreatePhysicalVolume(volumeName, index, true, pixelLog, 0, new G4ThreeVector(x,y,0), fEnvelopePhys);
            index++;
        }
    }
    
    InstantiateChildren(fEnvelopePhys);
    
    return fEnvelopePhys;
}
