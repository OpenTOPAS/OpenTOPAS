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

#ifndef MyBeginHistory_hh
#define MyBeginHistory_hh

class TsParameterManager;
class G4Run;
class G4Event;

class MyBeginHistory
{    
public:
	MyBeginHistory(TsParameterManager* pM, const G4Run* run, const G4Event* event);
	~MyBeginHistory();
};

#endif
