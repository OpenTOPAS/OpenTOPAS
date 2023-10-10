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

#ifndef TsTrackInformation_hh
#define TsTrackInformation_hh

#include "G4ParticleDefinition.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VUserTrackInformation.hh"
#include "G4ThreeVector.hh"
#include "G4Track.hh"
#include "G4PrimaryVertex.hh"

class G4VProcess;

class TsTrackInformation : public G4VUserTrackInformation
{
  public:
	TsTrackInformation();
	~TsTrackInformation();

	void AddParticleDef(G4ParticleDefinition* particleDef);
	void AddParticleCharge(G4int charge);
	void AddOriginVolume(G4VPhysicalVolume* volume);
	void AddTraversedVolume(G4VPhysicalVolume* volume);
	void AddAncestorTraversedVolume(G4VPhysicalVolume* volume);
	void AddInteractionVolume(G4VPhysicalVolume* volume);
	void AddAncestorInteractionVolume(G4VPhysicalVolume* volume);
	void AddCreatorProcess(G4VProcess* process);
	void AddParentTrack(const G4Track* parentTrack);
	void AddParentTrackID(G4int parentTrackID);
	void AddParentTrackVertexPosition(G4ThreeVector parentTrackVertexPosition);
	void AddParentTrackVertexMomentumDirection(G4ThreeVector parentTrackVertexMomentumDirection);
	void AddParentTrackVertexKineticEnergy(G4double parentTrackVertexKineticEnergy);
	void SetSplitTrackID(G4int splitTrackID);
	void IncrementInteractionCount();

	std::vector<G4ParticleDefinition*> GetParticleDefs();
	std::vector<G4int> GetParticleCharges();
	std::vector<G4VPhysicalVolume*> GetOriginVolumes();
	std::vector<G4VPhysicalVolume*> GetTraversedVolumes();
	std::vector<G4VPhysicalVolume*> GetAncestorTraversedVolumes();
	std::vector<G4VPhysicalVolume*> GetInteractionVolumes();
	std::vector<G4VPhysicalVolume*> GetAncestorInteractionVolumes();
	std::vector<G4VProcess*> GetCreatorProcesses();
	std::vector<const G4Track*> GetParentTracks();
	std::vector<G4int> GetParentTrackIDs();
	std::vector<G4ThreeVector> GetParentTrackVertexPositions();
	std::vector<G4ThreeVector> GetParentTrackVertexMomentumDirections();
	std::vector<G4double> GetParentTrackVertexKineticEnergies();
	G4int GetSplitTrackID();
	G4int GetInteractionCount();

  private:
	std::vector<G4ParticleDefinition*> fParticleDefs;
	std::vector<G4int> fParticleCharges;
	std::vector<G4VPhysicalVolume*> fOriginVolumes;
	std::vector<G4VPhysicalVolume*> fTraversedVolumes;
	std::vector<G4VPhysicalVolume*> fAncestorTraversedVolumes;
	std::vector<G4VPhysicalVolume*> fAncestorInteractionVolumes;
	std::vector<G4VPhysicalVolume*> fInteractionVolumes;
	std::vector<G4VProcess*> fCreatorProcesses;
	std::vector<const G4Track*> fParentTracks;
	std::vector<G4int> fParentTrackIDs;
	std::vector<G4ThreeVector> fParentTrackVertexPositions;
	std::vector<G4ThreeVector> fParentTrackVertexMomentumDirections;
	std::vector<G4double> fParentTrackVertexKineticEnergies;
	G4int fSplitTrackID;
	G4int fInteractionCount;
};

#endif
