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

#include "TsGeometryManager.hh"

#include "TsParameterManager.hh"
#include "TsMaterialManager.hh"
#include "TsExtensionManager.hh"
#include "TsScoringManager.hh"
#include "TsSequenceManager.hh"
#include "TsVarianceManager.hh"

#include "TsGeometryHub.hh"
#include "TsVGeometryComponent.hh"
#include "TsBox.hh"
#include "TsCylinder.hh"
#include "TsSphere.hh"
#include "TsParallelWorld.hh"
#include "TsVMagneticField.hh"
#include "TsVElectroMagneticField.hh"
#include "TsBorderSurface.hh"
#include "TsTopasConfig.hh"
#include "TsInelasticSplitOperator.hh"
#include "TsAutomaticImportanceSamplingOperator.hh"
#include "TsAutomaticImportanceSamplingParallelOperator.hh"

#include "G4VPhysicalVolume.hh"
#include "G4UIcommand.hh"
#include "G4RunManager.hh"
#include "G4ProductionCutsTable.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4GeometryManager.hh"
#include "G4GeometryTolerance.hh"
#include "G4SystemOfUnits.hh"

#ifdef TOPAS_MT
#include "G4AutoLock.hh"

namespace {
	G4Mutex constructSDandFieldMutex = G4MUTEX_INITIALIZER;
}
#endif

TsGeometryManager::TsGeometryManager(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM)
	: fPm(pM), fEm(eM), fMm(mM), fScm(nullptr), fVm(nullptr),
	  fWorldComponent(0), fWorldVolume(0), fHaveParallelComponentsThatAreNotGroups(false),
	  fTooComplexForOGLS(false), fHasDividedCylinderOrSphere(false),
	  fAlreadyConstructed(false), fCurrentComponent(0), fCurrentMagneticField(0), fCurrentElectroMagneticField(0)
{
	fVerbosity = fPm->GetIntegerParameter("Ge/Verbosity");

	if (fPm->ParameterExists("Ge/World/Field")) {
		G4cerr << "Topas is exiting due to a serious error in geometry setup." << G4endl;
		G4cerr << "The World component has been set to have a Field." << G4endl;
		G4cerr << "This is not permitted." << G4endl;
		G4cerr << "If you really want a field around your entire setup," << G4endl;
		G4cerr << "create a new component to contain this field," << G4endl;
		G4cerr << "and make everything else a child of that component." << G4endl;
		fPm->AbortSession(1);
	}

	fGeometryHub = new TsGeometryHub(pM);

	// Create the store for the Geometry Components
	fStore = new std::map<G4String, TsVGeometryComponent*>;

	fParallelWorldsWithMaterial = new std::set<G4String>;
	fNamesToIgnoreInUnusedComponentCheck = new std::set<G4String>;

	fBorderSurfaces = new std::map<G4String, TsBorderSurface*>;
	fOpticalSurfaces = new std::map<G4String, G4OpticalSurface*>;
}


TsGeometryManager::~TsGeometryManager()
{
	delete fStore;
}


TsGeometryHub* TsGeometryManager::GetGeometryHub() {
	return fGeometryHub;
}


void TsGeometryManager::SetCurrentComponent(TsVGeometryComponent* currentComponent) {
	fCurrentComponent = currentComponent;

	// Also update map from component name to component pointer.
	if (currentComponent) {
		G4String nameLower = currentComponent->GetNameWithCopyId();
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(nameLower);
#else
		nameLower.toLower();
#endif
		(*fStore)[nameLower] = currentComponent;
	}
}


void TsGeometryManager::SetCurrentMagneticField(TsVMagneticField* currentMagneticField) {
	fCurrentMagneticField = currentMagneticField;
}


void TsGeometryManager::SetCurrentElectroMagneticField(TsVElectroMagneticField* currentElectroMagneticField) {
	fCurrentElectroMagneticField = currentElectroMagneticField;
}


