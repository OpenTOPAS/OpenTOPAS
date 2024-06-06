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

#include "TsModularPhysicsList.hh"

#include "TsExtensionManager.hh"
#include "TsGeometryManager.hh"
#include "TsParameterManager.hh"
#include "TsVarianceManager.hh"

#include "TsPhotoNeutron.hh"
#include "TsGeometricalParticleSplit.hh"
#include "TsGeometrySampler.hh"
#include "TsIStore.hh"
#include "TsVGeometryComponent.hh"

#include "G4BaryonConstructor.hh"
#include "G4BosonConstructor.hh"
#include "G4IonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4ParticleTable.hh"
#include "G4ShortLivedConstructor.hh"

#include "G4BuilderType.hh"
#include "G4ChargeExchangePhysics.hh"
#include "G4CoupledTransportation.hh"
#include "G4DNAModelSubType.hh"
#include "G4DecayPhysics.hh"
#include "G4EmDNAChemistry.hh"
#include "G4EmDNAChemistry_option1.hh"
#include "G4EmDNAPhysics.hh"
#include "G4EmDNAPhysicsActivator.hh"
#include "G4EmDNAPhysics_option1.hh"
#include "G4EmDNAPhysics_option2.hh"
#include "G4EmDNAPhysics_option3.hh"
#include "G4EmDNAPhysics_option4.hh"
#include "G4EmDNAPhysics_option5.hh"
#include "G4EmDNAPhysics_option6.hh"
#include "G4EmDNAPhysics_option7.hh"
#include "G4EmDNAPhysics_option8.hh"
#include "G4EmDNAPhysics_stationary.hh"
#include "G4EmDNAPhysics_stationary_option2.hh"
#include "G4EmDNAPhysics_stationary_option4.hh"
#include "G4EmDNAPhysics_stationary_option6.hh"
#include "G4EmExtraPhysics.hh"
#include "G4EmLivermorePhysics.hh"
#include "G4EmLivermorePolarizedPhysics.hh"
#include "G4EmLowEPPhysics.hh"
#include "G4EmParameters.hh"
#include "G4EmPenelopePhysics.hh"
#include "G4EmStandardPhysics.hh"
#include "G4EmStandardPhysicsGS.hh"
#include "G4EmStandardPhysicsSS.hh"
#include "G4EmStandardPhysicsWVI.hh"
#include "G4EmStandardPhysics_option1.hh"
#include "G4EmStandardPhysics_option2.hh"
#include "G4EmStandardPhysics_option3.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4HadronDElasticPhysics.hh"
#include "G4HadronElasticPhysics.hh"
#include "G4HadronElasticPhysicsHP.hh"
#include "G4HadronElasticPhysicsLEND.hh"
#include "G4HadronElasticPhysicsXS.hh"
#include "G4HadronHElasticPhysics.hh"
#include "G4HadronInelasticQBBC.hh"
#include "G4HadronPhysicsFTFP_BERT.hh"
#include "G4HadronPhysicsFTFP_BERT_HP.hh"
#include "G4HadronPhysicsFTFP_BERT_TRV.hh"
#include "G4HadronPhysicsFTF_BIC.hh"
#include "G4HadronPhysicsQGSP_BERT.hh"
#include "G4HadronPhysicsQGSP_BERT_HP.hh"
#include "G4HadronPhysicsQGSP_BIC.hh"
#include "G4HadronPhysicsQGSP_BIC_AllHP.hh"
#include "G4HadronPhysicsQGSP_BIC_HP.hh"
#include "G4HadronPhysicsQGSP_FTFP_BERT.hh"
#include "G4HadronPhysicsQGS_BIC.hh"
#include "G4HadronPhysicsShielding.hh"
#include "G4IonBinaryCascadePhysics.hh"
#include "G4IonINCLXXPhysics.hh"
#include "G4IonPhysics.hh"
#include "G4IonQMDPhysics.hh"
#include "G4NeutronTrackingCut.hh"
#include "G4OpticalPhysics.hh"
#include "G4ParallelWorldProcess.hh"
#include "G4ProcessManager.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4StoppingPhysics.hh"
#include "G4Transportation.hh"
#include "G4VPhysicsConstructor.hh"

#include "G4RegionStore.hh"
#include "G4SystemOfUnits.hh"
#include "G4VUserParallelWorld.hh"

#include "G4UIcommand.hh"
#include "G4UImanager.hh"

#ifdef TOPAS_MT
#include "G4AutoLock.hh"
#include "G4Threading.hh"
#endif

#define G4MT_physicsVector ((G4VMPLsubInstanceManager.offset[g4vmplInstanceID]).physicsVector)

#include <typeinfo>

