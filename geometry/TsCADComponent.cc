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

#include "TsCADComponent.hh"

#include "TsParameterManager.hh"

#include "G4TessellatedSolid.hh"
#include "G4TriangularFacet.hh"
#include "G4QuadrangularFacet.hh"
#include "G4AssemblyVolume.hh"
#include "G4Box.hh"
#include "G4Tet.hh"
#include "G4UIcommand.hh"

#include <fstream>

TsCADComponent::TsCADComponent(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
							   TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name)
	: TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name),
	  fVertex(nullptr), fUnits(0), fSolid(nullptr)
{
}


TsCADComponent::~TsCADComponent()
{
}


G4VPhysicalVolume* TsCADComponent::Construct()
{
	BeginConstruction();

	fUnits = fPm->GetDoubleParameter(GetFullParmName("Units"), "Length");

	fFileName = fPm->GetStringParameter(GetFullParmName("InputFile"));

	G4String fileFormat = fPm->GetStringParameter(GetFullParmName("FileFormat"));
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(fileFormat);
#else
	fileFormat.toLower();
#endif

	if ( fileFormat == "stl" )
		ReadCADFromSTL();
	else if ( fileFormat == "ply" )
		ReadCADFromPLY();
	else if ( fileFormat == "tet" ) {
		ReadCADFromTET();
		InstantiateChildren();
		return fEnvelopePhys;
	}

	fEnvelopeLog = CreateLogicalVolume(fSolid);

	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

	InstantiateChildren();

	return fEnvelopePhys;
}


void TsCADComponent::ReadCADFromPLY() {
	fFileName += ".ply";

	std::ifstream cadFile(fFileName);
	if (!cadFile) {
		G4cerr << "TOPAS is exiting due a serious error in geometry setup." << G4endl;
		G4cerr << "PLY ascii file: " << fFileName << " is not found." << G4endl;
		fPm->AbortSession(1);
	}

	G4String aLine;
	G4int nvertex;
	G4int nfaces;

	while (!cadFile.eof()) {
		getline(cadFile,aLine);

#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(aLine);
#else
		aLine.toLower();
#endif
		if ( aLine == "end_header" ) break;

		if ( aLine.find("binary")!=std::string::npos) {
			G4cerr << "TOPAS is exiting due a serious error in geometry setup" << G4endl;
			G4cerr << "Only PLY files in ascii format are supported." << G4endl;
			fPm->AbortSession(1);
		}

		if (aLine.find("element vertex")!=std::string::npos) {
			aLine = aLine.substr(14,aLine.length());
			std::istringstream input(aLine);
			input >> nvertex;
			G4cout << nvertex << G4endl;
		}

		if (aLine.find("element face")!=std::string::npos) {
			aLine = aLine.substr(13,aLine.length());
			std::istringstream input(aLine);
			input >> nfaces;
			G4cout << nfaces << G4endl;
		}
	}

	double valueInCAD;
	fVertex = new double[3 * nvertex];
	for ( G4int i = 0; i < 3 * nvertex; i++ ) {
		cadFile >> valueInCAD ;
		fVertex[i] = valueInCAD;
	}

	G4int index = 0;
	G4int ntriangular = 0;
	G4int nquadrangular = 0;

	fSolid = new G4TessellatedSolid(fName);
	G4ThreeVector p1 = G4ThreeVector();
	G4ThreeVector p2 = G4ThreeVector();
	G4ThreeVector p3 = G4ThreeVector();
	G4ThreeVector p4 = G4ThreeVector();

	for ( G4int i = 0; i < nfaces; i++ ) {
		double x1, y1, z1;
		double x2, y2, z2;
		double x3, y3, z3;
		double x4, y4, z4;
		G4int n;
		cadFile >> n;
		if ( n == 3 ) { // Triagular facet
			cadFile >> index;
			x1 = fVertex[3*index];
			y1 = fVertex[3*index+1];
			z1 = fVertex[3*index+2];
			cadFile >> index;
			x2 = fVertex[3*index];
			y2 = fVertex[3*index+1];
			z2 = fVertex[3*index+2];
			cadFile >> index;
			x3 = fVertex[3*index];
			y3 = fVertex[3*index+1];
			z3 = fVertex[3*index+2];

			p1 = G4ThreeVector(x1*fUnits, y1*fUnits, z1*fUnits);
			p2 = G4ThreeVector(x2*fUnits, y2*fUnits, z2*fUnits);
			p3 = G4ThreeVector(x3*fUnits, y3*fUnits, z3*fUnits);
			G4TriangularFacet* facet = new G4TriangularFacet(p1, p2, p3, ABSOLUTE);
			fSolid->AddFacet((G4VFacet*) facet);

			ntriangular++;
		} else if ( n == 4 ) { // G4QuadrangularFacet
			cadFile >> index;
			x1 = fVertex[3*index];
			y1 = fVertex[3*index+1];
			z1 = fVertex[3*index+2];
			cadFile >> index;
			x2 = fVertex[3*index];
			y2 = fVertex[3*index+1];
			z2 = fVertex[3*index+2];
			cadFile >> index;
			x3 = fVertex[3*index];
			y3 = fVertex[3*index+1];
			z3 = fVertex[3*index+2];
			cadFile >> index;
			x4 = fVertex[3*index];
			y4 = fVertex[3*index+1];
			z4 = fVertex[3*index+2];

			p1 = G4ThreeVector(x1*fUnits, y1*fUnits, z1*fUnits);
			p2 = G4ThreeVector(x2*fUnits, y2*fUnits, z2*fUnits);
			p3 = G4ThreeVector(x3*fUnits, y3*fUnits, z3*fUnits);
			p4 = G4ThreeVector(x4*fUnits, y4*fUnits, z4*fUnits);
			G4QuadrangularFacet* facet = new G4QuadrangularFacet(p1, p2, p3, p4, ABSOLUTE);
			fSolid->AddFacet((G4VFacet*) facet);

			nquadrangular++;
		}
	}
	fSolid->SetSolidClosed(true);
	G4cout << "Imported CAD file in ASCII PLY format: " << fFileName << G4endl;
	G4cout << "Found " << ntriangular << " triangular facets" << G4endl;
	G4cout << "Found " << nquadrangular << " quadrangular facets" << G4endl;
}


