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

#ifndef TsVFilter_hh
#define TsVFilter_hh

#include "TsParameterManager.hh"

#include "TsVGenerator.hh"
#include "TsVScorer.hh"

#include "G4VSDFilter.hh"
#include "G4Step.hh"

class TsMaterialManager;
class TsFilterManager;
class TsVScorer;

class TsVGeometryComponent;

class TsVFilter : public G4VSDFilter
{
public:
	TsVFilter(G4String name, TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM,
			  TsFilterManager* fM, TsVGenerator* generator, TsVScorer* scorer, TsVFilter* parentFilter, G4bool invert = false);
	virtual ~TsVFilter();

	// This is where logic goes to filter scoring
	virtual G4bool Accept(const G4Step*) const = 0;

	// This is where logic goes to filter primary particles
	virtual G4bool AcceptTrack(const G4Track*) const = 0;

protected:
	// Gives full parameter name given just last part
	G4String GetFullParmName(const char* parmName);

	// Gives full name of filter (generator name or scorer name plus filter name)
	G4String GetFullName();

	// If you need to evaluate any parameters, do so here
	virtual void ResolveParameters();

	// If you need to obtain any pointers to geometry, do so here
	virtual void CacheGeometryPointers();

	// Methods to obtain pointers to geometry objects
	TsVGeometryComponent* GetComponent(G4String& nameWithCopyId);
	std::vector<G4String> GetChildComponentsOf(G4String& parentName);
	G4VPhysicalVolume* GetPhysicalVolume(G4String volumeName);

	// Methods to obtain pointers to materials
	G4Material* GetMaterial(G4String name);
	G4Material* GetMaterial(const char* name);

	// Tells whether filter has been inverted
	G4bool fInvert;

	// Pointer to parameter manager
	TsParameterManager* fPm;

	// User classes should not access any methods or data beyond this point
public:
	void UpdateForSpecificParameterChange(G4String parameter);
	void UpdateForNewRun(G4bool rebuiltSomeComponents);

protected:
	TsVGenerator* fGenerator;
	TsVScorer* fScorer;
	TsVFilter* fParentFilter;

private:
	TsMaterialManager* fMm;
	TsGeometryManager* fGm;
	TsFilterManager* fFm;

	G4String fFullName;
	G4int fVerbosity;
	G4bool fHadParameterChangeSinceLastRun;
};
#endif
