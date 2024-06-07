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

#include "TsScoringManager.hh"

#include "TsGeometryManager.hh"
#include "TsFilterManager.hh"

#include "TsScoringHub.hh"
#include "TsVScorer.hh"
#include "TsVFilter.hh"
#include "TsVGeometryComponent.hh"

#include "G4UIcommand.hh"
#include "G4Tokenizer.hh"
#if GEANT4_VERSION_MAJOR >= 11
#include "g4hntools_defs.hh"
#include "G4ToolsAnalysisManager.hh"
#include "G4AnalysisManager.hh"
#else
#include "g4analysis_defs.hh"
#endif

#ifdef TOPAS_MT
#include "G4Threading.hh"
#endif

TsScoringManager::TsScoringManager(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsFilterManager* fM)
:fPm(pM), fEm(eM), fMm(mM), fGm(gM), fFm(fM),
fAddUnitEvenIfItIsOne(false), fRootAnalysisManager(0), fXmlAnalysisManager(0), fUID(0)
{
#ifdef TOPAS_MT
	fCurrentScorerName.Put("");
	fCurrentScorer.Put(0);
#else
	fCurrentScorerName = "";
	fCurrentScorer = 0;
#endif

	fVerbosity = fPm->GetIntegerParameter("Sc/Verbosity");
	fTfVerbosity = fPm->GetIntegerParameter("Tf/Verbosity");

	fAddUnitEvenIfItIsOne = fPm->GetBooleanParameter("Sc/AddUnitEvenIfItIsOne");

	// Create the store for the G4MultiFunctionalDetectors
	fDetectors = new std::map<G4String,G4MultiFunctionalDetector*>;

	// Instantiate the scoringHub, used to link in the specfic scorers
	fScoringHub = new TsScoringHub(fPm);

	// GeometryManager needs pointer back so it can call ScoringManager::Initialize from ConstructSDandField
	fGm->SetScoringManager(this);
}


TsScoringManager::~TsScoringManager()
{
	delete fDetectors;
}


