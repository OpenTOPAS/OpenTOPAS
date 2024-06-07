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

#include "TsRidgeFilter.hh"

#include "TsParameterManager.hh"

#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4TessellatedSolid.hh"
#include "G4QuadrangularFacet.hh"
#include "G4SystemOfUnits.hh"

TsRidgeFilter::TsRidgeFilter(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
							 TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name)
: TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name)
{;}


TsRidgeFilter::~TsRidgeFilter()
{;}


G4VPhysicalVolume* TsRidgeFilter::Construct()
{
	BeginConstruction();

	const G4int n_z  = fPm->GetVectorLength(GetFullParmName("ZPoints"));
	G4double*     z  = fPm->GetDoubleVector(GetFullParmName("ZPoints"), "Length");
	const G4int n_x  = fPm->GetVectorLength(GetFullParmName("XPoints"));
	G4double*     x  = fPm->GetDoubleVector(GetFullParmName("XPoints"), "Length");
	const G4double width  = fPm->GetDoubleParameter(GetFullParmName("Width"), "Length");
	G4double*   pos_x     = fPm->GetDoubleVector(GetFullParmName("Displacement"), "Length");
	const G4int n_pos_x   = fPm->GetVectorLength(GetFullParmName("Displacement"));

	// for the ridge array and whole box
	G4double z_max = 0.0;
	for (int i = 0; i < n_z; ++i)
	{
		if  (z_max <= z[i])
			z_max = z[i];
	}

	G4double x_max = 0.0;
	G4double x_min = 0.0;
	for (int i = 0; i < n_pos_x; ++i)
	{
		if (x_max <= pos_x[i])
			x_max = pos_x[i];

		if (x_min >= pos_x[i])
			x_min = pos_x[i];
	}

	G4double hrx = 0.5*(x_max  - x_min + width);
	G4double hry = 0.5 * fPm->GetDoubleParameter(GetFullParmName("Length"), "Length");
	G4double hrz = 0.5 * z_max;

	//================================================================================
	// Geometry setup
	//================================================================================
	// Whole Box
	G4String envelopeMaterialName = fParentComponent->GetResolvedMaterialName();
	G4Box* svWholeBox = new G4Box("RidgeBox", hrx, hry, hrz);
	fEnvelopeLog = CreateLogicalVolume("RidgeBox", envelopeMaterialName, svWholeBox);
	fEnvelopeLog->SetVisAttributes(fPm->GetInvisible());
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

	//Build a Ridge solid
	G4TessellatedSolid* sv_ridge_i = new G4TessellatedSolid("Ridge");

	if ( fPm->ParameterExists(GetFullParmName("PrintInformation")) && fPm->GetBooleanParameter(GetFullParmName("PrintInformation")) ) {
		G4cout<<" Ridge points (x,z) ---   :"<<  n_x  <<G4endl;
		G4cout<<"       P initial : (0, 0) cm" <<G4endl;
	}

	//The most left part of a ridge
	if (x[0] != 0 ) {
		//First part: Pinit - P0
		G4TriangularFacet *frontInitial = new G4TriangularFacet(
																G4ThreeVector(0,    -hry, 0.0),
																G4ThreeVector(x[0], -hry, 0.0),
																G4ThreeVector(x[0], -hry, z[0]),
																ABSOLUTE);
		G4TriangularFacet *backInitial = new G4TriangularFacet(
															   G4ThreeVector(0, hry, 0.0),
															   G4ThreeVector(x[0], hry, z[0]),
															   G4ThreeVector(x[0], hry, 0.0),
															   ABSOLUTE);
		sv_ridge_i->AddFacet((G4VFacet*) frontInitial);
		sv_ridge_i->AddFacet((G4VFacet*) backInitial);
		G4QuadrangularFacet *topInitial = new G4QuadrangularFacet(
																  G4ThreeVector(0, -hry, 0),
																  G4ThreeVector(x[0], -hry, z[0]),
																  G4ThreeVector(x[0], hry, z[0]),
																  G4ThreeVector(0, hry, 0),
																  ABSOLUTE);
		G4QuadrangularFacet *bottomInitial = new G4QuadrangularFacet(
																	 G4ThreeVector(0, -hry, 0.0),
																	 G4ThreeVector(0,  hry, 0.0),
																	 G4ThreeVector(x[0], hry, 0.0),
																	 G4ThreeVector(x[0], -hry, 0.0),
																	 ABSOLUTE);
		sv_ridge_i->AddFacet((G4VFacet*) topInitial);
		sv_ridge_i->AddFacet((G4VFacet*) bottomInitial);
	} else {//x[0] == 0
		G4QuadrangularFacet *topInitial = new G4QuadrangularFacet(
																  G4ThreeVector(0, -hry, 0),
																  G4ThreeVector(x[0], -hry, z[0]),
																  G4ThreeVector(x[0], hry, z[0]),
																  G4ThreeVector(0, hry, 0),
																  ABSOLUTE);
		sv_ridge_i->AddFacet((G4VFacet*) topInitial);
	}

	//Iterate user input values in vector
	for (int i = 0; i < (n_x -1); ++i  ) {
		if ( fPm->ParameterExists(GetFullParmName("PrintInformation")) && fPm->GetBooleanParameter(GetFullParmName("PrintInformation")) ) {
			G4cout<<"       P "<<i<<"th     : ("<< x[i]/cm <<", " << z[i]/cm << ") cm" <<G4endl;
		}
		//top, bottom
		G4QuadrangularFacet *topf = new G4QuadrangularFacet(
															G4ThreeVector(x[i], -hry, z[i]),
															G4ThreeVector(x[i+1], -hry, z[i+1]),
															G4ThreeVector(x[i+1], hry, z[i+1]),
															G4ThreeVector(x[i], hry, z[i]),
															ABSOLUTE);
		sv_ridge_i->AddFacet((G4VFacet*) topf);

		if (x[i] == x[i+1]) continue;

		G4QuadrangularFacet *bottomf = new G4QuadrangularFacet(
															   G4ThreeVector(x[i], -hry, 0.0),
															   G4ThreeVector(x[i],  hry, 0.0),
															   G4ThreeVector(x[i+1], hry, 0.0),
															   G4ThreeVector(x[i+1], -hry, 0.0),
															   ABSOLUTE);
		sv_ridge_i->AddFacet((G4VFacet*) bottomf);

		G4QuadrangularFacet *frontf = new G4QuadrangularFacet(
															  G4ThreeVector(x[i], -hry, 0.0),
															  G4ThreeVector(x[i+1], -hry, 0.0),
															  G4ThreeVector(x[i+1], -hry, z[i+1]),
															  G4ThreeVector(x[i], -hry, z[i]),
															  ABSOLUTE);
		G4QuadrangularFacet *backf = new G4QuadrangularFacet(
															 G4ThreeVector(x[i], hry, 0.0),
															 G4ThreeVector(x[i], hry, z[i]),
															 G4ThreeVector(x[i+1], hry, z[i+1]),
															 G4ThreeVector(x[i+1], hry, 0.0),
															 ABSOLUTE);
	    sv_ridge_i->AddFacet((G4VFacet*) frontf);
	    sv_ridge_i->AddFacet((G4VFacet*) backf);
	}

	if ( fPm->ParameterExists(GetFullParmName("PrintInformation")) && fPm->GetBooleanParameter(GetFullParmName("PrintInformation")) ) {
		G4cout<<"       P "<< n_x-1 <<"th     : ("<< x[n_x-1]/cm <<", " << z[n_x-1]/cm << ") cm" <<G4endl;
		G4cout<<"       P final : ("<< width/cm <<", 0) cm" <<G4endl;

	}
	//The most right part
	if (x[n_x-1] != width ) {
	    G4TriangularFacet *frontfinal = new G4TriangularFacet(
															  G4ThreeVector(x[n_x-1],  -hry, 0.0),
															  G4ThreeVector(width, -hry, 0.0),
															  G4ThreeVector(x[n_x-1],  -hry, z[n_x-1]),
															  ABSOLUTE);
	    G4TriangularFacet *backfinal = new G4TriangularFacet(
															 G4ThreeVector(x[n_x-1], hry, 0.0),
															 G4ThreeVector(x[n_x-1], hry, z[n_x-1]),
															 G4ThreeVector(width, hry, 0.0),
															 ABSOLUTE);
	    sv_ridge_i->AddFacet((G4VFacet*) frontfinal);
	    sv_ridge_i->AddFacet((G4VFacet*) backfinal);
	    G4QuadrangularFacet *topfinal = new G4QuadrangularFacet(
																G4ThreeVector(x[n_x-1], -hry, z[n_x-1]),
																G4ThreeVector(width, -hry, 0),
																G4ThreeVector(width, hry, 0),
																G4ThreeVector(x[n_x-1], hry, z[n_x-1]),
																ABSOLUTE);
	    G4QuadrangularFacet *bottomfinal = new G4QuadrangularFacet(
																   G4ThreeVector(x[n_x-1], -hry, 0.0),
																   G4ThreeVector(x[n_x-1],  hry, 0.0),
																   G4ThreeVector(width, hry, 0.0),
																   G4ThreeVector(width, -hry, 0.0),
																   ABSOLUTE);
	    sv_ridge_i->AddFacet((G4VFacet*) topfinal);
	    sv_ridge_i->AddFacet((G4VFacet*) bottomfinal);
	} else {//x[n_x-1] == width
	    G4QuadrangularFacet *topfinal = new G4QuadrangularFacet(
																G4ThreeVector(x[n_x-1], -hry, z[n_x-1]),
																G4ThreeVector(width, -hry, 0),
																G4ThreeVector(width, hry, 0),
																G4ThreeVector(x[n_x-1], hry, z[n_x-1]),
																ABSOLUTE);
	    sv_ridge_i->AddFacet((G4VFacet*) topfinal);
	}

	sv_ridge_i->SetSolidClosed(true);
	G4LogicalVolume* lvRidgeUnit = CreateLogicalVolume(sv_ridge_i);

	//Ridge placement
	for (G4int i = 0; i < n_pos_x; ++i)
	{
		CreatePhysicalVolume("Ridge", i, true, lvRidgeUnit, 0, new G4ThreeVector(-width/2.0 + pos_x[i], 0., -hrz), fEnvelopePhys);
	}
	return fEnvelopePhys;
}
