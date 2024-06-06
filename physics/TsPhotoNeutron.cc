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

#include "TsPhotoNeutron.hh"

#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"
#include "G4HadronInelasticProcess.hh"
#include "G4LENDorBERTModel.hh"
#include "G4LENDCombinedCrossSection.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhotoNuclearCrossSection.hh"

// factory
#include "G4PhysicsConstructorFactory.hh"
//
G4_DECLARE_PHYSCONSTR_FACTORY(TsPhotoNeutron);

TsPhotoNeutron::TsPhotoNeutron(TsParameterManager*)
  : G4VPhysicsConstructor("PhotoNeutron")
{}


TsPhotoNeutron::TsPhotoNeutron(G4int)
: G4VPhysicsConstructor("PhotoNeutron")
{}


TsPhotoNeutron::~TsPhotoNeutron()
{}


void TsPhotoNeutron::ConstructProcess()
{
   G4ProcessManager* pManager = G4Gamma::Gamma()->GetProcessManager();
   G4HadronInelasticProcess* process = new G4HadronInelasticProcess("photonNuclear", G4Gamma::Definition());
   process->AddDataSet(new G4PhotoNuclearCrossSection);

   G4LENDorBERTModel* LEND = new G4LENDorBERTModel(G4Gamma::Gamma());
   LEND->SetMaxEnergy(40*MeV);
   process->RegisterMe(LEND);

   G4LENDCombinedCrossSection* LENDXS = new G4LENDCombinedCrossSection(G4Gamma::Gamma());
   LENDXS->SetMaxKinEnergy(40*MeV);
   LENDXS->DumpLENDTargetInfo(true);
   process->AddDataSet(LENDXS);

   pManager->AddDiscreteProcess(process);      
}