// Called per thread by TsGeometryManager::ConstructSDandField
void TsScoringManager::Initialize()
{
	#ifdef TOPAS_MT
		static G4ThreadLocal G4bool fAlreadyInitialized = false;
	#else
		static G4bool fAlreadyInitialized = false;
	#endif

	if (fAlreadyInitialized) {
		std::vector<TsVScorer*>::iterator iter;
		std::vector<G4int>::iterator iter2 = fWorkerScorerThreadIDs.begin();
		for (iter=fWorkerScorers.begin(); iter!=fWorkerScorers.end(); iter++) {
			if ((*iter2) == G4Threading::G4GetThreadId())
				(*iter)->CacheGeometryPointers();
			iter2++;
		}
		return;
	}
	fAlreadyInitialized = true;

	// Reset unique ID counter, so UIDs agree across threads
	fUID = 0;

	// Error on misspecified scorers (using Type instead of Quantity)
	std::vector<G4String>* badScorers = new std::vector<G4String>;
	fPm->GetParameterNamesBracketedBy("Sc", "/Type", badScorers);
	for (G4int iToken=0; iToken<(G4int)badScorers->size(); iToken++) {
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << "The parameter name: " << (*badScorers)[iToken] << G4endl;
		G4cerr << "should end with Quantity rather than Type." << G4endl;
		fPm->AbortSession(1);
	}

	// Loop over all defined scorers
	G4String prefix = "Sc";
	G4String suffix = "/Quantity";
	std::vector<G4String>* values = new std::vector<G4String>;
	fPm->GetParameterNamesBracketedBy(prefix, suffix, values);
	G4int length = values->size();

	for (G4int iToken=0; iToken<length; iToken++) {
		fQuantityParmName = (*values)[iToken];
		G4String quantityName = fPm->GetStringParameter(fQuantityParmName);

		G4String quantityParmNameLower = fQuantityParmName;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(quantityParmNameLower);
#else
		quantityParmNameLower.toLower();
#endif
		size_t pos1 = quantityParmNameLower.find("/quantity");
		G4String outFileName = fQuantityParmName.substr(3, pos1-3);
#ifdef TOPAS_MT
		fCurrentScorerName.Put(outFileName);
#else
		fCurrentScorerName = outFileName;
#endif

		if (fPm->ParameterExists(GetFullParmName("OutputFile")))
			outFileName = fPm->GetStringParameter(GetFullParmName("OutputFile"));

#ifdef TOPAS_MT
		if (!G4Threading::IsWorkerThread()) {
#endif
			// Protect against using the same file name for two different scorers.
			G4String testName = outFileName;
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(testName);
#else
			testName.toLower();
#endif
			std::vector<G4String>::iterator iter;
			for (iter=fOutFileNames.begin(); iter!=fOutFileNames.end(); iter++)
				if (*iter==testName) {
					G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
					G4cerr << "Two scorers are using the same output file name: " << outFileName << G4endl;
					fPm->AbortSession(1);
				}
			fOutFileNames.push_back(testName);
#ifdef TOPAS_MT
		}
#endif

		G4String quantityNameLower = quantityName;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(quantityNameLower);
#else
		quantityNameLower.toLower();
#endif

		// If scorer does not have Sc/MyScorer/SplitByTimeFeature, create a single scorer as usual.
		// If scorer does have Sc/MyScorer/SplitByTimeFeature, loop to create multiple scorers, each with own fCopyId.
		G4int nSplits = 1;
		G4String splitParmValueName = "";
		G4String splitFunction = "";
		G4String splitUnit = "";
		G4String splitUnitLower = "";
		G4String splitUnitCategory = "";
		G4String* splitStringValues = 0;
		G4bool* splitBooleanValues = 0;
		G4double* splitDoubleValues = 0;
		G4int* splitIntegerValues = 0;
		G4String splitStringValue;
		G4bool splitBooleanValue = false;
		G4double splitLowerValue = 0.;
		G4double splitUpperValue = 0.;

		G4String splitParmName = GetFullParmName("SplitByTimeFeature");
		if (fPm->ParameterExists(splitParmName)) {
			G4String timeFeatureName = fPm->GetStringParameter(splitParmName);
			G4String timeFeatureFunctionParmName = "Tf/" + timeFeatureName + "/Function";
			splitParmValueName = "Tf/" + timeFeatureName + "/Value";
			G4String timeFeatureFunction = fPm->GetStringParameter(timeFeatureFunctionParmName);
			G4Tokenizer next(timeFeatureFunction);
			splitFunction = next();
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(splitFunction);
#else
			splitFunction.toLower();
#endif

			if (splitFunction == "step") {
				// If time feature is a step feature, will create one scorer for each of the time feature's step values.
				G4String timeFeatureValueParamName = "Tf/" + timeFeatureName + "/Values";
				nSplits = fPm->GetVectorLength(timeFeatureValueParamName);
				G4String timeFeatureValueType = fPm->GetTypeOfParameter(timeFeatureValueParamName);
				if (timeFeatureValueType == "sv") {
					splitStringValues = fPm->GetStringVector(timeFeatureValueParamName);
					splitUnitLower = "string";
				} else if (timeFeatureValueType == "bv") {
					splitBooleanValues = fPm->GetBooleanVector(timeFeatureValueParamName);
					splitUnitLower = "boolean";
				} else if (timeFeatureValueType == "iv") {
					splitIntegerValues = fPm->GetIntegerVector(timeFeatureValueParamName);
					splitUnitLower = "integer";
				} else if (timeFeatureValueType == "uv") {
					splitDoubleValues = fPm->GetUnitlessVector(timeFeatureValueParamName);
					splitUnitLower = "";
				} else {
					splitUnit = fPm->GetUnitOfParameter(timeFeatureValueParamName);
					splitUnitLower = splitUnit;
#if GEANT4_VERSION_MAJOR >= 11
					G4StrUtil::to_lower(splitUnitLower);
#else
					splitUnitLower.toLower();
#endif
					splitUnitCategory = fPm->GetUnitCategory(splitUnit);
					splitDoubleValues = fPm->GetDoubleVector(timeFeatureValueParamName, splitUnitCategory);
				}
			} else {
				splitUnit = next();
				splitUnitLower = splitUnit;
#if GEANT4_VERSION_MAJOR >= 11
				G4StrUtil::to_lower(splitUnitLower);
#else
				splitUnitLower.toLower();
#endif

				// If time feature is not a step feature, will create one scorer for each specified range.
				G4String rangeParmName = GetFullParmName("SplitByTimeFeatureValues");
				nSplits = fPm->GetVectorLength(rangeParmName) - 1;
				if (splitUnitLower == "integer")
					splitIntegerValues = fPm->GetIntegerVector(rangeParmName);
				else if (splitUnitLower == "")
					splitDoubleValues = fPm->GetUnitlessVector(rangeParmName);
				else {
					splitUnitCategory = fPm->GetUnitCategory(splitUnit);
					splitDoubleValues = fPm->GetDoubleVector(rangeParmName, splitUnitCategory);
				}
			}

#ifdef TOPAS_MT
			G4cout << "The scorer " << fCurrentScorerName.Get() << " is being split into " << nSplits << " scorers controlled by Time Feature "
			<< timeFeatureName << " of type " << splitFunction << " " << splitUnit << G4endl;
#else
			G4cout << "The scorer " << fCurrentScorerName << " is being split into " << nSplits << " scorers controlled by Time Feature "
			<< timeFeatureName << " of type " << splitFunction << " " << splitUnit << G4endl;
#endif
		}

		for (G4int iSplit = 0; iSplit < nSplits; iSplit++) {
			G4String splitId = "";

			if (nSplits > 1) {
				// For split scorers, find control values and create copy ID
				if (splitFunction == "step") {
					if (splitUnitLower == "string") {
						splitStringValue = splitStringValues[iSplit];
						splitId += "-" + splitStringValue;
					} else if (splitUnitLower == "boolean") {
						splitBooleanValue = splitBooleanValues[iSplit];
						splitId += "-" + G4UIcommand::ConvertToString(splitBooleanValue);
					} else if (splitUnitLower == "integer") {
						splitLowerValue = splitIntegerValues[iSplit];
						splitId += "-" + G4UIcommand::ConvertToString(splitLowerValue);
					} else if (splitUnitLower == "") {
						splitLowerValue = splitDoubleValues[iSplit];
						splitId += "-" + G4UIcommand::ConvertToString(splitLowerValue);
					} else {
						splitLowerValue = splitDoubleValues[iSplit];
						splitId += "-" + G4UIcommand::ConvertToString(splitLowerValue / fPm->GetUnitValue(splitUnit)) + splitUnit;
					}
				} else {
					if (splitUnitLower == "integer") {
						splitLowerValue = splitIntegerValues[iSplit];
						splitUpperValue = splitIntegerValues[iSplit+1];
						splitId += "-" + G4UIcommand::ConvertToString(splitLowerValue) + "-" + G4UIcommand::ConvertToString(splitUpperValue);
					} else if (splitUnitLower == "") {
						splitLowerValue = splitDoubleValues[iSplit];
						splitUpperValue = splitDoubleValues[iSplit+1];
						splitId += "-" + G4UIcommand::ConvertToString(splitLowerValue) + "-" + G4UIcommand::ConvertToString(splitUpperValue);
					} else {
						splitLowerValue = splitDoubleValues[iSplit];
						splitUpperValue = splitDoubleValues[iSplit+1];
						splitId += "-" + G4UIcommand::ConvertToString(splitLowerValue / fPm->GetUnitValue(splitUnit)) + "-" + G4UIcommand::ConvertToString(splitUpperValue / fPm->GetUnitValue(splitUnit)) + splitUnit;
					}
				}
				std::replace(splitId.begin(), splitId.end(), '/', '_');
			}

			G4String fullOutFileName = outFileName + splitId;

#ifdef TOPAS_MT
			TsVScorer* scorer = InstantiateScorer(fCurrentScorerName.Get(), quantityName, fullOutFileName, false);
#else
			TsVScorer* scorer = InstantiateScorer(fCurrentScorerName, quantityName, fullOutFileName, false);
#endif

			if (nSplits > 1) {
				// For split scorers, send copy details to the scorer
				scorer->SetSplitInfo(splitParmValueName, splitId, splitFunction, splitUnitLower, splitUnitCategory,
									 splitStringValue, splitBooleanValue, splitLowerValue, splitUpperValue);

			}

	        // Checks that must wait until after scorer's CTor had opportunity to set SetSurfaceScorer(), SetUnit() and/or SuppressStandardOutputHandling()
			scorer->PostConstructor();

			// Could not call this from scorer's CTor because didn't have split info at that time.
			scorer->ResolveParameters();
		}
	}

	// Reset current scorer to zero to indicate that we are no longer building scorers.
	SetCurrentScorer(0);

	// Sub-scorers can be references to user-defined scorers
	for (std::vector<TsVScorer*>::iterator it = fMasterScorers.begin(); it != fMasterScorers.end(); ++it)
		(*it)->LinkReferencedSubScorers();
}


