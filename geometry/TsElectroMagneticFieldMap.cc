// * Created on 2025.10.07 by @srmarcballestero <marc.ballestero-ribo@psi.ch>
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

#include "TsParameterManager.hh"

#include "TsElectroMagneticFieldMap.hh"
#include "TsVGeometryComponent.hh"

#include "G4SystemOfUnits.hh"
#include "G4TransportationManager.hh"
#include "G4Tokenizer.hh"
#include "G4ChordFinder.hh"

#include <fstream>
#include <locale>
#include <map>
#include <set>

TsElectroMagneticFieldMap::TsElectroMagneticFieldMap(TsParameterManager* pM, TsGeometryManager* gM, TsVGeometryComponent* component):
TsVElectroMagneticField(pM, gM, component),
eInvertX(false), eInvertY(false), eInvertZ(false), eNX(0), eNY(0), eNZ(0),
bInvertX(false), bInvertY(false), bInvertZ(false), bNX(0), bNY(0), bNZ(0)   {
	ResolveParameters();
}


TsElectroMagneticFieldMap::~TsElectroMagneticFieldMap() {
	eFieldX.clear();
	eFieldY.clear();
	eFieldZ.clear();

    bFieldX.clear();
    bFieldY.clear();
    bFieldZ.clear();

	if(fChordFinder) delete fChordFinder;
}


void TsElectroMagneticFieldMap::ResolveParameters() {
    // Generic variables
    G4String filename;
    std::ifstream file;
    size_t f;
    G4String fileExtension;

    // Read the electric field map file
	filename = fPm->GetStringParameter(fComponent->GetFullParmName("ElectricField3DTable"));

	file.open(filename);
	if (!file) {
		G4cerr << "" << G4endl;
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "The parameter: " << fComponent->GetFullParmName("ElectricField3DTable") << G4endl;
		G4cerr << "references a ElectricField3DTable file that cannot be found:" << G4endl;
		G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("ElectricField3DTable")) << G4endl;
		fPm->AbortSession(1);
	}

    // Get the extension of the file and check if it is supported
    f = filename.find_last_of(".");
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
        G4cerr << "The parameter: " << fComponent->GetFullParmName("ElectricField3DTable") << G4endl;
        G4cerr << "references a ElectricField3DTable file with an unsupported extension:" << G4endl;
        G4cerr << fileExtension << G4endl;
        G4cerr << "Only .csv and .TABLE (Opera3D) files are supported." << G4endl;
        fPm->AbortSession(1);
    }

    // Call appropriate reader based on file extension
    if (fileExtension == "TABLE") {
        ReadOpera3DFile(filename, "E");
    } else if (fileExtension == "csv") {
        ReadCSVFile(filename, "E");
    }

    // -----------------------------------------------------------------------------------------------

    // Read the magnetic field map file
    filename = fPm->GetStringParameter(fComponent->GetFullParmName("MagneticField3DTable"));

	file.open(filename);
	if (!file) {
		G4cerr << "" << G4endl;
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "The parameter: " << fComponent->GetFullParmName("MagneticField3DTable") << G4endl;
		G4cerr << "references a MagneticField3DTable file that cannot be found:" << G4endl;
		G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("MagneticField3DTable")) << G4endl;
		fPm->AbortSession(1);
	}

    // Get the extension of the file and check if it is supported
    f = filename.find_last_of(".");
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
        ReadOpera3DFile(filename, "B");
    } else if (fileExtension == "csv") {
        ReadCSVFile(filename, "B");
    }

    // -----------------------------------------------------------------------------------------------

	const G4RotationMatrix* rotM = fComponent->GetRotRelToWorld();
	G4Point3D* fTransRelToWorld = GetComponent()->GetTransRelToWorld();
	G4ThreeVector transl = G4ThreeVector(fTransRelToWorld->x(),fTransRelToWorld->y(),fTransRelToWorld->z());
	fAffineTransf = G4AffineTransform(rotM,transl);

}