void TsCADComponent::ReadCADFromSTL() {
	fFileName += ".stl";

	fSolid = new G4TessellatedSolid(fName);

	std::ifstream cadFile(fFileName, std::ios::binary);
	if (!cadFile) {
		G4cerr << "TOPAS is exiting due a serious error in geometry setup" << G4endl;
		G4cerr << "STL binary file: " << fFileName << " is not found." << G4endl;
		fPm->AbortSession(1);
	}

	G4int facenum;
	cadFile.seekg(80);
	cadFile.read(reinterpret_cast<char*>(&facenum), sizeof facenum);

	G4ThreeVector p1 = G4ThreeVector();
	G4ThreeVector p2 = G4ThreeVector();
	G4ThreeVector p3 = G4ThreeVector();

	G4int ntriangular = 0;
	for ( int i = 0; i < facenum; ++i ) {
		short attr;
		G4float norm1, norm2, norm3;
		G4float x1, y1, z1;
		G4float x2, y2, z2;
		G4float x3, y3, z3;
		cadFile.read(reinterpret_cast<char*>(&norm1), sizeof norm1);
		cadFile.read(reinterpret_cast<char*>(&norm2), sizeof norm2);
		cadFile.read(reinterpret_cast<char*>(&norm3), sizeof norm3);
		cadFile.read(reinterpret_cast<char*>(&x1), sizeof x1);
		cadFile.read(reinterpret_cast<char*>(&y1), sizeof y1);
		cadFile.read(reinterpret_cast<char*>(&z1), sizeof z1);
		cadFile.read(reinterpret_cast<char*>(&x2), sizeof x2);
		cadFile.read(reinterpret_cast<char*>(&y2), sizeof y2);
		cadFile.read(reinterpret_cast<char*>(&z2), sizeof z2);
		cadFile.read(reinterpret_cast<char*>(&x3), sizeof x3);
		cadFile.read(reinterpret_cast<char*>(&y3), sizeof y3);
		cadFile.read(reinterpret_cast<char*>(&z3), sizeof z3);
		cadFile.read(reinterpret_cast<char*>(&attr), sizeof attr);
		if ( fabs(x1) <= 1e-6 ) x1 = 0.0;
		if ( fabs(x2) <= 1e-6 ) x2 = 0.0;
		if ( fabs(x3) <= 1e-6 ) x3 = 0.0;
		if ( fabs(y1) <= 1e-6 ) y1 = 0.0;
		if ( fabs(y2) <= 1e-6 ) y2 = 0.0;
		if ( fabs(y3) <= 1e-6 ) y3 = 0.0;
		if ( fabs(z1) <= 1e-6 ) z1 = 0.0;
		if ( fabs(z2) <= 1e-6 ) z2 = 0.0;
		if ( fabs(z3) <= 1e-6 ) z3 = 0.0;

		p1 = G4ThreeVector(x1*fUnits, y1*fUnits, z1*fUnits);
		p2 = G4ThreeVector(x2*fUnits, y2*fUnits, z2*fUnits);
		p3 = G4ThreeVector(x3*fUnits, y3*fUnits, z3*fUnits);
		G4TriangularFacet* facet = new G4TriangularFacet(p1, p2, p3, ABSOLUTE);
		fSolid->AddFacet((G4VFacet*) facet);
		ntriangular++;
	}
	cadFile.close();
	fSolid->SetSolidClosed(true);
	G4cout << "Imported CAD binary file in STL format: " << fFileName << G4endl;
	G4cout << "Found " << ntriangular << " triangular facets" << G4endl;
}

