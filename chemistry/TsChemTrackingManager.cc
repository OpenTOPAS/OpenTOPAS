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

#include "TsChemTrackingManager.hh"

#include "G4TrackingInformation.hh"
#include "G4VTrajectory.hh"
#include "G4Trajectory.hh"
#include "G4SmoothTrajectory.hh"
#include "G4RichTrajectory.hh"
#include "G4UserTrackingAction.hh"
#include "G4UserSteppingAction.hh"
#include "G4IT.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4VSteppingVerbose.hh"
#include "G4VisManager.hh"
#include "G4VSensitiveDetector.hh"

#include "G4SystemOfUnits.hh"

class G4Trajectory_Lock
{
	friend class TsChemTrackingManager;
	
	G4Trajectory_Lock(): fpTrajectory(0)
	{;}
	
	~G4Trajectory_Lock()
	{;}
	
	G4VTrajectory* fpTrajectory;
};


TsChemTrackingManager::TsChemTrackingManager()
{
	fpUserTrackingAction = 0;
	fpUserSteppingAction = 0;
	fStoreTrajectory = 0;
}


TsChemTrackingManager::~TsChemTrackingManager()
{
	G4EventManager* eventManager = G4EventManager::GetEventManager();
	
	if ( eventManager ) {
		G4UserTrackingAction* trackingAction = eventManager->GetUserTrackingAction();
		if ( fpUserTrackingAction != trackingAction && fpUserTrackingAction )
			delete fpUserTrackingAction;
		
		G4UserSteppingAction* steppingAction = eventManager->GetUserSteppingAction();
		if( fpUserSteppingAction != steppingAction&& fpUserSteppingAction )
			delete fpUserSteppingAction;
		
	} else {
		if(fpUserSteppingAction)
			delete fpUserSteppingAction;
		
		if(fpUserTrackingAction)
			delete fpUserTrackingAction;
	}
}


void TsChemTrackingManager::Initialize()
{
	G4TrackingManager* trackingManager = G4EventManager::GetEventManager()->GetTrackingManager();
	fStoreTrajectory = trackingManager->GetStoreTrajectory();
}


void TsChemTrackingManager::StartTracking(G4Track* track)
{
	if( 0 != fpUserTrackingAction )
		fpUserTrackingAction->PreUserTrackingAction(track);
	
	G4TrackingInformation* trackingInfo = GetIT(track)->GetTrackingInfo();
	G4Trajectory_Lock* trajectory_lock = trackingInfo->GetTrajectory_Lock();
	
	if( fStoreTrajectory && !trajectory_lock ) {
		trajectory_lock = new G4Trajectory_Lock();
		trackingInfo->SetTrajectory_Lock(trajectory_lock);
		G4VTrajectory* trajectory = 0;
		
		switch (fStoreTrajectory) {
			default:
			case 1: trajectory = new G4Trajectory(track); break;
			case 2: trajectory = new G4SmoothTrajectory(track); break;
			case 3: trajectory = new G4RichTrajectory(track); break;
		}
		trajectory_lock->fpTrajectory = trajectory;
	}
}


void TsChemTrackingManager::AppendStep(G4Track* track, G4Step* step)
{
	
	G4VPhysicalVolume* currentVolume = step->GetPreStepPoint()->GetPhysicalVolume();
	G4SteppingControl stepControlFlag =  step->GetControlFlag();
	
	if( 0 != currentVolume && stepControlFlag != AvoidHitInvocation) {
		G4VSensitiveDetector* sensitive = step->GetPreStepPoint()->GetSensitiveDetector();
		if( 0 != sensitive ){
			sensitive->Hit(step);
		}
	}
	
	if(fpUserSteppingAction)
		fpUserSteppingAction->UserSteppingAction(step);
	
	if(fStoreTrajectory) {
		G4TrackingInformation* trackingInfo = GetIT(track)->GetTrackingInfo();
		G4Trajectory_Lock* trajectory_lock = trackingInfo->GetTrajectory_Lock();
		trajectory_lock->fpTrajectory->AppendStep(step);
	}
}


void TsChemTrackingManager::EndTracking(G4Track* track)
{
	
	if ( 0!= fpUserTrackingAction )
		fpUserTrackingAction->PostUserTrackingAction(track);
	
	G4TrackingInformation* trackingInfo = GetIT(track)->GetTrackingInfo();
	G4Trajectory_Lock* trajectory_lock = trackingInfo->GetTrajectory_Lock();
	
	if(trajectory_lock) {
		G4VTrajectory*& trajectory = trajectory_lock->fpTrajectory;
		
		if(fStoreTrajectory && trajectory) {
			
			G4TrackStatus istop = track->GetTrackStatus();
			if (trajectory && (istop != fStopButAlive) && (istop != fSuspend)) {
				G4Event* currentEvent = G4EventManager::GetEventManager()->GetNonconstCurrentEvent();
				if (currentEvent) {
					G4TrajectoryContainer* trajectoryContainer = currentEvent->GetTrajectoryContainer();
					if (!trajectoryContainer) {
						trajectoryContainer = new G4TrajectoryContainer;
						currentEvent->SetTrajectoryContainer(trajectoryContainer);
					}
					trajectoryContainer->insert(trajectory);
				}
				else {
					fTrajectories.push_back(trajectory);
				}
			}
		}
		else if( (!fStoreTrajectory)&&trajectory ) {
			delete trajectory;
			trajectory = 0;
		}
		delete trajectory_lock;
		trackingInfo->SetTrajectory_Lock(0);
	}
}


void TsChemTrackingManager::Finalize()
{
	for (std::vector<G4VTrajectory*>::iterator it = fTrajectories.begin(); it != fTrajectories.end(); it++)
		G4VisManager::GetConcreteInstance()->Draw(**it);
}
