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

#ifndef TsElectroMagneticFieldMap_hh
#define TsElectroMagneticFieldMap_hh

#include "TsVElectroMagneticField.hh"

#include "G4AffineTransform.hh"

#include <vector>

class TsElectroMagneticFieldMap : public TsVElectroMagneticField
{
public:
	TsElectroMagneticFieldMap(TsParameterManager* pM, TsGeometryManager* gM,
					   TsVGeometryComponent* component);
	~TsElectroMagneticFieldMap();

	void GetFieldValue(const double p[3], double* Field) const;
	void ResolveParameters();

protected:
    // Read the field map file in CSV format
    // Field type: "E" for electric field, "B" for magnetic field
    // Expected header:
    //      E: x [unit], y [unit], z [unit], Ex [unit], Ey [unit], Ez [unit]
    //          Allowed units: m, cm, mm, V/m, kV/m, MV/m
    // or
    //      B: x [unit], y [unit], z [unit], Bx [unit], By [unit], Bz [unit]
    //          Allowed units: m, cm, mm, T, G (resp. Tesla, Gauss)
    void ReadCSVFile(const G4String& filename, const G4String& fieldType);

    // Read the field map file in Opera3D TABLE format
    // void ReadOpera3DFile(const G4String& filename, const G4String& fieldType);

private:
    // Attributes and methods for the electric field map
    // Read the electric field map file
    void ReadCSVFileE(const G4String& filename);
    // void ReadOpera3DFileE(const G4String& filename);

	// Physical limits of the defined region
	G4double eMinX, eMinY, eMinZ, eMaxX, eMaxY, eMaxZ;

	// Physical extent of the defined region
	G4double eDX, eDY, eDZ;

	// Allows handling of either direction of min and max positions
	G4bool eInvertX, eInvertY, eInvertZ;

	// Dimensions of the table
	G4int eNX, eNY, eNZ;

	// Storage for the table
	std::vector< std::vector< std::vector< double > > > eFieldX;
	std::vector< std::vector< std::vector< double > > > eFieldY;
	std::vector< std::vector< std::vector< double > > > eFieldZ;

    // Get the field value at a given point (in local coordinates)
    G4ThreeVector GetFieldValueE(const G4ThreeVector localPoint) const;

    // ------------------------------------------------------------------

    // Attributes and methods for the magnetic field map
    // Read the magnetic field map file
    void ReadCSVFileB(const G4String& filename);
    // void ReadOpera3DFileB(const G4String& filename);

    // Physical limits of the defined region
    G4double bMinX, bMinY, bMinZ, bMaxX, bMaxY, bMaxZ;

    // Physical extent of the defined region
    G4double bDX, bDY, bDZ;

    // Allows handling of either direction of min and max positions
    G4bool bInvertX, bInvertY, bInvertZ;

    // Dimensions of the table
    G4int bNX, bNY, bNZ;

    // Storage for the table
    std::vector< std::vector< std::vector< double > > > bFieldX;
    std::vector< std::vector< std::vector< double > > > bFieldY;
    std::vector< std::vector< std::vector< double > > > bFieldZ;

    // ------------------------------------------------------------------

	// Affine transformation to the world to resolve the position/rotation
	// when a daughter is placed in a mother holding the field
	G4AffineTransform fAffineTransf;

    // Get the field value at a given point (in local coordinates)
    G4ThreeVector GetFieldValueB(const G4ThreeVector localPoint) const;

};

#endif
