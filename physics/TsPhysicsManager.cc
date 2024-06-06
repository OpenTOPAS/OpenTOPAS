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

#include "TsPhysicsManager.hh"

#include "TsParameterManager.hh"
#include "TsExtensionManager.hh"
#include "TsGeometryManager.hh"

#include "TsModularPhysicsList.hh"
#include "TsTopasConfig.hh"
#include "TsVarianceManager.hh"
#include "TsInelasticSplitManager.hh"
#include "TsAutomaticImportanceSamplingManager.hh"
#include "TsAutomaticImportanceSamplingParallelManager.hh"

#include "G4StepLimiterPhysics.hh"
#include "G4EmParameters.hh"
#include "G4GenericBiasingPhysics.hh"
#include "G4PhysListFactory.hh"
#include "G4ProductionCutsTable.hh"
#include "G4SystemOfUnits.hh"

TsPhysicsManager::TsPhysicsManager(TsParameterManager* pM, TsExtensionManager* eM, TsGeometryManager* gM)
	: fPm(pM), fEm(eM), fGm(gM), fVm(nullptr)
{
	fPhysicsListName = fPm->GetStringParameter("Ph/ListName");
}

TsPhysicsManager::~TsPhysicsManager()
{
}

G4VUserPhysicsList* TsPhysicsManager::GetPhysicsList() {
	G4VUserPhysicsList* physicsList = 0;

	if (fPm->IsFindingSeed()) {
		TsModularPhysicsList* tsList = new TsModularPhysicsList(fPm, fEm, fGm, fVm, fPhysicsListName);
		tsList->AddModule("transportation_only");
		return tsList;
	}

	if (fPm->ParameterExists(GetFullParmName("Type"))) {
		G4String listType = fPm->GetStringParameter(GetFullParmName("Type"));
		G4String lowerListType = listType;
		G4String upperListType = listType;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(lowerListType);
		G4StrUtil::to_upper(upperListType);
#else
		lowerListType.toLower();
		upperListType.toUpper();
#endif

		G4PhysListFactory ReferenceList;
		if (ReferenceList.IsReferencePhysList( upperListType ) || lowerListType=="shielding") {
			if (fGm->HaveParallelComponentsThatAreNotGroups()) {
				G4cerr << "Topas is exiting due to inappropriate physics list for your setup." << G4endl;
				G4cerr << "Your geometry involves parallel worlds, either from explicit IsParallel parameters" << G4endl;
				G4cerr << "or due to scoring with different divisions than the component divisions." << G4endl;
				G4cerr << "Your setup will only work with physics list Geant4_Modular." << G4endl;
				fPm->AbortSession(1);
			}

			if (lowerListType=="shielding") upperListType = "Shielding";
			physicsList = ReferenceList.GetReferencePhysList( upperListType );

			if (fPm->ParameterExists(GetFullParmName("CutForAllParticles")))
				physicsList->SetDefaultCutValue(fPm->GetDoubleParameter(GetFullParmName("CutForAllParticles"), "Length"));

		} else if (lowerListType == "geant4_modular") {
			if (!fPm->ParameterExists(GetFullParmName("Modules"))) {
				G4cerr << "Topas is exiting due to a serious error in physics setup." << G4endl;
				G4cerr << "Your physics list is missing the parameter: " << GetFullParmName("Modules") << G4endl;
				fPm->AbortSession(1);
			}

			G4String* modules = fPm->GetStringVector(GetFullParmName("Modules"));
			G4int modules_size = fPm->GetVectorLength(GetFullParmName("Modules"));
			TsModularPhysicsList* tsList = new TsModularPhysicsList(fPm, fEm, fGm, fVm, fPhysicsListName);

			tsList->RegisterPhysics(new G4StepLimiterPhysics());
			if (fPm->UseVarianceReduction()) {
				G4int index = -1;
				if ( fVm->BiasingProcessExists("geometricalparticlesplit", index)) {
					tsList->AddBiasing(fProtonGeomSamplers);
				}
			}

			for (G4int i=0; i < modules_size; ++i)
				tsList->AddModule(modules[i]);

			if (fPm->UseVarianceReduction()) {
				G4int index = -1;
				if ( fVm->BiasingProcessExists("inelasticsplitting", index)) {
					G4GenericBiasingPhysics* bPIS = dynamic_cast<TsInelasticSplitManager*>
					(fVm->GetBiasingProcessFromList(index))->GetGenericBiasingPhysics();
					tsList->RegisterPhysics(bPIS);
				}
				index = -1;
				if ( fVm->BiasingProcessExists("automaticimportancesampling", index)) {
					G4GenericBiasingPhysics* bPAIS = dynamic_cast<TsAutomaticImportanceSamplingManager*>
					(fVm->GetBiasingProcessFromList(index))->GetGenericBiasingPhysics();
					tsList->RegisterPhysics(bPAIS);
				}
				index = -1;
				if ( fVm->BiasingProcessExists("automaticimportancesamplingparallel", index)) {
					G4GenericBiasingPhysics* bPAISP = dynamic_cast<TsAutomaticImportanceSamplingParallelManager*>
					(fVm->GetBiasingProcessFromList(index))->GetGenericBiasingPhysics();
					tsList->RegisterPhysics(bPAISP);
				}
			}

			if (!fPm->ParameterExists("Ph/SetEmParametersInTsModularPhysicsList") || !fPm->GetBooleanParameter("Ph/SetEmParametersInTsModularPhysicsList"))
				SetEmParameters();

			physicsList = tsList;
		} else {
			physicsList = fEm->InstantiatePhysicsList(fPm, lowerListType);

			if (physicsList==0) {
				G4cerr << "Topas is exiting due to a serious error in physics setup." << G4endl;
				G4cerr << "Your parameter: " << GetFullParmName("Type") << " has an unknown physics list type:" << listType << G4endl;
				fPm->AbortSession(1);
			}
		}

	} else {
		G4cerr << "Topas is exiting due to a serious error in physics setup." << G4endl;
		G4cerr << "You have specified a physics list with no type." << G4endl;
		G4cerr << "You need a parameter named: " << GetFullParmName("Type") << G4endl;
		fPm->AbortSession(1);
	}

	if (fPm->ParameterExists(GetFullParmName("SetProductionCutLowerEdge")) || fPm->ParameterExists(GetFullParmName("SetProductionCutHighEdge"))){
		G4double lowEdge = 990*eV;
		if (fPm->ParameterExists(GetFullParmName("SetProductionCutLowerEdge")))
			lowEdge = fPm->GetDoubleParameter(GetFullParmName("SetProductionCutLowerEdge"), "Energy");
		G4double highEdge = 1*GeV;
		if (fPm->ParameterExists(GetFullParmName("SetProductionCutHighEdge")))
			highEdge = fPm->GetDoubleParameter(GetFullParmName("SetProductionCutHighEdge"), "Energy");
		G4ProductionCutsTable::GetProductionCutsTable()->SetEnergyRange (lowEdge, highEdge);
	}

	return physicsList;
}


