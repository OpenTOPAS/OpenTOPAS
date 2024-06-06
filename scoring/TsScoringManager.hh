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

#ifndef TsScoringManager_hh
#define TsScoringManager_hh

#include "globals.hh"
#include <vector>
#include <map>

#include "TsTopasConfig.hh"

#ifdef TOPAS_MT
#include "G4Cache.hh"
#endif

class TsParameterManager;
class TsExtensionManager;
class TsMaterialManager;
class TsGeometryManager;
class TsFilterManager;

class TsScoringHub;
class TsVScorer;

class G4MultiFunctionalDetector;

#include "G4RootAnalysisManager.hh"
#include "G4XmlAnalysisManager.hh"

#if GEANT4_VERSION_MAJOR <= 10
#include "g4root.hh"
#include "g4xml.hh"
#endif

class TsScoringManager
{
public:
	TsScoringManager(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsFilterManager* fm);
	~TsScoringManager();

	void Initialize();
	TsVScorer* InstantiateScorer(G4String scorerName, G4String quantityName, G4String outFileName, G4bool isSubScorer);

	void SetCurrentScorer(TsVScorer* currentScorer);
	void RegisterScorer(TsVScorer* scorer);
	void InstantiateFilters();
	void NoteAnyUseOfChangeableParameters(const G4String& name);
	void UpdateForSpecificParameterChange(G4String parameter);
	void UpdateForNewRun(G4bool rebuiltSomeComponents);

	G4bool HasUnsatisfiedLimits();

	void UpdateForEndOfRun();
	void RestoreResultsFromFile();
	void Finalize();

	TsVScorer* GetMasterScorerByID(G4int uid);
	std::vector<TsVScorer*> GetMasterScorersByName(G4String scorerName);
	G4bool AddUnitEvenIfItIsOne();
	G4RootAnalysisManager* GetRootAnalysisManager();
	G4XmlAnalysisManager* GetXmlAnalysisManager();

	G4MultiFunctionalDetector* GetDetector(G4String componentName, TsVScorer* scorer);

	TsExtensionManager* GetExtensionManager();
	TsScoringHub* GetScoringHub();

private:
	G4String GetFullParmName(const char* parmName);

	TsParameterManager* fPm;
	TsExtensionManager* fEm;
	TsMaterialManager* fMm;
	TsGeometryManager* fGm;
	TsFilterManager* fFm;

	G4int fVerbosity;
	G4int fTfVerbosity;
	G4bool fAddUnitEvenIfItIsOne;

	G4RootAnalysisManager* fRootAnalysisManager;
	G4XmlAnalysisManager* fXmlAnalysisManager;

	TsScoringHub* fScoringHub;

	G4String fQuantityParmName;

	std::map<G4String,G4MultiFunctionalDetector*>* fDetectors;

#ifdef TOPAS_MT
	G4Cache<TsVScorer*> fCurrentScorer;
	G4Cache<G4String> fCurrentScorerName;
#else
	TsVScorer* fCurrentScorer;
	G4String fCurrentScorerName;
#endif

	G4int fUID;
	std::vector<G4String> fOutFileNames;
	std::vector<TsVScorer*> fMasterScorers;
	std::vector<TsVScorer*> fWorkerScorers;
	std::vector<G4int> fWorkerScorerThreadIDs;
	std::multimap< G4String, std::pair<TsVScorer*,G4String> > fChangeableParameterMap;

public:
	inline G4int GetVerbosity() const { return fVerbosity; }
	inline G4int GetTfVerbosity() const { return fTfVerbosity; }
};

#endif