// Construct the world component. All other construction is deferred until a little later so that any
// parallel worlds that are needed can refer to an already-constructed mass world.
G4VPhysicalVolume* TsGeometryManager::Construct()
{
	// If this is not the first run, then this call is just a byproduct of our calling
	// /run/reinitializeGeometry, something we need to do to make each worker thread
	// re-call ConstructSDandField. There is nothing to do here in Construct.
	if (fAlreadyConstructed)
		return fWorldVolume;
	else
		fAlreadyConstructed = true;

	G4String worldComponentType = fPm->GetStringParameter("Ge/World/Type");
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(worldComponentType);
#else
	worldComponentType.toLower();
#endif
	if (worldComponentType != "tsbox" && worldComponentType != "tscylinder" && worldComponentType != "tssphere") {
		G4cerr << "Ge/World/Type has been set to: " << fPm->GetStringParameter("Ge/World/Type") << G4endl;
		G4cerr << "but the only valid options are TsBox, TsCylinder and TsSphere." << G4endl;
		fPm->AbortSession(1);
	}
	
	if (fPm->ParameterExists("Ge/World/GeometricTolerance"))
		SetGeometricalTolerance(fPm->GetDoubleParameter("Ge/World/GeometricTolerance","Length"));
	else {
		SetGeometricalTolerance(std::min(fPm->GetDoubleParameter("Ge/World/HLX", "Length"), std::min(fPm->GetDoubleParameter("Ge/World/HLY", "Length"), fPm->GetDoubleParameter("Ge/World/HLZ", "Length") ) ) );
	}
	
	G4String s_World = "World";

	// Instantiate the top component.
	// The 0 indicates that this top component has no parent.
	if (worldComponentType == "tsbox")
		fWorldComponent = new TsBox(fPm, fEm, fMm, this, 0, 0, s_World);
	else if (worldComponentType == "tscylinder")
		fWorldComponent = new TsCylinder(fPm, fEm, fMm, this, 0, 0, s_World);
	else
		fWorldComponent = new TsSphere(fPm, fEm, fMm, this, 0, 0, s_World);

	fWorldVolume = fWorldComponent->Construct();

	// Reset current component to zero to indicate that we are no longer building components.
	SetCurrentComponent(0);

	return fWorldVolume;
}


void TsGeometryManager::ConstructSDandField() {
#ifdef TOPAS_MT
	G4AutoLock l(&constructSDandFieldMutex);

	if (G4Threading::IsWorkerThread()) {
		fScm->Initialize();

		std::map<G4String, TsVGeometryComponent*>::const_iterator iter;
		for (iter=fStore->begin(); iter!=fStore->end(); iter++)
			iter->second->InstantiateFields();
		SetCurrentMagneticField(0);
		SetCurrentElectroMagneticField(0);
		
		if (fPm->UseVarianceReduction()) {
			G4int index = -1;
			if ( fVm->BiasingProcessExists("inelasticsplitting", index) ) {
				TsInelasticSplitOperator* biasingOperatorIS = new TsInelasticSplitOperator(fPm, "inelasticsplitting");
				for ( iter=fStore->begin(); iter!=fStore->end(); iter++) {
					if ( biasingOperatorIS->IsApplicable(iter->second->GetEnvelopePhysicalVolume()->GetLogicalVolume()) )
						biasingOperatorIS->AttachTo(iter->second->GetEnvelopePhysicalVolume()->GetLogicalVolume());
				}
			}
			
			index = -1;
			if ( fVm->BiasingProcessExists("automaticimportancesampling", index) ) {
				TsAutomaticImportanceSamplingOperator* biasingOperatorAIS =
				new TsAutomaticImportanceSamplingOperator(fPm, "automaticimportancesampling");
				
				for ( iter=fStore->begin(); iter!=fStore->end(); iter++) {
					if ( biasingOperatorAIS->IsApplicable(iter->second->GetEnvelopePhysicalVolume()->GetLogicalVolume()) ) {
						biasingOperatorAIS->AttachTo(iter->second->GetEnvelopePhysicalVolume()->GetLogicalVolume());
						std::vector<G4VPhysicalVolume*> allVol = iter->second->GetAllPhysicalVolumes(true);
						for ( size_t t = 0; t < allVol.size(); t++ )
							biasingOperatorAIS->AttachTo(allVol[t]->GetLogicalVolume());
					}
				}
			}
			
			index = -1;
			if ( fVm->BiasingProcessExists("automaticimportancesamplingparallel", index) ) {
				TsAutomaticImportanceSamplingParallelOperator* biasingOperatorAISP =
				new TsAutomaticImportanceSamplingParallelOperator(fPm, "automaticimportancesamplingparallel");
				G4String pWorlName = biasingOperatorAISP->GetParallelWorldName();
				biasingOperatorAISP->SetParallelWorld(CreateParallelWorld(pWorlName, false));

				for ( iter=fStore->begin(); iter!=fStore->end(); iter++) {
					if ( biasingOperatorAISP->IsApplicable(iter->second->GetEnvelopePhysicalVolume()->GetLogicalVolume()) ) {
						biasingOperatorAISP->AttachTo(iter->second->GetEnvelopePhysicalVolume()->GetLogicalVolume());
						std::vector<G4VPhysicalVolume*> allVol = iter->second->GetAllPhysicalVolumes(true);
						for ( size_t t = 0; t < allVol.size(); t++ )
							biasingOperatorAISP->AttachTo(allVol[t]->GetLogicalVolume());
					}
				}
			}
		}
	}
#endif
}


