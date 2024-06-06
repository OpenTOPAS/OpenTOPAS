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

#include "TsCompensator.hh"

#include "TsParameterManager.hh"

#include "G4LogicalVolume.hh"
#include "G4Tubs.hh"
#include "G4Polyhedra.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4ExtrudedSolid.hh"
#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

#include <fstream>

TsCompensator::TsCompensator(TsParameterManager* pM, TsExtensionManager* eM,
							 TsMaterialManager* mM, TsGeometryManager* gM,
							 TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name)
	: TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name), fConversionFactor(0),
	  fMainCylinderThickness(0), fMainCylinderRadius(0), fMainCylinderRadiusSquared(0),
	  fDrillHoleRadius(0), fMainCylinder(nullptr), fNumRows(0)
{
}


TsCompensator::~TsCompensator()
{
}


G4VPhysicalVolume* TsCompensator::Construct()
{
	BeginConstruction();

	fFileName = fPm->GetStringParameter(GetFullParmName("InputFile"));
	std::ifstream inCompensator(fFileName);
	if (!inCompensator) {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "Unable to open compensator file: " << fFileName << G4endl;
		fPm->AbortSession(1);
	}

	G4String fileFormat = fPm->GetStringParameter(GetFullParmName("FileFormat"));
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(fileFormat);
#else
	fileFormat.toLower();
#endif
	if (fileFormat=="mgh" ) {
		// MGH format uses inches
		fConversionFactor = 2.54 * cm;

		// unusedString and unusedDouble absorb unused fields from the input file
		G4String unusedString;
		G4double unusedDouble;

		getline(inCompensator, unusedString);
		inCompensator >> fNumRows;

		inCompensator >> unusedDouble >> unusedDouble >> unusedDouble >> unusedDouble >> fMainCylinderThickness;
		fMainCylinderThickness *= fConversionFactor;

		// Convert from diameter in inches to radius in mm
		inCompensator >> unusedDouble >> unusedDouble >> unusedDouble >> unusedDouble>> unusedDouble >> unusedDouble >> unusedDouble >> fDrillHoleRadius;
		fDrillHoleRadius *= fConversionFactor / 2.;
	} else if (fileFormat=="rowsanddepths") {
		// RowsAndDepths format uses mm
		fConversionFactor = 1. * mm;

		// Convert from diameter to radius
		inCompensator >> fNumRows >> fMainCylinderThickness >> fDrillHoleRadius;
		fDrillHoleRadius *= .5;
	} else {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << fPm->GetStringParameter(GetFullParmName("FileFormat")) << " has invalid value: " << fPm->GetStringParameter(GetFullParmName("FileFormat")) << G4endl;
		G4cerr << "Must be either MGH or RowsAndDepths." << G4endl;
		fPm->AbortSession(1);
	}

	if (fNumRows <= 0 || fMainCylinderThickness <= 0. || fDrillHoleRadius <= 0. ||
		fNumRows > 1E6 || fMainCylinderThickness > 1E6 || fDrillHoleRadius > 1E6) {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cout << "Incorrect data format in Compensator file: " << fFileName << G4endl;
		fPm->AbortSession(1);
	}

	// Store the thickness as a parameter so that parameter calculations can take this value into account.
	G4String transName  = "dc:Ge/" + fName + "/Thickness";
	G4String transValue = G4UIcommand::ConvertToString(fMainCylinderThickness/cm) + " cm";
	fPm->AddParameter(transName, transValue);

	// Recalculate placement immediately since thickness may affect position
	if (fPm->GetBooleanParameter("Ge/CheckForOverlaps")) CalculatePlacement();

	G4int numColumns;
	G4double xStep, xStart, yStart, depth;
	for (G4int rowIter = 0; rowIter < fNumRows; rowIter++)
	{
		inCompensator >> numColumns;
		inCompensator >> xStep;
		inCompensator >> xStart;
		inCompensator >> yStart;
		fXSteps.push_back ( xStep * fConversionFactor );
		fXStarts.push_back ( xStart * fConversionFactor );
		fYStarts.push_back ( yStart * fConversionFactor );

		std::vector<G4double> oneRow;
		for (G4int columnIter = 0; columnIter < numColumns; columnIter++)
		{
			inCompensator >> depth;
			if (depth > fMainCylinderThickness) {
				G4cerr << "Topas is exiting due to a serious error." << G4endl;
				G4cout << "At least one depth in Compensator file " << fFileName << " exceeds the compensator thickness." << G4endl;
				fPm->AbortSession(1);
			}
			oneRow.push_back(depth * fConversionFactor);
		}
		fDepths.push_back(oneRow);
	}

	fMainCylinderRadius = fPm->GetDoubleParameter(GetFullParmName("RMax"),"Length");
	fMainCylinderRadiusSquared = fMainCylinderRadius*fMainCylinderRadius;

	fMainCylinder = new G4Tubs(fName, .0, fMainCylinderRadius, fMainCylinderThickness/2., 0., 360.);

	G4String method = fPm->GetStringParameter(GetFullParmName("Method"));
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(method);
#else
	method.toLower();
#endif

	if (method == "polyhedra")
		BuildAsPolyhedra();
	else if (method == "extrudedsolid")
		BuildAsExtrudedSolid();
	else if (method == "subtractioncylinders")
		BuildAsSubtractionSolid();
	else if (method == "unioncylinders")
		BuildAsUnionSolid();
	else {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << fPm->GetStringParameter(GetFullParmName("Method")) << " has invalid value: " << fPm->GetStringParameter(GetFullParmName("Method")) << G4endl;
		G4cerr << "Must be either Polyhedra, ExtrudedSolid, SubtractionCylinders or UnionCylinders." << G4endl;
		fPm->AbortSession(1);
	}

	return fEnvelopePhys;
}


