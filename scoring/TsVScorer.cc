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

#include "TsVScorer.hh"

#include "TsParameterManager.hh"
#include "TsMaterialManager.hh"
#include "TsGeometryManager.hh"
#include "TsVGeometryComponent.hh"
#include "TsScoringManager.hh"
#include "TsExtensionManager.hh"
#include "TsSequenceManager.hh"

#include "TsEventAction.hh"
#include "TsTrackingAction.hh"
#include "TsFilterByRTStructure.hh"
#include "TsTopasConfig.hh"
#include "TsChemTimeStepAction.hh"
#include "TsChemTrackingAction.hh"
#include "TsChemSteppingAction.hh"
#include "TsChemTrackingManager.hh"

#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4Step.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VPVParameterisation.hh"
#include "G4PSDirectionFlag.hh"
#include "G4UIcommand.hh"
#include "G4Scheduler.hh"
#include "G4ITTrackingInteractivity.hh"
#include "G4DNAChemistryManager.hh"

#include <sstream>
#include <iomanip>
#include <algorithm>

TsVScorer::TsVScorer(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
					 G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: G4VPrimitiveScorer(scorerName),
fSolid(NULL), fPm(pM), fOutFile(NULL),
fIsActive(true), fSkippedWhileInactive(0),
fUID(0), fScm(scM), fGm(gM), fEm(eM),
fScoredHistories(0), fHitsWithNoIncidentParticle(0), fUnscoredSteps(0), fUnscoredEnergy(0.), fMissedSubScorerVoxels(0),
fOutFileName(outFileName), fOutputAfterRun(false), fOutputAfterRunShouldAccumulate(false), fQuantity(quantity),
fComponent(NULL), fDetector(NULL), fComponentName(""),
fNDivisions(1), fSuppressStandardOutputHandling(false),
fIsSubScorer(isSubScorer), fHasCombinedSubScorers(false),
fHaveIncidentParticle(false), fIncidentParticleEnergy(-1.), fIncidentParticleMomentum(-1.),
fIncidentParticleCreatorProcess(NULL), fIncidentParticleDefinition(NULL), fIncidentParticleCharge(0), fIncidentParticleParentID(0),
fHadParameterChangeSinceLastRun(false), fMm(mM),
fSplitId(""), fSplitFunction(""), fSplitUnitLower(""), fSplitUnitCategory(""), fSplitStringValue(""),
fSplitBooleanValue(false), fSplitLowerValue(0), fSplitUpperValue(0),
fCachedCubicVolume(-1.), fPropagateToChildren(false),
fIsSurfaceScorer(false), fNeedsSurfaceAreaCalculation(false), fSurfaceName(""),
fSurfaceID(TsVGeometryComponent::None), fDirection(-1), fOnlyIncludeParticlesGoingIn(false),
fOnlyIncludeParticlesGoingOut(false), fSetBinToMinusOneIfNotInRTStructure(false), fRTStructureFilter(NULL)
{
	// All scorers need to be registered with the ScoringManager so they can receive parameter change updates
	fScm->SetCurrentScorer(this);
	fScm->RegisterScorer(this);

	// Worker scorers need to be registered with the EventAction and TrackingAction so they can receive IncidentTrack updates
#ifdef TOPAS_MT
	if (G4Threading::IsWorkerThread()) {
#endif
		((TsEventAction*)(G4RunManager::GetRunManager()->GetUserEventAction()))->RegisterScorer(this);
		((TsTrackingAction*)(G4RunManager::GetRunManager()->GetUserTrackingAction()))->RegisterScorer(this);
		if ( G4DNAChemistryManager::IsActivated() ) {
			((TsChemTimeStepAction*)(G4Scheduler::Instance()->GetUserTimeStepAction()))->RegisterScorer(this);
			((TsChemTrackingAction*)(((TsChemTrackingManager*)G4Scheduler::Instance()->GetInteractivity())->GetUserTrackingAction()))->RegisterScorer(this);
			((TsChemSteppingAction*)(((TsChemTrackingManager*)G4Scheduler::Instance()->GetInteractivity())->GetUserSteppingAction()))->RegisterScorer(this);
		}
#ifdef TOPAS_MT
	}
#endif

	fVerbosity = fPm->GetIntegerParameter("Sc/Verbosity");
	fTrackingVerbosity = fPm->GetIntegerParameter("Ts/TrackingVerbosity");

	fOutFileType = "csv";
	if (fPm->ParameterExists(GetFullParmName("OutputType")))
		fOutFileType = fPm->GetStringParameter(GetFullParmName("OutputType"));
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(fOutFileType);
#else
	fOutFileType.toLower();
#endif

	fOutFileMode = "";
	if (fPm->ParameterExists(GetFullParmName("IfOutputFileAlreadyExists")))
		fOutFileMode = fPm->GetStringParameter(GetFullParmName("IfOutputFileAlreadyExists"));

	if (fPm->ParameterExists(GetFullParmName("OutputAfterRun")))
		fOutputAfterRun = fPm->GetBooleanParameter(GetFullParmName("OutputAfterRun"));

	if (fPm->ParameterExists(GetFullParmName("OutputAfterRunShouldAccumulate")))
		fOutputAfterRunShouldAccumulate = fPm->GetBooleanParameter(GetFullParmName("OutputAfterRunShouldAccumulate"));
}


TsVScorer::~TsVScorer()
{;}


void TsVScorer::PostConstructor()
{
	if (fIsSurfaceScorer) {
		if (fPm->ParameterExists(GetFullParmName("Surface"))) {
			G4String volumePlusSurface = fPm->GetStringParameter(GetFullParmName("Surface"));
			size_t pos2 = volumePlusSurface.find_last_of("/");
			GetAppropriatelyBinnedCopyOfComponent(volumePlusSurface.substr(0,pos2));

			fSurfaceName = volumePlusSurface.substr(pos2+1);

		} else if (fPm->ParameterExists(GetFullParmName("Component"))) {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "Scorer name: " << GetName() << " has no Surface specified." << G4endl;
			G4cerr << "If this is a user volume scorer, remove SetSurfaceScorer() from its constructor." << G4endl;
			fPm->AbortSession(1);
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "Scorer name: " << GetName() << " has no Surface specified." << G4endl;
			fPm->AbortSession(1);
		}

		// If scorer needs surface area calculation, check that component provides this feature
		if (fNeedsSurfaceAreaCalculation &! fComponent->CanCalculateSurfaceArea()) {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "Scorer name: " << GetName() << " requires surface area calculation." << G4endl;
			G4cerr << "But the component: " << fComponent->GetName() << " is of a type" << G4endl;
			G4cerr << "that does not provide this calculation." << G4endl;
			fPm->AbortSession(1);
		}

		// Ask the component to convert the string surface name to an integer surface id.
		// The integer is needed because we must do very frequent surface id checks. String comparisons would waste time.
		fSurfaceID = fComponent->GetSurfaceID(fSurfaceName);

		if (fPm->ParameterExists(GetFullParmName("OnlyIncludeParticlesGoing"))) {
			G4String going = fPm->GetStringParameter(GetFullParmName("OnlyIncludeParticlesGoing"));
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(going);
#else
			going.toLower();
#endif
			if (going == "in")
				fOnlyIncludeParticlesGoingIn = true;
			else if (going == "out")
				fOnlyIncludeParticlesGoingOut = true;
			else {
				G4cerr << "Topas is exiting due to a serious error in scoring setup.";
				G4cerr << "Parameter " << GetFullParmName("OnlyIncludeParticlesGoing") << " has invalid value: "
				<< fPm->GetStringParameter(GetFullParmName("OnlyIncludeParticlesGoing")) << G4endl;
				G4cerr << "Only allowed values are in and out." << G4endl;
				fPm->AbortSession(1);
			}
		}
	}
	else { // volume scorer
		if (fPm->ParameterExists(GetFullParmName("Component"))) {
			GetAppropriatelyBinnedCopyOfComponent(fPm->GetStringParameter(GetFullParmName("Component")));

		} else if (fPm->ParameterExists(GetFullParmName("Surface"))) {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "Scorer name: " << GetName() << " has no Component specified." << G4endl;
			G4cerr << "If this is a user surface scorer, add SetSurfaceScorer() to its constructor." << G4endl;
			fPm->AbortSession(1);
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "Scorer name: " << GetName() << " has no Component specified." << G4endl;
			fPm->AbortSession(1);
		}

		if (fPm->ParameterExists(GetFullParmName("OnlyIncludeParticlesGoing"))) {
			G4cerr << "Topas is exiting due to a serious error in scoring setup.";
			G4cerr << "Parameter " << GetFullParmName("OnlyIncludeParticlesGoing") << " is not appropriate for volume scorers." << G4endl;
			fPm->AbortSession(1);
		}
	}

	for (std::map<G4String, TsVScorer*>::iterator it = fSubScorers.begin(); it != fSubScorers.end(); ++it)
		if (it->second->fIsSubScorer)
			it->second->PostConstructor();

	if (fPm->ParameterExists(GetFullParmName("SetBinToMinusOneIfNotInRTStructure")) &&
		fPm->GetBooleanParameter(GetFullParmName("SetBinToMinusOneIfNotInRTStructure"))) {
		if (fComponent->OriginalComponent()) {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "The parameter " << GetFullParmName("SetBinToMinusOneIfNotInRTStructure") << G4endl;
			G4cerr << "Can not be used when the scoring component is a parallel world copy." << G4endl;
			fPm->AbortSession(1);
		}
		fSetBinToMinusOneIfNotInRTStructure = true;
	}
}


void TsVScorer::GetAppropriatelyBinnedCopyOfComponent(G4String componentName)
{
	G4String componentNameLower = componentName;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(componentNameLower);
#else
	componentNameLower.toLower();
#endif
	if (componentNameLower == "world") {
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << GetName() << " is attempting to score in the World component." << G4endl;
		G4cerr << "This is the one component that can not have scorers." << G4endl;
		fPm->AbortSession(1);
	}

	G4String componentTypeString = "Ge/"+componentName+"/Type";
	G4String componentType = fPm->GetStringParameterWithoutMonitoring(componentTypeString);
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(componentType);
#else
	componentType.toLower();
#endif
	if (componentType == "group") {
		if (fIsSurfaceScorer) {
			G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
			G4cerr << GetName() << " is attempting to set up a Surface Scorer in the Group component: " << componentName << G4endl;
			G4cerr << "Group components do not have any surfaces." << G4endl;
			fPm->AbortSession(1);
		} else if (!(fPm->ParameterExists(GetFullParmName("PropagateToChildren"))) ||
				   !(fPm->GetBooleanParameter(GetFullParmName("PropagateToChildren")))) {
			G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
			G4cerr << GetName() << " is attempting to set up Volume Scorer in the Group component: " << componentName << G4endl;
			G4cerr << "Group components can only have Volume Scoring if the scorer also has PropagateToChildren" << G4endl;
			G4cerr << "since a Group comonent has no volume of its own." << G4endl;
			fPm->AbortSession(1);
		}
	}

	fComponent = fGm->GetComponent(componentName);
	if (!fComponent) {
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << GetName() << " has unfound component:" << componentName << G4endl;
		fPm->AbortSession(1);
	}

	// Find out what spatial binning has been requested. Default to the natural binning of the component.
	G4String binParmName;

	binParmName = fComponent->GetDivisionName(0) + "Bins";
	if (fPm->ParameterExists(GetFullParmName(binParmName))) {
		fNi = fPm->GetIntegerParameter(GetFullParmName(binParmName));
		if (fNi <= 0) OutOfRange( GetFullParmName(binParmName), "must be larger than zero");
	}
	else
		fNi = fComponent->GetDivisionCount(0);

	binParmName = fComponent->GetDivisionName(1) + "Bins";
	if (fPm->ParameterExists(GetFullParmName(binParmName))) {
		fNj = fPm->GetIntegerParameter(GetFullParmName(binParmName));
		if (fNj <= 0) OutOfRange( GetFullParmName(binParmName), "must be larger than zero");
	}
	else
		fNj = fComponent->GetDivisionCount(1);

	binParmName = fComponent->GetDivisionName(2) + "Bins";
	if (fPm->ParameterExists(GetFullParmName(binParmName))) {
		fNk = fPm->GetIntegerParameter(GetFullParmName(binParmName));
		if (fNk <= 0) OutOfRange( GetFullParmName(binParmName), "must be larger than zero");
	}
	else
		fNk = fComponent->GetDivisionCount(2);

	G4long testInLong = (G4long)fNi * (G4long)fNj * (G4long)fNk;

	if (testInLong > INT_MAX) {
		G4cerr << "Component " << GetName() << " has too many divisions." << G4endl;
		G4cerr << "Maximum allowed total number is " << INT_MAX << G4endl;
		G4cerr << "Number of Divisions was found to be: " << testInLong << G4endl;
		fPm->AbortSession(1);
	}

	fNDivisions = testInLong;

	// If there is an appropriately binned parallel copy of the component, use that instead
	G4String nameWithCopyId = componentName + fGm->GetCopyIdFromBinning(fNi, fNj, fNk);
	TsVGeometryComponent* parallelComponent = fGm->GetComponent(nameWithCopyId);
	if (parallelComponent)
		fComponent = parallelComponent;

	// Store component name including any copy Id so that if the geometry gets rebuilt, we can restore
	fComponentName = fComponent->GetNameWithCopyId();

	fDetector = fScm->GetDetector(fComponentName, this);
}


void TsVScorer::ResolveParameters()
{
	if (fVerbosity>0)
		G4cout << "TsVScorer::ResolveParameters" << G4endl;

	if (fPm->ParameterExists(GetFullParmName("Active")))
		fIsActive = fPm->GetBooleanParameter(GetFullParmName("Active"));
	else
		fIsActive = true;

	// If this scorer is a split, active is also affected by whether the split condition has been met
	if (fSplitId != "") {
		if (fSplitFunction == "step") {
			if (fSplitUnitLower == "string")
				fIsActive = fIsActive && fPm->GetStringParameter(fSplitParmValueName) == fSplitStringValue;
			else if (fSplitUnitLower == "boolean")
				fIsActive = fIsActive && fPm->GetBooleanParameter(fSplitParmValueName) == fSplitBooleanValue;
			else if (fSplitUnitLower == "integer")
				fIsActive = fIsActive && fPm->GetIntegerParameter(fSplitParmValueName) == fSplitLowerValue;
			else if (fSplitUnitLower == "")
				fIsActive = fIsActive && fPm->GetUnitlessParameter(fSplitParmValueName) == fSplitLowerValue;
			else
				fIsActive = fIsActive && fPm->GetDoubleParameter(fSplitParmValueName, fSplitUnitCategory) == fSplitLowerValue;
		} else {
			if (fSplitUnitLower == "integer") {
				G4double value = fPm->GetIntegerParameter(fSplitParmValueName);
				fIsActive = fIsActive && value >= fSplitLowerValue && value < fSplitUpperValue;
			} else if (fSplitUnitLower == "") {
				G4double value = fPm->GetUnitlessParameter(fSplitParmValueName);
				fIsActive = fIsActive && value >= fSplitLowerValue && value < fSplitUpperValue;
			} else {
				G4double value = fPm->GetDoubleParameter(fSplitParmValueName, fSplitUnitCategory);
				fIsActive = fIsActive && value >= fSplitLowerValue && value < fSplitUpperValue;
			}
		}
	}

	CacheGeometryPointers();

	for (std::map<G4String, TsVScorer*>::iterator it = fSubScorers.begin(); it != fSubScorers.end(); ++it)
		if (it->second->fIsSubScorer)
			it->second->ResolveParameters();
}


void TsVScorer::CacheGeometryPointers()
{
	if (fVerbosity>0)
		G4cout << "TsVScorer::CacheGeometryPointers" << G4endl;
	if (fIsActive) {
		fComponent = fGm->GetComponent(fComponentName);

		fPropagateToChildren = false;
		if (!fComponent->IsCopy() && fPm->ParameterExists(GetFullParmName("PropagateToChildren")))
			fPropagateToChildren = fPm->GetBooleanParameter(GetFullParmName("PropagateToChildren"));

        if (fComponent->GetLogicalVolumesToBeSensitive().empty()) {
			std::vector<G4VPhysicalVolume*> allDaughters = fComponent->GetAllPhysicalVolumes(fPropagateToChildren);
			for ( size_t t = 0; t < allDaughters.size(); t++ )
				allDaughters[t]->GetLogicalVolume()->SetSensitiveDetector(detector);
        } else {
			std::vector<G4LogicalVolume*> allSensitiveDaughters = fComponent->GetLogicalVolumesToBeSensitive();
			for ( size_t t = 0; t < allSensitiveDaughters.size(); t++ )
				allSensitiveDaughters[t]->SetSensitiveDetector(detector);
        }
    }
}


void TsVScorer::UpdateForSpecificParameterChange(G4String parameter)
{
	if (fVerbosity>0)
		G4cout << "TsVScorer::UpdateForParameterChange for parameter: " << parameter << G4endl;
	fHadParameterChangeSinceLastRun = true;
}


G4int TsVScorer::CallCombineOnSubScorers()
{
	G4int missedSubScorerVoxels = 0;

	for (std::map<G4String, TsVScorer*>::iterator it = fSubScorers.begin(); it != fSubScorers.end(); ++it)
		missedSubScorerVoxels += it->second->CallCombineOnSubScorers();

	if (!fHasCombinedSubScorers) {
		missedSubScorerVoxels += CombineSubScorers();
		fHasCombinedSubScorers = true;
	}

	return missedSubScorerVoxels;
}


void TsVScorer::UpdateForEndOfRun()
{
	fHaveIncidentParticle = false;
	UserHookForEndOfRun();

	if (fOutputAfterRun)
		Output();
}


void TsVScorer::PostUpdateForEndOfRun()
{
	if (fOutputAfterRun && !fOutputAfterRunShouldAccumulate)
		Clear();
}


void TsVScorer::Finalize()
{
	// If output wasn't triggered by OutputAfterRun option, output now
	if (!fOutputAfterRun)
		Output();
}


void TsVScorer::PostFinalize()
{
	if (!fOutputAfterRun)
		Clear();
}


void TsVScorer::UpdateForNewRun(G4bool rebuiltSomeComponents)
{
	if (fVerbosity>0) {
		G4cout << "TsVScorer::UpdateForNewRun for scorer: " << GetName() << " called with fHadParameterChangeSinceLastRun: " <<
		fHadParameterChangeSinceLastRun << ", rebuiltSomeComponents: " << rebuiltSomeComponents << G4endl;
	}

	fCachedCubicVolume = -1;

	fHasCombinedSubScorers = false;

	if (fHadParameterChangeSinceLastRun) {
		ResolveParameters();
		fHadParameterChangeSinceLastRun = false;
	} else if (rebuiltSomeComponents) {
		CacheGeometryPointers();
	}
}


void TsVScorer::UpdateFileNameForUpcomingRun()
{
	if (!fOutFile)
		return;

	if (fPm->GetIntegerParameter("Tf/NumberOfSequentialTimes") > 1 && fOutputAfterRun && !fPm->IsRandomMode()) {

		G4int nPadding = fPm->GetIntegerParameter("Ts/RunIDPadding");

		G4int runID = GetRunID();
		runID++;

		std::ostringstream runString;
		runString << "_Run_" << std::setw(nPadding) << std::setfill('0') << runID;

		fOutFile->SetFileName(fOutFileName + runString.str());
	} else {
		fOutFile->SetFileName(fOutFileName);
	}
}


void TsVScorer::SetRTStructureFilter(TsFilterByRTStructure* aFilter)
{
	fRTStructureFilter = aFilter;
}


void TsVScorer::SetSplitInfo(G4String splitParmValueName, G4String splitId, G4String splitFunction, G4String splitUnitLower, G4String splitUnitCategory,
							 G4String splitStringValue, G4bool splitBooleanValue, G4double splitLowerValue, G4double splitUpperValue)
{
	fSplitParmValueName = splitParmValueName;
	fSplitId = splitId;
	fSplitFunction = splitFunction;
	fSplitUnitLower = splitUnitLower;
	fSplitUnitCategory = splitUnitCategory;
	fSplitStringValue = splitStringValue;
	fSplitBooleanValue = splitBooleanValue;
	fSplitLowerValue = splitLowerValue;
	fSplitUpperValue = splitUpperValue;

	for (std::map<G4String, TsVScorer*>::iterator it = fSubScorers.begin(); it != fSubScorers.end(); ++it)
		if (it->second->fIsSubScorer)
			it->second->SetSplitInfo(splitParmValueName, splitId, splitFunction, splitUnitLower, splitUnitCategory, splitStringValue, splitBooleanValue, splitLowerValue, splitUpperValue);
}


void TsVScorer::SetFilter(TsVFilter* aFilter){
	G4VPrimitiveScorer::SetFilter(aFilter);

	for (std::map<G4String, TsVScorer*>::iterator it = fSubScorers.begin(); it != fSubScorers.end(); ++it)
		if (it->second->fIsSubScorer)
			it->second->SetFilter(aFilter);
}


// See if this index number is in any of the specified structures
G4bool TsVScorer::ExcludedByRTStructFilter(G4int idx) {
	if (fRTStructureFilter && fSetBinToMinusOneIfNotInRTStructure) {
		for (G4int i = 0; i < (int)fRTStructureFilter->GetStructureIDs()->size(); i++) {
			G4int j = (*(fRTStructureFilter->GetStructureIDs()))[i];
			if (fComponent->IsInNamedStructure(j, idx)) {
				if (fRTStructureFilter->IsInverted()) return true;
				else return false;
			}
		}

		if (fRTStructureFilter->IsInverted()) return false;
		else return true;
	} else {
		return false;
	}
}


G4String TsVScorer::GetFullParmName(const char* parmName) {
	G4String fullName = "Sc/" + GetName() + "/"+parmName;
	return fullName;
}


G4String TsVScorer::GetFullParmNameLower(const char* parmName) {
	G4String fullName = GetFullParmName(parmName);
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(fullName);
#else
	fullName.toLower();
#endif
	return fullName;
}


G4String TsVScorer::GetNameWithSplitId()
{
	return GetName() + fSplitId;
}


void TsVScorer::SetSurfaceScorer()
{
	fIsSurfaceScorer = true;
}


void TsVScorer::SetNeedsSurfaceAreaCalculation()
{
	fNeedsSurfaceAreaCalculation = true;
}


void TsVScorer::SuppressStandardOutputHandling()
{
	fSuppressStandardOutputHandling = true;
}


G4double TsVScorer::GetTime()
{
	return fPm->GetCurrentTime();
}


G4int TsVScorer::GetRunID()
{
	return fPm->GetRunID();
}


G4int TsVScorer::GetEventID()
{
	const G4Event* currentEvent = G4RunManager::GetRunManager()->GetCurrentEvent();
	if (currentEvent)
		return currentEvent->GetEventID();
	else
		return 0;
}


const G4String& TsVScorer::GetRandomNumberStatusForThisEvent()
{
	return G4RunManager::GetRunManager()->GetRandomNumberStatusForThisEvent();
}


void TsVScorer::ClearIncidentParticleInfo()
{
	if (fHaveIncidentParticle)
		UserHookForEndOfIncidentParticle();

	fHaveIncidentParticle = false;

	if (fTrackingVerbosity>0)
		G4cout << "ClearIncidentParticleInfo called" << G4endl;
}


void TsVScorer::NoteIncidentParticleInfo(const G4Step* aStep)
{
	if (fHaveIncidentParticle)
		UserHookForEndOfIncidentParticle();

	fHaveIncidentParticle = true;

	fIncidentParticleEnergy = aStep->GetPreStepPoint()->GetKineticEnergy();
	fIncidentParticleMomentum = aStep->GetPreStepPoint()->GetMomentum().mag();
	fIncidentParticleCreatorProcess = aStep->GetTrack()->GetCreatorProcess();
	fIncidentParticleDefinition = aStep->GetTrack()->GetDefinition();
	fIncidentParticleCharge = aStep->GetTrack()->GetDynamicParticle()->GetCharge();
	fIncidentParticleParentID = aStep->GetTrack()->GetParentID();

	if (fTrackingVerbosity>0)
		G4cout << "NoteIncidentParticleInfo called for step: " << aStep <<
		", fIncidentParticleEnergy = " << fIncidentParticleEnergy <<
		", fIncidentParticleMomentum = " << fIncidentParticleMomentum << G4endl;
}


void TsVScorer::NoteHitHadNoIncidentParticle()
{
	fHitsWithNoIncidentParticle++;
}


G4bool TsVScorer::HaveIncidentParticle()
{
	return fHaveIncidentParticle;
}


G4double TsVScorer::GetIncidentParticleEnergy()
{
	return fIncidentParticleEnergy;
}


G4double TsVScorer::GetIncidentParticleMomentum()
{
	return fIncidentParticleMomentum;
}


const G4VProcess* TsVScorer::GetIncidentParticleCreatorProcess()
{
	return fIncidentParticleCreatorProcess;
}


G4ParticleDefinition* TsVScorer::GetIncidentParticleDefinition()
{
	return fIncidentParticleDefinition;
}


G4int TsVScorer::GetIncidentParticleCharge()
{
	return fIncidentParticleCharge;
}


G4int TsVScorer::GetIncidentParticleParentID()
{
	return fIncidentParticleParentID;
}


G4long TsVScorer::GetScoredHistories()
{
	return fScoredHistories;
}


void TsVScorer::InstantiateSubScorer(G4String quantityName, G4String outFileName, G4String identifier, G4bool cloneParameters)
{
	G4String subScorerKey = identifier.empty() ? quantityName : identifier;

	if (fSubScorers.count(subScorerKey) > 0) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Scorer " << GetName() << " attempted to instantiate multiple" << G4endl;
		G4cerr << quantityName << " sub-scorers with the identifier " << subScorerKey << G4endl;
		fPm->AbortSession(1);
	}

	if (fPm->ParameterExists(GetFullParmName(G4String("ReferencedSubScorer_" + subScorerKey))))
	{
		fReferencedSubScorers[subScorerKey] = quantityName;
	}
	else
	{
		G4String subScorerName = GetName() + "_" + subScorerKey;

		#ifdef TOPAS_MT
		if (!G4Threading::IsWorkerThread()) {
		#endif
			if (fPm->ParameterExists("Sc/" + subScorerName + "/Quantity")) {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << "Scorer " << GetName() << " attempted to instantiate a " << quantityName << " sub-scorer" << G4endl;
				G4cerr << "called " << subScorerName << ", but this scorer already exists." << G4endl;
				fPm->AbortSession(1);
			}
		#ifdef TOPAS_MT
		}
		#endif

		if (cloneParameters)
		{
			std::vector<G4String>* parameterNames = new std::vector<G4String>;
			fPm->GetParameterNamesStartingWith(GetFullParmName(""), parameterNames);
			for (std::vector<G4String>::const_iterator param = parameterNames->begin(); param != parameterNames->end(); ++param)
			{
				if ((fPm->GetPartAfterLastSlash(*param) != "quantity") &&
					(fPm->GetPartAfterLastSlash(*param) != "outputtype") &&
					(fPm->GetPartAfterLastSlash(*param) != "histogrambins") &&
					(fPm->GetPartAfterLastSlash(*param) != "histogrammax") &&
					(fPm->GetPartAfterLastSlash(*param) != "histogrammin")) {
					G4String newParamName = *param;
					newParamName.erase(0, GetFullParmName("").length());
					newParamName.insert(0, "Sc/" + subScorerName + "/");
					if (!fPm->ParameterExists(newParamName))
						fPm->CloneParameter(*param, newParamName);
				}
			}
			delete parameterNames;
		}
		fSubScorers[subScorerKey] = fScm->InstantiateScorer(subScorerName, quantityName, outFileName, true);
		fScm->SetCurrentScorer(this);
	}
}


