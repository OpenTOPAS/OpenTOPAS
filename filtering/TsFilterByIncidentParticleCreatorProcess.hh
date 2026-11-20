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

#ifndef TsFilterByIncidentParticleCreatorProcess_hh
#define TsFilterByIncidentParticleCreatorProcess_hh

#include "TsVFilter.hh"

#include "G4Cache.hh"

#include <vector>

class G4VProcess;

class TsFilterByIncidentParticleCreatorProcess : public TsVFilter
{
public:
	TsFilterByIncidentParticleCreatorProcess(G4String name, TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM,
							 TsFilterManager* fM, TsVGenerator* generator, TsVScorer* scorer, TsVFilter* parentFilter, G4bool invert);
	virtual ~TsFilterByIncidentParticleCreatorProcess();

	void ResolveParameters();
	virtual void UpdateForNewRun(G4bool rebuiltSomeComponents);

	virtual G4bool Accept(const G4Step*) const;
	virtual G4bool AcceptTrack(const G4Track*) const;

private:
	const std::vector<G4VProcess*>& GetProcesses() const;

	struct ProcessCache {
		ProcessCache() : version(0) {}
		std::vector<G4VProcess*> processes;
		size_t version;
	};

	std::vector<G4String> fProcessNames;
	size_t fProcessCacheVersion;
	G4Cache<ProcessCache> fProcessCache;
};
#endif
