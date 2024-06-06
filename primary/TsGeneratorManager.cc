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

#include "TsGeneratorManager.hh"

#include "TsSourceManager.hh"
#include "TsFilterManager.hh"

#include "TsVFilter.hh"
#include "TsSource.hh"
#include "TsGeneratorBeam.hh"
#include "TsGeneratorDistributed.hh"
#include "TsGeneratorIsotropic.hh"
#include "TsGeneratorVolumetric.hh"
#include "TsGeneratorPhaseSpace.hh"
#include "TsGeneratorPhaseSpaceOld.hh"
#include "TsGeneratorEmittance.hh"
#include "TsGeneratorEnvironment.hh"
#include "TsExtensionManager.hh"
#include "TsSequenceManager.hh"

#include "G4UIcommand.hh"
#include "G4Tokenizer.hh"

TsGeneratorManager::TsGeneratorManager(TsParameterManager* pM, TsExtensionManager* eM, TsGeometryManager* gM, TsSourceManager* prM, TsFilterManager* fM,  TsSequenceManager* sqM)
:fPm(pM), fGm(gM), fPrm(prM), fFm(fM), fIsExecutingSequence(false), fPrimaryCounter(0), fCurrentGenerator(0)
{
	fVerbosity = fPm->GetIntegerParameter("So/Verbosity");

	sqM->RegisterGeneratorManager(this);

	// Create the map from primary counter to generator
	fGeneratorPerPrimary = new std::map<G4int,TsVGenerator*>;
	if (fPm->IsFindingSeed() || sqM->UsingRayTracer())
		return;

	// Loop over all defined generators
	G4String prefix = "So";
	G4String suffix = "/Type";
	std::vector<G4String>* values = new std::vector<G4String>;
	fPm->GetParameterNamesBracketedBy(prefix, suffix, values);
	G4int length = values->size();

	for (G4int iToken=0; iToken<length; iToken++) {
		G4String generatorParmName = (*values)[iToken];
		G4String generatorName = generatorParmName.substr(3,generatorParmName.length()-8);
		G4String generatorTypeParm = "So/" + generatorName + "/Type";
		G4String generatorType = fPm->GetStringParameter(generatorTypeParm);
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(generatorType);
#else
		generatorType.toLower();
#endif

		TsVGenerator* generator;

		// First see if the user's extensions include this particle generator
		generator = eM->InstantiateParticleGenerator(fPm, fGm, this, generatorType, generatorName);

		if (!generator) {
			// Generator is not from user's extensions
			if (generatorType == "beam") {
				generator = new TsGeneratorBeam(fPm, fGm, this, generatorName);
			} else if (generatorType == "distributed") {
				generator = new TsGeneratorDistributed(fPm, fGm, this, generatorName);
			} else if (generatorType == "isotropic") {
				generator = new TsGeneratorIsotropic(fPm, fGm, this, generatorName);
			} else if (generatorType == "volumetric") {
				generator = new TsGeneratorVolumetric(fPm, fGm, this, generatorName);
			} else if (generatorType == "phasespace") {
				generator = new TsGeneratorPhaseSpace(fPm, fGm, this, generatorName);
			} else if (generatorType == "phasespaceold") {
				generator = new TsGeneratorPhaseSpaceOld(fPm, fGm, this, generatorName);
			} else if (generatorType == "emittance") {
				generator = new TsGeneratorEmittance(fPm, fGm, this, generatorName);
			} else if (generatorType == "environment") {
			  	generator = new TsGeneratorEnvironment(fPm, fGm, this, generatorName);
	        } else {
				G4cerr << "Topas is exiting. Particle source \"" << generatorName << "\" has unknown Type \"" << generatorType << "\"" << G4endl;
				fPm->AbortSession(1);
			}
		}

		// Create any needed filters for this generator
		TsVFilter* parentFilter = fFm->InstantiateFilter(generator);

		if (parentFilter) generator->SetFilter(parentFilter);
	}

	// Reset current generator to zero to indicate that we are no longer building generators.
	SetCurrentGenerator(0);
}