void TsVScorer::LinkReferencedSubScorers()
{
	for (std::map<G4String, G4String>::const_iterator it = fReferencedSubScorers.begin(); it != fReferencedSubScorers.end(); ++it)
	{
		G4String subScorerKey = it->first;
		G4String quantityName = it->second;
		G4String subScorerName = fPm->GetStringParameter(GetFullParmName(G4String("ReferencedSubScorer_" + subScorerKey)));
		TsVScorer* subScorer = NULL;

		// Find the referenced sub-scorer with the same split info
		std::vector<TsVScorer*> matchedScorers = fScm->GetMasterScorersByName(subScorerName);
		for (std::vector<TsVScorer*>::iterator it2 = matchedScorers.begin(); it2 != matchedScorers.end(); ++it2) {
			if ((*it2)->GetNameWithSplitId() == G4String(subScorerName + fSplitId) &&
				(*it2)->fOutputAfterRun == fOutputAfterRun) {
				subScorer = *it2;
				break;
			}
		}
		if (!subScorer) {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "Unable to find " << GetFullParmName(G4String("ReferencedSubScorer_" + subScorerKey)) << ": " << subScorerName << G4endl;
			if (matchedScorers.size() > 0)
				G4cerr << "This may be because SplitByTimeFeature and/or OutputAfterRun" << G4endl;
				G4cerr << "is used inconsistently between the two scorers." << G4endl;
			fPm->AbortSession(1);
		}
		if (subScorer->GetQuantity() != quantityName) {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << GetFullParmName(G4String("ReferencedSubScorer_" + subScorerKey)) << " is not a " << quantityName << " scorer." << G4endl;
			fPm->AbortSession(1);
		}

		fSubScorers[subScorerKey] = subScorer;
	}
}