// Construct all other components (other than the World).
void TsGeometryManager::Initialize() {
	fWorldComponent->InstantiateChildren();

	// Create border surfaces
	std::map<G4String, TsBorderSurface*>::const_iterator iter;
	for (iter = fBorderSurfaces->begin(); iter != fBorderSurfaces->end(); iter++)
		iter->second->CreateBorderSurface();

	// Inform user about unused components
	if (fPm->GetBooleanParameter("Ge/CheckForUnusedComponents"))
		CheckForUnusedComponents();
}


void TsGeometryManager::SetScoringManager(TsScoringManager* scM) {
	fScm = scM;
}


void TsGeometryManager::NoteAnyUseOfChangeableParameters(const G4String& name)
{
	if (fCurrentComponent)
	{
		// Register use of this parameter for current component and LastDirectParameter.
		// LastDirectParameter is needed because we ultimately need to tell the component not the name of the changed parameter
		// but the name of the parameter the component directly accesses (that was affected by the changed parameter).
		// We store strings rather than pointers because we permit a parmeter to be overridden by another parameter
		// of the same name, and a component may be replaced by an updated component of the same name.
		G4String comp = fCurrentComponent->GetNameWithCopyId();
		G4String directParm = fPm->GetLastDirectParameterName();
		G4String directParmLower = directParm;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(directParmLower);
#else
		directParmLower.toLower();
#endif

		if (directParmLower == fCurrentComponent->GetFullParmNameLower("Type")) {
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "The Type parameter of component " << fCurrentComponent->GetNameWithCopyId() << " has been set to depend on a time feature." << G4endl;
			G4cerr << "This is not permitted." << G4endl;
			fPm->AbortSession(1);
		}

		// See if this parameter has already been registered as used by this component (otherwise if this parameter
		// is interrogated twice for the same component, the component would update twice for the same change).
		G4String nameToLower = name;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(nameToLower);
#else
		nameToLower.toLower();
#endif
		G4bool matched = false;
		std::multimap<G4String,std::pair<G4String,G4String> >::const_iterator iter;
		for (iter = fChangeableParameterMap.begin(); iter != fChangeableParameterMap.end() && !matched; iter++) {
			G4String gotParm = iter->first;
			G4String gotComp = iter->second.first;
			G4String gotDirectParm = iter->second.second;
			if (gotParm==nameToLower && gotComp==comp && gotDirectParm==directParm)
				matched = true;
		}

		if (!matched) {
			fChangeableParameterMap.insert(std::make_pair(nameToLower, std::make_pair(comp, directParm)));
			if (fPm->IGetIntegerParameter("Tf/Verbosity") > 0)
				G4cout << "Registered use of changeable parameter: " << name << "  by component: " << fCurrentComponent->GetNameWithCopyId() << G4endl;
		}

		if (matched && fVerbosity>0)
			G4cout << "TsGeometryManager::NoteAnyUse Again called with current component: " << fCurrentComponent->GetNameWithCopyId() << ", param: " << name <<
			", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
		else if (fVerbosity>0)
			G4cout << "TsGeometryManager::NoteAnyUse First called with current component: " << fCurrentComponent->GetNameWithCopyId() << ", param: " << name <<
			", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
	}

	if (fCurrentMagneticField)
	{
		// Register use of this parameter for current magnetic field and LastDirectParameter.
		TsVGeometryComponent* mComponent = fCurrentMagneticField->GetComponent();
		G4String comp = mComponent->GetNameWithCopyId();
		G4String directParm = fPm->GetLastDirectParameterName();
		G4String directParmLower = directParm;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(directParmLower);
#else
		directParmLower.toLower();
#endif

		if (directParmLower == mComponent->GetFullParmNameLower("Field")) {
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "The Field parameter of component " << mComponent->GetNameWithCopyId() << " has been set to depend on a time feature." << G4endl;
			G4cerr << "This is not permitted." << G4endl;
			fPm->AbortSession(1);
		}

		// See if this parameter has already been registered as used by this component (otherwise if this parameter
		// is interrogated twice for the same component, the component would update twice for the same change).
		G4String nameToLower = name;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(nameToLower);
#else
		nameToLower.toLower();
#endif
		G4bool matched = false;
		std::multimap<G4String,std::pair<G4String,G4String> >::const_iterator iter;
		for (iter = fChangeableMagneticFieldParameterMap.begin(); iter != fChangeableMagneticFieldParameterMap.end() && !matched; iter++) {
			G4String gotParm = iter->first;
			G4String gotComp = iter->second.first;
			G4String gotDirectParm = iter->second.second;
			if (gotParm==nameToLower && gotComp==comp && gotDirectParm==directParm)
				matched = true;
		}

		if (!matched) {
			fChangeableMagneticFieldParameterMap.insert(std::make_pair(nameToLower, std::make_pair(comp, directParm)));
			if (fPm->IGetIntegerParameter("Tf/Verbosity") > 0)
				G4cout << "Registered use of changeable magnetic field parameter: " << name << "  by component: " << mComponent->GetNameWithCopyId() << G4endl;
		}

		if (matched && fVerbosity>0)
			G4cout << "TsGeometryManager::NoteAnyUse Again called with current component: " << mComponent->GetNameWithCopyId() << ", param: " << name <<
			", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
		else if (fVerbosity>0)
			G4cout << "TsGeometryManager::NoteAnyUse First called with current component: " << mComponent->GetNameWithCopyId() << ", param: " << name <<
			", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
	}

	if (fCurrentElectroMagneticField)
	{
		// Register use of this parameter for current electric field and LastDirectParameter.
		TsVGeometryComponent* mComponent = fCurrentElectroMagneticField->GetComponent();
		G4String comp = mComponent->GetNameWithCopyId();
		G4String directParm = fPm->GetLastDirectParameterName();
		G4String directParmLower = directParm;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(directParmLower);
#else
		directParmLower.toLower();
#endif

		if (directParmLower == mComponent->GetFullParmNameLower("Field")) {
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "The Field parameter of component " << mComponent->GetNameWithCopyId() << " has been set to depend on a time feature." << G4endl;
			G4cerr << "This is not permitted." << G4endl;
			fPm->AbortSession(1);
		}

		// See if this parameter has already been registered as used by this component (otherwise if this parameter
		// is interrogated twice for the same component, the component would update twice for the same change).
		G4String nameToLower = name;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(nameToLower);
#else
		nameToLower.toLower();
#endif
		G4bool matched = false;
		std::multimap<G4String,std::pair<G4String,G4String> >::const_iterator iter;
		for (iter = fChangeableElectroMagneticFieldParameterMap.begin(); iter != fChangeableElectroMagneticFieldParameterMap.end() && !matched; iter++) {
			G4String gotParm = iter->first;
			G4String gotComp = iter->second.first;
			G4String gotDirectParm = iter->second.second;
			if (gotParm==nameToLower && gotComp==comp && gotDirectParm==directParm)
				matched = true;
		}

		if (!matched) {
			fChangeableElectroMagneticFieldParameterMap.insert(std::make_pair(nameToLower, std::make_pair(comp, directParm)));
			if (fPm->IGetIntegerParameter("Tf/Verbosity") > 0)
				G4cout << "Registered use of changeable electric field parameter: " << name << "  by component: " << mComponent->GetNameWithCopyId() << G4endl;
		}

		if (matched && fVerbosity>0)
			G4cout << "TsGeometryManager::NoteAnyUse Again called with current component: " << mComponent->GetNameWithCopyId() << ", param: " << name <<
			", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
		else if (fVerbosity>0)
			G4cout << "TsGeometryManager::NoteAnyUse First called with current component: " << mComponent->GetNameWithCopyId() << ", param: " << name <<
			", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
	}
}


