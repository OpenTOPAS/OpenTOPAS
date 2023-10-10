// EndRun for TOPAS
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

#include "MyEndRun.hh"

#include "TsParameterManager.hh"

MyEndRun::MyEndRun(TsParameterManager* pM)
{
	G4cout << "My End Run has been called" << G4endl;
}


MyEndRun::~MyEndRun()
{
}