TsVScorer* TsVScorer::GetSubScorer(const G4String& key)
{
	std::map<G4String, TsVScorer*>::iterator it = fSubScorers.find(key);
	if (it == fSubScorers.end()) {
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << "Unable to find " << key << " sub-scorer for scorer: " << GetName() << G4endl;
		fPm->AbortSession(1);
	}
	return it->second;
}


void TsVScorer::ResolveSolid(G4Step* aStep) {
	// Find physical volume
	G4VPhysicalVolume* physVol = aStep->GetPreStepPoint()->GetPhysicalVolume();
	G4VPVParameterisation* physParam = physVol->GetParameterisation();

	if (physParam)
	{
		// Parameterized volume
		G4int idx = ((G4TouchableHistory*)(aStep->GetPreStepPoint()->GetTouchable()))
		->GetReplicaNumber(indexDepth);

		// If non-physical value, we'll catch this later, in AccumulateHit.
		// For now, have those calculations use index 0 so that downstream calculations at least have something to work with.
		if (idx<0) idx = 0;

		fSolid = physParam->ComputeSolid(idx, physVol);
		fSolid->ComputeDimensions(physParam,idx,physVol);
	}
	else
	{
		if (fComponent->IsParameterized())
			fPm->GetSequenceManager()->NoteParameterizationError(aStep->GetTotalEnergyDeposit(), fComponent->GetNameWithCopyId(),
																 aStep->GetPreStepPoint()->GetTouchable()->GetVolume()->GetName());

		// Undivided volume
		fSolid = physVol->GetLogicalVolume()->GetSolid();
	}
}