void TsGeometryManager::UpdateForSpecificParameterChange(G4String parameter) {
	std::multimap<G4String,std::pair<G4String,G4String> >::const_iterator iter;
	for (iter = fChangeableParameterMap.begin(); iter != fChangeableParameterMap.end(); iter++) {
		if (iter->first==parameter) {
			G4String directParameterName = iter->second.second;
			G4String compName = iter->second.first;
			if (fVerbosity>0)
				G4cout << "TsGeometryManager::UpdateForSpecificParameterChange called for parameter: " << parameter <<
				", matched for component: " << compName << ", direct parameter: " << directParameterName << G4endl;
			TsVGeometryComponent* component = GetComponent(compName);
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(directParameterName);
#else
			directParameterName.toLower();
#endif
			if (component)
				component->UpdateForSpecificParameterChange(directParameterName);
			else {
				G4cerr << "TsGeometryManager::UpdateForSpecificParameterChange called for non-existing component: " << compName << G4endl;
				fPm->AbortSession(1);
			}
		}
	}

	for (iter = fChangeableMagneticFieldParameterMap.begin(); iter != fChangeableMagneticFieldParameterMap.end(); iter++) {
		if (iter->first==parameter) {
			G4String directParameterName = iter->second.second;
			G4String compName = iter->second.first;
			if (fVerbosity>0)
				G4cout << "TsGeometryManager::UpdateForSpecificParameterChange called for magnetic field parameter: " << parameter <<
				", matched for component: " << compName << ", direct parameter: " << directParameterName << G4endl;
			TsVGeometryComponent* component = GetComponent(compName);
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(directParameterName);
#else
			directParameterName.toLower();
#endif
			if (component)
				component->UpdateForSpecificMagneticFieldParameterChange(directParameterName);
			else {
				G4cerr << "TsGeometryManager::UpdateForSpecificParameterChange for magnetic field called for non-existing component: " << compName << G4endl;
				fPm->AbortSession(1);
			}
		}
	}

	for (iter = fChangeableElectroMagneticFieldParameterMap.begin(); iter != fChangeableElectroMagneticFieldParameterMap.end(); iter++) {
		if (iter->first==parameter) {
			G4String directParameterName = iter->second.second;
			G4String compName = iter->second.first;
			if (fVerbosity>0)
				G4cout << "TsGeometryManager::UpdateForSpecificParameterChange called for electric field parameter: " << parameter <<
				", matched for component: " << compName << ", direct parameter: " << directParameterName << G4endl;
			TsVGeometryComponent* component = GetComponent(compName);
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(directParameterName);
#else
			directParameterName.toLower();
#endif
			if (component)
				component->UpdateForSpecificElectroMagneticFieldParameterChange(directParameterName);
			else {
				G4cerr << "TsGeometryManager::UpdateForSpecificParameterChange for electric field called for non-existing component: " << compName << G4endl;
				fPm->AbortSession(1);
			}
		}
	}
}