TsGeneratorManager::~TsGeneratorManager()
{
}


TsSource* TsGeneratorManager::GetSource(G4String generatorName) {
	return fPrm->GetSource(generatorName);
}


void TsGeneratorManager::RegisterPrimary(TsVGenerator* generator) {
	fPrimaryCounter++;
	(*fGeneratorPerPrimary)[fPrimaryCounter] = generator;
}


TsVFilter* TsGeneratorManager::GetFilter(G4int primaryCounter) {
	return (*fGeneratorPerPrimary)[primaryCounter]->GetFilter();
}


void TsGeneratorManager::SetIsExecutingSequence(G4bool isExecutingSequence) {
	fIsExecutingSequence = isExecutingSequence;
	std::vector<TsVGenerator*>::iterator iter;
	for (iter=fGenerators.begin(); iter!=fGenerators.end(); iter++)
		(*iter)->SetIsExecutingSequence(isExecutingSequence);
}


void TsGeneratorManager::GeneratePrimaries(G4Event* anEvent)
{
	// Clear the map from track id to generator pointers
	fPrimaryCounter = 0;
	fGeneratorPerPrimary->clear();
	G4int limit;

	if (fPm->IsFindingSeed()) {
		if ((fPm->FindSeedRun() == fPm->GetRunID()) && (fPm->FindSeedHistory() == anEvent->GetEventID())) {
			G4Tokenizer next(G4RunManager::GetRunManager()->GetRandomNumberStatusForThisEvent());
			next();
			next();
			G4String token = next();
			G4int SeedPart1 = G4UIcommand::ConvertToInt(token);
			token = next();
			G4int SeedPart2 = G4UIcommand::ConvertToInt(token);
			token = next();
			G4int SeedPart3 = G4UIcommand::ConvertToInt(token);
			token = next();
			G4int SeedPart4 = G4UIcommand::ConvertToInt(token);
			G4cout << "Ts/FindSeedForHistory has found seed for Run: " << fPm->FindSeedRun() << ", History: " << fPm->FindSeedHistory() << G4endl;
			G4cout << "Seed: " << SeedPart1 << ", " << SeedPart2 << ", " << SeedPart3 << ", " << SeedPart4 << G4endl;

			G4String filespec = "TopasSeedForRun_" + G4UIcommand::ConvertToString(fPm->FindSeedRun()) + "_History_" + G4UIcommand::ConvertToString(fPm->FindSeedHistory()) + ".txt";
			std::ofstream seedFile(filespec);
			if (!seedFile) {
				G4cerr << "ERROR: Failed to open file " << filespec << G4endl;
				fPm->AbortSession(1);
			}

			seedFile << "Uvec\n" << SeedPart1 << "\n" << SeedPart2 << "\n" << SeedPart3 << "\n" << SeedPart4 << G4endl;
			seedFile.close();

			G4cout << "Wrote seed to file: " << filespec << G4endl;
			G4cout << "To start a new TOPAS session from this seed, use the parameter:" << G4endl;
			G4cout << "s:Ts/SeedFile = \"" << filespec << "\"\n" << G4endl;

			fPm->AbortSession(1);
		}
	} else {
		// Give each generator the opportunity to generate primaries.
		std::vector<TsVGenerator*>::iterator iter;
		for (iter=fGenerators.begin(); iter!=fGenerators.end(); iter++) {
			if (fPm->IsRandomMode())
				limit = (*iter)->GetSource()->GetNumberOfHistoriesInRandomJob();
			else
				limit = (*iter)->GetSource()->GetNumberOfHistoriesInRun();

			if (!fIsExecutingSequence || (anEvent->GetEventID() < limit))
				(*iter)->GeneratePrimaries(anEvent);
		}
	}
}