G4double TsVScorer::GetCubicVolume(G4Step* aStep) {
	// Volume can be taken from previously cached value unless this is a divided volume
	// that has different volume per division (such as cylinder and sphere).
	// Cache is also cleared after any update for time features in case volume has changed.
	if (((fNDivisions > 1) && (fComponent->HasDifferentVolumePerDivision())) || fCachedCubicVolume == -1.) {
		// For undivided components, volume comes from component.
		// For divided components, volume comes from the division (the current fSolid).
		if (fNDivisions==1)
			fCachedCubicVolume = fComponent->GetCubicVolume();
		else
			fCachedCubicVolume = fSolid->GetCubicVolume();

		// If this scorer will not be sensitive to hits in its children,
		// then the cubic volume should exclude the volume of those children.
		if (!fPropagateToChildren) {
			G4LogicalVolume* lvol = aStep->GetPreStepPoint()->GetPhysicalVolume()->GetLogicalVolume();
			for (size_t iChild = 0; iChild < lvol->GetNoDaughters(); iChild++)
				fCachedCubicVolume -= lvol->GetDaughter(iChild)->GetLogicalVolume()->GetSolid()->GetCubicVolume();
		}
	}

	return fCachedCubicVolume;
}


G4int TsVScorer::GetIndex(G4Step* aStep)
{
	return fComponent->GetIndex(aStep);
}