void TsElectroMagneticFieldMap::ReadCSVFileE(const G4String& filename) {
    std::ifstream file(filename);

    // Error: file not found
    if (!file) {
        G4cerr << "" << G4endl;
        G4cerr << "Topas is exiting due to a serious error." << G4endl;
        G4cerr << "The parameter: " << fComponent->GetFullParmName("ElectricField3DTable") << G4endl;
        G4cerr << "references a ElectricField3DTable file that cannot be found:" << G4endl;
        G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("ElectricField3DTable")) << G4endl;
        fPm->AbortSession(1);
    }

    // Read the CSV header line
    G4String headerLine;
    if (!std::getline(file, headerLine)) {
        G4cerr << "" << G4endl;
        G4cerr << "Topas is exiting due to a serious error." << G4endl;
        G4cerr << "CSV file is empty or unreadable:" << G4endl;
        G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("ElectricField3DTable")) << G4endl;
        fPm->AbortSession(1);
    }

    // Parse header to extract field names and units
    // Expected format (example): "X [mm]", "Y [mm]", "Z [mm]", "Ex [kV/cm]", "Ey [kV/cm]", "Ez [kV/cm]"
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
            G4cerr << "File: " << fPm->GetStringParameter(fComponent->GetFullParmName("ElectricField3DTable")) << G4endl;
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
        } else if (unitString == "V/m") {
            unitValue = volt / m;
        } else if (unitString == "kV/cm") {
            unitValue = kilovolt / cm;
        } else if (unitString == "V/cm") {
            unitValue = volt / cm;
        } else if (unitString == "MV/m") {
            unitValue = megavolt / m;
        } else {
            G4cerr << "" << G4endl;
            G4cerr << "Topas is exiting due to a serious error." << G4endl;
            G4cerr << "Unknown unit '" << unitString << "' for column '" << fieldName << "'" << G4endl;
            G4cerr << "Supported units: mm, cm, m, V/m, kV/cm, MV/m" << G4endl;
            G4cerr << "File: " << fPm->GetStringParameter(fComponent->GetFullParmName("ElectricField3DTable")) << G4endl;
            fPm->AbortSession(1);
        }

        headerUnits[fieldName] = unitValue;
        colIndex++;
    }

    // Verify required columns exist
    if (columnIndices.find("X") == columnIndices.end() ||
        columnIndices.find("Y") == columnIndices.end() ||
        columnIndices.find("Z") == columnIndices.end() ||
        columnIndices.find("Ex") == columnIndices.end() ||
        columnIndices.find("Ey") == columnIndices.end() ||
        columnIndices.find("Ez") == columnIndices.end()) {
        G4cerr << "" << G4endl;
        G4cerr << "Topas is exiting due to a serious error." << G4endl;
        G4cerr << "CSV header must contain columns: X, Y, Z, Ex, Ey, Ez" << G4endl;
        G4cerr << "File: " << fPm->GetStringParameter(fComponent->GetFullParmName("ElectricField3DTable")) << G4endl;
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
        G4cerr << "No data points found in ElectricField3DTable file:" << G4endl;
        G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("ElectricField3DTable")) << G4endl;
        fPm->AbortSession(1);
    }

    // Determine grid dimensions
    eNX = uniqueX.size();
    eNY = uniqueY.size();
    eNZ = uniqueZ.size();

    G4cout << "Electric field map dimensions: " << eNX << " x " << eNY << " x " << eNZ << G4endl;

    // Sorted vectors for coordinate mapping
    std::vector<double> sortedX(uniqueX.begin(), uniqueX.end());
    std::vector<double> sortedY(uniqueY.begin(), uniqueY.end());
    std::vector<double> sortedZ(uniqueZ.begin(), uniqueZ.end());
    std::sort(sortedX.begin(), sortedX.end());
    std::sort(sortedY.begin(), sortedY.end());
    std::sort(sortedZ.begin(), sortedZ.end());

    eMinX = sortedX.front(), eMaxX = sortedX.back();
    eMinY = sortedY.front(), eMaxY = sortedY.back();
    eMinZ = sortedZ.front(), eMaxZ = sortedZ.back();

    // Initialize 3D field arrays
    eFieldX.resize(eNX);
    eFieldY.resize(eNX);
    eFieldZ.resize(eNX);
    for (int index_x = 0; index_x < eNX; index_x++) {
        eFieldX[index_x].resize(eNY);
        eFieldY[index_x].resize(eNY);
        eFieldZ[index_x].resize(eNY);
        for (int index_y = 0; index_y < eNY; index_y++) {
            eFieldX[index_x][index_y].resize(eNZ);
            eFieldY[index_x][index_y].resize(eNZ);
            eFieldZ[index_x][index_y].resize(eNZ);
        }
    }

    // Re-read file to populate field arrays
    file.open(fPm->GetStringParameter(fComponent->GetFullParmName("ElectricField3DTable")));
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
        double ex = atof(values[columnIndices["Ex"]]) * headerUnits["Ex"];
        double ey = atof(values[columnIndices["Ey"]]) * headerUnits["Ey"];
        double ez = atof(values[columnIndices["Ez"]]) * headerUnits["Ez"];

        // Find indices in sorted coordinate arrays
        int ix = std::distance(sortedX.begin(), std::find(sortedX.begin(), sortedX.end(), x));
        int iy = std::distance(sortedY.begin(), std::find(sortedY.begin(), sortedY.end(), y));
        int iz = std::distance(sortedZ.begin(), std::find(sortedZ.begin(), sortedZ.end(), z));

        eFieldX[ix][iy][iz] = ex;
        eFieldY[ix][iy][iz] = ey;
        eFieldZ[ix][iy][iz] = ez;
    }
    file.close();

    eDX = eMaxX - eMinX;
    eDY = eMaxY - eMinY;
    eDZ = eMaxZ - eMinZ;

}


