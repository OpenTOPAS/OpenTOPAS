// Component for MyPropeller
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

#include "MyPropeller.hh"

#include "TsParameterManager.hh"

#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4SystemOfUnits.hh"

MyPropeller::MyPropeller(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
				   TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name)
	: TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name)
{;}


MyPropeller::~MyPropeller()
{;}


G4VPhysicalVolume* MyPropeller::Construct()
{
	BeginConstruction();

	//1. getting the information from parameters
	const G4int NbOfBlades  = fPm->GetIntegerParameter( GetFullParmName("NbOfBlades") );
	const G4double Rin      = fPm->GetDoubleParameter( GetFullParmName("Rin"), "Length");
	const G4double Rout     = fPm->GetDoubleParameter( GetFullParmName("Rout"), "Length");
	const G4int	nThickness  = fPm->GetVectorLength( GetFullParmName("Thickness"));
	G4double* Thickness     = fPm->GetDoubleVector( GetFullParmName("Thickness"), "Length");

	G4double* Angles    = fPm->GetDoubleVector( GetFullParmName("Angles"), "Angle");
	G4String* Materials = fPm->GetStringVector( GetFullParmName("Materials") );

	G4double TotalThickness = 0.0;
	for(int i=0; i < nThickness ; ++i){
		TotalThickness += Thickness[i];
	}

	//2. Creation of envolope geometry filled with mather's material
	G4String subComponentName     = "Blade";
	G4String envelopeMaterialName = GetResolvedMaterialName(subComponentName);
	G4Tubs* gPropeller = new G4Tubs(fName, 0.0*cm, Rout, TotalThickness/2.0, 0.0*deg, 360.0*deg);
	fEnvelopeLog       = CreateLogicalVolume(fName, envelopeMaterialName, gPropeller);
	fEnvelopeLog->SetVisAttributes(fPm->GetInvisible());
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

	//3. iterate creating each leaf: layer (bottom to top) -> leaf (counter-clock wise)
	G4double height_of_layer = 0.0;
	for(int i = 0 ; i < nThickness ; ++i){
		if( fPm->ParameterExists(GetFullParmName("PrintInformation"))
			&&
			fPm->GetBooleanParameter(GetFullParmName("PrintInformation"))) {
			G4cout<<" Layer: \"" << i <<"\" , Thickness: "<< Thickness[i]/cm << " (cm), Angle: "<< Angles[i]/deg
				  <<" (deg), Matrial: "<<GetMaterial(Materials[i])->GetName() <<G4endl;
		}
		//A leaf logical
		G4Tubs* gBlade = new G4Tubs("gBlade", Rin, Rout, Thickness[i]/2.0, -0.5*Angles[i], 1.0*Angles[i]);
		G4LogicalVolume* lBlade = CreateLogicalVolume(subComponentName, Materials[i], gBlade);
		for (int j = 0; j < NbOfBlades; ++j) {
			//angle & z-position
			G4double MidAngle = static_cast<G4double>(j)*360.0*deg/NbOfBlades;
			G4double ZPosition  = -0.5*TotalThickness + (height_of_layer + 0.5*Thickness[i]);
			G4ThreeVector* pos    = new G4ThreeVector(0.0, 0.0, ZPosition);
			G4RotationMatrix* rot = new G4RotationMatrix();
			rot->rotateX(0.0); rot->rotateY(0.0); rot->rotateZ(MidAngle);
			//physical volume reusing logical volume
			CreatePhysicalVolume(subComponentName, i*NbOfBlades+j, true, lBlade, rot, pos, fEnvelopePhys);
			if ( fPm->ParameterExists(GetFullParmName("PrintInformation"))
				 &&
				 fPm->GetBooleanParameter(GetFullParmName("PrintInformation"))) {
				  G4cout<<"    Blade \""<< i*NbOfBlades+j <<"\", Angle ("
						<< (MidAngle - 0.5*Angles[i])/2.0 <<" deg, "<< (MidAngle + 0.5*Angles[i])/2.0  <<" deg)" <<G4endl;
			}
	}
		height_of_layer += Thickness[i];
	}

	return fEnvelopePhys;
}
