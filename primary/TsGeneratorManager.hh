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

#ifndef TsGeneratorManager_hh
#define TsGeneratorManager_hh

#include "G4VUserPrimaryGeneratorAction.hh"

#include "globals.hh"

#include <vector>
#include <map>

class TsParameterManager;
class TsExtensionManager;
class TsGeometryManager;
class TsSourceManager;
class TsFilterManager;
class TsSequenceManager;

class TsVGenerator;
class TsSource;
class TsVFilter;

class TsGeneratorManager : public G4VUserPrimaryGeneratorAction
{
public:
	TsGeneratorManager(TsParameterManager* pM, TsExtensionManager* eM, TsGeometryManager* gM, TsSourceManager* prM, TsFilterManager* fM,  TsSequenceManager* sqM);
	~TsGeneratorManager();

	TsSource* GetSource(G4String sourceName);
	TsVFilter* GetFilter(G4int primaryCounter);

	void SetIsExecutingSequence(G4bool isExecutingSequence);

	void GeneratePrimaries(G4Event* anEvent);

	void RegisterPrimary(TsVGenerator* generator);

	void SetCurrentGenerator(TsVGenerator* generator);
	void NoteAnyUseOfChangeableParameters(const G4String& name);
	void UpdateForSpecificParameterChange(G4String parameter);
	void UpdateForNewRun(G4bool rebuiltSomeComponents);
	void ClearGenerators();

	void Finalize();

private:
	TsParameterManager* fPm;
	TsGeometryManager* fGm;
	TsSourceManager* fPrm;
	TsFilterManager* fFm;

	G4int fVerbosity;

	G4bool fIsExecutingSequence;

	G4int fPrimaryCounter;

	std::map<G4int,TsVGenerator*>* fGeneratorPerPrimary;

	TsVGenerator* fCurrentGenerator;
	std::vector<TsVGenerator*> fGenerators;
	std::multimap< G4String, std::pair<TsVGenerator*,G4String> > fChangeableParameterMap;
};

#endif