void TsElectroMagneticFieldMap::ReadCSVFileB(const G4String& filename) {
    std::ifstream file(filename);

    // Error: file not found
    if (!file) {
        G4cerr << "" << G4endl;
        G4cerr << "Topas is exiting due to a serious error." << G4endl;
        G4cerr << "The parameter: " << fComponent->GetFullParmName("MagneticField3DTable") << G4endl;
        G4cerr << "references a MagneticField3DTable file that cannot be found:" << G4endl;
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
    bNX = uniqueX.size();
    bNY = uniqueY.size();
    bNZ = uniqueZ.size();

    G4cout << "Magnetic field map dimensions: " << bNX << " x " << bNY << " x " << bNZ << G4endl;

    // Sorted vectors for coordinate mapping
    std::vector<double> sortedX(uniqueX.begin(), uniqueX.end());
    std::vector<double> sortedY(uniqueY.begin(), uniqueY.end());
    std::vector<double> sortedZ(uniqueZ.begin(), uniqueZ.end());
    std::sort(sortedX.begin(), sortedX.end());
    std::sort(sortedY.begin(), sortedY.end());
    std::sort(sortedZ.begin(), sortedZ.end());

    bMinX = sortedX.front(), bMaxX = sortedX.back();
    bMinY = sortedY.front(), bMaxY = sortedY.back();
    bMinZ = sortedZ.front(), bMaxZ = sortedZ.back();

    // Initialize 3D field arrays
    bFieldX.resize(bNX);
    bFieldY.resize(bNX);
    bFieldZ.resize(bNX);
    for (int index_x = 0; index_x < bNX; index_x++) {
        bFieldX[index_x].resize(bNY);
        bFieldY[index_x].resize(bNY);
        bFieldZ[index_x].resize(bNY);
        for (int index_y = 0; index_y < bNY; index_y++) {
            bFieldX[index_x][index_y].resize(bNZ);
            bFieldY[index_x][index_y].resize(bNZ);
            bFieldZ[index_x][index_y].resize(bNZ);
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

        bFieldX[ix][iy][iz] = bx;
        bFieldY[ix][iy][iz] = by;
        bFieldZ[ix][iy][iz] = bz;
    }
    file.close();

    bDX = bMaxX - bMinX;
    bDY = bMaxY - bMinY;
    bDZ = bMaxZ - bMinZ;

}

void TsElectroMagneticFieldMap::ReadCSVFile(const G4String& filename, const G4String& fieldType) {
	std::ifstream file(filename);

    if (fieldType == "E") {
        G4cout << "Reading electric field map from CSV file: " << filename << G4endl;
        ReadCSVFileE(filename);
    }
    else if (fieldType == "B") {
        G4cout << "Reading magnetic field map from CSV file: " << filename << G4endl;
        ReadCSVFileB(filename);
    }
    else {
        G4cerr << "" << G4endl;
        G4cerr << "Topas is exiting due to a serious error." << G4endl;
        G4cerr << "Unknown field type '" << fieldType << "' specified for CSV file reader." << G4endl;
        fPm->AbortSession(1);
    }

}

void TsElectroMagneticFieldMap::ReadOpera3DFileE(const G4String& filename) {
    std::ifstream file(filename);

    // Error: file not found
    if (!file) {
        G4cerr << "" << G4endl;
        G4cerr << "Topas is exiting due to a serious error." << G4endl;
        G4cerr << "The parameter: " << fComponent->GetFullParmName("ElectricField3DTable") << G4endl;
        G4cerr << "references a ElectricField3DTable file that cannot be found:" << G4endl;
        G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("ElectricField3DTable")) << G4endl;
        fPm->AbortSession(1);
    }

    G4String line;
    bool ReadingHeader = true;
    G4int counter = 0;
    double xval = 0., yval = 0., zval = 0., ex, ey, ez;
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
                    G4cerr << "Header information was not usable from ElectricField3DTable file:" << G4endl;
                    G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("ElectricField3DTable")) << G4endl;
                    G4cerr << "Only six fields (x,y,z,Ex,Ey,Ez) are allowed without specified units. Please include explicit unit declaration in the header" << G4endl;
                    fPm->AbortSession(1);
                } else {
                    G4cout << "No units specified, setting to 'm' for x,y,z and 'V/m' for Ex,Ey,Ez" << G4endl;
                    headerUnitStrings.push_back("m");
                    headerUnitStrings.push_back("m");
                    headerUnitStrings.push_back("m");
                    headerUnitStrings.push_back("V/m");
                    headerUnitStrings.push_back("V/m");
                    headerUnitStrings.push_back("V/m");
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
                        if (unitString == "V/m") {
                            headerUnits[headerFields[i]] = volt / m;
                        }
                        else {
                            headerUnits[headerFields[i]] = 1;
                        }
            }

            eFieldX.resize(eNX);
            eFieldY.resize(eNX);
            eFieldZ.resize(eNX);
            for (int index_x=0; index_x<eNX; index_x++) {
                eFieldX[index_x].resize(eNY);
                eFieldY[index_x].resize(eNY);
                eFieldZ[index_x].resize(eNY);
                for (int index_y=0; index_y<eNY; index_y++) {
                    eFieldX[index_x][index_y].resize(eNZ);
                    eFieldY[index_x][index_y].resize(eNZ);
                    eFieldZ[index_x][index_y].resize(eNZ);
                }
            }

            ReadingHeader = false;
            counter = 0;
            continue;
        }

        if (ReadingHeader) {
            if (counter == 0) {
                eNX = atoi(thisRow[0]);
                eNY = atoi(thisRow[1]);
                eNZ = atoi(thisRow[2]);
            } else {
                if (thisRow.size() < 2) continue;

                headerFields.push_back(thisRow[1]);
                if (thisRow.size() == 3)
                    headerUnitStrings.push_back(thisRow[2]);

                if (thisRow.size() > 3) {
                    G4cerr << "" << G4endl;
                    G4cerr << "Topas is exiting due to a serious error." << G4endl;
                    G4cerr << "Header information was not usable from ElectricField3DTable file:" << G4endl;
                    G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("ElectricField3DTable")) << G4endl;
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
                G4cerr << "Header information was not usable from ElectricField3DTable file:" << G4endl;
                G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("ElectricField3DTable")) << G4endl;
                G4cerr << "File contains columns not in the header." << G4endl;
                fPm->AbortSession(1);
            }

            xval = atof(thisRow[0]);
            yval = atof(thisRow[1]);
            zval = atof(thisRow[2]);
            ex = atof(thisRow[3]);
            ey = atof(thisRow[4]);
            ez = atof(thisRow[5]);

            if ( ix==0 && iy==0 && iz==0 ) {
                eMinX = xval * headerUnits["X"];
                eMinY = yval * headerUnits["Y"];
                eMinZ = zval * headerUnits["Z"];
            }

            eFieldX[ix][iy][iz] = ex * headerUnits["EX"];
            eFieldY[ix][iy][iz] = ey * headerUnits["EY"];
            eFieldZ[ix][iy][iz] = ez * headerUnits["EZ"];

            iz++;
            if (iz == eNZ) {
                iy++;
                iz = 0;
            }

            if (iy == eNY) {
                ix++;
                iy = 0;
            }
        }

        counter++;
    }

    file.close();

    if (eNX == 0) {
        G4cerr << "" << G4endl;
        G4cerr << "Topas is exiting due to a serious error." << G4endl;
        G4cerr << "Header information was not usable from ElectricField3DTable file:" << G4endl;
        G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("ElectricField3DTable")) << G4endl;
        fPm->AbortSession(1);
    }

    eMaxX = xval * headerUnits["X"];
    eMaxY = yval * headerUnits["Y"];
    eMaxZ = zval * headerUnits["Z"];

    if (eMaxX < eMinX) {
        std::swap(eMaxX,eMinX);
        eInvertX = true;
    }

    if (eMaxY < eMinY) {
        std::swap(eMaxY,eMinY);
        eInvertY = true;
    }

    if (eMaxZ < eMinZ) {
        std::swap(eMaxZ,eMinZ);
        eInvertZ = true;
    }

    eDX = eMaxX - eMinX;
    eDY = eMaxY - eMinY;
    eDZ = eMaxZ - eMinZ;
}


