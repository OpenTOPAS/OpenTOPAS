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

#include "TsSourceManager.hh"

#include "TsParameterManager.hh"
#include "TsGeneratorManager.hh"
#include "TsExtensionManager.hh"
#include "TsSequenceManager.hh"

#include "TsSource.hh"
#include "TsSourceDistributed.hh"
#include "TsSourcePhaseSpace.hh"
#include "TsSourcePhaseSpaceOld.hh"
#include "TsSourceEnvironment.hh"

TsSourceManager::TsSourceManager(TsParameterManager* pM, TsGeometryManager* gM, TsExtensionManager* eM)
	: fPm(pM), fGm(gM), fEm(eM), fSqm(0), fVerbosity(0), fCurrentSource(0), fSources(0)
{
}

TsSourceManager::~TsSourceManager()
{
	if (fSources)
		delete fSources;
}

void TsSourceManager::Initialize(TsSequenceManager* sqM) {
	fSqm = sqM;

	fVerbosity = fPm->GetIntegerParameter("So/Verbosity");

	// Create the store for the Sources
	fSources = new std::map<G4String, TsSource*>;

	// Loop over all defined sources
	G4String prefix = "So";
	G4String suffix = "/Type";
	std::vector<G4String>* values = new std::vector<G4String>;
	fPm->GetParameterNamesBracketedBy(prefix, suffix, values);
	G4int length = values->size();

	for (G4int iToken=0; iToken<length; iToken++) {
		G4String sourceParmName = (*values)[iToken];
		G4String sourceName = sourceParmName.substr(3,sourceParmName.length()-8);
		G4String sourceTypeParm = "So/" + sourceName + "/Type";
		G4String sourceType = fPm->GetStringParameter(sourceTypeParm);
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(sourceType);
#else
		sourceType.toLower();
#endif

		// First see if the user's extensions include this particle source
		TsSource* source = fEm->InstantiateParticleSource(fPm, this, sourceType, sourceName);

		if (!source) {
			// Source is not from user's extensions
			if (sourceType=="distributed")
				new TsSourceDistributed(fPm, this, sourceName);
			else if (sourceType=="phasespace")
				new TsSourcePhaseSpace(fPm, this, sourceName);
			else if (sourceType=="phasespaceold")
				new TsSourcePhaseSpaceOld(fPm, this, sourceName);
			else if (sourceType=="environment")
				new TsSourceEnvironment(fPm, this, sourceName);
			else
				new TsSource(fPm, this, sourceName);
		}
	}

	// Reset current source to zero to indicate that we are no longer building sources.
	SetCurrentSource(0);
}


void TsSourceManager::SetCurrentSource(TsSource* currentSource) {
	fCurrentSource = currentSource;
	if (currentSource) (*fSources)[currentSource->GetName()] = currentSource;
}


void TsSourceManager::NoteAnyUseOfChangeableParameters(const G4String& name)
{
	if (fCurrentSource)
	{
		// Register use of this parameter for current source and LastDirectParameter.
		// LastDirectParameter is needed because we ultimately need to tell the source not the name of the changed parameter
		// but the name of the parameter the source directly accesses (that was affected by the changed parameter).
		// We store string rather than pointer because we permit a parmeter to be overridden by another parameter
		// of the same name.
		G4String directParm = fPm->GetLastDirectParameterName();

		// See if this parameter has already been registered as used by this source (otherwise if this parameter
		// is interrogated twice for the same source, the source would update twice for the same change).
		G4bool matched = false;
		G4String nameToLower = name;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(nameToLower);
#else
		nameToLower.toLower();
#endif
		std::multimap< G4String, std::pair<TsSource*,G4String> >::const_iterator iter;
		for (iter = fChangeableParameterMap.begin(); iter != fChangeableParameterMap.end() && !matched; iter++) {
			G4String gotParm = iter->first;
			TsSource* gotSource = iter->second.first;
			G4String gotDirectParm = iter->second.second;
			if (gotParm==nameToLower && gotSource==fCurrentSource && gotDirectParm==directParm)
				matched = true;
		}

		if (!matched) {
			fChangeableParameterMap.insert(std::make_pair(nameToLower, std::make_pair(fCurrentSource, directParm)));
			if (fPm->IGetIntegerParameter("Tf/Verbosity") > 0)
				G4cout << "Registered use of changeable parameter: " << name << "  by source: " << fCurrentSource->GetName() << G4endl;
		}

		if (matched && fVerbosity>0)
			G4cout << "TsSourceManager::NoteAnyUse Again called with current source: " << fCurrentSource->GetName() << ", param: " << name <<
			", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
		else if (fVerbosity>0)
			G4cout << "TsSourceManager::NoteAnyUse First called with current source: " << fCurrentSource->GetName() << ", param: " << name <<
			", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;

		G4String directParmLower = directParm;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(directParmLower);
#else
		directParmLower.toLower();
#endif
		size_t pos = directParmLower.find_last_of("/");

		if (directParmLower.substr(pos+1) == "phasespacemultipleuse") {
			G4cerr << "Topas is exiting. " << directParm << " is not allowed to be controlled by a time feature." << G4endl;
			exit(1);
		}

		if (directParmLower.substr(pos+1) == "numberofhistoriesinrun") {
			G4String multipleUseParmName = directParmLower.substr(0,pos+1) + "PhaseSpaceMultipleUse";
			G4String includeEmptyHistories = directParmLower.substr(0,pos+1) + "PhaseSpaceIncludeEmptyHistories";
			if (fPm->ParameterExists(includeEmptyHistories) && fPm->GetBooleanParameter(includeEmptyHistories) &&
				fPm->ParameterExists(multipleUseParmName) && (fPm->GetIntegerParameter(multipleUseParmName) == 0)) {
				G4cerr << "Topas is exiting. " << directParm << " is not allowed to be controlled by a time feature" << G4endl;
				G4cerr << "when the same source has IncludeEmptyHistories since we can't tell where in the time sequence" << G4endl;
				G4cerr << "this empty histories are supposed to occur." << G4endl;
				exit(1);
			}
		}

	}
}


