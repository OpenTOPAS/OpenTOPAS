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

#ifndef TsFilterManager_hh
#define TsFilterManager_hh

#include "globals.hh"

#include <map>
#include <vector>

#include "TsTopasConfig.hh"

#ifdef TOPAS_MT
#include "G4Cache.hh"
#endif

class TsParameterManager;
class TsExtensionManager;
class TsMaterialManager;
class TsGeometryManager;
class TsFilterManager;

class TsVFilter;
class TsVGenerator;
class TsVScorer;
class TsFilterHub;

class G4Material;

class TsFilterManager
{
public:
	TsFilterManager(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM);
	~TsFilterManager();

	void SetCurrentFilter(TsVFilter* currentFilter);
	void NoteAnyUseOfChangeableParameters(const G4String& name);
	void UpdateForSpecificParameterChange(G4String parameter);
	void UpdateForNewRun(G4bool rebuiltSomeComponents);

	TsVFilter* InstantiateFilter(TsVGenerator* generator);
	TsVFilter* InstantiateFilter(TsVScorer* scorer);
	TsVFilter* InstantiateFilter(TsVGenerator* generator, TsVScorer* scorer);

private:
	TsParameterManager* fPm;
	TsExtensionManager* fEm;
	TsMaterialManager* fMm;
	TsGeometryManager* fGm;

	G4int fTfVerbosity;

	TsFilterHub* fFilterHub;

#ifdef TOPAS_MT
	G4Cache<TsVFilter*> fCurrentFilter;
#else
	TsVFilter* fCurrentFilter;
#endif

	std::vector<TsVFilter*> fFilters;
	std::multimap< G4String, std::pair<TsVFilter*,G4String> > fChangeableParameterMap;
};

#endif
