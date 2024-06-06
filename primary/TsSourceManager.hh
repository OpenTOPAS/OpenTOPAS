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

#ifndef TsSourceManager_hh
#define TsSourceManager_hh

#include "globals.hh"

#include <map>

class TsParameterManager;
class TsGeometryManager;
class TsExtensionManager;
class TsSequenceManager;

class TsSource;

class TsSourceManager
{
public:
	TsSourceManager(TsParameterManager* pM, TsGeometryManager* gM, TsExtensionManager* eM);
	~TsSourceManager();

	void Initialize(TsSequenceManager* sqM);
	void SetCurrentSource(TsSource* currentSource);
	void NoteAnyUseOfChangeableParameters(const G4String& name);
	void UpdateForSpecificParameterChange(G4String parameter);
	void UpdateForNewRun(G4bool rebuiltSomeComponents);
	void UpdateForEndOfRun();

	void Finalize();

	TsSource* GetSource(G4String sourceName);

	TsGeometryManager* GetGeometryManager();

	G4bool RandomModeNeedsMoreRuns();

	G4int GetNumberOfHistoriesInRun();

	void AddSourceFromGUI(G4String& sourceName, G4String& componentName, G4String& typeName);

private:
	TsParameterManager* fPm;
	TsGeometryManager* fGm;
	TsExtensionManager* fEm;
	TsSequenceManager* fSqm;

	G4int fVerbosity;

	TsSource* fCurrentSource;
	std::map<G4String, TsSource*>* fSources;
	std::multimap< G4String, std::pair<TsSource*,G4String> > fChangeableParameterMap;
};

#endif
