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

#ifndef TsSplitProcess_hh
#define TsSplitProcess_hh

#include "G4VProcess.hh"
#include "G4FieldTrack.hh"

class TsParameterManager;
class TsGeometryManager;
class TsVGeometryComponent;
class TsIStore;

class G4TransportationManager;
class G4Navigator;
class G4PathFinder;

class TsSplitProcess : public G4VProcess
{
public :
	TsSplitProcess(TsParameterManager*, TsGeometryManager*, const G4String&, TsIStore*);

	virtual ~TsSplitProcess();

	void ResolveParameters();

	virtual G4double PostStepGetPhysicalInteractionLength(const G4Track&, G4double, G4ForceCondition*);
	virtual G4VParticleChange* PostStepDoIt(const G4Track&, const G4Step&);
	virtual G4double AtRestGetPhysicalInteractionLength(const G4Track&, G4ForceCondition*);
	virtual G4VParticleChange* AtRestDoIt(const G4Track&, const G4Step&);
	virtual G4double AlongStepGetPhysicalInteractionLength(const G4Track&, G4double, G4double, G4double&, G4GPILSelection*);
	virtual G4VParticleChange* AlongStepDoIt(const G4Track&, const G4Step& );
	
	void SetParallelWorld(G4String);
	void SetParallelWorld(G4VPhysicalVolume*);
	void StartTracking(G4Track*);
	void Split(G4ParticleChange* , const G4Track& , G4int, G4int);
	void RussianRoulette(G4ParticleChange*, G4int);

	G4int CalculateNSplit(G4int, G4int);
	G4bool AcceptTrack(const G4Track&, G4double, G4double);
	G4String GetFullParmName(G4String parmName);

private:
	void CopyStep(const G4Step& step);

private:
	TsParameterManager* fPm;
	TsGeometryManager* fGm;
	TsVGeometryComponent* fMotherComponent;
	TsIStore* fAIStore;

	G4ParticleChange* fParticleChange;
	G4Step* fGhostStep;
	G4StepPoint* fGhostPreStepPoint;
	G4StepPoint* fGhostPostStepPoint;
	G4TransportationManager* fTransportationManager;
	G4PathFinder* fPathFinder;
	G4Navigator* fGhostNavigator;
	G4int fNavigatorID;
	G4FieldTrack fFieldTrack;
	G4double fGhostSafety;
	G4bool fOnBoundary;

	G4double fRadius;
	G4double fLimit;
	G4ThreeVector fTransVector;
	G4ThreeVector fAxisVector;
	G4double fKCarTolerance;
	G4int fFactor;
	G4ThreeVector fAxisVector1;
	G4ThreeVector fAxisVector2;
	G4String fName;
	G4bool fOnlySplitParticlesOfUnitaryWeight;
};

#endif