G4bool TsGeometryManager::UpdateForNewRun(TsSequenceManager* sqM, G4bool forceUpdatePlacement) {
	if (fVerbosity>0)
		G4cout << "TsGeometryManager::UpdateForNewRun called with forceUpdatePlacement: " << forceUpdatePlacement << G4endl;
	G4bool rebuiltSomeComponents = fWorldComponent->RebuildIfNeeded();
	fWorldComponent->UpdateForNewRun(forceUpdatePlacement);

	// May need to rebuild some border surfaces
	if (rebuiltSomeComponents) {
		std::map<G4String, TsBorderSurface*>::const_iterator iter;
		for (iter = fBorderSurfaces->begin(); iter != fBorderSurfaces->end(); iter++)
			iter->second->CreateBorderSurface();
	}

	std::list<G4LogicalVolume*>::const_iterator iter;
	if (fPm->GetBooleanParameter("Ts/DisableReoptimizeTestMode"))
		G4cout << "TsGeometryManager::UpdateForNewRun ignoring request to reoptimize volumes since Ts/DisableReoptimizeTestMode has been set true." << G4endl;
	else
		for (iter = fReoptimizeVolumes.begin(); iter != fReoptimizeVolumes.end(); ++iter)
			sqM->ReOptimize(*iter);
	fReoptimizeVolumes.clear();

	if (fVerbosity>0)
		G4cout << "TsGeometryManager::UpdateForNewRun returning with rebuiltSomeComponents: " << rebuiltSomeComponents << G4endl;
	return rebuiltSomeComponents;
}


void TsGeometryManager::UpdateWorldForNewRun() {
	fWorldComponent->UpdateForNewRun(true);
}


void TsGeometryManager::AddToReoptimizeList(G4LogicalVolume* volume) {
	if (std::find(fReoptimizeVolumes.begin(), fReoptimizeVolumes.end(), volume)==fReoptimizeVolumes.end())
		fReoptimizeVolumes.push_back(volume);
}


