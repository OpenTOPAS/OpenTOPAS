// This file was copied from
// geant4/source/physics_lists/constructors/electromagnetic to TOPAS
// as an example of a user-contributed physics module.
// The only change for TOPAS was to pass the pointer to
// the TsParameterManager so that you can use TOPAS parameters to further
// control your physics.
//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
// $Id: MyPhysicsModule1.hh 66704 2013-01-10 18:20:17Z gunter $
//
//---------------------------------------------------------------------------
//
// ClassName:   MyPhysicsModule1
//
// Author:      V.Ivanchenko 09.11.2005
//
// Modified:
// 05.12.2005 V.Ivanchenko add controlled verbosity
// 23.11.2006 V.Ivanchenko remove mscStepLimit option and improve cout
//
//----------------------------------------------------------------------------
//
// This class provides construction of default EM standard physics
//

#ifndef MyPhysicsModule1_h
#define MyPhysicsModule1_h 1

#include "G4VPhysicsConstructor.hh"
#include "globals.hh"

class TsParameterManager;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class MyPhysicsModule1 : public G4VPhysicsConstructor
{
public:
  MyPhysicsModule1(TsParameterManager* pM);
  MyPhysicsModule1(G4int ver = 0);

  virtual ~MyPhysicsModule1();

  virtual void ConstructParticle();
  virtual void ConstructProcess();

private:
  G4int  verbose;
	
	TsParameterManager* fPm;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif






