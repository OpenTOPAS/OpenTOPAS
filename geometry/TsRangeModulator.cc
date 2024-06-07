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

#include "TsRangeModulator.hh"
#include "TsParameterManager.hh"
#include "TsMaterialManager.hh"

#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4UIcommand.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"

TsRangeModulator::TsRangeModulator(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
								   TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name)
: TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name)
{
}


TsRangeModulator::~TsRangeModulator() {
}


G4VPhysicalVolume* TsRangeModulator::Construct()
{
	BeginConstruction();

	//RM wheel specifications: Dimensions of Drum, Hub, Upper/Middle/Lower shell
	const G4double  RinOfShell     = fPm->GetDoubleParameter( GetFullParmName("Shell","Rin"), "Length");
	const G4double  RoutOfShell    = fPm->GetDoubleParameter( GetFullParmName("Shell","Rout"), "Length");
	const G4double  HOfUpperShell  = fPm->GetDoubleParameter( GetFullParmName("HeightOfUpper"), "Length");
	const G4double  HOfMiddleShell = fPm->GetDoubleParameter( GetFullParmName("HeightOfMiddle"), "Length");
	const G4double  HOfLowerShell  = fPm->GetDoubleParameter( GetFullParmName("HeightOfLower"), "Length");
	const G4double  RinOfHub       = fPm->GetDoubleParameter( GetFullParmName("Hub","Rin"), "Length");
	const G4double  RoutOfHub      = fPm->GetDoubleParameter( GetFullParmName("Hub","Rout"), "Length");
	const G4double  TotalHeightOfShell = HOfUpperShell + HOfMiddleShell+ HOfLowerShell;
	const G4double  CenterOfInterDisk  = ( TotalHeightOfShell/2. - HOfUpperShell - HOfMiddleShell/2.  );

	//0. Envelop PhysicalVolume in which whole sub components are placed.
	G4Tubs* geoDrum = new G4Tubs("Drum", 0.0*mm, RoutOfShell, TotalHeightOfShell/2., 0.0*deg, 360*deg );
	fEnvelopeLog = CreateLogicalVolume(geoDrum);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

	//1. Shell of Drum
	G4Tubs* geoShell = new G4Tubs("Shell", RinOfShell, RoutOfShell, TotalHeightOfShell/2., 0.0*deg, 360.0*deg );
	G4LogicalVolume* logShell = CreateLogicalVolume("Shell", geoShell);
	CreatePhysicalVolume("Shell", logShell, fEnvelopePhys);

	//2. Hub of Drum
	G4Tubs* geoHub = new G4Tubs("Hub", RinOfHub, RoutOfHub, TotalHeightOfShell/2., 0.0*deg, 360.0*deg );
	G4LogicalVolume* logHub = CreateLogicalVolume("Hub", geoHub);
	CreatePhysicalVolume("Hub", logHub, fEnvelopePhys);

	//3. Tracks of Upper part of Range Modulator
	//4. Tracks of Middle part of Range Modulator
	//5. Tracks of Lower part of Range Modulator
	this -> ConstructTracks(UPPER,  RoutOfHub, RinOfShell, HOfUpperShell , (TotalHeightOfShell-HOfUpperShell)/2.);
	this -> ConstructTracks(MIDDLE, RoutOfHub, RinOfShell, HOfMiddleShell,  CenterOfInterDisk  );
	this -> ConstructTracks(LOWER,  RoutOfHub, RinOfShell, HOfLowerShell , (HOfLowerShell-TotalHeightOfShell)/2.);

	return  fEnvelopePhys;
}

