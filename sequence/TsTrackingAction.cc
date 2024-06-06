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

#include "TsTrackingAction.hh"

#include "TsParameterManager.hh"

#include "TsTrackInformation.hh"
#include "TsVScorer.hh"

#include "G4TrackingManager.hh"

TsTrackingAction::TsTrackingAction(TsParameterManager* pM):
fPm(pM), fInitialMomentum(0), fRequireSplitTrackID(false)
{
    if (fPm->ParameterExists("Vr/UseVarianceReduction") && fPm->GetBooleanParameter("Vr/UseVarianceReduction") &&
        fPm->ParameterExists("Vr/ParticleSplit/Active") && fPm->GetBooleanParameter("Vr/ParticleSplit/Active")) {
        G4String vrtType = fPm->GetStringParameter("Vr/ParticleSplit/Type");
#if GEANT4_VERSION_MAJOR >= 11
        G4StrUtil::to_lower(vrtType);
#else
	vrtType.toLower();
#endif
        if ( vrtType == "flaggeduniformsplit" ) {
            fRequireSplitTrackID = true;
        }
    }
}


TsTrackingAction::~TsTrackingAction()
{;}


void TsTrackingAction::PreUserTrackingAction(const G4Track* aTrack)
{
	fInitialMomentum = aTrack->GetMomentum().mag();

	if (aTrack->GetParentID() == 0) {
		std::vector<TsVScorer*>::iterator iter;
		for (iter=fScorers.begin(); iter!=fScorers.end(); iter++)
			(*iter)->ClearIncidentParticleInfo();
	}

	G4Track* newTrack = nullptr;
	if (fRequireSplitTrackID && aTrack->GetParentID() == 0) {
		TsTrackInformation* parentInformation = new TsTrackInformation();
		parentInformation->SetSplitTrackID(1);
		newTrack = (G4Track*)aTrack;
		newTrack->SetUserInformation(parentInformation);
	}

	if (fPm->GetBooleanParameter("Ph/SetNeutronToStable") &&
		aTrack->GetParticleDefinition()->GetParticleName() == "neutron")
	{
		newTrack = (G4Track*)aTrack;
		newTrack->GetDefinition()->SetPDGStable(true);
	}

	for (const auto& it : fScorers)
		it->UserHookForBeginOfTrack(newTrack == nullptr ? aTrack : newTrack);
}


