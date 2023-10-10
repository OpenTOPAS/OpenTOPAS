//
// ********************************************************************
// *                                                                  *
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

#ifndef TsSource_hh
#define TsSource_hh

#include "globals.hh"

class TsParameterManager;
class TsSourceManager;
class TsVGeometryComponent;

class TsSource
{
public:
	TsSource(TsParameterManager* pM, TsSourceManager* psM, G4String sourceName);
	~TsSource();

	// Handle time-dependent parameter changes
	virtual void UpdateForSpecificParameterChange(G4String parameter);

	// Do any special work needed at end of run
	virtual void UpdateForEndOfRun();

	// Get name of this particle source
	G4String GetName();

protected:
	// Resolve any parameters needed by the concrete source
	virtual void ResolveParameters();

	// Gives full parameter name given just last part
	G4String GetFullParmName(const char* parmName);

	// Pointer to parameter manager
	TsParameterManager* fPm;

	// Pointer to Component
	TsVGeometryComponent* fComponent;

	// User classes should not access any methods or data beyond this point

public:
	virtual void UpdateForNewRun(G4bool rebuiltSomeComponents);

	G4int GetNumberOfHistoriesInRun();
	G4long GetNumberOfHistoriesInRandomJob();
	G4double GetProbabilityOfUsingAGivenRandomTime();
	G4bool RandomModeNeedsMoreRuns();

	void NoteNumberOfHistoriesGenerated(G4int number);
	void NoteNumberOfParticlesGenerated(G4long number);
	void NoteNumberOfParticlesSkipped(G4long number);

	void Finalize();

protected:
	G4String fSourceName;

	G4long fNumberOfHistoriesInRun;

private:
	TsSourceManager* fPsm;

	G4int fVerbosity;

	G4bool fHadParameterChangeSinceLastRun;

	G4double fProbabilityOfUsingAGivenRandomTime;
	G4long fNumberOfHistoriesInRandomJob;
	G4long fTotalHistoriesGenerated;
	G4long fTotalParticlesGenerated;
	G4long fTotalParticlesSkipped;
};

#endif