void TsElectroMagneticFieldMap::ReadOpera3DFileB(const G4String& filename) {
    std::ifstream file(filename);

    // Error: file not found
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
    double xval = 0., yval = 0., zval = 0., bx, by, bz;
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
                    G4cout << "No units specified, setting to 'm' for x,y,z and 'tesla' for Bx,By,Bz" << G4endl;
                    headerUnitStrings.push_back("m");
                    headerUnitStrings.push_back("m");
                    headerUnitStrings.push_back("m");
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

            bFieldX.resize(bNX);
            bFieldY.resize(bNX);
            bFieldZ.resize(bNX);
            for (int index_x=0; index_x<bNX; index_x++) {
                bFieldX[index_x].resize(bNY);
                bFieldY[index_x].resize(bNY);
                bFieldZ[index_x].resize(bNY);
                for (int index_y=0; index_y<bNY; index_y++) {
                    bFieldX[index_x][index_y].resize(bNZ);
                    bFieldY[index_x][index_y].resize(bNZ);
                    bFieldZ[index_x][index_y].resize(bNZ);
                }
            }

            ReadingHeader = false;
            counter = 0;
            continue;
        }

        if (ReadingHeader) {
            if (counter == 0) {
                bNX = atoi(thisRow[0]);
                bNY = atoi(thisRow[1]);
                bNZ = atoi(thisRow[2]);
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
                bMinX = xval * headerUnits["X"];
                bMinY = yval * headerUnits["Y"];
                bMinZ = zval * headerUnits["Z"];
            }

            bFieldX[ix][iy][iz] = bx * headerUnits["BX"];
            bFieldY[ix][iy][iz] = by * headerUnits["BY"];
            bFieldZ[ix][iy][iz] = bz * headerUnits["BZ"];

            iz++;
            if (iz == bNZ) {
                iy++;
                iz = 0;
            }

            if (iy == bNY) {
                ix++;
                iy = 0;
            }
        }

        counter++;
    }

    file.close();

    if (bNX == 0) {
        G4cerr << "" << G4endl;
        G4cerr << "Topas is exiting due to a serious error." << G4endl;
        G4cerr << "Header information was not usable from MagneticField3DTable file:" << G4endl;
        G4cerr << fPm->GetStringParameter(fComponent->GetFullParmName("MagneticField3DTable")) << G4endl;
        fPm->AbortSession(1);
    }

    bMaxX = xval * headerUnits["X"];
    bMaxY = yval * headerUnits["Y"];
    bMaxZ = zval * headerUnits["Z"];

    if (bMaxX < bMinX) {
        std::swap(bMaxX,bMinX);
        bInvertX = true;
    }

    if (bMaxY < bMinY) {
        std::swap(bMaxY,bMinY);
        bInvertY = true;
    }

    if (bMaxZ < bMinZ) {
        std::swap(bMaxZ,bMinZ);
        bInvertZ = true;
    }

    bDX = bMaxX - bMinX;
    bDY = bMaxY - bMinY;
    bDZ = bMaxZ - bMinZ;

}

