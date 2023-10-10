// BeginHistory for TOPAS
//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * This file was obtained from Topas MC Inc under the license       *
// * agreement set forth at http://www.topasmc.org/registration       *
// * Any use of this file constitutes full acceptance of              *
// * this TOPAS MC license agreement.                                 *
// *                                                                  *
// ********************************************************************
//

#include "MyBeginHistory.hh"

#include "TsParameterManager.hh"
#include "MyExtraClass.hh"

#include "G4Run.hh"
#include "G4Event.hh"

MyBeginHistory::MyBeginHistory(TsParameterManager* pM, const G4Run* run, const G4Event* event)
{
	G4cout << "My Begin History has been called for run: " << run->GetRunID() <<
	", history: " << event->GetEventID() << G4endl;
	
	// Demonstrate that BeginHistory can call some other user-provided class
	new MyExtraClass();
}


MyBeginHistory::~MyBeginHistory()
{
}