TsModularPhysicsList::TsModularPhysicsList(TsParameterManager* pM, TsExtensionManager* eM, TsGeometryManager* gM, TsVarianceManager* vM, G4String name)
	: G4VModularPhysicsList(), fPm(pM), fEm(eM), fGm(gM), fVm(vM), fName(name), fNumberBuilt(0), fTransportationOnly(false)
{
	if (fPm->ParameterExists("Ph/Verbosity"))
		SetVerboseLevel(fPm->GetIntegerParameter("Ph/Verbosity"));

	fPhysicsTable.insert(std::make_pair("tsphotoneutron", new Creator<TsPhotoNeutron>()));
	fPhysicsTable.insert(std::make_pair("g4h-chargeexchange", new Creator<G4ChargeExchangePhysics>()));
	fPhysicsTable.insert(std::make_pair("g4decay", new Creator<G4DecayPhysics>()));
	fPhysicsTable.insert(std::make_pair("g4em-dna", new Creator<G4EmDNAPhysics>()));
	fPhysicsTable.insert(std::make_pair("g4em-dna_opt1", new Creator<G4EmDNAPhysics_option1>()));
	fPhysicsTable.insert(std::make_pair("g4em-dna_opt2", new Creator<G4EmDNAPhysics_option2>()));
	fPhysicsTable.insert(std::make_pair("g4em-dna_opt3", new Creator<G4EmDNAPhysics_option3>()));
	fPhysicsTable.insert(std::make_pair("g4em-dna_opt4", new Creator<G4EmDNAPhysics_option4>()));
	fPhysicsTable.insert(std::make_pair("g4em-dna_opt5", new Creator<G4EmDNAPhysics_option5>()));
	fPhysicsTable.insert(std::make_pair("g4em-dna-chemistry", new Creator<G4EmDNAChemistry>()));
	fPhysicsTable.insert(std::make_pair("g4em-dna_opt6", new Creator<G4EmDNAPhysics_option6>()));
	fPhysicsTable.insert(std::make_pair("g4em-dna_opt7", new Creator<G4EmDNAPhysics_option7>()));
	fPhysicsTable.insert(std::make_pair("g4em-dna_opt8", new Creator<G4EmDNAPhysics_option8>()));
	fPhysicsTable.insert(std::make_pair("g4em-dna-stationary", new Creator<G4EmDNAPhysics_stationary>()));
	fPhysicsTable.insert(std::make_pair("g4em-dna-stationary_opt2", new Creator<G4EmDNAPhysics_stationary_option2>()));
	fPhysicsTable.insert(std::make_pair("g4em-dna-stationary_opt4", new Creator<G4EmDNAPhysics_stationary_option4>()));
	fPhysicsTable.insert(std::make_pair("g4em-dna-stationary_opt6", new Creator<G4EmDNAPhysics_stationary_option6>()));
	fPhysicsTable.insert(std::make_pair("g4em-dna-chemistry_opt1", new Creator<G4EmDNAChemistry_option1>()));
	fPhysicsTable.insert(std::make_pair("g4em-dna-activator", new Creator<G4EmDNAPhysicsActivator>()));
	fPhysicsTable.insert(std::make_pair("g4em-standard_GS", new Creator<G4EmStandardPhysicsGS>()));
	fPhysicsTable.insert(std::make_pair("g4em-standard_SS", new Creator<G4EmStandardPhysicsSS>()));
	fPhysicsTable.insert(std::make_pair("g4em-standard_WVI", new Creator<G4EmStandardPhysicsWVI>()));
	fPhysicsTable.insert(std::make_pair("g4h-phy_QGSP_BIC_AllHP", new Creator<G4HadronPhysicsQGSP_BIC_AllHP>()));
	fPhysicsTable.insert(std::make_pair("g4em-extra", new Creator<G4EmExtraPhysics>()));
	fPhysicsTable.insert(std::make_pair("g4em-livermore", new Creator<G4EmLivermorePhysics>()));
	fPhysicsTable.insert(std::make_pair("g4em-polarized", new Creator<G4EmLivermorePolarizedPhysics>()));
	fPhysicsTable.insert(std::make_pair("g4em-lowep", new Creator<G4EmLowEPPhysics>()));
	fPhysicsTable.insert(std::make_pair("g4em-penelope", new Creator<G4EmPenelopePhysics>()));
	fPhysicsTable.insert(std::make_pair("g4em-standard_opt0", new Creator<G4EmStandardPhysics>()));
	fPhysicsTable.insert(std::make_pair("g4em-standard_opt1", new Creator<G4EmStandardPhysics_option1>()));
	fPhysicsTable.insert(std::make_pair("g4em-standard_opt2", new Creator<G4EmStandardPhysics_option2>()));
	fPhysicsTable.insert(std::make_pair("g4em-standard_opt3", new Creator<G4EmStandardPhysics_option3>()));
	fPhysicsTable.insert(std::make_pair("g4em-standard_opt4", new Creator<G4EmStandardPhysics_option4>()));
	fPhysicsTable.insert(std::make_pair("g4h-elastic_D", new Creator<G4HadronDElasticPhysics>()));
	fPhysicsTable.insert(std::make_pair("g4h-elastic", new Creator<G4HadronElasticPhysics>()));
	fPhysicsTable.insert(std::make_pair("g4h-elastic_HP", new Creator<G4HadronElasticPhysicsHP>()));
	fPhysicsTable.insert(std::make_pair("g4h-elastic_LEND", new Creator<G4HadronElasticPhysicsLEND>()));
	fPhysicsTable.insert(std::make_pair("g4h-elastic_XS", new Creator<G4HadronElasticPhysicsXS>()));
	fPhysicsTable.insert(std::make_pair("g4h-elastic_H", new Creator<G4HadronHElasticPhysics>()));
	fPhysicsTable.insert(std::make_pair("g4h-inelastic_QBBC", new Creator<G4HadronInelasticQBBC>()));
	fPhysicsTable.insert(std::make_pair("g4h-phy_FTFP_BERT", new Creator<G4HadronPhysicsFTFP_BERT>()));
	fPhysicsTable.insert(std::make_pair("g4h-phy_FTFP_BERT_HP", new Creator<G4HadronPhysicsFTFP_BERT_HP>()));
	fPhysicsTable.insert(std::make_pair("g4h-phy_FTFP_BERT_TRV", new Creator<G4HadronPhysicsFTFP_BERT_TRV>()));
	fPhysicsTable.insert(std::make_pair("g4h-phy_FTF_BIC", new Creator<G4HadronPhysicsFTF_BIC>()));
	fPhysicsTable.insert(std::make_pair("g4h-phy_QGSP_BERT", new Creator<G4HadronPhysicsQGSP_BERT>()));
	fPhysicsTable.insert(std::make_pair("g4h-phy_QGSP_BERT_HP", new Creator<G4HadronPhysicsQGSP_BERT_HP>()));
	fPhysicsTable.insert(std::make_pair("g4h-phy_QGSP_BIC", new Creator<G4HadronPhysicsQGSP_BIC>()));
	fPhysicsTable.insert(std::make_pair("g4h-phy_QGSP_BIC_HP", new Creator<G4HadronPhysicsQGSP_BIC_HP>()));
	fPhysicsTable.insert(std::make_pair("g4h-phy_QGSP_FTFP_BERT", new Creator<G4HadronPhysicsQGSP_FTFP_BERT>()));
	fPhysicsTable.insert(std::make_pair("g4h-phy_QGS_BIC", new Creator<G4HadronPhysicsQGS_BIC>()));
	fPhysicsTable.insert(std::make_pair("g4h-phy_Shielding", new Creator<G4HadronPhysicsShielding>()));
	fPhysicsTable.insert(std::make_pair("g4ion-binarycascade", new Creator<G4IonBinaryCascadePhysics>()));
	fPhysicsTable.insert(std::make_pair("g4ion-inclxx", new Creator<G4IonINCLXXPhysics>()));
	fPhysicsTable.insert(std::make_pair("g4ion", new Creator<G4IonPhysics>()));
	fPhysicsTable.insert(std::make_pair("g4ion-QMD", new Creator<G4IonQMDPhysics>()));
	fPhysicsTable.insert(std::make_pair("g4n-trackingcut", new Creator<G4NeutronTrackingCut>()));
	fPhysicsTable.insert(std::make_pair("g4optical", new Creator<G4OpticalPhysics>()));
	fPhysicsTable.insert(std::make_pair("g4radioactivedecay", new Creator<G4RadioactiveDecayPhysics>()));
	fPhysicsTable.insert(std::make_pair("g4stopping", new Creator<G4StoppingPhysics>()));

	if (fPm->ParameterExists("Ph/SetEmParametersInTsModularPhysicsList") && fPm->GetBooleanParameter("Ph/SetEmParametersInTsModularPhysicsList"))
		SetEmParameters();
}