void TsCompensator::BuildAsPolyhedra()
{
	// Create the main cylinder volume
	fEnvelopeLog = CreateLogicalVolume(fMainCylinder);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

	// Voids will be filled with the compensator's parent material
	G4String voidMaterial = fParentComponent->GetResolvedMaterialName();

	/* Polyhedra have regular hexagonal cross-section in xy, nested as below:
	 \ / \ / \ / \ /
	  |   |   |   |
	 / \ / \ / \ / \
	|   |   |   |   |
	 \ / \ / \ / \ /
	 ShortRadius is distance from center to a side.
	 LongRadous is distance from center to a corner.
	 From basic properties of regular hexagons:
	 LongRadius = ShortRadius over cos 30 deg = ( 2 / root3 ) * ShortRadius
	 XStep = 2. * ShortRadius
	 YStep = 1.5 * LongRadius
  	 Note that XStep may be either sign and may differ from row to row */
	G4double xAbsStep = fabs(fXSteps[0]);
	G4double shortRadius = xAbsStep / 2;
	G4double longRadius = ( 2./ sqrt(3.) ) * shortRadius;
	G4double yStep = 1.5 * longRadius;

	// Set sign of yStep according to direction of change in rows.
	if ( fYStarts[1] < fYStarts[0]) yStep *= -1.;

	// Tolerances for whether actual xy points are sufficiently regular to approximate with regular hexagons.
	G4double xTolerance = 0.01;
	if (fPm->ParameterExists(GetFullParmName("XTolerance")))
		xTolerance = fPm->GetDoubleParameter(GetFullParmName("XTolerance"),"Length");
	G4double yTolerance = 0.01;
	if (fPm->ParameterExists(GetFullParmName("YTolerance")))
		yTolerance = fPm->GetDoubleParameter(GetFullParmName("YTolerance"),"Length");

	// The xStarts for each row given in the file may not actually line up for perfect nesting.
	// We will move each point to an idealized xStarts, as close as possible to the actual xStart,
	// such that each row will perfectly nest with the previous row.
	// First calculate the set of possible idealized xStarts for even and odd rows.
	std::vector<G4double> possibleXStartsEven;
	std::vector<G4double> possibleXStartsOdd;

	// Store every x point that is exactly one xAbsStep before row 0's xStart
	for (G4double xPos = fXStarts[0]; xPos > - 2. * fMainCylinderRadius; xPos -= xAbsStep) {
		possibleXStartsOdd.push_back(xPos + xAbsStep/2);
		possibleXStartsEven.push_back(xPos);
	}

	// Store every x point that is exactly one xAbsStep beyond row 0's xStart
	for (G4double xPos = fXStarts[0]; xPos < 2. * fMainCylinderRadius; xPos += xAbsStep) {
		possibleXStartsOdd.push_back(xPos + xAbsStep/2);
		possibleXStartsEven.push_back(xPos);
	}

	for (G4int rowIter = 0; rowIter < fNumRows; rowIter++)
	{
		// Find the idealized x start nearest to the real one
		G4double idealXStart;
		if (rowIter==0) {
			idealXStart = fXStarts[0];
		} else {
			std::vector<G4double> differences;
			if (rowIter%2 > 0) { // odd-numbered row
				for (size_t i=0; i < possibleXStartsOdd.size(); i++)
					differences.push_back( fabs( possibleXStartsOdd[i] - fXStarts[rowIter]) );
				G4int nearestIndex = std::min_element(differences.begin(), differences.end()) - differences.begin();
				idealXStart = possibleXStartsOdd[nearestIndex];
			} else { // even-numbered row
				for (size_t i=0; i < possibleXStartsEven.size(); i++)
					differences.push_back( fabs( possibleXStartsEven[i] - fXStarts[rowIter]) );
				G4int nearestIndex = std::min_element(differences.begin(), differences.end()) - differences.begin();
				idealXStart = possibleXStartsEven[nearestIndex];
			}
		}

		// Insert the hexagonal polyhedra into the compensator
		G4double idealPosY = fYStarts[0] + rowIter * yStep;

		if (fPm->ParameterExists(GetFullParmName("PrintPoints")) &&
			fPm->GetBooleanParameter(GetFullParmName("PrintPoints")))
			G4cout << "\nrow: " << rowIter << ", realPosY: " << fYStarts[rowIter] << ", idealPosY: " << idealPosY <<
			", diffY: " << idealPosY - fYStarts[rowIter] << ", yStep: " << yStep << G4endl;

		if ( fabs( idealPosY - fYStarts[rowIter] ) > yTolerance )
		{
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "Compensator file cannot be represented with Polyhedra method." << G4endl;
			G4cerr << "The file's row " << rowIter << " has Y Postion too far from a regular haxagonal grid." << G4endl;
			G4cerr << "Y distance from grid is " << fabs( idealPosY - fYStarts[rowIter] ) << " mm while the yTolerance is " << yTolerance << " mm" << G4endl;
			fPm->AbortSession(1);
		}

		// Match the sign of this row's idealXStep with this row's actual XStep
		G4double idealXStep = xAbsStep;
		if (fXSteps[rowIter] < 0) idealXStep *= -1;

		G4double realPosX = fXStarts[rowIter];

		for (size_t columnIter = 0; columnIter < fDepths[rowIter].size(); columnIter++)
		{
			G4double depth = fDepths[rowIter][columnIter];
			if ( depth > 0.)
			{
				G4double Zvalues[2]= {-depth/2, depth/2};
				G4double RMinVec[2]= {0.0, 0.0};
				G4double RMaxVec[2]= {shortRadius, shortRadius};

				G4double idealPosX = idealXStart + columnIter * idealXStep;


				if (fPm->ParameterExists(GetFullParmName("PrintPoints")) &&
					fPm->GetBooleanParameter(GetFullParmName("PrintPoints")))
					G4cout << "realPosX: " << realPosX << ", idealPosX: " << idealPosX <<
					", diffX: " << idealPosX - realPosX << ", xSteps: " << fXSteps[rowIter] << G4endl;

				if ( fabs( idealPosX - realPosX ) > xTolerance )
				{
					G4cerr << "Topas is exiting due to a serious error." << G4endl;
					G4cerr << "Compensator file cannot be represented with Polyhedra method." << G4endl;
					G4cerr << "The file's point at row " << rowIter << ", column " << columnIter << " has X Postion too far from a regular haxagonal grid." << G4endl;
					G4cerr << "X distance from grid is " << fabs( idealPosX - realPosX ) << " mm while the XTolerance is " << xTolerance << " mmm" << G4endl;
					fPm->AbortSession(1);
				}

				if (idealPosX*idealPosX + idealPosY*idealPosY > fMainCylinderRadiusSquared) {
					G4cerr << "Topas is exiting due to a serious error." << G4endl;
					G4cout << "At least point in Compensator file " << fFileName << " exceeds the compensator radius." << G4endl;
					fPm->AbortSession(1);
				}

				G4double posZ = ( depth - fMainCylinderThickness ) / 2.;

				G4VSolid* voidSolid = new G4Polyhedra("Void", 0, 2.*pi, 6, 2, Zvalues, RMinVec, RMaxVec);
				G4LogicalVolume *voidLog = CreateLogicalVolume("Void", voidMaterial, voidSolid);
				CreatePhysicalVolume("Void", voidLog, 0, new G4ThreeVector(idealPosY,idealPosX,posZ), fEnvelopePhys);
			}
			realPosX += fXSteps[rowIter];
		}
	}
}


