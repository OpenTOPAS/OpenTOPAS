// Component for MyMultiWireChamber
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

#include "MyMultiWireChamber.hh"

#include "TsParameterManager.hh"

#include "G4LogicalVolume.hh"
#include "G4UIcommand.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4SystemOfUnits.hh"

MyMultiWireChamber::MyMultiWireChamber(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
									   TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name):
TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name)
{;}


MyMultiWireChamber::~MyMultiWireChamber() {;}

G4VPhysicalVolume* MyMultiWireChamber::Construct()
{
	BeginConstruction();

	const G4int NbOfLayers      = fPm->GetIntegerParameter(GetFullParmName("NbOfLayers"));

	//Wire chamber Box
	G4Box* gWireChamber = new G4Box(fName,
									fPm->GetDoubleParameter(GetFullParmName("HLX"), "Length"),
									fPm->GetDoubleParameter(GetFullParmName("HLY"), "Length"),
									fPm->GetDoubleParameter(GetFullParmName("HLZ"), "Length"));
	fEnvelopeLog = CreateLogicalVolume(gWireChamber);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

	for (G4int i = 0; i < NbOfLayers; ++i) {
		G4String  iLayer  	   = "Layer" + G4UIcommand::ConvertToString(i+1);
		G4int     NbWires 	   = fPm->GetVectorLength(GetFullParmName(iLayer, "Displacement"));
		G4double* Displacement = fPm->GetDoubleVector(GetFullParmName(iLayer, "Displacement"), "Length");
		G4double  PosX 		   = 0.0;
		G4double  PosY 		   = 0.0;
		G4double  PosZ	       = fPm->GetDoubleParameter(GetFullParmName(iLayer, "PosZ"), "Length");
		G4RotationMatrix* AlignMatrix = new G4RotationMatrix;
		G4bool    IsAlignX 			  = false;
		G4String  Align 			  = fPm->GetStringParameter(GetFullParmName(iLayer, "Align"));
		G4double  Rmin = fPm->GetDoubleParameter( GetFullParmName(iLayer, "RMin"), "Length");
		G4double  Rmax = fPm->GetDoubleParameter( GetFullParmName(iLayer, "RMax"), "Length");
		G4double  HL   = fPm->GetDoubleParameter( GetFullParmName(iLayer, "HL"), "Length");
		if ( fPm->ParameterExists(GetFullParmName("PrintInformation")) && fPm->GetBooleanParameter(GetFullParmName("PrintInformation")) ) {
			G4cout<<" Layer: \"" << i <<"\" , # of Wires: "<< NbWires << ", Alignment: "<< Align
			<<", Wire (Rmin= "<< Rmin/cm <<" cm, Rmax= "<< Rmax/cm <<" cm, HL= "<< HL/cm<<" cm)"
			<<" , Z Position in the Chamber: "<<PosZ/cm<< " (cm)" <<G4endl;
		}
		//Wire specification
		G4Tubs* gwire = new G4Tubs(iLayer, Rmin, Rmax, HL, 0.0*deg, 360.0*deg);
		G4LogicalVolume* lwire = CreateLogicalVolume( iLayer, gwire);

		//Alignment flags of Wires
		if ( Align == "X" || Align == "x") {
			AlignMatrix->rotateX(0.0);
			AlignMatrix->rotateY(90.0*deg);
			AlignMatrix->rotateZ(0.0);
			IsAlignX = true;
		} else if ( Align == "Y" || Align == "y") {
			AlignMatrix->rotateX(90.0*deg);
			AlignMatrix->rotateY(0.0);
			AlignMatrix->rotateZ(0.0);
			IsAlignX = false;
		} else {
			G4cout<<"ERROR: Align value should be X or Y axis: see "<< GetFullParmName(iLayer, "Align") << G4endl;
			exit(1);
		}

		//Actual wire displacement
		for (int j = 0; j < NbWires; ++j ) {
			if (IsAlignX)
				PosX = 0.0;
			else
				PosX = Displacement[j];

			if (IsAlignX)
				PosY = Displacement[j];
			else
				PosY = 0.0;

			if ( fPm->ParameterExists(GetFullParmName("PrintInformation")) && fPm->GetBooleanParameter(GetFullParmName("PrintInformation")) )
				G4cout<<"    Wire \""<< j <<"\", Position (" << PosX/cm<<" cm, "<< PosY/cm <<" cm)" <<G4endl;

			CreatePhysicalVolume("Wire", j, true, lwire, AlignMatrix, new G4ThreeVector(PosX,PosY,PosZ), fEnvelopePhys);
		}//for (NbWires, displacement)
	}//for (Wire,Layers)
	return fEnvelopePhys;
}