void TsModularPhysicsList::AddModule(const G4String& name)
{
	G4String nameLower = name;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(nameLower);
#else
	nameLower.toLower();
#endif

	if (nameLower == "transportation_only")
		fTransportationOnly = true;

	if (nameLower == "g4em-dna-chemistry")
		fPm->SetNeedsChemistry();

	std::map<G4String, VPhysicsCreator*>::const_iterator iter;
	if (fTransportationOnly)
		iter = fPhysicsTable.find("g4em-standard_opt0");
	else
		iter = LocatePhysicsModel(name, true);

	if (iter != fPhysicsTable.end()) {
		if (fTransportationOnly && fNumberBuilt > 0) {
			G4cerr << "Topas is exiting due to a serious error in physics setup." << G4endl;
			G4cerr << "When Transportation_Only is in your Module parameter, no other modules are allowed." << G4endl;
			fPm->AbortSession(1);
		}
        G4VPhysicsConstructor* ph = (*iter->second)();
        ph->SetVerboseLevel(GetVerboseLevel());
		RegisterPhysics(ph);
		ActiveG4EmModelPerRegion(nameLower);
		fNumberBuilt++;
	}
	else {
		G4cerr << "Topas is exiting due to a serious error in physics setup." << G4endl;
		G4cerr << "Attempt to register unknown module, " << name << ", in your physics list." << G4endl;
		fPm->AbortSession(1);
	}
}

void TsModularPhysicsList::ConstructProcess()
{
#ifdef TOPAS_MT
	static G4ThreadLocal G4bool fTransportationAdded = false;
#else
	static G4bool fTransportationAdded = false;
#endif

	if (fNumberBuilt == 0) {
		G4cerr << "Topas is exiting due to a serious error in physics setup." << G4endl;
		G4cerr << "Geant4_Modular physics list requires at least one value in: " << GetFullParmName("Modules") << G4endl;
		fPm->AbortSession(1);
	}

	if (!fTransportationAdded) {
		AddTransportationAndParallelScoring();
		fTransportationAdded = true;
	}

	G4PhysConstVector::iterator itr;
	for (itr = G4MT_physicsVector->begin(); itr != G4MT_physicsVector->end(); ++itr) {
        if (GetVerboseLevel() > 0)
            G4cerr << " -#- CONSTRUCTING PROCESS -#- " << (*itr)->GetPhysicsName() << G4endl;
		if (!fTransportationOnly)
			(*itr)->ConstructProcess();
	}

	if (fPm->UseVarianceReduction())
		AddBiasingProcess();
}

void TsModularPhysicsList::ConstructParticle()
{
	// Make sure all needed particles are present
	G4BosonConstructor pBosonConstructor;
	pBosonConstructor.ConstructParticle();

	G4LeptonConstructor pLeptonConstructor;
	pLeptonConstructor.ConstructParticle();

	G4MesonConstructor pMesonConstructor;
	pMesonConstructor.ConstructParticle();

	G4BaryonConstructor pBaryonConstructor;
	pBaryonConstructor.ConstructParticle();

	G4IonConstructor pIonConstructor;
	pIonConstructor.ConstructParticle();

	G4ShortLivedConstructor pShortLivedConstructor;
	pShortLivedConstructor.ConstructParticle();

	G4VModularPhysicsList::ConstructParticle();
}