// For each row of cylinders in the compensator specification, create one G4ExtrudedSolid.
// Adapted from code by Dae-Hyun (Danny) Kim at Catholic University of Korea.
void TsCompensator::BuildAsExtrudedSolid()
{
	// Create the main cylinder volume
	fEnvelopeLog = CreateLogicalVolume(fMainCylinder);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

	// Voids will be filled with the compensator's parent material
	G4String voidMaterial = fParentComponent->GetResolvedMaterialName();

	G4double yStep = fYStarts[1] - fYStarts[0];

	for (G4int rowIter = 0; rowIter < fNumRows; rowIter++)
	{
		G4int numColumns = fDepths[rowIter].size();
		G4double xStart = fXStarts[rowIter];
		G4double yStart = fYStarts[0] + yStep * rowIter;

		if (xStart*xStart + yStart*yStart > fMainCylinderRadiusSquared) {
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cout << "At least point in Compensator file " << fFileName << " exceeds the compensator radius." << G4endl;
			fPm->AbortSession(1);
		}

		G4double xStep = fXSteps[rowIter];

		std::vector<G4TwoVector> compensatorRowPointsXZ;

		// Add near side of flat top surface
		compensatorRowPointsXZ.push_back(G4TwoVector(xStart, 0.0));

		for (G4int columnIter = 0; columnIter < numColumns; columnIter++)
		{
			if (fDepths[rowIter][columnIter] > 0) {
 				G4double drillX = xStart + (columnIter * xStep);
				compensatorRowPointsXZ.push_back(G4TwoVector(drillX, fDepths[rowIter][columnIter]));
			}
		}

		// Add far side of flat top surface
		G4double finalXThisRow = xStart + (numColumns-1) * xStep;
		compensatorRowPointsXZ.push_back(G4TwoVector(finalXThisRow, 0.0));

		G4VSolid* rowSolid = new G4ExtrudedSolid("Void", compensatorRowPointsXZ, fabs(yStep/2), G4TwoVector(0, 0), 1.0, G4TwoVector(0, 0), 1.0);
		G4LogicalVolume* rowLog = CreateLogicalVolume("Void", voidMaterial, rowSolid);

		G4double centerX = xStart + (finalXThisRow - xStart)/2;
		G4double centerZ = - fMainCylinderThickness / 2.;
		G4ThreeVector* position = new G4ThreeVector(yStart, centerX, centerZ);
		G4RotationMatrix* rot = new G4RotationMatrix(90 * degree, 90 * degree, 0.0);
		CreatePhysicalVolume("Void", rowLog, rot, position, fEnvelopePhys);
	}
}


