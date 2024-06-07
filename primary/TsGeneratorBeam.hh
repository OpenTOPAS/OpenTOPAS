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

#ifndef TsGeneratorBeam_hh
#define TsGeneratorBeam_hh

#include "TsVGenerator.hh"

class TsGeneratorBeam : public TsVGenerator
{
public:
	TsGeneratorBeam(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName);
	~TsGeneratorBeam();

	void ResolveParameters();

	void GeneratePrimaries(G4Event* );

private:
	void SetPosition(TsPrimaryParticle &p) const;
	void SetDirection(TsPrimaryParticle &p) const;
	G4float AngleToMarsagliaCoordinate(G4float angle) const;

	enum DistributionType { NONE, FLAT, GAUSSIAN };
	DistributionType fPositionDistribution;
	DistributionType fAngularDistribution;

	enum Shape { ELLIPSE, RECTANGLE };
	Shape fBeamShape;
	G4double fPositionCutoffX;
	G4double fPositionCutoffY;
	G4double fPositionSpreadX;
	G4double fPositionSpreadY;
	G4double fMarsagliaCutoffX;
	G4double fMarsagliaCutoffY;
	G4double fMarsagliaSpreadX;
	G4double fMarsagliaSpreadY;
};

#endif
