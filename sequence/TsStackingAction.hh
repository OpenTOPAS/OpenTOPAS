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

#ifndef TsStackingAction_hh
#define TsStackingAction_hh

#include "globals.hh"
#include "G4UserStackingAction.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4ParticleDefinition.hh"
#include <vector>
#include <map>

class TsParameterManager;
class TsGeometryManager;
class TsGeneratorManager;
class TsVarianceManager;
class TsVGeometryComponent;
class G4Region;
class G4VPhysicalVolume;

class TsStackingAction : public G4UserStackingAction
{
  public:
	TsStackingAction(TsParameterManager*, TsGeometryManager*, TsGeneratorManager*, TsVarianceManager*);
	~TsStackingAction();

	void PrepareNewEvent();

	G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track*);

	void NewStage();
	
	void Quit(G4String, G4String);

private:
	TsParameterManager *fPm;
	TsGeneratorManager* fPgm;
	TsVarianceManager *fVm;

	G4int fStage;
	G4int fPrimaryCounter;
	
	G4bool fKillTrack;
	G4bool fDBSActive;
	G4bool fRangeRejection;
};

#endif