void TsRangeModulator::ConstructTracks(TrackLocation location, G4double Rin, G4double Rout, G4double HofShell, G4double Z) {
	G4String  TrackParm;
	G4double* RadialDivisions;
	G4int     NRadials;
	G4String  TrackName;
	switch(location) {
		case UPPER:
			TrackName = "Upper";
			break;
		case MIDDLE:
			TrackName = "Middle";
			break;
		case LOWER:
			TrackName = "Lower";
			break;
		default:
			return;
	}

	G4VisAttributes* Att_InterDisk = new G4VisAttributes( G4Colour(0.4, 0.4, 0.4));
	Att_InterDisk->SetForceSolid(true);
	Att_InterDisk->SetForceLineSegmentsPerCircle(360);
	RegisterVisAtt(Att_InterDisk);

	RadialDivisions = fPm->GetDoubleVector(GetFullParmName(G4String(TrackName+"/RadialDivisions")), "Length");
	NRadials        = fPm->GetVectorLength(GetFullParmName(G4String(TrackName+"/RadialDivisions")));
	for (int i=0; i <= NRadials; ++i ) {
		G4double spec[3];
		G4String    TrackID = G4UIcommand::ConvertToString(i+1);
		G4String subComponentName = TrackName + "/Track" + TrackID;
		G4String ParmPattern    = fPm->GetStringParameter( GetFullParmName(subComponentName,"Pattern") );
		if (ParmPattern == "NULL") { continue; }
		if ( i == 0) {
			spec[0] = Rin;
			spec[1] = RadialDivisions[0];
		} else if ( i == NRadials) {
			spec[0] = RadialDivisions[i-1];
			spec[1] = Rout;
		} else {
			spec[0] = RadialDivisions[i-1];
			spec[1] = RadialDivisions[i];
		}

		if (i == 0.0 && NRadials==0) {
			spec[0] = Rin; spec[1] = Rout;
		}
		spec[2] = HofShell;

		G4String envelopeMaterialName = GetResolvedMaterialName();
		G4Tubs* geoSide          = new G4Tubs(subComponentName, spec[0], spec[1], spec[2]/2., 0.0*deg, 360*deg );
		G4LogicalVolume* logSide = CreateLogicalVolume(subComponentName, envelopeMaterialName, geoSide);
		logSide->SetVisAttributes(fPm->GetInvisible());
		G4VPhysicalVolume* physSide = CreatePhysicalVolume(subComponentName, logSide, 0, new G4ThreeVector(0,0, Z), fEnvelopePhys);

		G4String  SegParm     = G4String("Ge/") + fPm->GetStringParameter( GetFullParmName(subComponentName,"Pattern"));
		if ( !fPm->ParameterExists(SegParm+"/Angles")) {
			G4cerr << "Vector Parameter does not exist: " << SegParm << "/Angles for Range Modulator angles" << G4endl;
			fPm->AbortSession(1);
		}
		if ( !fPm->ParameterExists(SegParm+"/Heights")) {
			G4cerr << "Vector Parameter does not exist: " << SegParm << "/Heights for Range Modulator angles" << G4endl;
			fPm->AbortSession(1);
		}
		if ( !fPm->ParameterExists(SegParm+"/Materials")) {
			G4cerr << "Vector Parameter does not exist: " << SegParm << "/Materials for Range Modulator angles" << G4endl;
			fPm->AbortSession(1);
		}

		G4int NbDivisions = fPm->GetVectorLength( G4String(SegParm+"/Angles") );
		G4double* Angles      = fPm->GetDoubleVector( G4String(SegParm+"/Angles"), "Angle");

		if (NbDivisions != fPm->GetVectorLength(G4String(SegParm+"/Heights"))) {
			G4cerr << "Topas is exiting due to a serious error attempting to construct Range Modulator: " << fName << G4endl;
			G4cerr << "Number of angle divisions should be same with number of Heights" << G4endl;
			fPm->AbortSession(1);
		}
		if (NbDivisions != fPm->GetVectorLength(G4String(SegParm+"/Materials"))) {
			G4cerr << "Topas is exiting due to a serious error attempting to construct Range Modulator: " << fName << G4endl;
			G4cerr << "Number of angle divisions should be same with number of Materials" << G4endl;
			fPm->AbortSession(1);
		}

		G4double* Heights = fPm->GetDoubleVector( G4String(SegParm+"/Heights"), "Length");
		G4String* Materials = fPm->GetStringVector( G4String(SegParm+"/Materials") );

		if ( fPm->ParameterExists( G4String( SegParm +"/Offset")) ) {
			G4double ZeroOffset  = fPm->GetDoubleParameter( G4String(SegParm + "/Offset"), "Angle");
			for (int iDiv=0; iDiv < NbDivisions; ++iDiv) {
				Angles[iDiv] += ZeroOffset;
			}
		}

		G4String segmentName = TrackName.substr(0,3) + "_T" + TrackID + "_B";

		std::vector<Segment>* Layer = new std::vector<Segment>;
		G4String M;
		for (int iDiv = 0; iDiv < NbDivisions; ++iDiv) {
			Segment a;
			a.A[0] = Angles[iDiv];
			if ( iDiv == NbDivisions - 1 ) {
				a.A[1] = Angles[0] + 360.0*deg;
			} else {
				a.A[1] = Angles[iDiv+1];
			}
			a.H = Heights[iDiv];
			//if the segment's height is zero, we skip to append segment information to the Trackpattern vector
			if ( a.H == 0.0 ) { continue;}
			a.M = GetMaterial(Materials[iDiv]);
			if ( location == MIDDLE ) {
				a.V = Att_InterDisk;
			} else {
				a.V = fPm->GetColor( GetDefaultMaterialColor(Materials[iDiv]) );
				a.V->SetForceLineSegmentsPerCircle(360);
			}
			Layer->push_back(a);

			//Create individual segment
			G4Tubs* geoSeg = new G4Tubs(segmentName, spec[0], spec[1], 0.5*a.H , a.A[0] , a.A[1] - a.A[0]);
			G4LogicalVolume* logSeg = CreateLogicalVolume(segmentName, Materials[iDiv], geoSeg);
			logSeg->SetVisAttributes(a.V);
			G4double lz  = location*(spec[2] - a.H);
			CreatePhysicalVolume(segmentName, logSeg, 0, new G4ThreeVector(0,0,0.5*lz), physSide);
		}//for(NbDivisions)

		//To print out the input values through parameter
		if ( fPm->ParameterExists(GetFullParmName("PrintInformation")) && fPm->GetBooleanParameter(GetFullParmName("PrintInformation")) ) {
			G4cout<<" ---" << TrackName << "Track" << TrackID <<" , # of Blocks: "<< NbDivisions << G4endl;
			for (size_t j = 0; j < Layer->size(); ++j) {
				G4cout<<" "<<j<<" th Block" << G4endl;
				G4cout<<"    Angle   : "<< (*Layer)[j].A[0]/deg<<", "<< (*Layer)[j].A[1]/deg <<" deg"<< G4endl;
				G4cout<<"    Height  : "<< (*Layer)[j].H/cm<<" cm"<<G4endl;
				G4cout<<"    Material: "<< ((*Layer)[j].M)->GetName() <<G4endl;
			}
		}
	}//for(NRadials)
}
