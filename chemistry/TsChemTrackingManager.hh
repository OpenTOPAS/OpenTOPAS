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

#ifndef TsChemTrackingManager_hh
#define TsChemTrackingManager_hh

#include "G4ITTrackingInteractivity.hh"
#include <vector>

class G4VTrajectory;

class TsChemTrackingManager : public G4ITTrackingInteractivity {
	
	G4UserTrackingAction* fpUserTrackingAction;
	G4UserSteppingAction* fpUserSteppingAction;
	int fStoreTrajectory;
	std::vector<G4VTrajectory*> fTrajectories;
	
public:
	TsChemTrackingManager();
	virtual ~TsChemTrackingManager();
	
	virtual void Initialize();
	virtual void StartTracking(G4Track*);
	virtual void AppendStep(G4Track* track, G4Step* step);
	virtual void EndTracking(G4Track*);
	virtual void Finalize();
	
	void SetUserAction(G4UserTrackingAction*);
	inline G4UserTrackingAction* GetUserTrackingAction();
	
	void SetUserAction(G4UserSteppingAction*);
	inline G4UserSteppingAction* GetUserSteppingAction();
	
};


inline void TsChemTrackingManager::SetUserAction(G4UserTrackingAction* trackingAction) {
	fpUserTrackingAction = trackingAction;
}


inline void TsChemTrackingManager::SetUserAction(G4UserSteppingAction* steppingAction) {
	fpUserSteppingAction = steppingAction;
}


inline G4UserSteppingAction* TsChemTrackingManager::GetUserSteppingAction() {
	return fpUserSteppingAction;
}


inline G4UserTrackingAction* TsChemTrackingManager::GetUserTrackingAction() {
	return fpUserTrackingAction;
}


#endif