TsVScorer* TsScoringManager::InstantiateScorer(G4String scorerName, G4String quantityName, G4String outFileName, G4bool isSubScorer)
{
	TsVScorer* scorer = fScoringHub->InstantiateScorer(fPm, fEm, fMm, fGm, this, scorerName, quantityName, outFileName, isSubScorer);

	if (!scorer)
	{
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;

		if (isSubScorer)
			G4cerr << " A scorer has attempted to create a SubScorer with unknown Quantity value: " << quantityName << G4endl;
		else
			G4cerr << fQuantityParmName << " has unknown value: " << quantityName << G4endl;

		fPm->AbortSession(1);
	}

	return scorer;
}


void TsScoringManager::InstantiateFilters() {
	if (fVerbosity > 0)
		G4cout << "TsScoringManager::InstantiateFilters called" << G4endl;

	std::vector<TsVScorer*>::iterator iter;
	for (iter=fMasterScorers.begin(); iter!=fMasterScorers.end(); iter++)
		(*iter)->SetFilter(fFm->InstantiateFilter(*iter));

	for (iter=fWorkerScorers.begin(); iter!=fWorkerScorers.end(); iter++)
		(*iter)->SetFilter(fFm->InstantiateFilter(*iter));
}


// Find or create G4MultiFunctionalDetector for the given component, and associated it with the scorer
G4MultiFunctionalDetector* TsScoringManager::GetDetector(G4String componentName, TsVScorer* scorer) {
	G4String componentNameWithThreadID = componentName;
#ifdef TOPAS_MT
	componentNameWithThreadID += G4Threading::G4GetThreadId();
#endif

	// SDManager doesn't allow sensitive detector names to have slash in the name.
	G4String componentNameWithUnderscores = componentNameWithThreadID;
	std::replace(componentNameWithUnderscores.begin(), componentNameWithUnderscores.end(), '/', '_');

	std::map<G4String, G4MultiFunctionalDetector*>::const_iterator iter = fDetectors->find(componentNameWithThreadID);
	if (iter != fDetectors->end()) {
		iter->second->RegisterPrimitive(scorer);
		return iter->second;
	}

	G4MultiFunctionalDetector* detector = new G4MultiFunctionalDetector(componentNameWithUnderscores);
	(*fDetectors)[componentNameWithThreadID] = detector;
	detector->RegisterPrimitive(scorer);
	return detector;
}