void TsSourceManager::UpdateForSpecificParameterChange(G4String parameter) {
	if (fSqm) {
		std::multimap< G4String, std::pair<TsSource*,G4String> >::const_iterator iter;
		for (iter = fChangeableParameterMap.begin(); iter != fChangeableParameterMap.end(); iter++) {
			if (iter->first==parameter) {
				G4String directParameterName = iter->second.second;
				if (fVerbosity>0)
					G4cout << "TsSourceManager::UpdateForSpecificParameterChange called for parameter: " << parameter <<
					", matched for source: " << iter->second.first->GetName() << ", direct parameter: " << directParameterName << G4endl;
				iter->second.first->UpdateForSpecificParameterChange(directParameterName);
			}
		}
	}
}


void TsSourceManager::UpdateForNewRun(G4bool rebuiltSomeComponents) {
	if (fSqm) {
		if (fVerbosity>0)
			G4cout << "TsSourceManager::UpdateForNewRun called" << G4endl;

		std::map<G4String, TsSource*>::const_iterator iter;
		for (iter=fSources->begin(); iter!=fSources->end(); iter++)
			iter->second->UpdateForNewRun(rebuiltSomeComponents);
	}
}


void TsSourceManager::UpdateForEndOfRun() {
	if (fSqm) {
		if (fVerbosity>0)
			G4cout << "TsSourceManager::UpdateForEndOfRun called" << G4endl;

		std::map<G4String, TsSource*>::const_iterator iter;
		for (iter=fSources->begin(); iter!=fSources->end(); iter++)
			iter->second->UpdateForEndOfRun();
	}
}

void TsSourceManager::Finalize()
{
	// Give each source the opportunity to generate a final report.
	std::map<G4String, TsSource*>::const_iterator iter;
	for (iter=fSources->begin(); iter!=fSources->end(); iter++)
		iter->second->Finalize();
}


TsSource* TsSourceManager::GetSource(G4String sourceName) {
	std::map<G4String, TsSource*>::const_iterator iter = fSources->find(sourceName);
	if (iter != fSources->end())
		return iter->second;

	G4cerr << "Topas is exiting. Particle source: " << sourceName << " not found at generate time" << G4endl;
	exit(1);
}


TsGeometryManager* TsSourceManager::GetGeometryManager() {
	return fGm;
}


G4bool TsSourceManager::RandomModeNeedsMoreRuns() {
	G4bool needMoreRuns = false;

	std::map<G4String, TsSource*>::const_iterator iter;
	for (iter=fSources->begin(); iter!=fSources->end(); iter++)
		if (iter->second->RandomModeNeedsMoreRuns())
			needMoreRuns = true;

	return needMoreRuns;
}


G4int TsSourceManager::GetNumberOfHistoriesInRun() {
	G4int maxNumber = 0;
	G4int number;
	std::map<G4String, TsSource*>::const_iterator iter;
	for (iter=fSources->begin(); iter!=fSources->end(); iter++) {
		number = iter->second->GetNumberOfHistoriesInRun();
		if (number>maxNumber)
			maxNumber = number;
	}

	return maxNumber;
}


