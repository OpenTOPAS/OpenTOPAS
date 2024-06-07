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

#ifndef TsGraphicsView_hh
#define TsGraphicsView_hh

#include "globals.hh"

class TsParameterManager;
class TsGraphicsManager;
class TsGeometryManager;

class TsGraphicsView
{
public:
	TsGraphicsView(TsParameterManager* pM, TsGraphicsManager* grM, TsGeometryManager* gM, G4String viewerName);
	~TsGraphicsView();

	void CreateView();
	void UpdateForSpecificParameterChange(G4String parameter);
	void UpdateForNewRun(G4bool rebuiltSomeComponents);
	void UpdateForEndOfRun();
	void UpdateForEndOfSession();

	G4String GetName();

private:
	void ResolveParameters();
	void CacheGeometryPointers();
	void SaveToHardCopyIfRequested();

	void SetView();

	G4String GetColorAsRGBA(G4String color);
	G4String GetFullParmName(const char* parmName);
	G4String GetFullParmNameLower(const char* parmName);

	TsParameterManager* fPm;
	TsGraphicsManager* fGrm;
	TsGeometryManager* fGm;

	G4int fVerbosity;

	G4String fViewerName;
	G4String fViewerType;
	G4String fRefreshEvery;
	G4String fColorModel;
	G4bool fIncludeGeometry;
	G4bool fIncludeTrajectories;
	G4bool fUseSmoothTrajectories;
	G4bool fIncludeStepPoints;
	G4bool fIncludeAxes;
	G4bool fIsActive;
	G4bool fAlreadyCreated;
	G4int fMagneticFieldArrowDensity;
	G4bool fHadParameterChangeSinceLastRun;
};

#endif