TsExtensionManager* TsScoringManager::GetExtensionManager() {
	return fEm;
}


TsScoringHub* TsScoringManager::GetScoringHub() {
	return fScoringHub;
}


void TsScoringManager::SetCurrentScorer(TsVScorer* scorer) {
#ifdef TOPAS_MT
	fCurrentScorer.Put(scorer);
#else
	fCurrentScorer = scorer;
#endif
}

void TsScoringManager::RegisterScorer(TsVScorer* scorer) {
#ifdef TOPAS_MT
	if (scorer) {
		if (G4Threading::IsWorkerThread()) {
			fWorkerScorers.push_back(scorer);
			fWorkerScorerThreadIDs.push_back(G4Threading::G4GetThreadId());
		} else
			fMasterScorers.push_back(scorer);
	}
#else
	if (scorer)
		fMasterScorers.push_back(scorer);
#endif

	scorer->fUID = ++fUID;
}


void TsScoringManager::NoteAnyUseOfChangeableParameters(const G4String& name)
{
#ifdef TOPAS_MT
	if (fCurrentScorer.Get())
#else
	if (fCurrentScorer)
#endif
	{
		// Register use of this parameter for current scorer and LastDirectParameter.
		// LastDirectParameter is needed because we ultimately need to tell the scorer not the name of the changed parameter
		// but the name of the parameter the scorer direclty accesses (that was affected by the changed parameter).
		// We store strings rather than pointers because we permit a parmeter to be overridden by another parameter
		// of the same name.
		G4String directParm = fPm->GetLastDirectParameterName();
		G4String directParmLower = directParm;
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(directParmLower);
#else
			directParmLower.toLower();
#endif

#ifdef TOPAS_MT
		if (directParmLower == fCurrentScorer.Get()->GetFullParmNameLower("Component")) {
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "A scorer's Component has been set to depend on a time feature." << G4endl;
			G4cerr << "This is not permitted." << G4endl;
			fPm->AbortSession(1);
		}
#else
		if (directParmLower == fCurrentScorer->GetFullParmNameLower("Component")) {
			G4cerr << "Topas is exiting due to a serious error." << G4endl;
			G4cerr << "A scorer's Component has been set to depend on a time feature." << G4endl;
			G4cerr << "This is not permitted." << G4endl;
			fPm->AbortSession(1);
		}
#endif

		G4String binParmName;

		for (G4int i = 0; i < 3; i++) {
#ifdef TOPAS_MT
			if (fCurrentScorer.Get()->GetComponent()) {
				binParmName = fCurrentScorer.Get()->GetComponent()->GetDivisionName(i) + "Bins";
				if (directParmLower == fCurrentScorer.Get()->GetFullParmNameLower(binParmName)) {
					G4cerr << "Topas is exiting due to a serious error." << G4endl;
					G4cerr << "The " << binParmName << " parameter of component " << fCurrentScorer.Get()->GetNameWithSplitId() << " has been set to depend on a time feature." << G4endl;
					G4cerr << "This is not permitted." << G4endl;
					fPm->AbortSession(1);
				}
			}
#else
			if (fCurrentScorer->GetComponent()) {
				binParmName = fCurrentScorer->GetComponent()->GetDivisionName(i) + "Bins";
				if (directParmLower == fCurrentScorer->GetFullParmNameLower(binParmName)) {
					G4cerr << "Topas is exiting due to a serious error." << G4endl;
					G4cerr << "The " << binParmName << " parameter of component " << fCurrentScorer->GetNameWithSplitId() << " has been set to depend on a time feature." << G4endl;
					G4cerr << "This is not permitted." << G4endl;
					fPm->AbortSession(1);
				}
			}
#endif
		}

		// See if this parameter has already been registered as used by this scorer (otherwise if this parameter
		// is interrogated twice for the same scorer, the scorer would update twice for the same change).
		G4bool matched = false;
		G4String nameToLower = name;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(nameToLower);
#else
		nameToLower.toLower();
#endif
		std::multimap< G4String, std::pair< TsVScorer*,G4String> >::const_iterator iter;
		for (iter = fChangeableParameterMap.begin(); iter != fChangeableParameterMap.end() && !matched; iter++) {
			G4String gotParm = iter->first;
			TsVScorer* gotScorer = iter->second.first;
			G4String gotDirectParm = iter->second.second;
#ifdef TOPAS_MT
			if (gotParm==nameToLower && gotScorer==fCurrentScorer.Get() && gotDirectParm==directParm)
#else
			if (gotParm==nameToLower && gotScorer==fCurrentScorer && gotDirectParm==directParm)
#endif
				matched = true;
		}

#ifdef TOPAS_MT
		if (!matched) {
			fChangeableParameterMap.insert(std::make_pair(nameToLower, std::make_pair(fCurrentScorer.Get(), directParm)));
			if (fTfVerbosity > 0)
				G4cout << "Registered use of changeable parameter: " << name << "  by scorer: " << fCurrentScorer.Get()->GetNameWithSplitId() << G4endl;
		}

		if (matched && fTfVerbosity > 0)
			G4cout << "TsScoringManager::NoteAnyUse Again called with current scorer: " << fCurrentScorer.Get()->GetNameWithSplitId() << ", param: " << name <<
			", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
		else if (fTfVerbosity > 0)
			G4cout << "TsScoringManager::NoteAnyUse First called with current scorer: " << fCurrentScorer.Get()->GetNameWithSplitId() << ", param: " << name <<
			", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
#else
		if (!matched) {
			fChangeableParameterMap.insert(std::make_pair(nameToLower, std::make_pair(fCurrentScorer, directParm)));
			if (fTfVerbosity > 0)
				G4cout << "Registered use of changeable parameter: " << name << "  by scorer: " << fCurrentScorer->GetNameWithSplitId() << G4endl;
		}

		if (matched && fTfVerbosity > 0)
			G4cout << "TsScoringManager::NoteAnyUse Again called with current scorer: " << fCurrentScorer->GetNameWithSplitId() << ", param: " << name <<
			", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
		else if (fTfVerbosity > 0)
			G4cout << "TsScoringManager::NoteAnyUse First called with current scorer: " << fCurrentScorer->GetNameWithSplitId() << ", param: " << name <<
			", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
#endif
	}
}