void TsModularPhysicsList::AddTransportationAndParallelScoring()
{
	G4VProcess* transportationProcess;
	if (fGm->HaveParallelComponentsThatAreNotGroups()) {
		// Chemistry does not support parallel world physics
		if (fPm->NeedsChemistry()) {
			G4cerr << "Topas is exiting due to a serious error in physics setup." << G4endl;
			G4cerr << "The geometry is asking for creation of a parallel world" << G4endl;
			G4cerr << "(either because some component explicitly has IsParallel set true, or because" << G4endl;
			G4cerr << "some scorer is asking for different geometry divisions than its component)." << G4endl;
			G4cerr << "However, the physics modules includes Chemistry." << G4endl;
			G4cerr << "Geant4 Chemistry does not currently support parallel world physics." << G4endl;
			fPm->AbortSession(1);
		}

		transportationProcess = new G4CoupledTransportation(0);
	}
	else if (fPm->GetBooleanParameter("Ge/ForceUseOfCoupledTransportation"))
		transportationProcess = new G4CoupledTransportation(0);
	else
		transportationProcess = new G4Transportation(0);

	G4ParticleDefinition* particle;
	G4ProcessManager* pmanager;
	G4ParticleTable::G4PTblDicIterator* theParticleIterator = GetParticleIterator();
	theParticleIterator->reset();
	while ((*theParticleIterator)()) {
		particle = theParticleIterator->value();
		if (particle->GetParticleType() == "Molecule")
			continue;

		pmanager = particle->GetProcessManager();
		pmanager->AddProcess(transportationProcess);
		pmanager->SetProcessOrderingToFirst(transportationProcess, idxAlongStep);
		pmanager->SetProcessOrderingToFirst(transportationProcess, idxPostStep);
	}

	// Skip parallel world physics setup if the only components in parallel worlds are Groups
	if (fGm->HaveParallelComponentsThatAreNotGroups()) {
		// Check if layered mass geometry is to be used
		G4String* layeredMassWorlds = 0;
		G4int layeredMassWorlds_size = 0;
		if (fPm->ParameterExists(GetFullParmName("LayeredMassGeometryWorlds"))) {
			layeredMassWorlds = fPm->GetStringVector(GetFullParmName("LayeredMassGeometryWorlds"));
			layeredMassWorlds_size = fPm->GetVectorLength(GetFullParmName("LayeredMassGeometryWorlds"));
		}

		// Check that all of the parallel worlds that have material are in layeredMassWorlds;
		std::set<G4String>* parallelWorldsWithMaterial = fGm->GetParallelWorldsWithMaterial();
		std::set<G4String>::iterator it;
		for (it = parallelWorldsWithMaterial->begin(); it != parallelWorldsWithMaterial->end(); ++it) {
			G4bool found = false;
			for (G4int iLayeredMassWorld = 0; iLayeredMassWorld < layeredMassWorlds_size && !found; ++iLayeredMassWorld)
				if (*it == layeredMassWorlds[iLayeredMassWorld])
					found = true;

			if (!found) {
				G4cerr << "Topas is exiting due to a serious error in physics setup." << G4endl;
				G4cerr << "Parallel world " << *it << " has material," << G4endl;
				G4cerr << "but this world not specified in the parameter " << GetFullParmName("LayeredMassGeometryWorlds") << G4endl;
				fPm->AbortSession(1);
			}
		}

		// Add all the worlds that are not named in Ge/LayeredMassGeometryWorlds
		for (G4int iParallelWorld = 0; iParallelWorld < fGm->GetNumberOfParallelWorld(); iParallelWorld++) {
			G4String parallelWorldName = fGm->GetParallelWorld(iParallelWorld)->GetName();

			G4bool found = false;
			for (G4int iLayeredMassWorld = 0; iLayeredMassWorld < layeredMassWorlds_size && !found; ++iLayeredMassWorld)
				if (parallelWorldName == layeredMassWorlds[iLayeredMassWorld])
					found = true;

			if (!found)
				SetUpParallelWorldProcess(parallelWorldName, false);
		}

		// Add all the worlds that are named in Ge/LayeredMassGeometryWorlds.
		for (G4int iLayeredMassWorld = 0; iLayeredMassWorld < layeredMassWorlds_size; ++iLayeredMassWorld) {

			// Make sure this is a known world
			G4bool found = false;
			for (G4int iParallelWorld = 0; iParallelWorld < fGm->GetNumberOfParallelWorld() && !found; iParallelWorld++)
				if (fGm->GetParallelWorld(iParallelWorld)->GetName() == layeredMassWorlds[iLayeredMassWorld])
					found = true;

			if (!found) {
				G4cerr << "Topas is exiting due to a serious error in physics setup." << G4endl;
				G4cerr << GetFullParmName("LayeredMassGeometryWorlds") << G4endl;
				G4cerr << "includes a world that does not exist: " << layeredMassWorlds[iLayeredMassWorld] << G4endl;
				G4cerr << "The only known worlds are:" << G4endl;
				for (G4int iParallelWorld = 0; iParallelWorld < fGm->GetNumberOfParallelWorld(); iParallelWorld++)
					G4cerr << fGm->GetParallelWorld(iParallelWorld)->GetName() << G4endl;
				fPm->AbortSession(1);
			}

			SetUpParallelWorldProcess(layeredMassWorlds[iLayeredMassWorld], true);
		}
	}
}

void TsModularPhysicsList::SetUpParallelWorldProcess(G4String parallelWorldName, G4bool useLMG)
{
	G4ParallelWorldProcess* theParallelWorldProcess = new G4ParallelWorldProcess(parallelWorldName);
	theParallelWorldProcess->SetParallelWorld(parallelWorldName);

	if (useLMG)
		theParallelWorldProcess->SetLayeredMaterialFlag();

	G4ParticleTable::G4PTblDicIterator* theParticleIterator = GetParticleIterator();
	theParticleIterator->reset();

	G4ParticleDefinition* particle;
	G4ProcessManager* pmanager;

	while ((*theParticleIterator)()) {
		particle = theParticleIterator->value();
		pmanager = particle->GetProcessManager();
		pmanager->AddProcess(theParallelWorldProcess);
		if (theParallelWorldProcess->IsAtRestRequired(particle))
		{
			pmanager->SetProcessOrdering(theParallelWorldProcess, idxAtRest, 9999);
		}
		pmanager->SetProcessOrdering(theParallelWorldProcess, idxAlongStep, 1);
		pmanager->SetProcessOrdering(theParallelWorldProcess, idxPostStep, 9999);
	}
}