void TsGeometryManager::RemoveFromReoptimizeList(G4LogicalVolume* volume) {
	fReoptimizeVolumes.remove(volume);
}


TsVGeometryComponent* TsGeometryManager::GetComponent(G4String& nameWithCopyId)
{
	G4String nameWithCopyIdLower = nameWithCopyId;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(nameWithCopyIdLower);
#else
	nameWithCopyIdLower.toLower();
#endif

	std::map<G4String, TsVGeometryComponent*>::const_iterator iter = fStore->find(nameWithCopyIdLower);
	if (iter == fStore->end())
		return 0;

	return iter->second;
}


std::vector<G4String> TsGeometryManager::GetComponentNames()
{
	std::vector<G4String> componentNames;

	std::map<G4String, TsVGeometryComponent*>::const_iterator iter;
	for (iter=fStore->begin(); iter!=fStore->end(); iter++)
		componentNames.push_back(iter->first);

	return componentNames;
}


G4String TsGeometryManager::GetCopyIdFromBinning(G4int* nBins) {
	return GetCopyIdFromBinning(nBins[0], nBins[1], nBins[2]);
}


G4String TsGeometryManager::GetCopyIdFromBinning(G4int i, G4int j, G4int k) {
	return "_" + G4UIcommand::ConvertToString(i) +"x"+ G4UIcommand::ConvertToString(j) +"x"+ G4UIcommand::ConvertToString(k);
}


// Find the names of all Geometry Components that have Parent parameter matching the given value
std::vector<G4String> TsGeometryManager::GetChildComponentsOf(G4String& parentName)
{
	G4String parentNameLower = parentName;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(parentNameLower);
#else
	parentNameLower.toLower();
#endif
	std::vector<G4String> components;

	// Find all parameters bracketed by Ge/ and /Parent
	G4String prefix = "Ge/";
	G4String suffix = "/Parent";
	std::vector<G4String>* values = new std::vector<G4String>;
	fPm->GetParameterNamesBracketedBy(prefix, suffix, values);
	G4int length = values->size();

	// Loop over all those found parameters
	for (G4int iToken=0; iToken<length; iToken++) {
		G4String parameterName = (*values)[iToken];
		G4String value = fPm->GetStringParameter(parameterName);
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(value);
#else
		value.toLower();
#endif

		if (value==parentNameLower) {
			G4String componentName = parameterName.substr(3,parameterName.length()-10);

			// If a particlar Ge/Comp/.../Parent is defined in one file and then again defined (the same or differently)
			// in another file, we will find two occurrences of this component. Only want to actually add it once.
			G4bool alreadyHave = false;
			std::vector<G4String>::iterator iter;
			for (iter = components.begin(); !alreadyHave && iter!=components.end(); iter++)
				if (*iter == componentName) alreadyHave = true;

			if (!alreadyHave)
				components.push_back(componentName);
		}
	}

	return components;
}


void TsGeometryManager::UnRegisterComponent(G4String& name) {
	G4String nameLower = name;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(nameLower);
#else
	nameLower.toLower();
#endif
	fStore->erase(nameLower);
}


G4VPhysicalVolume* TsGeometryManager::CreateParallelWorld(G4String name, G4bool componentIsGroup) {
	TsParallelWorld* aParallelWorld;
	G4bool found = false;

	for (G4int iWorld = 0; !found && iWorld < GetNumberOfParallelWorld(); iWorld++) {
		aParallelWorld = (TsParallelWorld*)GetParallelWorld(iWorld);
		if (aParallelWorld->GetName() == name) {
			found = true;
		}
	}

	if (!found) {
		aParallelWorld = new TsParallelWorld(name);
		G4cout << "Creating parallel world: " << aParallelWorld->GetName() << G4endl;
		RegisterParallelWorld(aParallelWorld);
	}

	if (!componentIsGroup) fHaveParallelComponentsThatAreNotGroups = true;

	return aParallelWorld->GetPhysicalVolume();
}


