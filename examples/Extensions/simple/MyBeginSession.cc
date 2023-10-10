// BeginSession for TOPAS
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

#include "MyBeginSession.hh"

#include "TsParameterManager.hh"

MyBeginSession::MyBeginSession(TsParameterManager* pM)
{
	G4cout << "My Begin Session has been called" << G4endl;
}


MyBeginSession::~MyBeginSession()
{
}
