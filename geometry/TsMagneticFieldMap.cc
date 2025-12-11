//
// ********************************************************************
// *                                                                  *
// * Copyright 2025 The TOPAS Collaboration                           *
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

#include "TsParameterManager.hh"

#include "TsMagneticFieldMap.hh"
#include "TsVGeometryComponent.hh"

#include "G4SystemOfUnits.hh"
#include "G4TransportationManager.hh"
#include "G4Tokenizer.hh"
#include "G4ChordFinder.hh"

#include <array>
#include <fstream>
#include <locale>
#include <map>
#include <set>

TsMagneticFieldMap::TsMagneticFieldMap(TsParameterManager* pM,TsGeometryManager* gM, TsVGeometryComponent* component):
TsVMagneticField(pM, gM, component), fInvertX(false), fInvertY(false), fInvertZ(false), fNX(0), fNY(0), fNZ(0) {
	ResolveParameters();
}


TsMagneticFieldMap::~TsMagneticFieldMap() {
	fFieldX.clear();
	fFieldY.clear();
	fFieldZ.clear();
	if(fChordFinder) delete fChordFinder;
}


void TsMagneticFieldMap::ResolveParameters() {
	G4String filename = fPm->GetStringParameter(fComponent->GetFullParmName("MagneticField3DTable"));

	std::ifstream file(filename);
	if (!file) {
		G4cerr << "" << G4endl;
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "The parameter: " << fComponent->GetFullParmName("MagneticField3DTable") << G4endl;
		G4cerr << "references a MagneticField3DTable file that cannot be found:" << G4endl;
		G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("MagneticField3DTable")) << G4endl;
		fPm->AbortSession(1);
	}

	// Get the extension of the file and check if it is supported
    size_t f = filename.find_last_of(".");
    G4String fileExtension;
    if (f != std::string::npos) {
        fileExtension = filename.substr(f + 1);
    } else {
        fileExtension = "";
    }
    G4cout << "File extension: " << fileExtension << G4endl;

	file.close();

    if (fileExtension != "csv" && fileExtension != "TABLE") {
        G4cerr << "" << G4endl;
        G4cerr << "Topas is exiting due to a serious error." << G4endl;
        G4cerr << "The parameter: " << fComponent->GetFullParmName("MagneticField3DTable") << G4endl;
        G4cerr << "references a MagneticField3DTable file with an unsupported extension:" << G4endl;
        G4cerr << fileExtension << G4endl;
        G4cerr << "Only .csv and .TABLE (Opera3D) files are supported." << G4endl;
        fPm->AbortSession(1);
    }

    // Call appropriate reader based on file extension
    if (fileExtension == "TABLE") {
        ReadOpera3DFile(filename);
    } else if (fileExtension == "csv") {
        ReadCSVFile(filename);
    }

	const G4RotationMatrix* rotM = fComponent->GetRotRelToWorld();
	G4Point3D* fTransRelToWorld = GetComponent()->GetTransRelToWorld();
	G4ThreeVector transl = G4ThreeVector(fTransRelToWorld->x(),fTransRelToWorld->y(),fTransRelToWorld->z());
	fAffineTransf = G4AffineTransform(rotM,transl);
}


