// BeginRun for TOPAS
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

#include "MyBeginRun.hh"

#include "TsParameterManager.hh"

MyBeginRun::MyBeginRun(TsParameterManager* pM)
{
	G4cout << "My Begin Run has been called" << G4endl;
}


MyBeginRun::~MyBeginRun()
{
}
