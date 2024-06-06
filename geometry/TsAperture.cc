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

#include "TsAperture.hh"

#include "TsParameterManager.hh"

#include "G4LogicalVolume.hh"
#include "G4Tubs.hh"
#include "G4TwoVector.hh"
#include "G4ExtrudedSolid.hh"
#include "G4UIcommand.hh"

#include <fstream>

TsAperture::TsAperture(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
					   TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name)
:TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name)
{
}

TsAperture::~TsAperture()
{
}


G4VPhysicalVolume* TsAperture::Construct()
{
	BeginConstruction();

	G4String fileName = fPm->GetStringParameter(GetFullParmName("InputFile"));
	std::ifstream inAperture(fileName);
	if (!inAperture) {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "Unable to open aperture file: " << fileName << G4endl;
		fPm->AbortSession(1);
	}

	G4String fileFormat = fPm->GetStringParameter(GetFullParmName("FileFormat"));
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(fileFormat);
#else
	fileFormat.toLower();
#endif
	if (fileFormat == "mgh") {
		// MGH format begins with some unused values. Remove them now.
		G4String unusedString;
		G4double unusedDouble;
		G4int unusedInt;
		std::getline(inAperture,unusedString);
		std::getline(inAperture,unusedString);
		inAperture >> unusedDouble;
		inAperture >> unusedInt;
		G4int nDummyPoints;
		inAperture >> nDummyPoints;
		for (G4int i=1; i<=2*nDummyPoints; i++)
			inAperture >> unusedDouble;
	} else if (fileFormat != "xycoordinates") {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << fPm->GetStringParameter(GetFullParmName("FileFormat")) << " has invalid value: " << fPm->GetStringParameter(GetFullParmName("FileFormat")) << G4endl;
		G4cerr << "Must be either MGH or XYCoordinates." << G4endl;
		fPm->AbortSession(1);
	}

	G4int nPoints;
	G4double x, y;
	std::string tmp;
	std::vector<G4TwoVector> points;
	inAperture >> nPoints;
	G4int  numberOfDuplicatePoints = 0;
	for (G4int i = 0; i < nPoints; i++) {
		if (fileFormat == "mgh") {
			// Convert from cm to mm
			inAperture >> x >> y;
			x*=10.;
			y*=10.;
		} else {
			std::getline(inAperture,tmp,',');
			x = atof(tmp.c_str());
			std::getline(inAperture,tmp);
			y = atof(tmp.c_str());
		}

		// Skip duplicated points
		if (points.size() > 0 && x == points.back().x() && y == points.back().y()){
			G4cout << "   TOPAS WARNING: Aperturefile " << fileName << " had duplicate point in corner specifications, TOPAS will skip the duplicate point." << G4endl;
			numberOfDuplicatePoints++;
		}
		else
			points.push_back(G4TwoVector(x, y));
	}

	// Skip last point if it duplicates first point
	if ( points.size() > 1 && points.back().x() == points.front().x() && points.back().y() == points.front().y() ) {
		numberOfDuplicatePoints++;
		points.pop_back();
	}

	if (numberOfDuplicatePoints)
		nPoints -= numberOfDuplicatePoints;

	G4cout << "npoints " << nPoints << " " << points.size() << " " << numberOfDuplicatePoints << G4endl;

	if (fPm->ParameterExists(GetFullParmName("PrintPoints")) &&
		fPm->GetBooleanParameter(GetFullParmName("PrintPoints"))) {
		G4cout << "\nAperture data read from " << fileName << " using format " << fileFormat << G4endl;
		G4cout << "Number of points = " << points.size() << G4endl;
		for (size_t i = 0; i < points.size(); i++)
			G4cout << "point: " << i << " values: " << points[i][0] << ", " << points[i][1] << G4endl;
	}

	if (nPoints < 3) {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "Aperture file " << fileName << " has fewer than 3 points (excluding any beginning/end duplicates)." << G4endl;
		fPm->AbortSession(1);
	}

	if (nPoints > 1E6) {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "Aperture file " << fileName << " has more than one million points." << G4endl;
		fPm->AbortSession(1);
	}

	G4double RMax = fPm->GetDoubleParameter(GetFullParmName("RMax"),"Length");
	G4double HL = fPm->GetDoubleParameter(GetFullParmName("HL"),"Length");
	G4VSolid* apertureSolid = new G4Tubs(fName, 0., RMax, HL, 0., 360.);
	fEnvelopeLog = CreateLogicalVolume(apertureSolid);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

	// Voids will be filled with the aperture's parent material
	G4String voidMaterial = fParentComponent->GetResolvedMaterialName();

	G4VSolid* voidSolid = new G4ExtrudedSolid("Void", points, HL, G4TwoVector(0, 0), 1.0, G4TwoVector(0, 0), 1.0);
	G4LogicalVolume* voidLog = CreateLogicalVolume("Void", voidMaterial, voidSolid);
	G4RotationMatrix* rot = new G4RotationMatrix(0.0, 0.0, 0.0); //rotation
	G4ThreeVector* position = new G4ThreeVector(0.0, 0.0, 0.0);
	CreatePhysicalVolume("Void", voidLog, rot, position, fEnvelopePhys);

  	return fEnvelopePhys;
}
