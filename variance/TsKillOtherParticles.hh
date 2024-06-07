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

#ifndef TsKillOtherParticles_hh
#define TsKillOtherParticles_hh

#include "TsVBiasingProcess.hh"

#include "G4VPhysicalVolume.hh"
#include "G4Track.hh"
#include "G4ClassificationOfNewTrack.hh"

#include <vector>

class TsParameterManager;
class TsGeometryManager;

class TsKillOtherParticles : public TsVBiasingProcess
{
public:
	TsKillOtherParticles(G4String name, TsParameterManager*, TsGeometryManager*);
	~TsKillOtherParticles();
	
	void ResolveParameters();
	void Initialize();
	void Clear();
	void AddBiasingProcess() {return;};

	G4ClassificationOfNewTrack Apply(const G4Track* );
	G4bool AcceptTrack(const G4Track*);
	G4bool AcceptVolume(G4VPhysicalVolume* pv);
	
private:
	TsParameterManager* fPm;
	TsGeometryManager* fGm;
	
	G4String fName;
	G4bool fAvoid;
	G4bool fInvertFilter;
	G4int fLengthAvoidComponents;
	std::vector<G4VPhysicalVolume*> fAvoidVolumes;
	G4String* fParticleName;
	std::vector<G4ParticleDefinition*> fParticleDefs;
	std::vector<G4bool> fIsGenericIon;
	std::vector<G4int> fAtomicNumbers;
	std::vector<G4int> fAtomicMasses;
	std::vector<G4int> fCharges;
};
#endif