void TsCompensator::BuildAsUnionSolid()
{
	std::vector<G4Tubs*> drillHoleCylinders = GetDrillHoleCylinders();
	std::vector<G4ThreeVector*> drillHoleLocations = GetDrillHoleLocations();

	// Combine the first two cylinders
	G4ThreeVector* offset = new G4ThreeVector( (*drillHoleLocations[1]).x() - (*drillHoleLocations[0]).x(),
											  (*drillHoleLocations[1]).y() - (*drillHoleLocations[0]).y(),
											  (*drillHoleLocations[1]).z() - (*drillHoleLocations[0]).z() );
	G4UnionSolid* unionSolid = new G4UnionSolid(fName, drillHoleCylinders[0], drillHoleCylinders[1], 0, *offset);

	// Combine the rest of the cylinders
	for (size_t i = 2; i < drillHoleLocations.size(); i++)
	{
		offset = new G4ThreeVector( (*drillHoleLocations[i]).x() - (*drillHoleLocations[0]).x(),
								   (*drillHoleLocations[i]).y() - (*drillHoleLocations[0]).y(),
								   (*drillHoleLocations[i]).z() - (*drillHoleLocations[0]).z() );
		unionSolid = new G4UnionSolid(fName, unionSolid, drillHoleCylinders[i], 0, *offset);
	}

	// Subtract the combined set of cylinders from the main cylinder solid
	G4SubtractionSolid* subtractionSolid = new G4SubtractionSolid(fName, fMainCylinder, unionSolid, 0, *(drillHoleLocations[0]));

	fEnvelopeLog = CreateLogicalVolume(subtractionSolid);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);
}