void TsGeometryManager::CheckForUnusedComponents() {
	G4cout << G4endl;
	G4cout << "Checking for Unused Components." << G4endl;
	G4cout << "These are Components that were never built even though they were at least" << G4endl;
	G4cout << "partially defined in some parameters. This is not necessarily an error." << G4endl;
	G4cout << "It may be an intentional choice to have these Component definitions handy for later use." << G4endl;
	G4cout << "To turn off this check, set Ge/CheckForUnusedComponents to False" << G4endl;

	G4String lastUnusedFound = "";
	G4String prefix = "Ge/";
	std::vector<G4String>* values = new std::vector<G4String>;
	fPm->GetParameterNamesStartingWith(prefix, values);
	G4int length = values->size();

	// Loop over all Geometry parameters
	for (G4int iToken=0; iToken<length; iToken++) {
		G4String parameterName = (*values)[iToken];
		size_t pos = parameterName.find_last_of("/");

		if (pos > 3) {
			G4String componentName = parameterName.substr(3,pos-3);
			size_t firstSlashPos = componentName.find_first_of("/");
			if (firstSlashPos != G4String::npos)
				componentName = componentName.substr(0,firstSlashPos);
			if (componentName != lastUnusedFound && componentName != "Params" &&
				!GetComponent(componentName) && !IgnoreInUnusedComponentCheck(componentName)) {
				lastUnusedFound = componentName;
				G4String parentParameterString = "Ge/"+componentName+"/Parent";
				if (fPm->ParameterExists(parentParameterString))
					G4cout << "  The Parent of this Component was never built:     " << componentName << G4endl;
				else
					G4cout << "  The Parent of this Component was never specified: " << componentName << G4endl;
			}
		}
	}

	if (lastUnusedFound == "") {
		G4cout << "  No unused components were found." << G4endl;
	}

	G4cout << G4endl;
}


G4VPhysicalVolume* TsGeometryManager::GetPhysicalVolume(G4String volumeName) {
	G4VPhysicalVolume* volume;
	std::map<G4String, TsVGeometryComponent*>::const_iterator iter;
	for (iter=fStore->begin(); iter!=fStore->end(); iter++) {
		volume = iter->second->GetPhysicalVolume(volumeName);
		if (volume) return volume;
	}

	return 0;
}


void TsGeometryManager::SetHaveParallelComponentsThatAreNotGroups() {
	fHaveParallelComponentsThatAreNotGroups = true;
}


G4bool TsGeometryManager::HaveParallelComponentsThatAreNotGroups() {
	return fHaveParallelComponentsThatAreNotGroups;
}


void TsGeometryManager::SetParallelWorldHasMaterial(G4String worldName) {
	fParallelWorldsWithMaterial->insert(worldName);
}


std::set<G4String>* TsGeometryManager::GetParallelWorldsWithMaterial() {
	return fParallelWorldsWithMaterial;
}


void TsGeometryManager::RegisterToIgnoreInUnusedComponentCheck(G4String name) {
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(name);
#else
	name.toLower();
#endif
	fNamesToIgnoreInUnusedComponentCheck->insert(name);
}

void TsGeometryManager::SetGeometricalTolerance(G4double newTolerance){
	G4GeometryManager::GetInstance()->SetWorldMaximumExtent(newTolerance);
	G4cout << "TOPAS set the tolerances based on the World size to:" << G4endl;
	G4cout << "      Surface tolerance = " << G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()/nm
	<< " nm, and radial tolerance = " << G4GeometryTolerance::GetInstance()->GetRadialTolerance()/nm << " nm." << G4endl;
}


G4bool TsGeometryManager::IgnoreInUnusedComponentCheck(G4String name) {
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(name);
#else
	name.toLower();
#endif
	std::set<G4String>::const_iterator iter;
	for (iter=fNamesToIgnoreInUnusedComponentCheck->begin(); iter!=fNamesToIgnoreInUnusedComponentCheck->end(); iter++)
		if (*iter == name)
			return true;
	return false;
}


void TsGeometryManager::SetTooComplexForOGLS() {
	if (!fTooComplexForOGLS) {
		G4cout << "OpenGL graphics will be switched to Immediate mode" << G4endl;
		fTooComplexForOGLS = true;
	}
}


G4bool TsGeometryManager::IsTooComplexForOGLS() {
	return fTooComplexForOGLS;
}


void TsGeometryManager::SetHasDividedCylinderOrSphere() {
	fHasDividedCylinderOrSphere = true;
}


G4bool TsGeometryManager::HasDividedCylinderOrSphere() {
	return fHasDividedCylinderOrSphere;
}


G4bool TsGeometryManager::GeometryHasOverlaps() {
	fPm->NoteGeometryOverlap(false);

	std::map<G4String, TsVGeometryComponent*>::const_iterator iter;
	for (iter=fStore->begin(); iter!=fStore->end(); iter++)
		iter->second->CheckForOverlaps(iter->second->GetEnvelopePhysicalVolume());

	return fPm->HasGeometryOverlap();
}