void TsElectroMagneticFieldMap::ReadOpera3DFile(const G4String& filename, const G4String& fieldType) {
	std::ifstream file(filename);

    if (fieldType == "E") {
        G4cout << "Reading electric field map from Opera3D TABLE file: " << filename << G4endl;
        ReadOpera3DFileE(filename);
        G4cout << "Electric field map dimensions: " << eNX << " x " << eNY << " x " << eNZ << G4endl;
    }
    else if (fieldType == "B") {
        G4cout << "Reading magnetic field map from Opera3D TABLE file: " << filename << G4endl;
        ReadOpera3DFileB(filename);
        G4cout << "Magnetic field map dimensions: " << bNX << " x " << bNY << " x " << bNZ << G4endl;
    }
    else {
        G4cerr << "" << G4endl;
        G4cerr << "Topas is exiting due to a serious error." << G4endl;
        G4cerr << "Unknown field type '" << fieldType << "' specified for Opera3D file reader." << G4endl;
        fPm->AbortSession(1);
    }

}


G4ThreeVector TsElectroMagneticFieldMap::GetFieldValueE(const G4ThreeVector localPoint) const {
    // Tabulated 3D table has it's own field area and the region is supposed to be smaller than volume
	// Therefore it is necessary to check that the point is inside the field area.
	G4double FieldX;
	G4double FieldY;
	G4double FieldZ;

    if (localPoint.x() >= eMinX && localPoint.x() <= eMaxX &&
        localPoint.y() >= eMinY && localPoint.y() <= eMaxY &&
        localPoint.z() >= eMinZ && localPoint.z() <= eMaxZ) {

        // Position of a given point within region, normalized to the range [0,1]
        G4double xFraction = (localPoint.x() - eMinX) / eDX;
        G4double yFraction = (localPoint.y() - eMinY) / eDY;
        G4double zFraction = (localPoint.z() - eMinZ) / eDZ;

        if (eInvertX)
            xFraction = 1 - xFraction;
        if (eInvertY)
            yFraction = 1 - yFraction;
        if (eInvertZ)
            zFraction = 1 - zFraction;

        // Position of the point within the cuboid defined by the
        // nearest surrounding tabulated points
        G4double xDIndex;
        G4double yDIndex;
        G4double zDIndex;

        G4double xLocal = ( std::modf(xFraction * (eNX - 1), &xDIndex) );
        G4double yLocal = ( std::modf(yFraction * (eNY - 1), &yDIndex) );
        G4double zLocal = ( std::modf(zFraction * (eNZ - 1), &zDIndex) );

        // The indices of the nearest tabulated point whose coordinates are
        // less than or equal to the coordinates of the point
        G4double xIndex = static_cast<int>(xDIndex);
        G4double yIndex = static_cast<int>(yDIndex);
        G4double zIndex = static_cast<int>(zDIndex);

        // In rare case, value is all the way to the end of the first bin.
        // Need to make sure it is assigned to that bin and not to the non-existant next bin.
        if (xIndex + 1 == eNX) {
            xIndex--;
            xLocal = 1;
        }

        if (yIndex + 1 == eNY) {
            yIndex--;
            yLocal = 1;
        }

        if (zIndex + 1 == eNZ) {
            zIndex--;
            zLocal = 1;
        }

        // Trilinear interpolation
        FieldX =
            eFieldX[xIndex  ][yIndex  ][zIndex  ] * (1-xLocal) * (1-yLocal) * (1-zLocal) +
            eFieldX[xIndex  ][yIndex  ][zIndex+1] * (1-xLocal) * (1-yLocal) *    zLocal  +
            eFieldX[xIndex  ][yIndex+1][zIndex  ] * (1-xLocal) *    yLocal  * (1-zLocal) +
            eFieldX[xIndex  ][yIndex+1][zIndex+1] * (1-xLocal) *    yLocal  *    zLocal  +
            eFieldX[xIndex+1][yIndex  ][zIndex  ] *    xLocal  * (1-yLocal) * (1-zLocal) +
            eFieldX[xIndex+1][yIndex  ][zIndex+1] *    xLocal  * (1-yLocal) *    zLocal  +
            eFieldX[xIndex+1][yIndex+1][zIndex  ] *    xLocal  *    yLocal  * (1-zLocal) +
            eFieldX[xIndex+1][yIndex+1][zIndex+1] *    xLocal  *    yLocal  *    zLocal;
        FieldY =
            eFieldY[xIndex  ][yIndex  ][zIndex  ] * (1-xLocal) * (1-yLocal) * (1-zLocal) +
            eFieldY[xIndex  ][yIndex  ][zIndex+1] * (1-xLocal) * (1-yLocal) *    zLocal  +
            eFieldY[xIndex  ][yIndex+1][zIndex  ] * (1-xLocal) *    yLocal  * (1-zLocal) +
            eFieldY[xIndex  ][yIndex+1][zIndex+1] * (1-xLocal) *    yLocal  *    zLocal  +
            eFieldY[xIndex+1][yIndex  ][zIndex  ] *    xLocal  * (1-yLocal) * (1-zLocal) +
            eFieldY[xIndex+1][yIndex  ][zIndex+1] *    xLocal  * (1-yLocal) *    zLocal  +
            eFieldY[xIndex+1][yIndex+1][zIndex  ] *    xLocal  *    yLocal  * (1-zLocal) +
            eFieldY[xIndex+1][yIndex+1][zIndex+1] *    xLocal  *    yLocal  *    zLocal;
        FieldZ =
            eFieldZ[xIndex  ][yIndex  ][zIndex  ] * (1-xLocal) * (1-yLocal) * (1-zLocal) +
            eFieldZ[xIndex  ][yIndex  ][zIndex+1] * (1-xLocal) * (1-yLocal) *    zLocal  +
            eFieldZ[xIndex  ][yIndex+1][zIndex  ] * (1-xLocal) *    yLocal  * (1-zLocal) +
            eFieldZ[xIndex  ][yIndex+1][zIndex+1] * (1-xLocal) *    yLocal  *    zLocal  +
            eFieldZ[xIndex+1][yIndex  ][zIndex  ] *    xLocal  * (1-yLocal) * (1-zLocal) +
            eFieldZ[xIndex+1][yIndex  ][zIndex+1] *    xLocal  * (1-yLocal) *    zLocal  +
            eFieldZ[xIndex+1][yIndex+1][zIndex  ] *    xLocal  *    yLocal  * (1-zLocal) +
            eFieldZ[xIndex+1][yIndex+1][zIndex+1] *    xLocal  *    yLocal  *    zLocal;

        G4ThreeVector E_local = G4ThreeVector(FieldX, FieldY, FieldZ);
        G4ThreeVector E_global = fAffineTransf.TransformAxis(E_local);

        return E_global;

    } else {
        return G4ThreeVector(0., 0., 0.);

    }
}