void TsCompensator::BuildAsSubtractionSolid()
{
	std::vector<G4Tubs*> drillHoleCylinders = GetDrillHoleCylinders();
	std::vector<G4ThreeVector*> drillHoleLocations = GetDrillHoleLocations();

	// Subtract the first cylinder from the main cylinder solid
	G4SubtractionSolid* subtractionSolid = new G4SubtractionSolid(fName, fMainCylinder, drillHoleCylinders[0], 0, *(drillHoleLocations[0]));

	// Subrtract the rest of the cylinders
	for (size_t i = 1; i < drillHoleLocations.size(); i++)
		subtractionSolid = new G4SubtractionSolid(fName, subtractionSolid, drillHoleCylinders[i], 0, *( drillHoleLocations[i]));

	fEnvelopeLog = CreateLogicalVolume(subtractionSolid);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);
}


std::vector<G4Tubs*> TsCompensator::GetDrillHoleCylinders()
{
	std::vector<G4Tubs*> cylinders;
	for (G4int rowIter = 0; rowIter < fNumRows; rowIter++)
	{
		for (size_t columnIter = 0; columnIter < fDepths[rowIter].size(); columnIter++)
		{
			G4double depth = fDepths[rowIter][columnIter];
			if (depth > 0)
				cylinders.push_back(new G4Tubs("Void", 0, fDrillHoleRadius, (depth/2), 0, 360.));
		}
	}
	return cylinders;
}


std::vector<G4ThreeVector*> TsCompensator::GetDrillHoleLocations()
{
	std::vector<G4ThreeVector*> locations;
	G4double drillX;
	G4double drillY;
	G4double drillZ;

	for (G4int rowIter = 0; rowIter < fNumRows; rowIter++)
	{
		if ( fabs(fXSteps[rowIter]) > fDrillHoleRadius*2. )
		{
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "Compensator file cannot be represented with SubtractionCylinders or UnionCylinders method" << G4endl;
			G4cerr << "since columns one or more columns are farther apart than drill hole diameter." << G4endl;
			fPm->AbortSession(1);
		}

		for (size_t columnIter = 0; columnIter < fDepths[rowIter].size(); columnIter++)
		{
			if ( fabs( fYStarts[rowIter] - fYStarts[rowIter-1] ) > fDrillHoleRadius*2. )
			{
				G4cerr << "Topas is exiting due to a serious error." << G4endl;
				G4cerr << "Compensator file cannot be represented with SubtractionCylinders or UnionCylinders method" << G4endl;
				G4cerr << "since one or more rows are farther apart than drill hole diameter." << G4endl;
				fPm->AbortSession(1);
			}

			if (fDepths[rowIter][columnIter] > 0)
			{
				drillX = fXStarts[rowIter] + (columnIter * fXSteps[rowIter]);
				drillY = fYStarts[rowIter];

				if (drillX*drillX + drillY*drillY > fMainCylinderRadiusSquared) {
					G4cerr << "Topas is exiting due to a serious error." << G4endl;
					G4cout << "At least point in Compensator file " << fFileName << " exceeds the compensator radius." << G4endl;
					fPm->AbortSession(1);
				}

				drillZ = ( fDepths[rowIter][columnIter] - fMainCylinderThickness ) / 2.;

				// Order of coordinates is set to match Polyhedra method
				locations.push_back(new G4ThreeVector(drillY, drillX, drillZ));
			}
		}
	}
	return locations;
}