void TsModularPhysicsList::SetCuts()
{
	if (fPm->IsFindingSeed())
		return;

	SetDefaultCutValue(0.05 * mm);
	SetCutsWithDefault();

	if (fPm->ParameterExists(GetFullParmName("CutForAllParticles"))) {
		SetDefaultCutValue(fPm->GetDoubleParameter(GetFullParmName("CutForAllParticles"), "Length"));
		SetCutsWithDefault();
	}

	if (fPm->ParameterExists(GetFullParmName("CutForGamma")))
		SetCutValue(fPm->GetDoubleParameter(GetFullParmName("CutForGamma"), "Length"),
					"gamma", "DefaultRegionForTheWorld");

	if (fPm->ParameterExists(GetFullParmName("CutForElectron")))
		SetCutValue(fPm->GetDoubleParameter(GetFullParmName("CutForElectron"), "Length"),
					"e-", "DefaultRegionForTheWorld");

	if (fPm->ParameterExists(GetFullParmName("CutForPositron")))
		SetCutValue(fPm->GetDoubleParameter(GetFullParmName("CutForPositron"), "Length"),
					"e+", "DefaultRegionForTheWorld");

	if (fPm->ParameterExists(GetFullParmName("CutForProton")))
		SetCutValue(fPm->GetDoubleParameter(GetFullParmName("CutForProton"), "Length"),
					"proton", "DefaultRegionForTheWorld");

	if (fPm->ParameterExists(GetFullParmName("CutForAlpha")))
		SetCutValue(fPm->GetDoubleParameter(GetFullParmName("CutForAlpha"), "Length"),
					"alpha", "DefaultRegionForTheWorld");

	if (fPm->ParameterExists(GetFullParmName("CutForDeuteron")))
		SetCutValue(fPm->GetDoubleParameter(GetFullParmName("CutForDeuteron"), "Length"),
					"deuteron", "DefaultRegionForTheWorld");

	if (fPm->ParameterExists(GetFullParmName("CutForTriton")))
		SetCutValue(fPm->GetDoubleParameter(GetFullParmName("CutForTriton"), "Length"),
					"triton", "DefaultRegionForTheWorld");

	std::vector<G4String>* namesOfCompWithRegion = new std::vector<G4String>;
	fPm->GetParameterNamesBracketedBy("Ge", "AssignToRegionNamed", namesOfCompWithRegion);
	G4int numberOfRegions = namesOfCompWithRegion->size();
	G4bool found;
	if (numberOfRegions > 0) {
		for (int i = 0; i < numberOfRegions; i++) {
			G4String aRegionName = fPm->GetStringParameter((*namesOfCompWithRegion)[i]);
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(aRegionName);
#else
			aRegionName.toLower();
#endif
			found = false;
			// Look for repeated region names
			for (int j = i + 1; j < numberOfRegions; j++) {
				G4String tempRegName = (*namesOfCompWithRegion)[j];
#if GEANT4_VERSION_MAJOR >= 11
				G4StrUtil::to_lower(tempRegName);
#else
				tempRegName.toLower();
#endif
				if (aRegionName == tempRegName)
					found = true;
			}

			if (!found) {
				if (aRegionName == "defaultregionfortheworld")
					aRegionName = "DefaultRegionForTheWorld";

				G4ProductionCuts* regionCuts = new G4ProductionCuts();

				G4String parmCut = GetFullParmName("ForRegion") + "/" + aRegionName + "/";
#if GEANT4_VERSION_MAJOR >= 11
				G4StrUtil::to_lower(parmCut);
#else
				parmCut.toLower();
#endif

				G4double cutForAllParticles = GetDefaultCutValue();
				if (fPm->ParameterExists(parmCut + "CutForAllParticles"))
					cutForAllParticles = fPm->GetDoubleParameter(parmCut + "CutForAllParticles", "Length");

				regionCuts->SetProductionCut(cutForAllParticles, idxG4GammaCut);
				regionCuts->SetProductionCut(cutForAllParticles, idxG4ElectronCut);
				regionCuts->SetProductionCut(cutForAllParticles, idxG4PositronCut);
				regionCuts->SetProductionCut(cutForAllParticles, idxG4ProtonCut);

				if (fPm->ParameterExists(parmCut + "CutForGamma"))
					regionCuts->SetProductionCut(fPm->GetDoubleParameter(parmCut + "CutForGamma", "Length"),
												 idxG4GammaCut);

				if (fPm->ParameterExists(parmCut + "CutForElectron"))
					regionCuts->SetProductionCut(fPm->GetDoubleParameter(parmCut + "CutForElectron", "Length"),
												 idxG4ElectronCut);

				if (fPm->ParameterExists(parmCut + "CutForPositron"))
					regionCuts->SetProductionCut(fPm->GetDoubleParameter(parmCut + "CutForPositron", "Length"),
												 idxG4PositronCut);

				if (fPm->ParameterExists(parmCut + "CutForProton"))
					regionCuts->SetProductionCut(fPm->GetDoubleParameter(parmCut + "CutForProton", "Length"),
												 idxG4ProtonCut);

				G4Region* aRegion = (G4RegionStore::GetInstance())->GetRegion(aRegionName);
				aRegion->SetProductionCuts(regionCuts);

				if (verboseLevel > 0) {
					G4cout << "Assigned production cut for region named:" << G4endl;
					G4cout << "  " << aRegion->GetName() << G4endl;
					G4cout << "      gamma   : " << regionCuts->GetProductionCut("gamma") << G4endl;
					G4cout << "      electron: " << regionCuts->GetProductionCut("e-") << G4endl;
					G4cout << "      positron: " << regionCuts->GetProductionCut("e+") << G4endl;
					G4cout << "      proton  : " << regionCuts->GetProductionCut("proton") << G4endl;
				}
			}
		}
	}

	if (verboseLevel > 0) DumpCutValuesTable();
}

void TsModularPhysicsList::AddBiasingProcess()
{
	G4int index = -1;
	if (fVm->BiasingProcessExists("geometricalparticlesplit", index)) {
		G4String biasWorldName = dynamic_cast<TsGeometricalParticleSplit*>(fVm->GetBiasingProcessFromList(index))->GetName();

		biasWorldName = fPm->GetStringParameter("Vr/" + biasWorldName + "/Component");
		if (!fGm->GetComponent(biasWorldName)->IsParallel())
			biasWorldName = "World";

		for (int i = 0; i < int(fProtonGeomSamplers.size()); i++) {
			if (!G4Threading::IsWorkerThread()) {
				fProtonGeomSamplers[i]->PrepareSplitSampling(TsIStore::GetInstance(biasWorldName));
				fProtonGeomSamplers[i]->Configure();
			}
			fProtonGeomSamplers[i]->AddProcess();
		}
	}
	else {
		fVm->AddBiasingProcess();
	}

	return;
}

void TsModularPhysicsList::ActiveG4DNAPerRegion(G4String moduleName)
{
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(moduleName);
#else
	moduleName.toLower();
#endif
	if (moduleName.substr(0, 4) != "g4em")
		return;

	if (fPm->ParameterExists(GetFullParmName("ActiveG4DNAInRegionsNamed"))) {
		if (moduleName.substr(0, 8) == "g4em-dna") {
			G4cerr << "Topas is exiting due to a serious error in modular physics setup." << G4endl;
			G4cerr << "Parameter name: " << GetFullParmName("ActiveG4DNAInRegionsNamed") << G4endl;
			G4cerr << "Cannot be used if g4em-dna or g4em-dna_optN is used in the physics modules" << G4endl;
			fPm->AbortSession(1);
		}

		G4String* regionsNames = fPm->GetStringVector(GetFullParmName("ActiveG4DNAInRegionsNamed"));
		G4int nbOfRegions = fPm->GetVectorLength(GetFullParmName("ActiveG4DNAInRegionsNamed"));
		G4String modelG4DNA = "Opt0";
		if (fPm->ParameterExists(GetFullParmName("DNAModelName")))
			modelG4DNA = fPm->GetStringParameter(GetFullParmName("DNAModelName"));

		for (int i = 0; i < nbOfRegions; i++)
			G4EmParameters::Instance()->AddDNA(regionsNames[i], modelG4DNA);
	}
}

