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

#ifndef TsRangeModulator_hh
#define TsRangeModulator_hh

#include "TsVGeometryComponent.hh"

#include "G4VPVParameterisation.hh"

struct Segment
{
	G4double H;
	G4double A[2]; //departure and span angles
	G4Material* M;
	G4VisAttributes* V;
};
enum TrackLocation{ UPPER=-1, MIDDLE=0, LOWER=1 };


class TsRangeModulator : public TsVGeometryComponent
{
public:
	TsRangeModulator(TsParameterManager*, TsExtensionManager* Em, TsMaterialManager*, TsGeometryManager*,
					TsVGeometryComponent*, G4VPhysicalVolume*, G4String& );
	~TsRangeModulator();

	G4VPhysicalVolume* Construct();

private:
	G4String fParmBase;

	void ConstructTracks(TrackLocation l,G4double Rin, G4double Rout, G4double HofShell, G4double Z );
};

#endif