void TsSourceManager::AddSourceFromGUI(G4String& sourceName, G4String& componentName, G4String& typeName) {
	G4String parameterName;
	G4String transValue;

	G4String typeNameLower = typeName;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(typeNameLower);
#else
	typeNameLower.toLower();
#endif
	if (typeNameLower == "beam" || typeNameLower == "isotropic" || typeNameLower == "volumetric" ||
		typeNameLower == "emittance" || typeNameLower == "phasespace") {
		parameterName = "s:So/" + sourceName + "/Type";
		transValue = "\"" + typeName + "\"";
		fPm->AddParameter(parameterName, transValue);

		parameterName = "sc:So/" + sourceName + "/Component";
		transValue = "\"" + componentName + "\"";
		fPm->AddParameter(parameterName, transValue);

		if (typeNameLower == "beam" || typeNameLower == "isotropic" || typeNameLower == "volumetric" ||
			typeNameLower == "emittance") {
			parameterName = "sc:So/" + sourceName + "/BeamParticle";
			transValue = "\"proton\"";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "dc:So/" + sourceName + "/BeamEnergy";
			transValue = "169.23 MeV";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "uc:So/" + sourceName + "/BeamEnergySpread";
			transValue = "0.757504";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "ic:So/" + sourceName + "/NumberOfHistoriesInRun";
			transValue = "10";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "ic:So/" + sourceName + "/NumberOfHistoriesInRandomJob";
			transValue = "10";
			fPm->AddParameter(parameterName, transValue);

			if (typeNameLower == "beam") {
				parameterName = "sc:So/" + sourceName + "/BeamPositionDistribution";
				transValue = "\"Gaussian\"";
				fPm->AddParameter(parameterName, transValue);

				parameterName = "sc:So/" + sourceName + "/BeamPositionCutoffShape";
				transValue = "\"Ellipse\"";
				fPm->AddParameter(parameterName, transValue);

				parameterName = "dc:So/" + sourceName + "/BeamPositionCutoffX";
				transValue = "10. cm";
				fPm->AddParameter(parameterName, transValue);

				parameterName = "dc:So/" + sourceName + "/BeamPositionCutoffY";
				transValue = "10. cm";
				fPm->AddParameter(parameterName, transValue);

				parameterName = "dc:So/" + sourceName + "/BeamPositionSpreadX";
				transValue = "0.65 cm";
				fPm->AddParameter(parameterName, transValue);

				parameterName = "dc:So/" + sourceName + "/BeamPositionSpreadY";
				transValue = "0.65 cm";
				fPm->AddParameter(parameterName, transValue);

				parameterName = "sc:So/" + sourceName + "/BeamAngularDistribution";
				transValue = "\"Gaussian\"";
				fPm->AddParameter(parameterName, transValue);

				parameterName = "dc:So/" + sourceName + "/BeamAngularCutoffX";
				transValue = "90. deg";
				fPm->AddParameter(parameterName, transValue);

				parameterName = "dc:So/" + sourceName + "/BeamAngularCutoffY";
				transValue = "90. deg";
				fPm->AddParameter(parameterName, transValue);

				parameterName = "dc:So/" + sourceName + "/BeamAngularSpreadX";
				transValue = "0.0032 rad";
				fPm->AddParameter(parameterName, transValue);

				parameterName = "dc:So/" + sourceName + "/BeamAngularSpreadY";
				transValue = "0.0032 rad";
				fPm->AddParameter(parameterName, transValue);
			} else if (typeNameLower == "emittance") {
				parameterName = "sc:So/" + sourceName + "/Distribution";
				transValue = "\"BiGaussian\"";
				fPm->AddParameter(parameterName, transValue);

				parameterName = "dc:So/" + sourceName + "/SigmaX";
				transValue = "0.2 mm";
				fPm->AddParameter(parameterName, transValue);

				parameterName = "uc:So/" + sourceName + "/SigmaXprime";
				transValue = "0.032";
				fPm->AddParameter(parameterName, transValue);

				parameterName = "uc:So/" + sourceName + "/CorrelationX";
				transValue = "-0.9411";
				fPm->AddParameter(parameterName, transValue);

				parameterName = "dc:So/" + sourceName + "/SigmaY";
				transValue = "0.2 mm";
				fPm->AddParameter(parameterName, transValue);

				parameterName = "uc:So/" + sourceName + "/SigmaYprime";
				transValue = "0.032";
				fPm->AddParameter(parameterName, transValue);

				parameterName = "uc:So/" + sourceName + "/CorrelationY";
				transValue = "0.9411";
				fPm->AddParameter(parameterName, transValue);
			}
		} else {
			parameterName = "sc:So/" + sourceName + "/PhaseSpaceFileName";
			transValue = "\"ASCIIOutput\"";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "uc:So/" + sourceName + "/PhaseSpaceScaleXPosBy";
			transValue = "1.0";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "uc:So/" + sourceName + "/PhaseSpaceScaleYPosBy";
			transValue = "1.0";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "uc:So/" + sourceName + "/PhaseSpaceScaleZPosBy";
			transValue = "1.0";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "bc:So/" + sourceName + "/PhaseSpaceInvertXAxis";
			transValue = "\"False\"";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "bc:So/" + sourceName + "/PhaseSpaceInvertYAxis";
			transValue = "\"False\"";
			fPm->AddParameter(parameterName, transValue);

			parameterName = "bc:So/" + sourceName + "/PhaseSpaceInvertZAxis";
			transValue = "\"False\"";
			fPm->AddParameter(parameterName, transValue);
		}
	} else {
		G4cout << "Sorry, don't have the logic yet to add a source of type " << typeName << G4endl;
	}
}