void TsModularPhysicsList::ActiveG4EmModelPerRegion(G4String moduleName)
{
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(moduleName);
#else
	moduleName.toLower();
#endif
	if (moduleName.find("em") == std::string::npos ||
		moduleName.find("chemistry") != std::string::npos) {
		return;
	}

	std::vector<G4String>* namesOfCompWithRegion = new std::vector<G4String>;
	fPm->GetParameterNamesBracketedBy("Ge", "AssignToRegionNamed", namesOfCompWithRegion);
	G4int numberOfRegions = namesOfCompWithRegion->size();
	G4bool activated = false;
	G4bool found;
	if (numberOfRegions > 0) {
		std::map<G4String, VPhysicsCreator*>::const_iterator iter;

		for (G4int i = 0; i < numberOfRegions; i++) {
			G4String aRegionName = fPm->GetStringParameter((*namesOfCompWithRegion)[i]);
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(aRegionName);
#else
			aRegionName.toLower();
#endif
			found = false;

			for (G4int j = i + 1; j < numberOfRegions; j++) {
				G4String tempRegName = (*namesOfCompWithRegion)[j];
#if GEANT4_VERSION_MAJOR >= 11
				G4StrUtil::to_lower(tempRegName);
#else
				tempRegName.toLower();
#endif
				if (aRegionName == tempRegName)
					found = true;
			}

			if (!found) {
				if (aRegionName == "defaultregionfortheworld")
					aRegionName = "DefaultRegionForTheWorld";

				G4String emModel = GetFullParmName("ForRegion") + "/" + aRegionName + "/";
				if (fPm->ParameterExists(emModel + "ActiveG4EmModelFromModule")) {
					emModel = fPm->GetStringParameter(emModel + "ActiveG4EmModelFromModule");
					G4String emModelLower = emModel;
#if GEANT4_VERSION_MAJOR >= 11
					G4StrUtil::to_lower(emModelLower);
#else
					emModelLower.toLower();
#endif

					if (emModel.find("dna") == std::string::npos && emModel.find("DNA") == std::string::npos) {
						// The G4EmModelActivator for non-DNA physics is called within the
						// ConstructProcess() methods for those, see e.g. G4EmPenelopePhysics.cc.
						// So we have to register the physics if it isn't in the physicsVector yet,
						// and add this physics name to the region through the G4EmParameters.
						iter = LocatePhysicsModel(emModel, true);
						if (iter == fPhysicsTable.end()) {
							G4cerr << "Topas is exiting due to a serious error in modular physics setup." << G4endl;
							G4cerr << "Parameter name: " << GetFullParmName("ForRegion") + "/" + aRegionName + "/" + "ActiveG4EmModelFromModule" << G4endl;
							G4cerr << "Physics module: " << emModel << " was not found." << G4endl;
							fPm->AbortSession(1);
						}
						G4VPhysicsConstructor* ph = (*iter->second)();
                        ph->SetVerboseLevel(GetVerboseLevel());
						if (!IsPhysicsRegistered(G4MT_physicsVector, ph)) {
							// Need to change physics type to force it onto the stack
							if (G4MT_physicsVector->cend() != std::find_if(G4MT_physicsVector->cbegin(),
																		   G4MT_physicsVector->cend(),
																		   [](const G4VPhysicsConstructor* const ph) { return ph->GetPhysicsType() == bElectromagnetic; }))
							{
								G4cout << G4endl;
								G4cout << "WARNING in TsModularPhysicsList::ActiveG4EmModelPerRegion" << G4endl;
								G4cout << "We are registering an EM physics list to activate in region " + aRegionName << G4endl;
								G4cout << "However, another (general) EM physics list is already registered." << G4endl;
								G4cout << "Verify that the physics tables are correct and are not overlapping!" << G4endl;
								G4cout << G4endl;
							}
							auto phTypeStr = G4UIcommand::ConvertToString(ph->GetPhysicsType()) + G4UIcommand::ConvertToString(ph->GetPhysicsType());
							ph->SetPhysicsType(G4UIcommand::ConvertToInt(phTypeStr.c_str()));
							RegisterPhysics(ph);
						}
						else {
							delete ph;
                            ph = nullptr;
						}

						emModel = (*iter->second).GetPhysicsName();
						G4EmParameters::Instance()->AddPhysics(aRegionName, emModel);
					}
					else { // emModel contains DNA
						if (!activated) {
							// Find and activator for DNA Physics
							G4String activatorName = "g4em-dna-activator"; // fall back, throw warning if used
							if (fPm->ParameterExists(GetFullParmName("EmDNAPhysicsActivator")))
								activatorName = fPm->GetStringParameter(GetFullParmName("EmDNAPhysicsActivator"));

							G4String activatorNameLower = activatorName;
#if GEANT4_VERSION_MAJOR >= 11
							G4StrUtil::to_lower(activatorNameLower);
#else
							activatorNameLower.toLower();
#endif

							if (activatorNameLower.find("activator") == std::string::npos) {
								G4cout << G4endl;
								G4cout << "WARNING in TsModularPhysicsList::ActiveG4EmModelPerRegion" << G4endl;
								G4cout << "Topas requires a physics activator (e.g. G4EmDNAPhysicsActivator)" << G4endl;
								G4cout << "to activate different EM DNA models per region." << G4endl;
								G4cout << GetFullParmName("EmDNAPhysicsActivator") << " is set to " << activatorName << G4endl;
								G4cout << "This does not look like an activator, but we will use it anyways -- VERIFY YOUR RESULTS" << G4endl;
								G4cout << G4endl;
							}

							iter = LocatePhysicsModel(activatorName);

							if (iter == fPhysicsTable.end()) {
								G4cout << G4endl;
								G4cout << "WARNING in TsModularPhysicsList::ActiveG4EmModelPerRegion" << G4endl;
								G4cout << "Topas could not find the EM DNA physics activator module: " << activatorName << G4endl;
								G4cout << "Specified by: " << GetFullParmName("EmDNAPhysicsActivator") << G4endl;
								G4cout << "We will use G4EmDNAPhysicsActivator instead." << G4endl;
								G4cout << "Beware, this requires to use built-in Geant4 EM DNA modules with specific names:" << G4endl;
								G4cout << "(*) DNA_Opt2 to use G4EmDNAPhysics_option2," << G4endl;
								G4cout << "(*) DNA_Opt4 to use G4EmDNAPhysics_option4, etc." << G4endl;
								G4cout << "For a full list, we refer to the implementation at G4EmDNAPhysicsActivator.cc" << G4endl;
								G4cout << G4endl;
								iter = LocatePhysicsModel("g4em-dna-activator");
							}
							if (iter == fPhysicsTable.end()) {
								G4cerr << "Topas is exiting due to a serious error in modular physics setup." << G4endl;
								G4cerr << "Could not locate: " << GetFullParmName("EmDNAPhysicsActivator") + " = " << activatorName << G4endl;
								G4cerr << "Nor the backup: g4em-dna-activator (G4EmDNAPhysicsActivator)" << G4endl;
								G4cerr << "Something is really wrong." << G4endl;
								fPm->AbortSession(1);
							}

							G4VPhysicsConstructor* ph = (*iter->second)();
							ph->SetVerboseLevel(GetVerboseLevel());
							RegisterPhysics(ph);
							activated = true;
							if (GetVerboseLevel() > 0) {
								G4cout << "Topas registered " << activatorName << " as EM DNA physics activator." << G4endl;
							}
						}

						iter = LocatePhysicsModel(emModel);

						if (iter == fPhysicsTable.end()) {
							G4cerr << "Topas is exiting due to a serious error in modular physics setup." << G4endl;
							G4cerr << "Parameter name: " << GetFullParmName("ForRegion") + "/" + aRegionName + "/" + "ActiveG4EmModelFromModule" << G4endl;
							G4cerr << "Physics module: " << emModel << " was not found." << G4endl;
							fPm->AbortSession(1);
						}

						emModel = (*iter->second).GetPhysicsName();
						G4EmParameters::Instance()->AddDNA(aRegionName, emModel);
					}
				}
			}
		}
	}
}