G4ThreeVector TsElectroMagneticFieldMap::GetFieldValueB(const G4ThreeVector localPoint) const {
    // Tabulated 3D table has it's own field area and the region is supposed to be smaller than volume
	// Therefore it is necessary to check that the point is inside the field area.
	G4double FieldX;
	G4double FieldY;
	G4double FieldZ;

    if (localPoint.x() >= bMinX && localPoint.x() <= bMaxX &&
        localPoint.y() >= bMinY && localPoint.y() <= bMaxY &&
        localPoint.z() >= bMinZ && localPoint.z() <= bMaxZ) {

        // Position of a given point within region, normalized to the range [0,1]
        G4double xFraction = (localPoint.x() - bMinX) / bDX;
        G4double yFraction = (localPoint.y() - bMinY) / bDY;
        G4double zFraction = (localPoint.z() - bMinZ) / bDZ;

        if (bInvertX)
            xFraction = 1 - xFraction;
        if (bInvertY)
            yFraction = 1 - yFraction;
        if (bInvertZ)
            zFraction = 1 - zFraction;

        // Position of the point within the cuboid defined by the
        // nearest surrounding tabulated points
        G4double xDIndex;
        G4double yDIndex;
        G4double zDIndex;

        G4double xLocal = ( std::modf(xFraction * (bNX - 1), &xDIndex) );
        G4double yLocal = ( std::modf(yFraction * (bNY - 1), &yDIndex) );
        G4double zLocal = ( std::modf(zFraction * (bNZ - 1), &zDIndex) );

        // The indices of the nearest tabulated point whose coordinates are
        // less than or equal to the coordinates of the point
        G4double xIndex = static_cast<int>(xDIndex);
        G4double yIndex = static_cast<int>(yDIndex);
        G4double zIndex = static_cast<int>(zDIndex);

        // In rare case, value is all the way to the end of the first bin.
        // Need to make sure it is assigned to that bin and not to the non-existant next bin.
        if (xIndex + 1 == bNX) {
            xIndex--;
            xLocal = 1;
        }

        if (yIndex + 1 == bNY) {
            yIndex--;
            yLocal = 1;
        }

        if (zIndex + 1 == bNZ) {
            zIndex--;
            zLocal = 1;
        }

        // Trilinear interpolation
        FieldX =
            bFieldX[xIndex  ][yIndex  ][zIndex  ] * (1-xLocal) * (1-yLocal) * (1-zLocal) +
            bFieldX[xIndex  ][yIndex  ][zIndex+1] * (1-xLocal) * (1-yLocal) *    zLocal  +
            bFieldX[xIndex  ][yIndex+1][zIndex  ] * (1-xLocal) *    yLocal  * (1-zLocal) +
            bFieldX[xIndex  ][yIndex+1][zIndex+1] * (1-xLocal) *    yLocal  *    zLocal  +
            bFieldX[xIndex+1][yIndex  ][zIndex  ] *    xLocal  * (1-yLocal) * (1-zLocal) +
            bFieldX[xIndex+1][yIndex  ][zIndex+1] *    xLocal  * (1-yLocal) *    zLocal  +
            bFieldX[xIndex+1][yIndex+1][zIndex  ] *    xLocal  *    yLocal  * (1-zLocal) +
            bFieldX[xIndex+1][yIndex+1][zIndex+1] *    xLocal  *    yLocal  *    zLocal;
        FieldY =
            bFieldY[xIndex  ][yIndex  ][zIndex  ] * (1-xLocal) * (1-yLocal) * (1-zLocal) +
            bFieldY[xIndex  ][yIndex  ][zIndex+1] * (1-xLocal) * (1-yLocal) *    zLocal  +
            bFieldY[xIndex  ][yIndex+1][zIndex  ] * (1-xLocal) *    yLocal  * (1-zLocal) +
            bFieldY[xIndex  ][yIndex+1][zIndex+1] * (1-xLocal) *    yLocal  *    zLocal  +
            bFieldY[xIndex+1][yIndex  ][zIndex  ] *    xLocal  * (1-yLocal) * (1-zLocal) +
            bFieldY[xIndex+1][yIndex  ][zIndex+1] *    xLocal  * (1-yLocal) *    zLocal  +
            bFieldY[xIndex+1][yIndex+1][zIndex  ] *    xLocal  *    yLocal  * (1-zLocal) +
            bFieldY[xIndex+1][yIndex+1][zIndex+1] *    xLocal  *    yLocal  *    zLocal;
        FieldZ =
            bFieldZ[xIndex  ][yIndex  ][zIndex  ] * (1-xLocal) * (1-yLocal) * (1-zLocal) +
            bFieldZ[xIndex  ][yIndex  ][zIndex+1] * (1-xLocal) * (1-yLocal) *    zLocal  +
            bFieldZ[xIndex  ][yIndex+1][zIndex  ] * (1-xLocal) *    yLocal  * (1-zLocal) +
            bFieldZ[xIndex  ][yIndex+1][zIndex+1] * (1-xLocal) *    yLocal  *    zLocal  +
            bFieldZ[xIndex+1][yIndex  ][zIndex  ] *    xLocal  * (1-yLocal) * (1-zLocal) +
            bFieldZ[xIndex+1][yIndex  ][zIndex+1] *    xLocal  * (1-yLocal) *    zLocal  +
            bFieldZ[xIndex+1][yIndex+1][zIndex  ] *    xLocal  *    yLocal  * (1-zLocal) +
            bFieldZ[xIndex+1][yIndex+1][zIndex+1] *    xLocal  *    yLocal  *    zLocal;

        G4ThreeVector B_local = G4ThreeVector(FieldX, FieldY, FieldZ);
        G4ThreeVector B_global = fAffineTransf.TransformAxis(B_local);

        return B_global;

    } else {
        return G4ThreeVector(0., 0., 0.);

    }

}

void TsElectroMagneticFieldMap::GetFieldValue(const G4double Point[3], G4double* Field) const {
	const G4ThreeVector localPoint = fAffineTransf.Inverse().TransformPoint(G4ThreeVector(Point[0], Point[1], Point[2]));

    G4ThreeVector E_global = GetFieldValueE(localPoint);
    G4ThreeVector B_global = GetFieldValueB(localPoint);

    Field[0] = B_global.x();
    Field[1] = B_global.y();
    Field[2] = B_global.z();
    Field[3] = E_global.x();
    Field[4] = E_global.y();
    Field[5] = E_global.z();

}
