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