void TsMagneticFieldMap::ReadCSVFile(const G4String& filename) {
	std::ifstream file(filename);
    // Error: file not found
    if (!file) {
        G4cerr << "" << G4endl;
        G4cerr << "Topas is exiting due to a serious error." << G4endl;
        G4cerr << "The parameter: " << fComponent->GetFullParmName("MagneticField3DTable") << G4endl;
        G4cerr << "references an MagneticField3DTable file that cannot be found:" << G4endl;
        G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("MagneticField3DTable")) << G4endl;
        fPm->AbortSession(1);
    }

    // Read the CSV header line
    G4String headerLine;
    if (!std::getline(file, headerLine)) {
        G4cerr << "" << G4endl;
        G4cerr << "Topas is exiting due to a serious error." << G4endl;
        G4cerr << "CSV file is empty or unreadable:" << G4endl;
        G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("MagneticField3DTable")) << G4endl;
        fPm->AbortSession(1);
    }

    // Parse header to extract field names and units
    // Expected format (example): "X [mm]", "Y [mm]", "Z [mm]", "Bx [T]", "By [T]", "Bz [T]"
    std::vector<G4String> headerFields;
    std::map<G4String, double> headerUnits;
    std::map<G4String, int> columnIndices;

    std::stringstream headerStream(headerLine);
    G4String column;
    int colIndex = 0;

    while (std::getline(headerStream, column, ',')) {
        // Trim the whitespaces
        column.erase(0, column.find_first_not_of(" \t\n\r\f\v"));
        column.erase(column.find_last_not_of(" \t\n\r\f\v") + 1);

        // Parse the field name and unit from "fieldname [unit]" format
        size_t bracketStart = column.find('[');
        size_t bracketEnd = column.find(']');

        G4String fieldName;
        G4String unitString;

        if (bracketStart != std::string::npos && bracketEnd != std::string::npos) {
            fieldName = column.substr(0, bracketStart);
            // Trim whitespace from field name
            fieldName.erase(fieldName.find_last_not_of(" \t\n\r\f\v") + 1);

            unitString = column.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
            // Trim whitespace from unit string
            unitString.erase(0, unitString.find_first_not_of(" \t\n\r\f\v"));
            unitString.erase(unitString.find_last_not_of(" \t\n\r\f\v") + 1);

        } else {
            // Throw error if no units are specified
            G4cerr << "" << G4endl;
            G4cerr << "Topas is exiting due to a serious error." << G4endl;
            G4cerr << "CSV header must specify units for all columns in the format: fieldname [unit]" << G4endl;
            G4cerr << "File: " << fPm->GetStringParameter(fComponent->GetFullParmName("MagneticField3DTable")) << G4endl;
            fPm->AbortSession(1);
        }

        headerFields.push_back(fieldName);
        columnIndices[fieldName] = colIndex;

        // Map unit strings to Geant4 units
        double unitValue = 1.0;
        if (unitString == "mm") {
            unitValue = mm;
        } else if (unitString == "m") {
            unitValue = m;
        } else if (unitString == "cm") {
            unitValue = cm;
        } else if (unitString == "T") {
            unitValue = tesla;
        } else if (unitString == "G") {
            unitValue = tesla * 1.e-4;
        } else {
            G4cerr << "" << G4endl;
            G4cerr << "Topas is exiting due to a serious error." << G4endl;
            G4cerr << "Unknown unit '" << unitString << "' for column '" << fieldName << "'" << G4endl;
            G4cerr << "Supported units: mm, cm, m, T (Tesla), G (Gauss)" << G4endl;
            G4cerr << "File: " << fPm->GetStringParameter(fComponent->GetFullParmName("MagneticField3DTable")) << G4endl;
            fPm->AbortSession(1);
        }

        headerUnits[fieldName] = unitValue;
        colIndex++;
    }

    // Verify required columns exist
    if (columnIndices.find("X") == columnIndices.end() ||
        columnIndices.find("Y") == columnIndices.end() ||
        columnIndices.find("Z") == columnIndices.end() ||
        columnIndices.find("Bx") == columnIndices.end() ||
        columnIndices.find("By") == columnIndices.end() ||
        columnIndices.find("Bz") == columnIndices.end()) {
        G4cerr << "" << G4endl;
        G4cerr << "Topas is exiting due to a serious error." << G4endl;
        G4cerr << "CSV header must contain columns: X, Y, Z, Bx, By, Bz" << G4endl;
        G4cerr << "File: " << fPm->GetStringParameter(fComponent->GetFullParmName("MagneticField3DTable")) << G4endl;
        fPm->AbortSession(1);
    }

    // Read all the data points to determine the grid dimensions
    std::vector<double> xValues, yValues, zValues;
    std::set<double> uniqueX, uniqueY, uniqueZ;
    std::map<std::tuple<int,int,int>, std::array<double,3>> fieldData;

    G4String line;
    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.find_first_not_of(" \t\n\r\f\v") == std::string::npos)
            continue;

        std::stringstream lineStream(line);
        std::vector<G4String> values;
        G4String value;

        while (std::getline(lineStream, value, ',')) {
            values.push_back(value);
        }

        if (values.size() != headerFields.size()) {
            G4cerr << "" << G4endl;
            G4cerr << "Topas is exiting due to a serious error." << G4endl;
            G4cerr << "CSV line has " << values.size() << " columns but header has " << headerFields.size() << " columns" << G4endl;
            G4cerr << "Line: " << line << G4endl;
            fPm->AbortSession(1);
        }

        double x = atof(values[columnIndices["X"]]) * headerUnits["X"];
        double y = atof(values[columnIndices["Y"]]) * headerUnits["Y"];
        double z = atof(values[columnIndices["Z"]]) * headerUnits["Z"];

        xValues.push_back(x);
        yValues.push_back(y);
        zValues.push_back(z);

        uniqueX.insert(x);
        uniqueY.insert(y);
        uniqueZ.insert(z);

    }

    file.close();

    if (xValues.size() == 0) {
        G4cerr << "" << G4endl;
        G4cerr << "Topas is exiting due to a serious error." << G4endl;
        G4cerr << "No data points found in MagneticField3DTable file:" << G4endl;
        G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("MagneticField3DTable")) << G4endl;
        fPm->AbortSession(1);
    }

    // Determine grid dimensions
    fNX = uniqueX.size();
    fNY = uniqueY.size();
    fNZ = uniqueZ.size();

    G4cout << "Magnetic field map dimensions: " << fNX << " x " << fNY << " x " << fNZ << G4endl;

    // Sorted vectors for coordinate mapping
    std::vector<double> sortedX(uniqueX.begin(), uniqueX.end());
    std::vector<double> sortedY(uniqueY.begin(), uniqueY.end());
    std::vector<double> sortedZ(uniqueZ.begin(), uniqueZ.end());
    std::sort(sortedX.begin(), sortedX.end());
    std::sort(sortedY.begin(), sortedY.end());
    std::sort(sortedZ.begin(), sortedZ.end());

    fMinX = sortedX.front(), fMaxX = sortedX.back();
    fMinY = sortedY.front(), fMaxY = sortedY.back();
    fMinZ = sortedZ.front(), fMaxZ = sortedZ.back();

    // Initialize 3D field arrays
    fFieldX.resize(fNX);
    fFieldY.resize(fNX);
    fFieldZ.resize(fNX);
    for (int index_x = 0; index_x < fNX; index_x++) {
        fFieldX[index_x].resize(fNY);
        fFieldY[index_x].resize(fNY);
        fFieldZ[index_x].resize(fNY);
        for (int index_y = 0; index_y < fNY; index_y++) {
            fFieldX[index_x][index_y].resize(fNZ);
            fFieldY[index_x][index_y].resize(fNZ);
            fFieldZ[index_x][index_y].resize(fNZ);
        }
    }

    // Re-read file to populate field arrays
    file.open(fPm->GetStringParameter(fComponent->GetFullParmName("MagneticField3DTable")));
    // Skip header line
    std::getline(file, headerLine);

    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.find_first_not_of(" \t\n\r\f\v") == std::string::npos)
            continue;

        std::stringstream lineStream(line);
        std::vector<G4String> values;
        G4String value;

        while (std::getline(lineStream, value, ',')) {
            values.push_back(value);
        }

        double x = atof(values[columnIndices["X"]]) * headerUnits["X"];
        double y = atof(values[columnIndices["Y"]]) * headerUnits["Y"];
        double z = atof(values[columnIndices["Z"]]) * headerUnits["Z"];
        double bx = atof(values[columnIndices["Bx"]]) * headerUnits["Bx"];
        double by = atof(values[columnIndices["By"]]) * headerUnits["By"];
        double bz = atof(values[columnIndices["Bz"]]) * headerUnits["Bz"];

        // Find indices in sorted coordinate arrays
        int ix = std::distance(sortedX.begin(), std::find(sortedX.begin(), sortedX.end(), x));
        int iy = std::distance(sortedY.begin(), std::find(sortedY.begin(), sortedY.end(), y));
        int iz = std::distance(sortedZ.begin(), std::find(sortedZ.begin(), sortedZ.end(), z));

        fFieldX[ix][iy][iz] = bx;
        fFieldY[ix][iy][iz] = by;
        fFieldZ[ix][iy][iz] = bz;
    }
    file.close();

    fDX = fMaxX - fMinX;
    fDY = fMaxY - fMinY;
    fDZ = fMaxZ - fMinZ;

}