void TsGeneratorManager::SetCurrentGenerator(TsVGenerator* currentGenerator) {
	fCurrentGenerator = currentGenerator;
	if (currentGenerator) fGenerators.push_back(currentGenerator);
}


void TsGeneratorManager::NoteAnyUseOfChangeableParameters(const G4String& name)
{
	if (fCurrentGenerator)
	{
		// Register use of this parameter for current generator and LastDirectParameter.
		// LastDirectParameter is needed because we ultimately need to tell the generator not the name of the changed parameter
		// but the name of the parameter the generator directly accesses (that was affected by the changed parameter).
		// We store string rather than pointer because we permit a parmeter to be overridden by another parameter
		// of the same name.
		G4String directParm = fPm->GetLastDirectParameterName();

		// See if this parameter has already been registered as used by this generator (otherwise if this parameter
		// is interrogated twice for the same generator, the generator would update twice for the same change).
		G4bool matched = false;
		G4String nameToLower = name;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(nameToLower);
#else
		nameToLower.toLower();
#endif
		std::multimap< G4String, std::pair<TsVGenerator*,G4String> >::const_iterator iter;
		for (iter = fChangeableParameterMap.begin(); iter != fChangeableParameterMap.end() && !matched; iter++) {
			G4String gotParm = iter->first;
			TsVGenerator* gotGenerator = iter->second.first;
			G4String gotDirectParm = iter->second.second;
			if (gotParm==nameToLower && gotGenerator==fCurrentGenerator && gotDirectParm==directParm)
				matched = true;
		}

		if (!matched) {
			fChangeableParameterMap.insert(std::make_pair(nameToLower, std::make_pair(fCurrentGenerator, directParm)));
			if (fPm->IGetIntegerParameter("Tf/Verbosity") > 0)
				G4cout << "Registered use of changeable parameter: " << name << "  by generator: " << fCurrentGenerator->GetName() << G4endl;
		}

		if (matched && fVerbosity>0)
			G4cout << "TsGeneratorManager::NoteAnyUse Again called with current generator: " << fCurrentGenerator->GetName() << ", param: " << name <<
			", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
		else if (fVerbosity>0)
			G4cout << "TsGeneratorManager::NoteAnyUse First called with current generator: " << fCurrentGenerator->GetName() << ", param: " << name <<
			", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
	}
}


void TsGeneratorManager::UpdateForSpecificParameterChange(G4String parameter) {
	std::multimap< G4String, std::pair<TsVGenerator*,G4String> >::const_iterator iter;
	for (iter = fChangeableParameterMap.begin(); iter != fChangeableParameterMap.end(); iter++) {
		if (iter->first==parameter) {
			G4String directParameterName = iter->second.second;
			if (fVerbosity>0)
				G4cout << "TsGeneratorManager::UpdateForSpecificParameterChange called for parameter: " << parameter <<
				", matched for generator: " << iter->second.first->GetName() << ", direct parameter: " << directParameterName << G4endl;
			iter->second.first->UpdateForSpecificParameterChange(directParameterName);
		}
	}
}


void TsGeneratorManager::UpdateForNewRun(G4bool rebuiltSomeComponents) {
	std::vector<TsVGenerator*>::iterator iter;
	for (iter=fGenerators.begin(); iter!=fGenerators.end(); iter++)
		(*iter)->UpdateForNewRun(rebuiltSomeComponents);
}


void TsGeneratorManager::ClearGenerators() {
	std::vector<TsVGenerator*>::iterator iter;
	for (iter=fGenerators.begin(); iter!=fGenerators.end(); iter++)
		(*iter)->ClearGenerator();
}


void TsGeneratorManager::Finalize() {
	std::vector<TsVGenerator*>::iterator iter;
	for (iter=fGenerators.begin(); iter!=fGenerators.end(); iter++)
		(*iter)->Finalize();
}
