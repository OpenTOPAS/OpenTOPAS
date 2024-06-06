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

#ifndef TsSequenceManager_hh
#define TsSequenceManager_hh

#include "TsTopasConfig.hh"

#ifdef TOPAS_MT
#include "G4MTRunManager.hh"
#else
#include "G4RunManager.hh"
#endif
#include "G4Timer.hh"

#include <vector>

class TsParameterManager;
class TsExtensionManager;
class TsMaterialManager;
class TsGeometryManager;
class TsPhysicsManager;
class TsVarianceManager;
class TsFilterManager;
class TsScoringManager;
class TsGraphicsManager;
class TsChemistryManager;
class TsSourceManager;
class TsGeneratorManager;

class TsSteppingAction;
class TsQt;

#ifdef TOPAS_MT
class TsSequenceManager : public G4MTRunManager
#else
class TsSequenceManager : public G4RunManager
#endif
{

public:
	TsSequenceManager(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsPhysicsManager* phM, TsVarianceManager* vM,
					  TsFilterManager* fM, TsScoringManager* scM, TsGraphicsManager* grM, TsChemistryManager* chM, TsSourceManager* soM,
					  G4int argc, char** argv);
	~TsSequenceManager();

	void Sequence();

	G4bool IsExecutingSequence();

	void ExtraSequence(G4String);

	void NoteAnyUseOfChangeableParameters(const G4String& name);

	void UpdateForSpecificParameterChange(const G4String& name);
	void UpdateForNewRunOrQtChange();
	void ClearGenerators();

	void Run(G4double time);

	G4int GetRunID();

	G4double GetTime();

	G4bool UsingRayTracer();

	void NoteKilledTrack(G4double energy, G4String particleName, G4String processName, G4String volumeName);
	void NoteUnscoredHit(G4double energy, G4String scorerName);
	void NoteParameterizationError(G4double energy, G4String componentName, G4String volumeName);
	void NoteIndexError(G4double energy, G4String componentName, G4String coordinate, G4int value, G4int limit);
	void NoteInterruptedHistory();

	void RegisterGeneratorManager(TsGeneratorManager* pgM);
	void RegisterSteppingAction(TsSteppingAction* steppingAction);
	void HandleFirstEvent();

	void AbortSession(G4int exitCode);

private:
	TsParameterManager* fPm;
	TsExtensionManager* fEm;
	TsMaterialManager*  fMm;
	TsGeometryManager*  fGm;
	TsPhysicsManager*   fPhm;
	TsVarianceManager*  fVm;
	TsFilterManager*	fFm;
	TsScoringManager*   fScm;
	TsGraphicsManager*  fGrm;
	TsSourceManager*	fSom;
	TsChemistryManager*	fChm;

	std::vector<TsGeneratorManager*> fGeneratorManagers;
	std::vector<TsSteppingAction*> fSteppingActions;

	G4bool fUseQt;
	TsQt* fTsQt;

	G4int fVerbosity;
	G4int fRunID;

	G4Timer fTimer[3];
	G4double fTime;

	G4int fMaxStepNumber;
	G4double fKilledTrackEnergy;
	G4double fKilledTrackMaxEnergy;
	G4int fKilledTrackCount;
	G4int fKilledTrackMaxCount;
	G4int fKilledTrackMaxReports;

	G4double fUnscoredHitEnergy;
	G4double fUnscoredHitMaxEnergy;
	G4int fUnscoredHitCount;
	G4int fUnscoredHitMaxCount;
	G4int fUnscoredHitMaxReports;

	G4double fParameterizationErrorEnergy;
	G4double fParameterizationErrorMaxEnergy;
	G4int fParameterizationErrorCount;
	G4int fParameterizationErrorMaxCount;
	G4int fParameterizationErrorMaxReports;

	G4double fIndexErrorEnergy;
	G4double fIndexErrorMaxEnergy;
	G4int fIndexErrorCount;
	G4int fIndexErrorMaxCount;
	G4int fIndexErrorMaxReports;

	G4int fInterruptedHistoryCount;
	G4int fInterruptedHistoryMaxCount;
	G4int fInterruptedHistoryMaxReports;

	G4bool fIsExecutingSequence;

	std::ofstream fBCMFile;
};

#endif