void TsMagneticFieldMap::ReadOpera3DFile(const G4String& filename) {
    std::ifstream file(filename);
    if (!file) {
        G4cerr << "" << G4endl;
        G4cerr << "Topas is exiting due to a serious error." << G4endl;
        G4cerr << "The parameter: " << fComponent->GetFullParmName("MagneticField3DTable") << G4endl;
        G4cerr << "references a MagneticField3DTable file that cannot be found:" << G4endl;
        G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("MagneticField3DTable")) << G4endl;
        fPm->AbortSession(1);
    }

    G4String line;
        bool ReadingHeader = true;
        G4int counter = 0;
        double xval = 0.,yval = 0.,zval = 0.,bx,by,bz;
        int ix = 0;
        int iy = 0;
        int iz = 0;
        std::map<G4String,double> headerUnits;
        std::vector<G4String> headerUnitStrings;
        std::vector<G4String> headerFields;

        while (file.good()) {
            getline(file,line);
            if (line.find_last_not_of(" \t\f\v\n\r") == std::string::npos)
                continue;

            std::string::size_type pos = line.find_last_not_of(' ');
            if(pos != std::string::npos) {
                line.erase(pos + 1);
                pos = line.find_first_not_of(" \t\n\f\v\r\n");
                if(pos != std::string::npos) line.erase(0, pos);
            } else {
                line.erase(line.begin(), line.end());
            }

            std::vector<G4String> thisRow;

            G4Tokenizer next(line);
            G4String token = next();
            while (token != "" && token != "\t" && token != "\n" && token != "\r" && token != "\f" && token != "\v") {
                thisRow.push_back(token);
                token = next();
            }

            if ((thisRow[0] == "0") && (counter > 0)) {
                // Found end of header, signal start of data read
                if (headerUnitStrings.size() == 0) {
                    if (headerFields.size() > 6) {
                        G4cerr << "" << G4endl;
                        G4cerr << "Topas is exiting due to a serious error." << G4endl;
                        G4cerr << "Header information was not usable from MagneticField3DTable file:" << G4endl;
                        G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("MagneticField3DTable")) << G4endl;
                        G4cerr << "Only six fields (x,y,z,Bx,By,Bz) are allowed without specified units. Please include explicit unit declaration in the header" << G4endl;
                        fPm->AbortSession(1);
                    } else {
                        G4cout << "No units specified, setting to 'mm' for x,y,z and 'tesla' for Bx,By,Bz" << G4endl;
                        headerUnitStrings.push_back("mm");
                        headerUnitStrings.push_back("mm");
                        headerUnitStrings.push_back("mm");
                        headerUnitStrings.push_back("tesla");
                        headerUnitStrings.push_back("tesla");
                        headerUnitStrings.push_back("tesla");
                    }
                }

                for(G4int i = 0; i < (G4int)headerFields.size(); i++) {
                    G4String unitString = headerUnitStrings[i];
                    std::locale loc;

                    size_t f = unitString.find("[");

                    if (f != std::string::npos) {
                        unitString.replace(f, std::string("[").length(), "");
                    };

                    f = unitString.find("]");

                    if (f != std::string::npos) {
                        unitString.replace(f, std::string("]").length(), "");
                    };

                    for (std::string::size_type j = 0; j < unitString.length(); j++) {
                        unitString[j] = std::tolower(unitString[j],loc);
                    }

                    if (unitString == "mm") {
                        headerUnits[headerFields[i]] = mm;
                    } else
                        if (unitString == "m" || unitString == "metre" || unitString == "meter") {
                            headerUnits[headerFields[i]] = m;
                        } else
                            if (unitString == "tesla") {
                                headerUnits[headerFields[i]] = tesla;
                            }
                            else {
                                headerUnits[headerFields[i]] = 1;
                            }
                }

                fFieldX.resize(fNX);
                fFieldY.resize(fNX);
                fFieldZ.resize(fNX);
                for (int index_x=0; index_x<fNX; index_x++) {
                    fFieldX[index_x].resize(fNY);
                    fFieldY[index_x].resize(fNY);
                    fFieldZ[index_x].resize(fNY);
                    for (int index_y=0; index_y<fNY; index_y++) {
                        fFieldX[index_x][index_y].resize(fNZ);
                        fFieldY[index_x][index_y].resize(fNZ);
                        fFieldZ[index_x][index_y].resize(fNZ);
                    }
                }

                ReadingHeader = false;
                counter = 0;
                continue;
            }

            if (ReadingHeader) {
                if (counter == 0) {
                    fNX = atoi(thisRow[0]);
                    fNY = atoi(thisRow[1]);
                    fNZ = atoi(thisRow[2]);
                } else {
                    if (thisRow.size() < 2) continue;

                    headerFields.push_back(thisRow[1]);
                    if (thisRow.size() == 3)
                        headerUnitStrings.push_back(thisRow[2]);

                    if (thisRow.size() > 3) {
                        G4cerr << "" << G4endl;
                        G4cerr << "Topas is exiting due to a serious error." << G4endl;
                        G4cerr << "Header information was not usable from MagneticField3DTable file:" << G4endl;
                        G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("MagneticField3DTable")) << G4endl;
                        G4cerr << "Header has an unknown format on line" << G4endl;
                        G4cerr << line << G4endl;
                        G4cerr << "This error can be triggered by mismatch of linux/windows end-of-line characters." << G4endl;
                        G4cerr << "If the opera file was created in windows, try converting it with dos2unix" << G4endl;
                        fPm->AbortSession(1);
                    }
                }
            } else {
                if (thisRow.size() != headerFields.size()) {
                    G4cerr << "" << G4endl;
                    G4cerr << "Topas is exiting due to a serious error." << G4endl;
                    G4cerr << "Header information was not usable from MagneticField3DTable file:" << G4endl;
                    G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("MagneticField3DTable")) << G4endl;
                    G4cerr << "File contains columns not in the header." << G4endl;
                    fPm->AbortSession(1);
                }

                xval = atof(thisRow[0]);
                yval = atof(thisRow[1]);
                zval = atof(thisRow[2]);
                bx = atof(thisRow[3]);
                by = atof(thisRow[4]);
                bz = atof(thisRow[5]);

                if ( ix==0 && iy==0 && iz==0 ) {
                    fMinX = xval * headerUnits["X"];
                    fMinY = yval * headerUnits["Y"];
                    fMinZ = zval * headerUnits["Z"];
                }

                fFieldX[ix][iy][iz] = bx * headerUnits["BX"];
                fFieldY[ix][iy][iz] = by * headerUnits["BY"];
                fFieldZ[ix][iy][iz] = bz * headerUnits["BZ"];

                iz++;
                if (iz == fNZ) {
                    iy++;
                    iz = 0;
                }

                if (iy == fNY) {
                    ix++;
                    iy = 0;
                }
            }

            counter++;
        }

        file.close();

        if (fNX == 0) {
            G4cerr << "" << G4endl;
            G4cerr << "Topas is exiting due to a serious error." << G4endl;
            G4cerr << "Header information was not usable from MagneticField3DTable file:" << G4endl;
            G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("MagneticField3DTable")) << G4endl;
            fPm->AbortSession(1);
        }

        fMaxX = xval * headerUnits["X"];
        fMaxY = yval * headerUnits["Y"];
        fMaxZ = zval * headerUnits["Z"];

        if (fMaxX < fMinX) {
            std::swap(fMaxX,fMinX);
            fInvertX = true;
        }

        if (fMaxY < fMinY) {
            std::swap(fMaxY,fMinY);
            fInvertY = true;
        }

        if (fMaxZ < fMinZ) {
            std::swap(fMaxZ,fMinZ);
            fInvertZ = true;
        }

        fDX = fMaxX - fMinX;
        fDY = fMaxY - fMinY;
        fDZ = fMaxZ - fMinZ;
}

