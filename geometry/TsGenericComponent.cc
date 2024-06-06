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

#include "TsGenericComponent.hh"

#include "TsParameterManager.hh"

#include "G4VSolid.hh"
#include "G4CutTubs.hh"
#include "G4Cons.hh"
#include "G4Para.hh"
#include "G4Trd.hh"
#include "G4Trap.hh"
#include "G4Orb.hh"
#include "G4Torus.hh"
#include "G4Polycone.hh"
#include "G4GenericPolycone.hh"
#include "G4Polyhedra.hh"
#include "G4EllipticalTube.hh"
#include "G4Ellipsoid.hh"
#include "G4EllipticalCone.hh"
#include "G4Paraboloid.hh"
#include "G4Hype.hh"
#include "G4Tet.hh"
#include "G4ExtrudedSolid.hh"
#include "G4TwistedBox.hh"
#include "G4TwistedTrap.hh"
#include "G4TwistedTrd.hh"
#include "G4GenericTrap.hh"
#include "G4TwistedTubs.hh"
#include "G4GeometryTolerance.hh"
#include "G4SystemOfUnits.hh"

TsGenericComponent::TsGenericComponent(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
									   TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name) :
TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name)
{
}


TsGenericComponent::~TsGenericComponent()
{
}


G4VPhysicalVolume* TsGenericComponent::Construct()
{
	// Get the component type
	G4String compTypeMixed = fPm->GetStringParameterWithoutMonitoring(GetFullParmName("Type"));
	G4String compType = compTypeMixed;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(compType);
#else
	compType.toLower();
#endif

	// Set flag to indicate if this is a Group component
	fIsGroup = compType=="group";

	// This needs to come after fIsGroup is set, since rotations and translations calculated as part
	// of the BeginConstruction method are done differently if this is a group
	BeginConstruction();

	// Load the component type's list of required parameters into a vector of G4Strings.
	G4String paramList="Ge/Params/"+compTypeMixed;

	G4String* parameterNames = new G4String[1];

	if (fPm->ParameterExists(paramList))
		parameterNames = fPm->GetStringVector(paramList);
	else if (compType!="group") {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "The parameter:" << GetFullParmName("Type") << G4endl;
		G4cerr << "refers to an unsupported Component Type: " << compTypeMixed << G4endl;
		fPm->AbortSession(1);
	}

	G4VSolid* solid = 0;

	// Instantiate the G4 solid for this component
	if (compType=="g4cuttubs") {
		G4double RMin;
		if (fPm->ParameterExists(GetFullParmName("RMin")))
			RMin = fPm->GetDoubleParameter(GetFullParmName("RMin"),"Length");
		else
			RMin = 0.;

		G4double RMax = fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Length");
		G4double HL =	fPm->GetDoubleParameter(GetFullParmName(parameterNames[1]),"Length");

		G4double SPhi;
		if (fPm->ParameterExists(GetFullParmName("SPhi")))
			SPhi = fPm->GetDoubleParameter(GetFullParmName("SPhi"),"Angle");
		else
			SPhi = 0. * deg;

		G4double DPhi;
		if (fPm->ParameterExists(GetFullParmName("DPhi")))
			DPhi = fPm->GetDoubleParameter(GetFullParmName("DPhi"),"Angle");
		else
			DPhi = 360. * deg;

		G4double* LowNorm = fPm->GetUnitlessVector(GetFullParmName(parameterNames[2]));
		G4double* HighNorm = fPm->GetUnitlessVector(GetFullParmName(parameterNames[3]));
		if (RMin < 0.  ) OutOfRange( GetFullParmName("RMin"), "can not be negative");
		if (RMax < RMin) OutOfRange( GetFullParmName("RMax"), "can not be less than RMin");
		if (HL <= 0.   ) OutOfRange( GetFullParmName("HL"), "must be larger than zero");
		if (fPm->GetVectorLength(GetFullParmName(parameterNames[2])) !=3 )
			OutOfRange( GetFullParmName("LowNorm"), "must have length of 3");
		if (fPm->GetVectorLength(GetFullParmName(parameterNames[3])) !=3 )
			OutOfRange( GetFullParmName("HighNorm"), "must have length of 3");
		fDivisionNames[0] = "R";
		fDivisionNames[1] = "Phi";
		fDivisionNames[2] = "Z";
		fDivisionUnits[0] = "cm";
		fDivisionUnits[1] = "deg";
		fDivisionUnits[2] = "cm";
		fFullWidths[0] = RMax - RMin;
		fFullWidths[1] = DPhi;
		fFullWidths[2] = 2. * HL;
		solid = new G4CutTubs(fName, RMin, RMax, HL, SPhi, DPhi,
							  G4ThreeVector(LowNorm[0],LowNorm[1],LowNorm[2]),
							  G4ThreeVector(HighNorm[0],HighNorm[1],HighNorm[2]));
	} else if (compType=="g4cons") {
		G4double RMin1;
		if (fPm->ParameterExists(GetFullParmName("RMin1")))
			RMin1 = fPm->GetDoubleParameter(GetFullParmName("RMin1"),"Length");
		else
			RMin1 = 0.;

		G4double RMax1 = fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Length");

		G4double RMin2;
		if (fPm->ParameterExists(GetFullParmName("RMin2")))
			RMin2 = fPm->GetDoubleParameter(GetFullParmName("RMin2"),"Length");
		else
			RMin2 = 0.;

		G4double RMax2 = fPm->GetDoubleParameter(GetFullParmName(parameterNames[1]),"Length");
		G4double HL    = fPm->GetDoubleParameter(GetFullParmName(parameterNames[2]),"Length");

		G4double SPhi;
		if (fPm->ParameterExists(GetFullParmName("SPhi")))
			SPhi = fPm->GetDoubleParameter(GetFullParmName("SPhi"),"Angle");
		else
			SPhi = 0. * deg;

		G4double DPhi;
		if (fPm->ParameterExists(GetFullParmName("DPhi")))
			DPhi = fPm->GetDoubleParameter(GetFullParmName("DPhi"),"Angle");
		else
			DPhi = 360. * deg;

		if (RMin1 < 0.   ) OutOfRange( GetFullParmName("RMin1"), "can not be negative");
		if (RMax1 < RMin1) OutOfRange( GetFullParmName("RMax1"), "can not be less than RMin1");
		if (RMin2 < 0.   ) OutOfRange( GetFullParmName("RMin2"), "can not be negative");
		if (RMax2 < RMin2) OutOfRange( GetFullParmName("RMax2"), "can not be less than RMin2");
		if (HL <= 0.     ) OutOfRange( GetFullParmName("HL"), "must be larger than zero");
		solid = new G4Cons(fName, RMin1, RMax1, RMin2, RMax2, HL, SPhi, DPhi);
	} else if (compType=="g4para") {
		G4double HLX   = fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Length");
		G4double HLY   = fPm->GetDoubleParameter(GetFullParmName(parameterNames[1]),"Length");
		G4double HLZ   = fPm->GetDoubleParameter(GetFullParmName(parameterNames[2]),"Length");
		G4double Alpha = fPm->GetDoubleParameter(GetFullParmName(parameterNames[3]),"Angle");
		G4double Theta = fPm->GetDoubleParameter(GetFullParmName(parameterNames[4]),"Angle");
		G4double Phi   = fPm->GetDoubleParameter(GetFullParmName(parameterNames[5]),"Angle");
		if (HLX <= 0.) OutOfRange( GetFullParmName("HLX"), "must be larger than zero");
		if (HLY <= 0.) OutOfRange( GetFullParmName("HLY"), "must be larger than zero");
		if (HLZ <= 0.) OutOfRange( GetFullParmName("HLZ"), "must be larger than zero");
		solid = new G4Para(fName, HLX, HLY, HLZ, Alpha, Theta, Phi);
	} else if (compType=="g4trd") {
		G4double HLX1 = fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Length");
		G4double HLX2 = fPm->GetDoubleParameter(GetFullParmName(parameterNames[1]),"Length");
		G4double HLY1 = fPm->GetDoubleParameter(GetFullParmName(parameterNames[2]),"Length");
		G4double HLY2 = fPm->GetDoubleParameter(GetFullParmName(parameterNames[3]),"Length");
		G4double HLZ  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[4]),"Length");
		if (HLX1 < 0.) OutOfRange( GetFullParmName("HLX1"), "can not be negative");
		if (HLX2 < 0.) OutOfRange( GetFullParmName("HLX2"), "can not be negative");
		if (HLY1 < 0.) OutOfRange( GetFullParmName("HLY1"), "can not be negative");
		if (HLY2 < 0.) OutOfRange( GetFullParmName("HLY2"), "can not be negative");
		if (HLZ <= 0.) OutOfRange( GetFullParmName("HLZ"), "must be larger than zero");
		solid = new G4Trd(fName, HLX1, HLX2, HLY1, HLY2, HLZ);
	} else if (compType=="g4rtrap") {
		G4double LZ  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Length");
		G4double LY  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[1]),"Length");
		G4double LX  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[2]),"Length");
		G4double LTX = fPm->GetDoubleParameter(GetFullParmName(parameterNames[3]),"Length");
		if (LZ  <= 0.) OutOfRange( GetFullParmName("LZ"), "must be larger than zero");
		if (LY  <= 0.) OutOfRange( GetFullParmName("LY"), "must be larger than zero");
		if (LX  < 0. ) OutOfRange( GetFullParmName("LX"), "can not be negative");
		if (LTX < 0. ) OutOfRange( GetFullParmName("LTX"), "can not be negative");
		solid = new G4Trap(fName, LZ, LY, LX, LTX);
	} else if (compType=="g4gtrap") {
		G4double HLZ   = fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Length");
		G4double Theta = fPm->GetDoubleParameter(GetFullParmName(parameterNames[1]),"Angle");
		G4double Phi   = fPm->GetDoubleParameter(GetFullParmName(parameterNames[2]),"Angle");
		G4double HLY1  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[3]),"Length");
		G4double HLX1  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[4]),"Length");
		G4double HLX2  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[5]),"Length");
		G4double Alp1  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[6]),"Angle");
		G4double HLY2  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[7]),"Length");
		G4double HLX3  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[8]),"Length");
		G4double HLX4  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[9]),"Length");
		G4double Alp2  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[10]),"Angle");
		if (HLZ  <= 0.) OutOfRange( GetFullParmName("HLZ"), "must be larger than zero");
		if (HLY1 < 0. ) OutOfRange( GetFullParmName("HLY1"), "can not be negative");
		if (HLX1 < 0. ) OutOfRange( GetFullParmName("HLX1"), "can not be negative");
		if (HLX2 < 0. ) OutOfRange( GetFullParmName("HLX2"), "can not be negative");
		if (HLY2 < 0. ) OutOfRange( GetFullParmName("HLY2"), "can not be negative");
		if (HLX3 < 0. ) OutOfRange( GetFullParmName("HLX3"), "can not be negative");
		if (HLX4 < 0. ) OutOfRange( GetFullParmName("HLX4"), "can not be negative");
		solid = new G4Trap(fName, HLZ, Theta, Phi, HLY1, HLX1, HLX2, Alp1, HLY2, HLX3, HLX4, Alp2);
	} else if (compType=="g4orb") {
		G4double R = fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Length");
		if (R <= 0.) OutOfRange( GetFullParmName("R"), "must be larger than zero");
		solid = new G4Orb(fName, R);
	} else if (compType=="g4torus") {
		G4double RMin;
		if (fPm->ParameterExists(GetFullParmName("RMin")))
			RMin = fPm->GetDoubleParameter(GetFullParmName("RMin"),"Length");
		else
			RMin = 0.;

		G4double RMax = fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Length");
		G4double RTor =	fPm->GetDoubleParameter(GetFullParmName(parameterNames[1]),"Length");

		G4double SPhi;
		if (fPm->ParameterExists(GetFullParmName("SPhi")))
			SPhi = fPm->GetDoubleParameter(GetFullParmName("SPhi"),"Angle");
		else
			SPhi = 0. * deg;

		G4double DPhi;
		if (fPm->ParameterExists(GetFullParmName("DPhi")))
			DPhi = fPm->GetDoubleParameter(GetFullParmName("DPhi"),"Angle");
		else
			DPhi = 360. * deg;

		if (RMin < 0.  ) OutOfRange( GetFullParmName("RMin"), "can not be negative");
		if (RMax < RMin) OutOfRange( GetFullParmName("RMax"), "can not be less than RMin");
		if (RTor < RMax) OutOfRange( GetFullParmName("RTor"), "can not be less than RMax");
		solid = new G4Torus(fName, RMin, RMax, RTor, SPhi, DPhi);
	} else if (compType=="g4hpolycone") {
		G4double PhiStart;
		if (fPm->ParameterExists(GetFullParmName("PhiStart")))
			PhiStart = fPm->GetDoubleParameter(GetFullParmName("PhiStart"),"Angle");
		else
			PhiStart = 0. * deg;

		G4double PhiTotal;
		if (fPm->ParameterExists(GetFullParmName("PhiTotal")))
			PhiTotal = fPm->GetDoubleParameter(GetFullParmName("PhiTotal"),"Angle");
		else
			PhiTotal = 360. * deg;

		CheckLengthMatch(parameterNames[0], parameterNames[1]);
		CheckLengthMatch(parameterNames[0], parameterNames[2]);
		solid = new G4Polycone(fName, PhiStart, PhiTotal,
							   fPm->GetVectorLength(GetFullParmName(parameterNames[0])),
							   fPm->GetDoubleVector(GetFullParmName(parameterNames[0]),"Length"),
							   fPm->GetDoubleVector(GetFullParmName(parameterNames[1]),"Length"),
							   fPm->GetDoubleVector(GetFullParmName(parameterNames[2]),"Length"));
	} else if (compType=="g4spolycone") {
		G4double PhiStart;
		if (fPm->ParameterExists(GetFullParmName("PhiStart")))
			PhiStart = fPm->GetDoubleParameter(GetFullParmName("PhiStart"),"Angle");
		else
			PhiStart = 0. * deg;

		G4double PhiTotal;
		if (fPm->ParameterExists(GetFullParmName("PhiTotal")))
			PhiTotal = fPm->GetDoubleParameter(GetFullParmName("PhiTotal"),"Angle");
		else
			PhiTotal = 360. * deg;

		CheckLengthMatch(parameterNames[0], parameterNames[1]);
		solid = new G4Polycone(fName, PhiStart, PhiTotal,
							   fPm->GetVectorLength(GetFullParmName(parameterNames[0])),
							   fPm->GetDoubleVector(GetFullParmName(parameterNames[0]),"Length"),
							   fPm->GetDoubleVector(GetFullParmName(parameterNames[1]),"Length"));
	} else if (compType=="g4genericpolycone") {
		G4double PhiStart;
		if (fPm->ParameterExists(GetFullParmName("PhiStart")))
			PhiStart = fPm->GetDoubleParameter(GetFullParmName("PhiStart"),"Angle");
		else
			PhiStart = 0. * deg;

		G4double PhiTotal;
		if (fPm->ParameterExists(GetFullParmName("PhiTotal")))
			PhiTotal = fPm->GetDoubleParameter(GetFullParmName("PhiTotal"),"Angle");
		else
			PhiTotal = 360. * deg;

		CheckLengthMatch(parameterNames[0], parameterNames[1]);
		solid = new G4GenericPolycone(fName, PhiStart, PhiTotal,
							   fPm->GetVectorLength(GetFullParmName(parameterNames[0])),
							   fPm->GetDoubleVector(GetFullParmName(parameterNames[0]),"Length"),
							   fPm->GetDoubleVector(GetFullParmName(parameterNames[1]),"Length"));
	} else if (compType=="g4hpolyhedra") {
		G4double PhiStart;
		if (fPm->ParameterExists(GetFullParmName("PhiStart")))
			PhiStart = fPm->GetDoubleParameter(GetFullParmName("PhiStart"),"Angle");
		else
			PhiStart = 0. * deg;

		G4double PhiTotal;
		if (fPm->ParameterExists(GetFullParmName("PhiTotal")))
			PhiTotal = fPm->GetDoubleParameter(GetFullParmName("PhiTotal"),"Angle");
		else
			PhiTotal = 360. * deg;

		CheckLengthMatch(parameterNames[1], parameterNames[2]);
		CheckLengthMatch(parameterNames[1], parameterNames[3]);
		solid = new G4Polyhedra(fName, PhiStart, PhiTotal,
								fPm->GetIntegerParameter(GetFullParmName(parameterNames[0])),
								fPm->GetVectorLength(GetFullParmName(parameterNames[1])),
								fPm->GetDoubleVector(GetFullParmName(parameterNames[1]),"Length"),
								fPm->GetDoubleVector(GetFullParmName(parameterNames[2]),"Length"),
								fPm->GetDoubleVector(GetFullParmName(parameterNames[3]),"Length"));
	} else if (compType=="g4spolyhedra") {
		G4double PhiStart;
		if (fPm->ParameterExists(GetFullParmName("PhiStart")))
			PhiStart = fPm->GetDoubleParameter(GetFullParmName("PhiStart"),"Angle");
		else
			PhiStart = 0. * deg;

		G4double PhiTotal;
		if (fPm->ParameterExists(GetFullParmName("PhiTotal")))
			PhiTotal = fPm->GetDoubleParameter(GetFullParmName("PhiTotal"),"Angle");
		else
			PhiTotal = 360. * deg;

		CheckLengthMatch(parameterNames[1], parameterNames[2]);
		solid = new G4Polyhedra(fName, PhiStart, PhiTotal,
								fPm->GetIntegerParameter(GetFullParmName(parameterNames[0])),
								fPm->GetVectorLength(GetFullParmName(parameterNames[1])),
								fPm->GetDoubleVector(GetFullParmName(parameterNames[1]),"Length"),
								fPm->GetDoubleVector(GetFullParmName(parameterNames[2]),"Length"));
	} else if (compType=="g4ellipticaltube") {
		G4double HLX = fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Length");
		G4double HLY = fPm->GetDoubleParameter(GetFullParmName(parameterNames[1]),"Length");
		G4double HLZ = fPm->GetDoubleParameter(GetFullParmName(parameterNames[2]),"Length");
		if (HLX <= 0.) OutOfRange( GetFullParmName("HLX"), "must be larger than zero");
		if (HLY <= 0.) OutOfRange( GetFullParmName("HLY"), "must be larger than zero");
		if (HLZ <= 0.) OutOfRange( GetFullParmName("HLZ"), "must be larger than zero");
		solid = new G4EllipticalTube(fName, HLX, HLY, HLZ);
	} else if (compType=="g4ellipsoid") {
		G4double HLX     = fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Length");
		G4double HLY     = fPm->GetDoubleParameter(GetFullParmName(parameterNames[1]),"Length");
		G4double HLZ     = fPm->GetDoubleParameter(GetFullParmName(parameterNames[2]),"Length");

		G4double ZBottom;
		if (fPm->ParameterExists(GetFullParmName("ZBottom")))
			ZBottom = fPm->GetDoubleParameter(GetFullParmName("ZBottom"),"Length");
		else
			ZBottom = -HLZ;

		G4double ZTop;
		if (fPm->ParameterExists(GetFullParmName("ZTop")))
			ZTop = fPm->GetDoubleParameter(GetFullParmName("ZTop"),"Length");
		else
			ZTop = HLZ;

		if (HLX <= 0.   ) OutOfRange( GetFullParmName("HLX"), "must be larger than zero");
		if (HLY <= 0.   ) OutOfRange( GetFullParmName("HLY"), "must be larger than zero");
		if (HLZ <= 0.   ) OutOfRange( GetFullParmName("HLZ"), "must be larger than zero");
		solid = new G4Ellipsoid(fName, HLX, HLY, HLZ, ZBottom, ZTop);
	} else if (compType=="g4ellipticalcone") {
		G4double HLX  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Length");
		G4double HLY  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[1]),"Length");
		G4double ZMax = fPm->GetDoubleParameter(GetFullParmName(parameterNames[2]),"Length");

		G4double ZTop;
		if (fPm->ParameterExists(GetFullParmName("ZTop")))
			ZTop = fPm->GetDoubleParameter(GetFullParmName("ZTop"),"Length");
		else
			ZTop = ZMax;

		if (HLX <= 0. ) OutOfRange( GetFullParmName("HLX"), "must be larger than zero");
		if (HLY <= 0. ) OutOfRange( GetFullParmName("HLY"), "must be larger than zero");
		if (ZMax <= 0.) OutOfRange( GetFullParmName("ZMax"), "must be larger than zero");
		solid = new G4EllipticalCone(fName, HLX, HLY, ZMax, ZTop);
	} else if (compType=="g4paraboloid") {
		G4double HLZ = fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Length");
		G4double R1  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[1]),"Length");
		G4double R2  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[2]),"Length");
		if (HLZ <= 0.) OutOfRange( GetFullParmName("HLZ"), "must be larger than zero");
		if (R1 < 0.  ) OutOfRange( GetFullParmName("R1"), "can not be negative");
		if (R2 < 0.  ) OutOfRange( GetFullParmName("R2"), "can not be negative");
		solid = new G4Paraboloid(fName, HLZ, R1, R2);
	} else if (compType=="g4hype") {
		G4double IR;
		if (fPm->ParameterExists(GetFullParmName("IR")))
			IR = fPm->GetDoubleParameter(GetFullParmName("IR"),"Length");
		else
			IR = 0.;

		G4double OR  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Length");

		G4double IS;
		if (fPm->ParameterExists(GetFullParmName("IS")))
			IS = fPm->GetDoubleParameter(GetFullParmName("IS"),"Angle");
		else
			IS = 0. * deg;

		G4double OS_ = fPm->GetDoubleParameter(GetFullParmName(parameterNames[1]),"Angle");
		G4double HLZ = fPm->GetDoubleParameter(GetFullParmName(parameterNames[2]),"Length");
		if (IR < 0.  ) OutOfRange( GetFullParmName("IR"), "can not be negative");
		if (OR < IR  ) OutOfRange( GetFullParmName("OR"), "can not be less than IR");
		if (HLZ <= 0.) OutOfRange( GetFullParmName("HLZ"), "must be larger than zero");
		solid = new G4Hype(fName, IR, OR, IS, OS_, HLZ);
	} else if (compType=="g4tet") {
		solid = new G4Tet(fName,
						  fPm->GetThreeVectorParameter(GetFullParmName(parameterNames[0]),"Length"),
						  fPm->GetThreeVectorParameter(GetFullParmName(parameterNames[1]),"Length"),
						  fPm->GetThreeVectorParameter(GetFullParmName(parameterNames[2]),"Length"),
						  fPm->GetThreeVectorParameter(GetFullParmName(parameterNames[3]),"Length"));
	} else if (compType=="g4extrudedsolid") {
		G4double* polygonCoordinates = fPm->GetDoubleVector(GetFullParmName(parameterNames[0]),"Length");
		G4int length = fPm->GetVectorLength(GetFullParmName(parameterNames[0]));
		std::vector<G4TwoVector> polygons;
		for (G4int iPolygon = 0; iPolygon < length/2; iPolygon++)
			polygons.push_back(G4TwoVector(polygonCoordinates[2*iPolygon], polygonCoordinates[2*iPolygon+1]));

		solid = new G4ExtrudedSolid(fName, polygons,
									fPm->GetDoubleParameter(GetFullParmName(parameterNames[1]),"Length"),
									fPm->GetTwoVectorParameter(GetFullParmName(parameterNames[2]),"Length"),
									fPm->GetUnitlessParameter(GetFullParmName(parameterNames[3])),
									fPm->GetTwoVectorParameter(GetFullParmName(parameterNames[4]),"Length"),
									fPm->GetUnitlessParameter(GetFullParmName(parameterNames[5])));
	} else if (compType=="g4twistedbox") {
		G4double Twist = fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Angle");
		G4double HLX   = fPm->GetDoubleParameter(GetFullParmName(parameterNames[1]),"Length");
		G4double HLY   = fPm->GetDoubleParameter(GetFullParmName(parameterNames[2]),"Length");
		G4double HLZ   = fPm->GetDoubleParameter(GetFullParmName(parameterNames[3]),"Length");
		if (HLX <= 0.) OutOfRange( GetFullParmName("HLX"), "must be larger than zero");
		if (HLY <= 0.) OutOfRange( GetFullParmName("HLY"), "must be larger than zero");
		if (HLZ <= 0.) OutOfRange( GetFullParmName("HLZ"), "must be larger than zero");
		solid = new G4TwistedBox(fName, Twist, HLX, HLY, HLZ);
	} else if (compType=="g4rtwistedtrap") {
		G4double Twist = fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Angle");
		G4double HLX1  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[1]),"Length");
		G4double HLX2  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[2]),"Length");
		G4double HLY   = fPm->GetDoubleParameter(GetFullParmName(parameterNames[3]),"Length");
		G4double HLZ   = fPm->GetDoubleParameter(GetFullParmName(parameterNames[4]),"Length");
		if (HLX1  < 0.) OutOfRange( GetFullParmName("HLX1"), "can not be negative");
		if (HLX2  < 0.) OutOfRange( GetFullParmName("HLX2"), "can not be negative");
		if (HLY  <= 0.) OutOfRange( GetFullParmName("HLY"), "must be larger than zero");
		if (HLZ <= 0. ) OutOfRange( GetFullParmName("HLZ"), "must be larger than zero");
		solid = new G4TwistedTrap(fName, Twist, HLX1, HLX2, HLY, HLZ);
	} else if (compType=="g4gtwistedtrap") {
		G4double Twist = fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Angle");
		G4double HLZ   = fPm->GetDoubleParameter(GetFullParmName(parameterNames[1]),"Length");
		G4double Theta = fPm->GetDoubleParameter(GetFullParmName(parameterNames[2]),"Angle");
		G4double Phi   = fPm->GetDoubleParameter(GetFullParmName(parameterNames[3]),"Angle");
		G4double HLY1  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[4]),"Length");
		G4double HLX1  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[5]),"Length");
		G4double HLX2  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[6]),"Length");
		G4double HLY2  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[7]),"Length");
		G4double HLX3  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[8]),"Length");
		G4double HLX4  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[9]),"Length");
		G4double Alpha  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[10]),"Angle");
		if (HLZ <= 0.) OutOfRange( GetFullParmName("HLZ"), "must be larger than zero");
		if (HLY1 < 0.) OutOfRange( GetFullParmName("LZ"), "can not be negative");
		if (HLX1 < 0.) OutOfRange( GetFullParmName("HLX1"), "can not be negative");
		if (HLX2 < 0.) OutOfRange( GetFullParmName("HLX2"), "can not be negative");
		if (HLY2 < 0.) OutOfRange( GetFullParmName("HLY2"), "can not be negative");
		if (HLX3 < 0.) OutOfRange( GetFullParmName("HLX3"), "can not be negative");
		if (HLX4 < 0.) OutOfRange( GetFullParmName("HLX4"), "can not be negative");
		solid = new G4TwistedTrap(fName, Twist, HLZ, Theta, Phi, HLY1, HLX1, HLX2, HLY2, HLX3, HLX4, Alpha);
	} else if (compType=="g4twistedtrd") {
		G4double HLX1  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Length");
		G4double HLX2  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[1]),"Length");
		G4double HLY1  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[2]),"Length");
		G4double HLY2  = fPm->GetDoubleParameter(GetFullParmName(parameterNames[3]),"Length");
		G4double HLZ   = fPm->GetDoubleParameter(GetFullParmName(parameterNames[4]),"Length");
		G4double Twist = fPm->GetDoubleParameter(GetFullParmName(parameterNames[5]),"Angle");
		if (HLX1 < 0.) OutOfRange( GetFullParmName("HLX1"), "can not be negative");
		if (HLX2 < 0.) OutOfRange( GetFullParmName("HLX2"), "can not be negative");
		if (HLY1 < 0.) OutOfRange( GetFullParmName("HLY1"), "can not be negative");
		if (HLY2 < 0.) OutOfRange( GetFullParmName("HLY2"), "can not be negative");
		if (HLZ <= 0.) OutOfRange( GetFullParmName("HLZ"), "must be larger than zero");
		solid = new G4TwistedTrd(fName, HLX1, HLX2, HLY1, HLY2, HLZ, Twist);
	} else if (compType=="g4generictrap") {
		G4double* vertexCoordinates = fPm->GetDoubleVector(GetFullParmName(parameterNames[1]),"Length");
		G4int length = fPm->GetVectorLength(GetFullParmName(parameterNames[1]));
		std::vector<G4TwoVector> vertices;
		for (G4int iVertex = 0; iVertex < length/2; iVertex++)
			vertices.push_back(G4TwoVector(vertexCoordinates[2*iVertex], vertexCoordinates[2*iVertex+1]));

		solid = new G4GenericTrap(fName,
								  fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Length"),
								  vertices);
	} else if (compType=="g4twistedtubs") {
		G4double Twist       = fPm->GetDoubleParameter(GetFullParmName(parameterNames[0]),"Angle");
		G4double EndInnerRad = fPm->GetDoubleParameter(GetFullParmName(parameterNames[1]),"Length");
		G4double EndOuterRad = fPm->GetDoubleParameter(GetFullParmName(parameterNames[2]),"Length");
		G4double HLZ         = fPm->GetDoubleParameter(GetFullParmName(parameterNames[3]),"Length");
		G4double Phi         = fPm->GetDoubleParameter(GetFullParmName(parameterNames[4]),"Angle");
		if (EndInnerRad < 0.         ) OutOfRange( GetFullParmName("EndInnerRad"), "can not be negative");
		if (EndOuterRad < EndInnerRad) OutOfRange( GetFullParmName("EndOuterRad"), "can not be less than EndInnerRad");
		if (HLZ <= 0.                ) OutOfRange( GetFullParmName("HLZ"), "must be larger than zero");
		solid = new G4TwistedTubs(fName, Twist, EndInnerRad, EndOuterRad, HLZ, Phi);
	}

	if (fIsGroup) {
		// Reuse the parent volume.
		fEnvelopePhys = fParentVolume;
	} else {
		fEnvelopeLog = CreateLogicalVolume(solid);
		fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);
	}

	InstantiateChildren();

	return fEnvelopePhys;
}