void TsScoringManager::UpdateForSpecificParameterChange(G4String parameter) {
	std::multimap< G4String, std::pair<TsVScorer*,G4String> >::const_iterator iter;
	for (iter = fChangeableParameterMap.begin(); iter != fChangeableParameterMap.end(); iter++) {
		if (iter->first==parameter) {
			G4String directParameterName = iter->second.second;
			if (fTfVerbosity > 0)
				G4cout << "TsScoringManager::UpdateForSpecificParameterChange called for parameter: " << parameter <<
				", matched for scorer: " << iter->second.first->GetNameWithSplitId() << ", direct parameter: " << directParameterName << G4endl;
			iter->second.first->UpdateForSpecificParameterChange(directParameterName);
		}
	}
}


void TsScoringManager::UpdateForNewRun(G4bool rebuiltSomeComponents) {
	if (fVerbosity > 0)
		G4cout << "TsScoringManager::UpdateForNewRun called" << G4endl;

	std::vector<TsVScorer*>::iterator iter;
	for (iter=fMasterScorers.begin(); iter!=fMasterScorers.end(); iter++)
		(*iter)->UpdateForNewRun(rebuiltSomeComponents);

	for (iter=fWorkerScorers.begin(); iter!=fWorkerScorers.end(); iter++)
		(*iter)->UpdateForNewRun(rebuiltSomeComponents);
}