G4OpticalSurface* TsGeometryManager::GetOpticalSurface(G4String surfaceName) {
	G4OpticalSurface* opticalSurface = (*fOpticalSurfaces)[surfaceName];

	if (!opticalSurface) {
		// Create a new optical surface
		opticalSurface = new G4OpticalSurface(surfaceName);

		G4String model = fPm->GetStringParameter(GetFullSurfaceParmName(surfaceName, "Model"));
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(model);
#else
		model.toLower();
#endif
		if (model == "glisur")
			opticalSurface->SetModel(glisur);
		else if (model == "unified")
			opticalSurface->SetModel(unified);
		else
			Quit(GetFullSurfaceParmName(surfaceName, "Model"),
				 "Refers to an unkown optical surface model");

		G4String finish = fPm->GetStringParameter(GetFullSurfaceParmName(surfaceName, "Finish"));
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(finish);
#else
		finish.toLower();
#endif
		if (finish == "ground")
			opticalSurface->SetFinish(ground);
		else if (finish == "groundfrontpainted")
			opticalSurface->SetFinish(groundfrontpainted);
		else if (finish == "groundbackpainted")
			opticalSurface->SetFinish(groundbackpainted);
		else if (finish == "polished")
			opticalSurface->SetFinish(polished);
		else if (finish == "polishedfrontpainted")
			opticalSurface->SetFinish(polishedfrontpainted);
		else if (finish == "polishedbackpainted")
			opticalSurface->SetFinish(polishedbackpainted);
		else
			Quit(GetFullSurfaceParmName(surfaceName, "Finish"),
				 "Refers to an unknown optical surface finish");

		G4String type = fPm->GetStringParameter(GetFullSurfaceParmName(surfaceName, "Type"));
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(type);
#else
		type.toLower();
#endif
		if (type == "dielectric_dielectric")
			opticalSurface->SetType(dielectric_dielectric);
		else if (type == "dielectric_metal")
			opticalSurface->SetType(dielectric_metal);
		else
			Quit(GetFullSurfaceParmName(surfaceName, "Type"),
				 "Refers to an unknown optical surface type");

		G4double sigmaAlpha;
		if (!fPm->ParameterExists(GetFullSurfaceParmName(surfaceName, "SigmaAlpha")))
			sigmaAlpha = 0.0;
		else
			sigmaAlpha = fPm->GetUnitlessParameter(GetFullSurfaceParmName(surfaceName, "SigmaAlpha"));
		opticalSurface->SetSigmaAlpha(sigmaAlpha);

		G4MaterialPropertiesTable* propertiesTable = new G4MaterialPropertiesTable();
		fMm->FillMaterialPropertiesTable(propertiesTable, "Su/" + surfaceName + "/");
		opticalSurface->SetMaterialPropertiesTable(propertiesTable);

		if (fVerbosity > 0) {
			opticalSurface->DumpInfo();
			opticalSurface->GetMaterialPropertiesTable()->DumpTable();
			G4cout << G4endl;
		}

		(*fOpticalSurfaces)[surfaceName] = opticalSurface;
	}

	return opticalSurface;
}


void TsGeometryManager::RegisterBorderSurface(G4String borderSurfaceName) {
	if (!(*fBorderSurfaces)[borderSurfaceName])
		(*fBorderSurfaces)[borderSurfaceName] = new TsBorderSurface(fPm, this, borderSurfaceName);
}


void TsGeometryManager::CheckForEffectOnBorderSurfaceDestinations(G4String toComponentName) {
	std::map<G4String, TsBorderSurface*>::const_iterator iter;
	for (iter = fBorderSurfaces->begin(); iter != fBorderSurfaces->end(); iter++)
		iter->second->MarkForRebuildIfMatchesDestination(toComponentName);
}


void TsGeometryManager::SetVarianceManager(TsVarianceManager* vM) {
	fVm = vM;
}


G4String TsGeometryManager::GetFullSurfaceParmName(G4String surfaceName, const char* parmName) {
	G4String fullName = "Su/" + surfaceName + "/" + parmName;
	return fullName;
}


void TsGeometryManager::Quit(const G4String& name, const char* message) {
	G4cerr << "Topas is exiting due to a serious error in surface setup." << G4endl;
	G4cerr << "Parameter name: " << name << G4endl;
	G4cerr << message << G4endl;
	fPm->AbortSession(1);
}