void TsGenericComponent::CheckLengthMatch(G4String parameterName1, G4String parameterName2) {
	if (fPm->GetVectorLength(GetFullParmName(parameterName1)) != fPm->GetVectorLength(GetFullParmName(parameterName2))) {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "Length of parameter: " << GetFullParmName(parameterName1) << " does not match length of parameter: " << GetFullParmName(parameterName2) << G4endl;
		fPm->AbortSession(1);
	}
}


G4bool TsGenericComponent::CreateDefaults(TsParameterManager* pM, G4String& childName, G4String&, G4String& childCompType) {
	G4bool handled = true;
	G4String parameterName;
	G4String transValue;

	if (childCompType == "G4CutTubs") {
		parameterName = "dc:Ge/" + childName + "/RMin";
		transValue = "0. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/RMax";
		transValue = "20. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HL";
		transValue = "30. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/SPhi";
		transValue = "0. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/DPhi";
		transValue = "270. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "uvc:Ge/" + childName + "/LowNorm";
		transValue = "3 0. -0.7  -0.71";
		pM->AddParameter(parameterName, transValue);

		parameterName = "uvc:Ge/" + childName + "/HighNorm";
		transValue = "3 0.7 0. 0.71";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4Cons") {
		parameterName = "dc:Ge/" + childName + "/RMin1";
		transValue = "0. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/RMax1";
		transValue = "20. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/RMin2";
		transValue = "10. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/RMax2";
		transValue = "30. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HL";
		transValue = "30. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/SPhi";
		transValue = "0. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/DPhi";
		transValue = "270. deg";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4Para") {
		parameterName = "dc:Ge/" + childName + "/HLX";
		transValue = "20. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLY";
		transValue = "30. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLZ";
		transValue = "40. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/Alpha";
		transValue = "20. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/Theta";
		transValue = "0. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/Phi";
		transValue = "0. deg";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4Trd") {
		parameterName = "dc:Ge/" + childName + "/HLX1";
		transValue = "20. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLX2";
		transValue = "30. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLY1";
		transValue = "40. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLY2";
		transValue = "50. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLZ";
		transValue = "60. cm";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4RTrap") {
		parameterName = "dc:Ge/" + childName + "/LZ";
		transValue = "80. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/LY";
		transValue = "60. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/LX";
		transValue = "40. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/LTX";
		transValue = "20. cm";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4GTrap") {
		parameterName = "dc:Ge/" + childName + "/HLZ";
		transValue = "80. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/Theta";
		transValue = "20. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/Phi";
		transValue = "30. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLY1";
		transValue = "40. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLX1";
		transValue = "30. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLX2";
		transValue = "40. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/Alp1";
		transValue = "10. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLY2";
		transValue = "16. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLX3";
		transValue = "10. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLX4";
		transValue = "14. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/Alp2";
		transValue = "10. deg";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4Orb") {
		parameterName = "dc:Ge/" + childName + "/R";
		transValue = "80. cm";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4Torus") {
		parameterName = "dc:Ge/" + childName + "/RMin";
		transValue = "10. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/RMax";
		transValue = "20. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/RTor";
		transValue = "80. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/SPhi";
		transValue = "0. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/DPhi";
		transValue = "270. deg";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4HPolycone") {
		parameterName = "dc:Ge/" + childName + "/PhiStart";
		transValue = "0. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/PhiTotal";
		transValue = "270. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dvc:Ge/" + childName + "/RInner";
		transValue = "9 0 1. 1. 1. 2. 2. 3. .5 .2 cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dvc:Ge/" + childName + "/ROuter";
		transValue = "9 0 10 10 5 5 10 10 2 2 cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dvc:Ge/" + childName + "/Z";
		transValue = "9 5 7 9 11 25 27 29 31 35 cm";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4SPolycone") {
		parameterName = "dc:Ge/" + childName + "/PhiStart";
		transValue = "0. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/PhiTotal";
		transValue = "270. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dvc:Ge/" + childName + "/R";
		transValue = "10 0 10 10 5 5 10 10 2 2 0 cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dvc:Ge/" + childName + "/ROuter";
		transValue = "9 0 10 10 5 5 10 10 2 2 cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dvc:Ge/" + childName + "/Z";
		transValue = "10 5 7 9 11 25 27 29 31 35 35 cm";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4HPolyhedra") {
		parameterName = "dc:Ge/" + childName + "/PhiStart";
		transValue = "0. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/PhiTotal";
		transValue = "270. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "ic:Ge/" + childName + "/NSides";
		transValue = "3";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dvc:Ge/" + childName + "/Z";
		transValue = "7 0 5 18 23 50 72 85 cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dvc:Ge/" + childName + "/RInner";
		transValue = "7 0 10 14 18 11 11 12 cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dvc:Ge/" + childName + "/ROuter";
		transValue = "7 0 20 25 34 34 30 20 cm";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4SPolyhedra") {
		parameterName = "dc:Ge/" + childName + "/PhiStart";
		transValue = "0. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/PhiTotal";
		transValue = "270. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "ic:Ge/" + childName + "/NSides";
		transValue = "100";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dvc:Ge/" + childName + "/R";
		transValue = "5 0 14 32 28 0 cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dvc:Ge/" + childName + "/Z";
		transValue = "5 0 0 28 56 56 cm";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4EllipticalTube") {
		parameterName = "dc:Ge/" + childName + "/HLX";
		transValue = "15. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLY";
		transValue = "30. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLZ";
		transValue = "30. cm";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4Ellipsoid") {
		parameterName = "dc:Ge/" + childName + "/HLX";
		transValue = "25. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLY";
		transValue = "50. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLZ";
		transValue = "50. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/ZBottom";
		transValue = "-20. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/ZTop";
		transValue = "40. cm";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4EllipticalCone") {
		parameterName = "dc:Ge/" + childName + "/HLX";
		transValue = ".4 cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLY";
		transValue = ".8 cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/ZMax";
		transValue = "30. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/ZTop";
		transValue = "15. cm";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4Paraboloid") {
		parameterName = "dc:Ge/" + childName + "/HLZ";
		transValue = "20. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/R1";
		transValue = "20. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/R2";
		transValue = "35. cm";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4Hype") {
		parameterName = "dc:Ge/" + childName + "/IR";
		transValue = "20. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/OR";
		transValue = "30. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/IS";
		transValue = "70. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/OS";
		transValue = "20. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLZ";
		transValue = "30. cm";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4Tet") {
		parameterName = "dvc:Ge/" + childName + "/Anchor";
		transValue = "3 0 0 17.3 cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dvc:Ge/" + childName + "/P2";
		transValue = "3 0 16.3 -5.8 cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dvc:Ge/" + childName + "/P3";
		transValue = "3 -14.1 -8.2 -5.8 cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dvc:Ge/" + childName + "/P4";
		transValue = "3 14.1 -8.2 -5.8 cm";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4ExtrudedSolid") {
		parameterName = "dvc:Ge/" + childName + "/Polygons";
		transValue = "16 -30 -30 -30 30 30 30 30 -30 15 -30 15 15 -15 15 -15 -30 cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLZ";
		transValue = "20. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dvc:Ge/" + childName + "/Off1";
		transValue = "2 10. 10. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "uc:Ge/" + childName + "/Scale1";
		transValue = "1.";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dvc:Ge/" + childName + "/Off2";
		transValue = "2 -10. -10. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "uc:Ge/" + childName + "/Scale2";
		transValue = "0.6";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4TwistedBox") {
		parameterName = "dc:Ge/" + childName + "/Twist";
		transValue = "30. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLX";
		transValue = "30. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLY";
		transValue = "40. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLZ";
		transValue = "60. cm";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4RTwistedTrap") {
		parameterName = "dc:Ge/" + childName + "/Twist";
		transValue = "30. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLX1";
		transValue = "30. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLX2";
		transValue = "40. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLY";
		transValue = "40. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLZ";
		transValue = "60. cm";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4GTwistedTrap") {
		parameterName = "dc:Ge/" + childName + "/Twist";
		transValue = "30. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLZ";
		transValue = "60. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/Theta";
		transValue = "20. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/Phi";
		transValue = "30. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLY1";
		transValue = "40. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLX1";
		transValue = "30. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLX2";
		transValue = "40. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLY2";
		transValue = "16. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLX3";
		transValue = "10. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLX4";
		transValue = "14. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/Alpha";
		transValue = "10. deg";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4TwistedTrd") {
		parameterName = "dc:Ge/" + childName + "/Twist";
		transValue = "30. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLX1";
		transValue = "20. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLX2";
		transValue = "30. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLY1";
		transValue = "40. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLY2";
		transValue = "50. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLZ";
		transValue = "60. cm";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4GenericTrap") {
		parameterName = "dc:Ge/" + childName + "/HLZ";
		transValue = "25. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dvc:Ge/" + childName + "/Vertices";
		transValue = "16 -30 -30 -30 30 30 30 30 -30 -5 -20 -20 20 20 20 20 -20 cm";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType == "G4TwistedTubs") {
		parameterName = "dc:Ge/" + childName + "/Twist";
		transValue = "30. deg";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/EndInnerRad";
		transValue = "20. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/EndOuterRad";
		transValue = "40. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/HLZ";
		transValue = "40. cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/Phi";
		transValue = "270. deg";
		pM->AddParameter(parameterName, transValue);
	} else if (childCompType != "Group") {
		handled = false;
	}

	return handled;
}