void TsModularPhysicsList::SetEmParameters()
{
	if (fPm->ParameterExists(GetFullParmName("EMRangeMin")))
		G4EmParameters::Instance()->SetMinEnergy(fPm->GetDoubleParameter(GetFullParmName("EMRangeMin"), "Energy"));

	if (fPm->ParameterExists(GetFullParmName("EMRangeMax")))
		G4EmParameters::Instance()->SetMaxEnergy(fPm->GetDoubleParameter(GetFullParmName("EMRangeMax"), "Energy"));

#if GEANT4_VERSION_MAJOR >= 11
	if (fPm->ParameterExists(GetFullParmName("EMBins")) && !fPm->ParameterExists(GetFullParmName("EMBinsPerDecade")))
		G4EmParameters::Instance()->SetNumberOfBinsPerDecade(fPm->GetIntegerParameter(GetFullParmName("EMBins")));

	else if (!fPm->ParameterExists(GetFullParmName("EMBins")) && fPm->ParameterExists(GetFullParmName("EMBinsPerDecade")))
		G4EmParameters::Instance()->SetNumberOfBinsPerDecade(fPm->GetIntegerParameter(GetFullParmName("EMBinsPerDecade")));

	else if (fPm->ParameterExists(GetFullParmName("EMBins")) && fPm->ParameterExists(GetFullParmName("EMBinsPerDecade"))) {
		G4cerr << "Topas is exiting due to a serious error in physics setup." << G4endl;
		G4cerr << GetFullParmName("EMBins") << " and " << GetFullParmName("EMBinsPerDecade") << "are not compatible at the same time." << G4endl;
		G4cerr << "Remove either of them and re-run Topas." << G4endl;
		fPm->AbortSession(1);
	}
#else
	if (fPm->ParameterExists(GetFullParmName("EMBins")))
		G4EmParameters::Instance()->SetNumberOfBins(fPm->GetIntegerParameter(GetFullParmName("EMBins")));

	if (fPm->ParameterExists(GetFullParmName("EMBinsPerDecade")))
		G4EmParameters::Instance()->SetNumberOfBinsPerDecade(fPm->GetIntegerParameter(GetFullParmName("EMBinsPerDecade")));
#endif

	if (fPm->ParameterExists(GetFullParmName("dEdXBins"))) {
		G4cerr << "Topas is exiting due to a serious error in physics setup." << G4endl;
		G4cerr << GetFullParmName("dEdXBins") << " is no longer supported in Geant4.10" << G4endl;
		fPm->AbortSession(1);
	}

	if (fPm->ParameterExists(GetFullParmName("LambdaBins"))) {
		G4cerr << "Topas is exiting due to a serious error in physics setup." << G4endl;
		G4cerr << GetFullParmName("LambdaBins") << " is no longer supported in Geant4.10" << G4endl;
		fPm->AbortSession(1);
	}

	if (fPm->ParameterExists(GetFullParmName("MottCorrection")))
		G4EmParameters::Instance()->SetUseMottCorrection(fPm->GetBooleanParameter(GetFullParmName("MottCorrection")));

	if (fPm->ParameterExists(GetFullParmName("Fluorescence")))
		G4EmParameters::Instance()->SetFluo(fPm->GetBooleanParameter(GetFullParmName("Fluorescence")));

	if (fPm->ParameterExists(GetFullParmName("Auger")))
		G4EmParameters::Instance()->SetAuger(fPm->GetBooleanParameter(GetFullParmName("Auger")));

	if (fPm->ParameterExists(GetFullParmName("AugerCascade")))
		G4EmParameters::Instance()->SetAugerCascade(fPm->GetBooleanParameter(GetFullParmName("AugerCascade")));

	// Begin of temporal pameters
	if (fPm->ParameterExists(GetFullParmName("ICRU90")))
		G4EmParameters::Instance()->SetUseICRU90Data(fPm->GetBooleanParameter(GetFullParmName("ICRU90")));

	if (fPm->ParameterExists(GetFullParmName("EMRoverRange"))) {
		G4double roverRange = fPm->GetUnitlessParameter(GetFullParmName("EMRoverRange"));
		G4double finalRange = fPm->GetDoubleParameter(GetFullParmName("EMFinalRange"), "Length");
		G4EmParameters::Instance()->SetStepFunction(roverRange, finalRange);
	}

	if (fPm->ParameterExists(GetFullParmName("LowestElectronEnergy")))
		G4EmParameters::Instance()->SetLowestElectronEnergy(fPm->GetDoubleParameter(GetFullParmName("LowestElectronEnergy"), "Energy"));

	if (fPm->ParameterExists(GetFullParmName("MSCGeometryFactor")))
		G4EmParameters::Instance()->SetMscGeomFactor(fPm->GetUnitlessParameter(GetFullParmName("MSCGeometryFactor")));

	if (fPm->ParameterExists(GetFullParmName("MSCSkinFactor")))
		G4EmParameters::Instance()->SetMscSkin(fPm->GetIntegerParameter(GetFullParmName("MSCSkinFactor")));

	if (fPm->ParameterExists(GetFullParmName("MSCRangeFactor")))
		G4EmParameters::Instance()->SetMscRangeFactor(fPm->GetUnitlessParameter(GetFullParmName("MSCRangeFactor")));

	if (fPm->ParameterExists(GetFullParmName("MSCSafetyFactor")))
		G4EmParameters::Instance()->SetMscSafetyFactor(fPm->GetUnitlessParameter(GetFullParmName("MSCSafetyFactor")));

	if (fPm->ParameterExists(GetFullParmName("MSCStepLimitType"))) {
		G4String mscStepLimitType = fPm->GetStringParameter(GetFullParmName("MSCStepLimitType"));
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(mscStepLimitType);
#else
		mscStepLimitType.toLower();
#endif
		if (mscStepLimitType == "safety") {
			G4EmParameters::Instance()->SetMscStepLimitType(fUseSafety);
		}
		else if (mscStepLimitType == "safetyplus") {
			G4EmParameters::Instance()->SetMscStepLimitType(fUseSafetyPlus);
		}
		else if (mscStepLimitType == "distancetoboundary") {
			G4EmParameters::Instance()->SetMscStepLimitType(fUseDistanceToBoundary);
		}
		else {
			G4cerr << "Topas is exiting due to a serious error in physics setup." << G4endl;
			G4cerr << GetFullParmName("MSCStepLimitType") << " refers to an unknown type of step limit for multiplescattering" << G4endl;
			fPm->AbortSession(1);
		}
	}
	// end of temporal parameters

	if (fPm->ParameterExists(GetFullParmName("SolvatedElectronThermalizationModel"))) {
		G4String eaqModel = fPm->GetStringParameter(GetFullParmName("SolvatedElectronThermalizationModel"));

#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(eaqModel);
#else
		eaqModel.toLower();
#endif
		if (eaqModel == "ritchie") {
			G4EmParameters::Instance()->SetDNAeSolvationSubType(fRitchie1994eSolvation);
		}
		else if (eaqModel == "terrisol") {
			G4EmParameters::Instance()->SetDNAeSolvationSubType(fTerrisol1990eSolvation);
		}
		else if (eaqModel == "meesungnoen") {
			G4EmParameters::Instance()->SetDNAeSolvationSubType(fMeesungnoen2002eSolvation);
		}
		else if (eaqModel == "meesungnoensolid") {
			G4EmParameters::Instance()->SetDNAeSolvationSubType(fMeesungnoensolid2002eSolvation);
		}
		else if (eaqModel == "kreipl") {
			G4EmParameters::Instance()->SetDNAeSolvationSubType(fKreipl2009eSolvation);
		}
		else {
			G4cerr << "Topas is exiting due to a serious error in physics setup." << G4endl;
			G4cerr << GetFullParmName("SolvatedElectronThermalizationModel") << " refers to an unknown model" << G4endl;
			fPm->AbortSession(1);
		}
		G4cout << "Solvation model selected: " << G4EmParameters::Instance()->DNAeSolvationSubType() << G4endl;
		G4cout << "   1. Ritchie." << G4endl;
		G4cout << "   2. Terrisol." << G4endl;
		G4cout << "   3. Meesungnoen." << G4endl;
		G4cout << "   4. Kreipl." << G4endl;
		G4cout << "   5. Meesugnoen solid." << G4endl;
	}

	if (fPm->ParameterExists(GetFullParmName("DeexcitationIgnoreCut")))
		G4EmParameters::Instance()->SetDeexcitationIgnoreCut(fPm->GetBooleanParameter(GetFullParmName("DeexcitationIgnoreCut")));

	if (fPm->ParameterExists(GetFullParmName("PIXE")))
		G4EmParameters::Instance()->SetPixe(fPm->GetBooleanParameter(GetFullParmName("PIXE")));

	if (verboseLevel > 0) G4EmParameters::Instance()->Dump();
}

