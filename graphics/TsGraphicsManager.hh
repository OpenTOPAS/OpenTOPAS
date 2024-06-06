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

#ifndef TsGraphicsManager_hh
#define TsGraphicsManager_hh

#include "globals.hh"
#include <map>

class TsParameterManager;
class TsGeometryManager;
class TsGraphicsView;

class G4VisManager;
class G4Event;
class G4VViewer;

class TsGraphicsManager
{
public:
	TsGraphicsManager(TsParameterManager* pM, TsGeometryManager* gM);
	~TsGraphicsManager();

	void Initialize();

	void SetCurrentView(TsGraphicsView* currentView);
	void NoteAnyUseOfChangeableParameters(const G4String& name);
	void UpdateForSpecificParameterChange(G4String parameter);
	void UpdateForNewRun(G4bool rebuiltSomeComponents);
	void UpdateForEndOfRun();
	void UpdateForEndOfSession();
	G4bool UsingOpenGL();
	G4bool UsingRayTracer();
	G4VViewer* GetCurrentViewer();

private:
	TsParameterManager* fPm;
	TsGeometryManager* fGm;

	G4bool fEnabled;
	G4int fVerbosity;
	G4bool fUsingOpenGL;
	G4bool fUsingRayTracer;

	G4VisManager* fVisManager;

	TsGraphicsView* fCurrentView;
	std::map<G4String, TsGraphicsView*>* fViews;
	std::multimap< G4String, std::pair<TsGraphicsView*,G4String> > fChangeableParameterMap;
};

#endif
