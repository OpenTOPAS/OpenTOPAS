// EndSession for TOPAS
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

#include "MyEndSession.hh"

#include "TsParameterManager.hh"

MyEndSession::MyEndSession(TsParameterManager* pM)
{
	G4cout << "My End Session has been called" << G4endl;
}


MyEndSession::~MyEndSession()
{
}