G4bool TsScoringManager::HasUnsatisfiedLimits() {
	std::vector<TsVScorer*>::iterator mIter;
	for (mIter=fMasterScorers.begin(); mIter!=fMasterScorers.end(); mIter++)
		if ((*mIter)->HasUnsatisfiedLimits())
			return true;

	return false;
}

void TsScoringManager::UpdateForEndOfRun() {
	// Absorb results from workers into associated masters
	std::vector<TsVScorer*>::iterator wIter;
	std::vector<TsVScorer*>::iterator mIter;
	for (wIter=fWorkerScorers.begin(); wIter!=fWorkerScorers.end(); wIter++)
		for (mIter=fMasterScorers.begin(); mIter!=fMasterScorers.end(); mIter++)
			if ((*mIter)->fUID == (*wIter)->fUID)
				(*mIter)->AbsorbResultsFromWorkerScorer(*wIter);

	// Update the masters
	for (mIter=fMasterScorers.begin(); mIter!=fMasterScorers.end(); mIter++)
		(*mIter)->UpdateForEndOfRun();

	for (mIter=fMasterScorers.begin(); mIter!=fMasterScorers.end(); mIter++)
		(*mIter)->PostUpdateForEndOfRun();
}


void TsScoringManager::RestoreResultsFromFile() {
	std::vector<TsVScorer*>::iterator iter;
	for (iter=fMasterScorers.begin(); iter!=fMasterScorers.end(); iter++)
		(*iter)->RestoreResultsFromFile();
}


