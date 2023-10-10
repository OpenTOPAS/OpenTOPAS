//
// ********************************************************************
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * TOPAS collaboration.                                             *
// * Use or redistribution of this code is not permitted without the  *
// * explicit approval of the TOPAS collaboration.                    *
// * Contact: Joseph Perl, perl@slac.stanford.edu                     *
// *                                                                  *
// ********************************************************************
//

#include "TsPeriodicBoundaryConditionProcess.hh"
#include "TsParameterManager.hh"
#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4TouchableHandle.hh"
#include "G4VSolid.hh"
#include "G4ParticleChange.hh"
#include "G4GeometryTolerance.hh"
#include "G4Box.hh"

TsPeriodicBoundaryConditionProcess::TsPeriodicBoundaryConditionProcess(TsParameterManager* fPm, G4String name)
: G4VBiasingOperation(name), fParticleChange(), fParticleChangeForNothing(), fName(name)
{
	fMinParentID = 1;
	if (fPm->ParameterExists("Vr/" + name + "/MinParentID"))
		fMinParentID = fPm->GetIntegerParameter("Vr/" + name + "/MinParentID");

	kCarTolerance = G4GeometryTolerance::GetInstance()->GetSurfaceTolerance();
}


TsPeriodicBoundaryConditionProcess::TsPeriodicBoundaryConditionProcess(G4String name)
: G4VBiasingOperation(name), fParticleChange(), fParticleChangeForNothing(), fName(name)
{}


TsPeriodicBoundaryConditionProcess::~TsPeriodicBoundaryConditionProcess()
{}


G4double TsPeriodicBoundaryConditionProcess::DistanceToApplyOperation(const G4Track*,
																	  G4double,
																	  G4ForceCondition* condition)
{
	*condition = Forced;
	return DBL_MAX;
}


G4VParticleChange* TsPeriodicBoundaryConditionProcess::GenerateBiasingFinalState(const G4Track* aTrack,
																				 const G4Step* aStep) {
	G4StepPoint* postStep = aStep->GetPostStepPoint();

	G4int ParentID = aTrack->GetParentID();
	G4int TrackID  = aTrack->GetTrackID();
	//G4cout << ParentID << "  " << TrackID << "  " << (postStep->GetStepStatus() == fGeomBoundary) << G4endl;
	
	if (ParentID >= fMinParentID && TrackID >= 0 && (postStep->GetStepStatus() == fGeomBoundary)) { // particle leaving volume
		
		G4TouchableHandle theTouchable = aStep->GetPreStepPoint()->GetTouchableHandle();
		G4ThreeVector stppos = aStep->GetPostStepPoint()->GetPosition();
		
		G4double x, y, z;
		G4double halfLengthX = ((G4Box*)(theTouchable->GetVolume()->GetLogicalVolume()->GetSolid()))->GetXHalfLength();
		G4double halfLengthY = ((G4Box*)(theTouchable->GetVolume()->GetLogicalVolume()->GetSolid()))->GetYHalfLength();
		G4double halfLengthZ = ((G4Box*)(theTouchable->GetVolume()->GetLogicalVolume()->GetSolid()))->GetZHalfLength();
		
		if ( std::abs(stppos.x()) ==  halfLengthX) {
			if (stppos.x() == halfLengthX )
				x = -halfLengthX;
			else
				x = halfLengthX;
			y = stppos.y();
			z = stppos.z();
			fParticleChange.Initialize(*aTrack);
			
			G4Track* newTrack = new G4Track(*aTrack);
			newTrack->SetPosition(G4ThreeVector(x,y,z));
			fParticleChange.AddSecondary(newTrack);
			fParticleChange.ProposeTrackStatus(fStopAndKill);
			return &fParticleChange;
		}
		if ( std::abs(stppos.y()) == halfLengthY ) {
			if (stppos.y() == halfLengthY )
				y = -halfLengthY;
			else
				y = halfLengthY;
			x = stppos.x();
			z = stppos.z();
			fParticleChange.Initialize(*aTrack);
			G4Track* newTrack = new G4Track(*aTrack);
			newTrack->SetPosition(G4ThreeVector(x,y,z));
			fParticleChange.AddSecondary(newTrack);
			fParticleChange.ProposeTrackStatus(fStopAndKill);
			return &fParticleChange;
		}
		
		if ( std::abs(stppos.z()) == halfLengthZ) {
			if (stppos.z() == halfLengthZ )
				z = -halfLengthZ;
			else
				z = halfLengthZ;
			y = stppos.y();
			x = stppos.x();
			fParticleChange.Initialize(*aTrack);
			G4Track* newTrack = new G4Track(*aTrack);
			newTrack->SetPosition(G4ThreeVector(x,y,z));
			fParticleChange.AddSecondary(newTrack);
			fParticleChange.ProposeTrackStatus(fStopAndKill);
			return &fParticleChange;
		}
	}
	
	fParticleChangeForNothing.Initialize(*aTrack);
	return &fParticleChangeForNothing;
}


void TsPeriodicBoundaryConditionProcess::SetRegions(G4String* names) {
	fNamesOfRegions = names;
	fRegions.clear();
	for ( int i = 0; i < fNumberOfRegions; i++ )
		fRegions.push_back(G4RegionStore::GetInstance()->FindOrCreateRegion(fNamesOfRegions[i]));
}


void TsPeriodicBoundaryConditionProcess::SetNumberOfRegions(G4int nRegions) {
	fNumberOfRegions = nRegions;
}