G4int TsVScorer::GetBin(G4int index, G4int iBin) {
	if (index < 0) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "The scorer named " << GetName() << " is calling GetBin with a negative index." << G4endl;
		fPm->AbortSession(1);
	}

	if (iBin < 0 || iBin > 2) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "The scorer named " << GetName() << " is calling GetBin with an incorrect bin number." << G4endl;
		G4cerr << "The bin number can only be 0, 1 or 2." << G4endl;
		fPm->AbortSession(1);
	}

	return fComponent->GetBin(index, iBin);
}


G4Material* TsVScorer::GetMaterial(G4String name) {
	return fMm->GetMaterial(name);
}


G4Material* TsVScorer::GetMaterial(const char* c) {
	return fMm->GetMaterial(c);
}


G4bool TsVScorer::IsSelectedSurface(G4Step* theStep)
{
	if (!fIsSurfaceScorer) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Scorer name: " << GetName() << " is a volume scorer" << G4endl;
		G4cerr << "but uses the method IsSelectedSurface()" << G4endl;
		fPm->AbortSession(1);
	}

	G4TouchableHandle theTouchable = theStep->GetPreStepPoint()->GetTouchableHandle();
	/*G4cout << "In IsSelectedSurface, touchable: " << theTouchable << G4endl;
	G4cout << "PreStepPoint Status: " << theStep->GetPreStepPoint()->GetStepStatus() << G4endl;
	G4cout << "PostStepPoint Status: " << theStep->GetPostStepPoint()->GetStepStatus() << G4endl;
	G4cout << "PreStepPoint Material: " << theStep->GetPreStepPoint()->GetMaterial() << G4endl;
	G4cout << "PostStepPoint Material: " << theStep->GetPostStepPoint()->GetMaterial() << G4endl;
	G4int test = fGeomBoundary;
	G4cout << "status for fGeomBoundary: " << test << G4endl;
	if (!fOnlyIncludeParticlesGoingIn) G4cout << "Allowed to score going out" << G4endl;
	*/

	// Remember that the same step may have PreStepPoint on one boundary of the sensitive volume and
	// PostStepPoint on the other boundary of the same volume.

	// If we want to score particles that are going out, and the post step is on a boundary, we may want to score this.
	if (!fOnlyIncludeParticlesGoingIn && theStep->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) {
		// OK, we know the particle is going out of some surface. Now ask if it is the correct surface.
		// Get the exit point's position in the local coordinate system of our sensitive volume
		G4ThreeVector stppos = theStep->GetPostStepPoint()->GetPosition();
		G4ThreeVector localpos = theTouchable->GetHistory()->GetTopTransform().TransformPoint(stppos);

		// Ask our component whether this position is on the relevant surface
		if (fComponent->IsOnBoundary(localpos, fSolid, fSurfaceID)) {
			fDirection = fFlux_Out;
			return true;
		}
	}
	//if (!fOnlyIncludeParticlesGoingOut) G4cout << "Allowed to score going in" << G4endl;

	// If we want to score particles that are going in, and the pre step is on a boundary, score this.
	if (!fOnlyIncludeParticlesGoingOut && theStep->GetPreStepPoint()->GetStepStatus() == fGeomBoundary) {
		// OK, we know the particle is going in to some surface. Now ask if it is the correct surface.
		// Get the entry point's position in the local coordinate system of our sensitive volume
		G4ThreeVector stppos = theStep->GetPreStepPoint()->GetPosition();
		G4ThreeVector localpos = theTouchable->GetHistory()->GetTopTransform().TransformPoint(stppos);

		// Ask our component whether this position is on the relevant surface
		if (fComponent->IsOnBoundary(localpos, fSolid, fSurfaceID)) {
			fDirection = fFlux_In;
			return true;
		}
	}
	return false;
}


G4double TsVScorer::GetAreaOfSelectedSurface(G4Step* aStep)
{
	if (!fIsSurfaceScorer) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Scorer name: " << GetName() << " is a volume scorer" << G4endl;
		G4cerr << "but uses the method GetAreaOfSelectedSurface()" << G4endl;
		fPm->AbortSession(1);
	}

	G4int i;
	G4int j;
	G4int k;

	if (fNDivisions > 1)
	{
		const G4VTouchable* touchable = aStep->GetPreStepPoint()->GetTouchable();
		i = touchable->GetReplicaNumber(2);
		j = touchable->GetReplicaNumber(1);
		k = touchable->GetReplicaNumber(0);
	} else {
		i = 1;
		j = 1;
		k = 1;
	}

	return fComponent->GetAreaOfSelectedSurface(fSolid, fSurfaceID, i, j, k);
}


G4String TsVScorer::GetSurfaceName()
{
	return fSurfaceName;
}


G4int TsVScorer::GetDirection()
{
	return fDirection;
}


void TsVScorer::OutOfRange(G4String parameterName, G4String requirement)
{
	G4cerr << "Topas is exiting due to a serious error." << G4endl;
	G4cerr << parameterName << " " << requirement << G4endl;
	fPm->AbortSession(1);
}