void TsTrackingAction::PostUserTrackingAction(const G4Track* aTrack)
{
	for (const auto& it : fScorers)
		it->UserHookForEndOfTrack(aTrack);

	if (fPm->NeedsTrackingAction()) {
		TsTrackInformation* parentInformation = (TsTrackInformation*)(aTrack->GetUserInformation());

		// Fill each secondary's track information object with ancestor information
		G4TrackVector* secondaries = fpTrackingManager->GimmeSecondaries();
		if (secondaries)
		{
			size_t nSecondaries = secondaries->size();
			for (size_t iSecondary=0; iSecondary<nSecondaries; iSecondary++) {
				// Create user information class for the secondary
				TsTrackInformation* secondaryInformation = new TsTrackInformation();
				(*secondaries)[iSecondary]->SetUserInformation(secondaryInformation);

				// Add information from the parent track
				secondaryInformation->AddParticleDef(const_cast<G4ParticleDefinition*>(aTrack->GetParticleDefinition()));
				secondaryInformation->AddParticleCharge((G4int) (aTrack->GetDynamicParticle()->GetCharge() - aTrack->GetDynamicParticle()->GetTotalOccupancy()));

				if (aTrack->GetOriginTouchable())
					secondaryInformation->AddOriginVolume(aTrack->GetOriginTouchable()->GetVolume());

				secondaryInformation->AddCreatorProcess(const_cast<G4VProcess*>(aTrack->GetCreatorProcess()));
				secondaryInformation->AddParentTrack(aTrack);
				secondaryInformation->AddParentTrackID(aTrack->GetTrackID());
				secondaryInformation->AddParentTrackVertexPosition(aTrack->GetVertexPosition());
				secondaryInformation->AddParentTrackVertexMomentumDirection(aTrack->GetVertexMomentumDirection());
				secondaryInformation->AddParentTrackVertexKineticEnergy(aTrack->GetVertexKineticEnergy());

				// Add information from the parent track's ancestors
				if (parentInformation) {
					secondaryInformation->SetSplitTrackID(parentInformation->GetSplitTrackID());

					std::vector<G4ParticleDefinition*> particleDefs = parentInformation->GetParticleDefs();
					for (size_t i=0; i < particleDefs.size(); i++)
						secondaryInformation->AddParticleDef(particleDefs[i]);

					std::vector<G4int> charges = parentInformation->GetParticleCharges();
					for (size_t i=0; i < charges.size(); i++)
						secondaryInformation->AddParticleCharge(charges[i]);

					std::vector<G4VPhysicalVolume*> originVolumes = parentInformation->GetOriginVolumes();
					for (size_t i=0; i < originVolumes.size(); i++)
						secondaryInformation->AddOriginVolume(originVolumes[i]);

					std::vector<G4VPhysicalVolume*> traversedVolumes = parentInformation->GetTraversedVolumes();
					for (size_t i=0; i < traversedVolumes.size(); i++)
						secondaryInformation->AddAncestorTraversedVolume(traversedVolumes[i]);

					std::vector<G4VPhysicalVolume*> ancestorTraversedVolumes = parentInformation->GetAncestorTraversedVolumes();
					for (size_t i=0; i < ancestorTraversedVolumes.size(); i++)
						secondaryInformation->AddAncestorTraversedVolume(ancestorTraversedVolumes[i]);

					std::vector<G4VPhysicalVolume*> interactionVolumes = parentInformation->GetInteractionVolumes();
					for (size_t i=0; i < interactionVolumes.size(); i++)
						secondaryInformation->AddAncestorInteractionVolume(interactionVolumes[i]);

					std::vector<G4VPhysicalVolume*> ancestorInteractionVolumes = parentInformation->GetAncestorInteractionVolumes();
					for (size_t i=0; i < ancestorInteractionVolumes.size(); i++)
						secondaryInformation->AddAncestorInteractionVolume(ancestorInteractionVolumes[i]);

					std::vector<G4VProcess*> processes = parentInformation->GetCreatorProcesses();
					for (size_t i=0; i < processes.size(); i++)
						secondaryInformation->AddCreatorProcess(processes[i]);

					std::vector<const G4Track*> tracks = parentInformation->GetParentTracks();
					for (size_t i=0; i < tracks.size(); i++)
						secondaryInformation->AddParentTrack(tracks[i]);

					std::vector<G4int> ids = parentInformation->GetParentTrackIDs();
					for (size_t i=0; i < ids.size(); i++)
						secondaryInformation->AddParentTrackID(ids[i]);

					std::vector<G4ThreeVector> vertexPositions = parentInformation->GetParentTrackVertexPositions();
					for (size_t i=0; i < vertexPositions.size(); i++)
						secondaryInformation->AddParentTrackVertexPosition(vertexPositions[i]);

					std::vector<G4ThreeVector> vertexMomentumDirections = parentInformation->GetParentTrackVertexMomentumDirections();
					for (size_t i=0; i < vertexMomentumDirections.size(); i++)
						secondaryInformation->AddParentTrackVertexMomentumDirection(vertexMomentumDirections[i]);

					std::vector<G4double> vertexKineticEnergies = parentInformation->GetParentTrackVertexKineticEnergies();
					for (size_t i=0; i < vertexKineticEnergies.size(); i++)
						secondaryInformation->AddParentTrackVertexKineticEnergy(vertexKineticEnergies[i]);
				}
			}
		}
	} else if ( fRequireSplitTrackID ) {
        TsTrackInformation* parentInformation = (TsTrackInformation*)(aTrack->GetUserInformation());
        G4TrackVector* secondaries = fpTrackingManager->GimmeSecondaries();
        if (secondaries)
        {
            size_t nSecondaries = secondaries->size();
            for (size_t iSecondary=0; iSecondary<nSecondaries; iSecondary++) {
                if ((*secondaries)[iSecondary]->GetUserInformation() == 0 ) {

					// Create user information class for the secondary
					TsTrackInformation* secondaryInformation = new TsTrackInformation();
					(*secondaries)[iSecondary]->SetUserInformation(secondaryInformation);

					G4int splitTrackID = parentInformation->GetSplitTrackID();
					secondaryInformation->SetSplitTrackID(splitTrackID);
				}
			}
		}
    }
}


void TsTrackingAction::RegisterScorer(TsVScorer* scorer) {
	fScorers.push_back(scorer);
}


G4double TsTrackingAction::GetInitialMomentum() {
	return fInitialMomentum;
}