void TsPhysicsManager::SetEmParameters() {
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
		G4double finalRange = fPm->GetDoubleParameter(GetFullParmName("EMFinalRange"),"Length");
		G4EmParameters::Instance()->SetStepFunction(roverRange, finalRange);
	}

	if (fPm->ParameterExists(GetFullParmName("LowestElectronEnergy")))
		G4EmParameters::Instance()->SetLowestElectronEnergy(fPm->GetDoubleParameter(GetFullParmName("LowestElectronEnergy"),"Energy"));

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
		if (mscStepLimitType == "safety" ) {
			G4EmParameters::Instance()->SetMscStepLimitType(fUseSafety);
		} else if (mscStepLimitType == "safetyplus") {
			G4EmParameters::Instance()->SetMscStepLimitType(fUseSafetyPlus);
		} else if (mscStepLimitType == "distancetoboundary") {
			G4EmParameters::Instance()->SetMscStepLimitType(fUseDistanceToBoundary);
		} else {
			G4cerr << "Topas is exiting due to a serious error in physics setup." << G4endl;
			G4cerr << GetFullParmName("MSCStepLimitType") << " refers to an unknown type of step limit for multiplescattering" << G4endl;
			fPm->AbortSession(1);
		}
	}
// end of temporal parameters

	if ( fPm->ParameterExists(GetFullParmName("SolvatedElectronThermalizationModel")) ) {
		G4String eaqModel = fPm->GetStringParameter(GetFullParmName("SolvatedElectronThermalizationModel"));

#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(eaqModel);
#else
		eaqModel.toLower();
#endif
		if ( eaqModel == "ritchie" ) {
			G4EmParameters::Instance()->SetDNAeSolvationSubType(fRitchie1994eSolvation);
		} else if ( eaqModel == "terrisol" ) {
			G4EmParameters::Instance()->SetDNAeSolvationSubType(fTerrisol1990eSolvation);
		} else if ( eaqModel == "meesungnoen" ) {
			G4EmParameters::Instance()->SetDNAeSolvationSubType(fMeesungnoen2002eSolvation);
		} else if ( eaqModel == "meesungnoensolid" ) {
			G4EmParameters::Instance()->SetDNAeSolvationSubType(fMeesungnoensolid2002eSolvation);
		} else if ( eaqModel == "kreipl" ) {
			G4EmParameters::Instance()->SetDNAeSolvationSubType(fKreipl2009eSolvation);
		} else {
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

	if (fPm->ParameterExists("Ph/Verbosity") && fPm->GetIntegerParameter("Ph/Verbosity") > 0 )
		G4EmParameters::Instance()->Dump();
}


void TsPhysicsManager::SetVarianceManager(TsVarianceManager* vM) {
	fVm = vM;
}


G4String TsPhysicsManager::GetFullParmName(const char* parmName) {
	G4String fullName = "Ph/" + fPhysicsListName + "/"+parmName;
	return fullName;
}