void TsScoringManager::Finalize() {
	std::vector<TsVScorer*>::iterator iter;
	for (iter=fMasterScorers.begin(); iter!=fMasterScorers.end(); iter++)
		(*iter)->Finalize();

	for (iter=fMasterScorers.begin(); iter!=fMasterScorers.end(); iter++)
		(*iter)->PostFinalize();

	if (fRootAnalysisManager) {
		fRootAnalysisManager->Write();
		fRootAnalysisManager->CloseFile();
	}

	if (fXmlAnalysisManager) {
		fXmlAnalysisManager->Write();
		fXmlAnalysisManager->CloseFile();
	}
}


TsVScorer* TsScoringManager::GetMasterScorerByID(G4int uid) {
	std::vector<TsVScorer*>::iterator iter;
	for (iter=fMasterScorers.begin(); iter!=fMasterScorers.end(); iter++)
		if ((*iter)->fUID == uid)
			return *iter;

	G4cerr << "Topas is exiting due to a serious error." << G4endl;
	G4cerr << "Unable to find master scorer for worker scorer #" << uid << G4endl;
	fPm->AbortSession(1);
	return 0;
}


std::vector<TsVScorer*> TsScoringManager::GetMasterScorersByName(G4String scorerName) {
	std::vector<TsVScorer*> matchedScorers;
	std::vector<TsVScorer*>::iterator iter;
	for (iter=fMasterScorers.begin(); iter!=fMasterScorers.end(); iter++)
		if ((*iter)->GetName() == scorerName)
			matchedScorers.push_back(*iter);

	return matchedScorers;
}


G4bool TsScoringManager::AddUnitEvenIfItIsOne() {
	return fAddUnitEvenIfItIsOne;
}


G4RootAnalysisManager* TsScoringManager::GetRootAnalysisManager() {
	// If analysis manager already exists, just return it.
	// Otherwise, instantiate it and open the output file.
	if (!fRootAnalysisManager) {
#if GEANT4_VERSION_MAJOR >= 11
		fRootAnalysisManager = G4RootAnalysisManager::Instance();
#else
		fRootAnalysisManager = G4Root::G4AnalysisManager::Instance();
#endif
		fRootAnalysisManager->OpenFile(fPm->GetStringParameter("Sc/RootFileName"));
	}

	return fRootAnalysisManager;
}


G4XmlAnalysisManager* TsScoringManager::GetXmlAnalysisManager() {
	// If analysis manager already exists, just return it.
	// Otherwise, instantiate it and open the output file.
	if (!fXmlAnalysisManager) {
#if GEANT4_VERSION_MAJOR >= 11
		fXmlAnalysisManager = G4XmlAnalysisManager::Instance();
#else
		fXmlAnalysisManager = G4Xml::G4AnalysisManager::Instance();
#endif
		fXmlAnalysisManager->OpenFile(fPm->GetStringParameter("Sc/XmlFileName"));
	}

	return fXmlAnalysisManager;
}


G4String TsScoringManager::GetFullParmName(const char* parmName) {
#ifdef TOPAS_MT
	G4String fullName = "Sc/"+fCurrentScorerName.Get()+"/"+parmName;
#else
	G4String fullName = "Sc/"+fCurrentScorerName+"/"+parmName;
#endif
	return fullName;
}
