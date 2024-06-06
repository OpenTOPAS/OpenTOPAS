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

#ifndef TsMagneticFieldMap_hh
#define TsMagneticFieldMap_hh

#include "TsVMagneticField.hh"

#include "G4AffineTransform.hh"

#include <vector>

class TsMagneticFieldMap : public TsVMagneticField
{
public:
	TsMagneticFieldMap(TsParameterManager* pM, TsGeometryManager* gM,
					   TsVGeometryComponent* component);
	~TsMagneticFieldMap();

	void GetFieldValue(const double p[3], double* Field) const;
	void ResolveParameters();

private:
	// Physical limits of the defined region
	G4double fMinX, fMinY, fMinZ, fMaxX, fMaxY, fMaxZ;

	// Physical extent of the defined region
	G4double fDX, fDY, fDZ;

	// Allows handling of either direction of min and max positions
	G4bool fInvertX, fInvertY, fInvertZ;

	// Dimensions of the table
	G4int fNX, fNY, fNZ;

	// Storage for the table
	std::vector< std::vector< std::vector< double > > > fFieldX;
	std::vector< std::vector< std::vector< double > > > fFieldY;
	std::vector< std::vector< std::vector< double > > > fFieldZ;

	// Affine transformation to the world to resolve the position/rotation
	// when a daughter is placed in a mother holding the field
	G4AffineTransform fAffineTransf;
};

#endif
