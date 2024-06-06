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

#ifndef TsVarianceManager_hh
#define TsVarianceManager_hh

#include <vector>
#include <map>
#include "G4Track.hh"
#include "G4ClassificationOfNewTrack.hh"
#include "globals.hh"

class TsParameterManager;
class TsGeometryManager;
class TsPhysicsManager;
class TsVBiasingProcess;

class TsVarianceManager
{
public:
	TsVarianceManager(TsParameterManager* pM, TsGeometryManager* gM, TsPhysicsManager* phM);
	virtual ~TsVarianceManager();

public:
	void Configure();
	void Initialize();
	void Clear();
	void AddBiasingProcess();
	void UpdateForNewRun(G4bool);
	
	G4bool BiasingProcessExists(G4String type, G4int& index);
	
	TsVBiasingProcess* GetBiasingProcessFromList(G4int index);
	
	G4ClassificationOfNewTrack ApplyKillOtherParticles(const G4Track*);
	inline G4bool UseKillOtherParticles() {return fUseKillOtherParticles;};
	
	G4ClassificationOfNewTrack ApplyDirectionalRussianRoulette(const G4Track*);
	inline G4bool UseDirectionalRussianRoulette() {return fUseDirectionalRussianRoulette;};
	
	G4ClassificationOfNewTrack ApplyRangeRejection(const G4Track*);
	inline G4bool UseRangeRejection() {return fUseRangeRejection;};
	
private:
	TsParameterManager* fPm;
	TsGeometryManager* fGm;
	TsPhysicsManager* fPhm;
	
	std::vector<TsVBiasingProcess*> fBiasingProcesses;
	G4bool fUseKillOtherParticles;
	G4int fIndexKillOtherParticles;
	G4bool fUseDirectionalRussianRoulette;
	G4int fIndexDirectionalRussianRoulette;
	G4bool fUseRangeRejection;
	G4int fIndexRangeRejection;
	
};
#endif