G4String TsModularPhysicsList::GetFullParmName(const char* parmName)
{
	G4String fullName = "Ph/" + fName + "/" + parmName;
	return fullName;
}

std::map<G4String, VPhysicsCreator*>::const_iterator TsModularPhysicsList::LocatePhysicsModel(G4String model, G4bool allow_extensions)
{
	std::map<G4String, VPhysicsCreator*>::const_iterator it;

	G4String nameLower = model;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(nameLower);
#else
	nameLower.toLower();
#endif

	it = fPhysicsTable.find(model);
	if (it != fPhysicsTable.cend())
		return it;

	if (allow_extensions) {
		VPhysicsCreator* physicsCreator = fEm->InstantiatePhysicsModule(fPm, nameLower);
		if (physicsCreator != nullptr) {
			fPhysicsTable.emplace(std::make_pair(model, physicsCreator));
			it = fPhysicsTable.find(model);
			return it;
		}
	}

	return fPhysicsTable.cend();
}

G4bool TsModularPhysicsList::IsPhysicsRegistered(const std::vector<G4VPhysicsConstructor*>* const physicsVector, G4VPhysicsConstructor* physics) const
{
	if (physics == nullptr)
		return false;
	for (auto it = physicsVector->cbegin(); it != physicsVector->cend(); ++it) {
		G4VPhysicsConstructor* lhs = *it;
		if (lhs == nullptr)
			continue;
		if (typeid(*lhs) == typeid(*physics))
			return true;
	}
	return false;
}