void TsCADComponent::ReadCADFromTET() {

	G4String materialName = GetResolvedMaterialName();

	G4String fileName = fFileName + ".node";
	std::ifstream nodeFile(fileName);
	if (!nodeFile) {
		G4cerr << "TOPAS is exiting due a serious error in geometry setup" << G4endl;
		G4cerr << "NODE file: " << fileName << " is not found." << G4endl;
		fPm->AbortSession(1);
	}

	fileName = fFileName + ".ele";
	std::ifstream eleFile(fileName);
	if (!eleFile) {
		G4cerr << "TOPAS is exiting due a serious error in geometry setup" << G4endl;
		G4cerr << "ELE file: " << fileName << " is not found." << G4endl;
		fPm->AbortSession(1);
	}

	G4int nLinesInEle, nLinesInNode, dummy;
	G4double x, y, z;
	nodeFile >> nLinesInNode >> dummy >> dummy >> dummy;
	std::vector< G4ThreeVector > node_xyz;
	G4ThreeVector xyz = G4ThreeVector();
	G4double minX = 1e9, maxX = 0, minY = 1e9, maxY = 0, minZ = 1e9, maxZ = 0;

	for ( int i = 1; i <= nLinesInNode; i++ ) {
		nodeFile >> dummy >> x >> y >> z;
		xyz = G4ThreeVector(x*fUnits, y*fUnits, z*fUnits);
		node_xyz.push_back(xyz);

		if ( minX > x )
			minX = x;

		if ( minY > y )
			minY = y;

		if ( minZ > z )
			minZ = z;

		if ( maxX < x )
			maxX = x;

		if ( maxY < y )
			maxY = y;

		if ( maxZ < z )
			maxZ = z;
	}

	G4double HLX, HLY, HLZ;
	if ( fabs(minX) < fabs(maxX) )
		HLX = fabs(maxX)/2;
	else
		HLX = fabs(minX)/2;

	if ( fabs(minY) < fabs(maxY) )
		HLY = fabs(maxY)/2;
	else
		HLY = fabs(minY)/2;

	if ( fabs(minZ) < fabs(maxZ) )
		HLZ = fabs(maxZ)/2;
	else
		HLZ = fabs(minZ)/2;

	G4VSolid* envelope = new G4Box(fName, HLX*fUnits, HLY*fUnits, HLZ*fUnits);
	// Let the envelope's material the same as the parent component
	G4String envelopeMaterialName = fParentComponent->GetResolvedMaterialName();
	fEnvelopeLog = CreateLogicalVolume(fName, envelopeMaterialName, envelope);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

	eleFile >> nLinesInEle >> dummy >> dummy;
	G4ThreeVector p1 = G4ThreeVector();
	G4ThreeVector p2 = G4ThreeVector();
	G4ThreeVector p3 = G4ThreeVector();
	G4ThreeVector p4 = G4ThreeVector();
	G4int id1, id2, id3, id4;
	G4RotationMatrix* tetRot = new G4RotationMatrix();
	G4ThreeVector tetPos = G4ThreeVector();

	G4AssemblyVolume* assemblyVolume = new G4AssemblyVolume();
	for ( int i = 1; i <= nLinesInEle; i++ ) {
		eleFile >> dummy >> id1 >> id2 >> id3 >> id4;
		p1 = node_xyz[id1];
		p2 = node_xyz[id2];
		p3 = node_xyz[id3];
		p4 = node_xyz[id4];
		G4String tetName = fFileName + G4UIcommand::ConvertToString(i);
		G4VSolid* tet = new G4Tet(tetName, p1, p2, p3, p4, 0);
		G4LogicalVolume* tetLog = CreateLogicalVolume(tetName, materialName, tet);
		assemblyVolume->AddPlacedVolume(tetLog, tetPos, tetRot);
	}
	G4ThreeVector* position = new G4ThreeVector(-HLX*fUnits,-HLY*fUnits,-HLZ*fUnits);
	assemblyVolume->MakeImprint(fEnvelopeLog, *position, tetRot);
	G4cout << "Imported file with tetrahedra: " << fFileName << G4endl;
	G4cout << "Found " << nLinesInEle << " tetrahedra volumes" << G4endl;
}