void TsMagneticFieldMap::GetFieldValue(const G4double Point[3], G4double* Field) const {
	const G4ThreeVector localPoint = fAffineTransf.Inverse().TransformPoint(G4ThreeVector(Point[0],Point[1],Point[2]));

	// Tabulated 3D table has it's own field area and the region is supposed to be smaller than volume
	// Therefore it is necessary to check that the point is inside the field area.
	G4double FieldX;
	G4double FieldY;
	G4double FieldZ;

	if ( localPoint.x() >= fMinX && localPoint.x() <= fMaxX && localPoint.y() >= fMinY && localPoint.y() <=fMaxY && localPoint.z() >=fMinZ && localPoint.z() <= fMaxZ ) {
		// Position of given point within region, normalized to the range [0,1]
		G4double xFraction = (localPoint.x() - fMinX)/fDX;
		G4double yFraction = (localPoint.y() - fMinY)/fDY;
		G4double zFraction = (localPoint.z() - fMinZ)/fDZ;

		if (fInvertX)
			xFraction = 1 - xFraction;
		if (fInvertY)
			yFraction = 1 - yFraction;
		if (fInvertZ)
			zFraction = 1 - zFraction;

		// Position of the point within the cuboid defined by the
		// nearest surrounding tabulated points
		G4double xDIndex;
		G4double yDIndex;
		G4double zDIndex;
		G4double xLocal = ( std::modf(xFraction*(fNX-1), &xDIndex));
		G4double yLocal = ( std::modf(yFraction*(fNY-1), &yDIndex));
		G4double zLocal = ( std::modf(zFraction*(fNZ-1), &zDIndex));

		// The indices of the nearest tabulated point whose coordinates
		// are all less than those of the given point
		G4double xIndex = static_cast<int>(xDIndex);
		G4double yIndex = static_cast<int>(yDIndex);
		G4double zIndex = static_cast<int>(zDIndex);

		// In rare cases, value is all the way to the end of the first bin.
		// Need to make sure it is assigned to that bin and not to the non-existant next bin.
//		if (xIndex + 1 == fNX) xIndex--;
		if (xIndex + 1 == fNX) {
			xIndex--;
			xLocal = 1;
		}
//		if (yIndex + 1 == fNY) yIndex--;
		if (yIndex + 1 == fNY) {
			yIndex--;
			yLocal = 1;
		}
//		if (zIndex + 1 == fNZ) zIndex--;
		if (zIndex + 1 == fNZ) {
			zIndex--;
			zLocal = 1;
		}

		// Full 3-dimensional version
		FieldX =
		fFieldX[xIndex  ][yIndex  ][zIndex  ] * (1-xLocal) * (1-yLocal) * (1-zLocal) +
		fFieldX[xIndex  ][yIndex  ][zIndex+1] * (1-xLocal) * (1-yLocal) *    zLocal  +
		fFieldX[xIndex  ][yIndex+1][zIndex  ] * (1-xLocal) *    yLocal  * (1-zLocal) +
		fFieldX[xIndex  ][yIndex+1][zIndex+1] * (1-xLocal) *    yLocal  *    zLocal  +
		fFieldX[xIndex+1][yIndex  ][zIndex  ] *    xLocal  * (1-yLocal) * (1-zLocal) +
		fFieldX[xIndex+1][yIndex  ][zIndex+1] *    xLocal  * (1-yLocal) *    zLocal  +
		fFieldX[xIndex+1][yIndex+1][zIndex  ] *    xLocal  *    yLocal  * (1-zLocal) +
		fFieldX[xIndex+1][yIndex+1][zIndex+1] *    xLocal  *    yLocal  *    zLocal;
		FieldY =
		fFieldY[xIndex  ][yIndex  ][zIndex  ] * (1-xLocal) * (1-yLocal) * (1-zLocal) +
		fFieldY[xIndex  ][yIndex  ][zIndex+1] * (1-xLocal) * (1-yLocal) *    zLocal  +
		fFieldY[xIndex  ][yIndex+1][zIndex  ] * (1-xLocal) *    yLocal  * (1-zLocal) +
		fFieldY[xIndex  ][yIndex+1][zIndex+1] * (1-xLocal) *    yLocal  *    zLocal  +
		fFieldY[xIndex+1][yIndex  ][zIndex  ] *    xLocal  * (1-yLocal) * (1-zLocal) +
		fFieldY[xIndex+1][yIndex  ][zIndex+1] *    xLocal  * (1-yLocal) *    zLocal  +
		fFieldY[xIndex+1][yIndex+1][zIndex  ] *    xLocal  *    yLocal  * (1-zLocal) +
		fFieldY[xIndex+1][yIndex+1][zIndex+1] *    xLocal  *    yLocal  *    zLocal;
		FieldZ =
		fFieldZ[xIndex  ][yIndex  ][zIndex  ] * (1-xLocal) * (1-yLocal) * (1-zLocal) +
		fFieldZ[xIndex  ][yIndex  ][zIndex+1] * (1-xLocal) * (1-yLocal) *    zLocal  +
		fFieldZ[xIndex  ][yIndex+1][zIndex  ] * (1-xLocal) *    yLocal  * (1-zLocal) +
		fFieldZ[xIndex  ][yIndex+1][zIndex+1] * (1-xLocal) *    yLocal  *    zLocal  +
		fFieldZ[xIndex+1][yIndex  ][zIndex  ] *    xLocal  * (1-yLocal) * (1-zLocal) +
		fFieldZ[xIndex+1][yIndex  ][zIndex+1] *    xLocal  * (1-yLocal) *    zLocal  +
		fFieldZ[xIndex+1][yIndex+1][zIndex  ] *    xLocal  *    yLocal  * (1-zLocal) +
		fFieldZ[xIndex+1][yIndex+1][zIndex+1] *    xLocal  *    yLocal  *    zLocal;

		G4ThreeVector B_local = G4ThreeVector(FieldX,FieldY,FieldZ);
		G4ThreeVector B_global = fAffineTransf.TransformAxis(B_local);

		Field[0] = B_global.x() ;
		Field[1] = B_global.y() ;
		Field[2] = B_global.z() ;
	} else {
		Field[0] = 0.0;
		Field[1] = 0.0;
		Field[2] = 0.0;
	}
}
