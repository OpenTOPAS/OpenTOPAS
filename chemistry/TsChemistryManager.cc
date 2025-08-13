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

#include "TsChemistryManager.hh"
#include "TsParameterManager.hh"

#include "TsChemSteppingAction.hh"
#include "TsChemTrackingAction.hh"
#include "TsChemTrackingManager.hh"

#include "TsChemTimeStepAction.hh"

#include "G4DNAChemistryManager.hh"
#include "G4H2O.hh"
#include "G4MoleculeCounter.hh"
#include "G4MoleculeReactionCounter.hh"
#include "G4Scheduler.hh"

#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"

TsChemistryManager::TsChemistryManager(TsParameterManager* pM)
	: fPm(pM), fName("Default"), fActiveTransport(false), fMaxChemicalStepNumber(1000),
	  fMaxChemicalStageTime(100 * nanosecond), fUseTimmingCut(false), fUseStepNumberingCut(false),
	  fCounterManagerVerbosity(0), fResetCountersBeforeEvent(true), fResetCountersBeforeRun(true), fAccumulateWorkerCountersIntoMaster(true), fResetMasterCounterWithWorkers(false),
	  fVerbosity(0)
{
	ResolveParameters();
}

void TsChemistryManager::ResolveParameters()
{
	if (fPm->ParameterExists("Ch/ChemistryName"))
		fName = fPm->GetStringParameter("Ch/ChemistryName");

	if (fPm->ParameterExists(GetFullParmName("ChemicalStageTransportActive")))
		fActiveTransport = fPm->GetBooleanParameter(GetFullParmName("ChemicalStageTransportActive"));

	if (fPm->ParameterExists(GetFullParmName("ChemicalStageTimeEnd"))) {
		fMaxChemicalStageTime = fPm->GetDoubleParameter(GetFullParmName("ChemicalStageTimeEnd"), "Time");
		fUseTimmingCut = true;
	}

	if (fPm->ParameterExists(GetFullParmName("ChemicalStageMaximumStepNumber"))) {
		fMaxChemicalStepNumber = fPm->GetIntegerParameter(GetFullParmName("ChemicalStageMaximumStepNumber"));
		fUseStepNumberingCut = true;
	}

	if (fUseTimmingCut && fUseStepNumberingCut) {
		G4cerr << "TOPAS is exiting due to an incompatibility in parameters." << G4endl;
		G4cerr << "Parameter: " << GetFullParmName("ChemicalStageTimeEnd") << " cannot be used in conjunction with "
			   << "parameter: " << GetFullParmName("ChemicalStageMaximumStepNumber") << G4endl;
		fPm->AbortSession(1);
	}

	if (fPm->ParameterExists("Ts/ChemistryVerbosity"))
		fVerbosity = fPm->GetIntegerParameter("Ts/ChemistryVerbosity");

	//
	// Molecule Counters

	fCounterManagerVerbosity = fVerbosity;
	if (fPm->ParameterExists(GetFullParmName("Counters/Verbosity")))
		fCounterManagerVerbosity = fPm->GetIntegerParameter(GetFullParmName("Counters/Verbosity"));

	if (fPm->ParameterExists(GetFullParmName("Counters/ResetBeforeEvent")))
		fResetCountersBeforeEvent = fPm->GetBooleanParameter(GetFullParmName("Counters/ResetBeforeEvent"));
	if (fPm->ParameterExists(GetFullParmName("Counters/ResetBeforeRun")))
		fResetCountersBeforeRun = fPm->GetBooleanParameter(GetFullParmName("Counters/ResetBeforeRun"));

	if (fPm->ParameterExists(GetFullParmName("Counters/AccumulateWorkerCountersIntoMaster")))
		fResetCountersBeforeEvent = fPm->GetBooleanParameter(GetFullParmName("Counters/AccumulateWorkerCountersIntoMaster"));
	if (fPm->ParameterExists(GetFullParmName("Counters/ResetMasterCounterWithWorkers")))
		fResetCountersBeforeRun = fPm->GetBooleanParameter(GetFullParmName("Counters/ResetMasterCounterWithWorkers"));
}

void TsChemistryManager::Configure()
{
	// called by TsActionInitialization::Build()

	if (G4DNAChemistryManager::IsActivated())
	{
		G4Scheduler::Instance()->SetUserAction(new TsChemTimeStepAction(fPm, fName));

		if (fUseTimmingCut)
			G4Scheduler::Instance()->SetEndTime(fMaxChemicalStageTime);

		if (fUseStepNumberingCut)
			G4Scheduler::Instance()->SetMaxNbSteps(fMaxChemicalStepNumber);

		if (0 < fVerbosity)
			G4Scheduler::Instance()->SetVerbose(fVerbosity - 1);
		else
			G4Scheduler::Instance()->SetVerbose(0);

		if (fActiveTransport) {
			TsChemTrackingManager* chemTrackingManager = new TsChemTrackingManager();
			chemTrackingManager->SetUserAction(new TsChemSteppingAction);
			chemTrackingManager->SetUserAction(new TsChemTrackingAction(fPm));
			G4Scheduler::Instance()->SetInteractivity(chemTrackingManager);
		}

		{ // Molecule Counters
			// may add some more sophisticated functionality that facilitates creating and registering
			// custom counters through the parameter interface at some point [TBD]
			G4MoleculeCounterManager::Instance()->SetVerbosity(fCounterManagerVerbosity);
			G4MoleculeCounterManager::Instance()->SetResetCountersBeforeEvent(fResetCountersBeforeEvent);
			G4MoleculeCounterManager::Instance()->SetResetCountersBeforeRun(fResetCountersBeforeRun);
			G4MoleculeCounterManager::Instance()->SetAccumulateCounterIntoMaster(fAccumulateWorkerCountersIntoMaster);
			G4MoleculeCounterManager::Instance()->SetResetMasterCounterWithWorkers(fResetMasterCounterWithWorkers);

			BuildMoleculeCounters();
		}

		if (fPm->ParameterExists("Ch/ReportPreChemicalStageToAsciiFile")) {
			G4String fileName_mt = fPm->GetStringParameter("Ch/ReportPreChemicalStageToAsciiFile");
			fileName_mt += ".chem";

			G4DNAChemistryManager::Instance()->WriteInto(fileName_mt);
		}
	}
}

void TsChemistryManager::UpdateForNewRun(G4bool /* rebuiltSomeComponents */)
{
	// ensure that the chemistry is notified!
	if (G4DNAChemistryManager::GetInstanceIfExists() != nullptr)
		G4DNAChemistryManager::GetInstanceIfExists()->BeginOfRunAction(G4RunManager::GetRunManager()->GetCurrentRun());
}

void TsChemistryManager::UpdateForEndOfRun()
{
	// ensure that the chemistry is notified!
	if (G4DNAChemistryManager::GetInstanceIfExists() != nullptr)
		G4DNAChemistryManager::GetInstanceIfExists()->EndOfRunAction(G4RunManager::GetRunManager()->GetCurrentRun());
}

void TsChemistryManager::BuildMoleculeCounters() const
{
	//
	// DEFAULT COUNTERS
	//

	//
	// [1] Fixed time precision (akin to the original G4MoleculeCounter)

	if (fPm->GetBooleanParameterOrDefault(GetFullParmName("Counters/DefaultWithFixedTimePrecision/Enabled"), true))
	{
		auto timePrecision = 1 * ps;
		if (fPm->ParameterExists(GetFullParmName("Counters/DefaultWithFixedTimePrecision/TimePrecision")))
			timePrecision = fPm->GetDoubleParameter(GetFullParmName("Counters/DefaultWithFixedTimePrecision/TimePrecision"), "Time");

		auto activeLowerBound = 0 * ps;
		if (fPm->ParameterExists(GetFullParmName("Counters/DefaultWithFixedTimePrecision/ActiveAboveTime")))
			activeLowerBound = fPm->GetDoubleParameter(GetFullParmName("Counters/DefaultWithFixedTimePrecision/ActiveAboveTime"), "Time");
		auto activeUpperBound = DBL_MAX;
		if (fPm->ParameterExists(GetFullParmName("Counters/DefaultWithFixedTimePrecision/ActiveBelowTime")))
			activeUpperBound = fPm->GetDoubleParameter(GetFullParmName("Counters/DefaultWithFixedTimePrecision/ActiveAboveTime"), "Time");

		auto counter = std::make_unique<G4MoleculeCounter>(GetDefaultFixedPrecisionCounterName());
		counter->SetTimeComparer(G4MoleculeCounterTimeComparer::CreateWithFixedPrecision(timePrecision));
		counter->SetActiveLowerBound(activeLowerBound);
		counter->SetActiveUpperBound(activeUpperBound);
		if (fPm->GetBooleanParameterOrDefault("Counters/DefaultWithFixedTimePrecision/IgnoreH2O", true))
			counter->IgnoreMolecule(G4H2O::Definition());

		G4MoleculeCounterManager::Instance()->RegisterCounter(std::move(counter));

		if (fVerbosity > 0)
			G4cout << "Chemistry `" << fName << "` registered fixed time precision molecule counter: " << GetDefaultFixedPrecisionCounterName() << G4endl;

		if (fPm->GetBooleanParameterOrDefault("Counters/DefaultWithFixedTimePrecision/IncludeReactions", false)) {
			auto reactionCounter = std::make_unique<G4MoleculeReactionCounter>(GetDefaultFixedPrecisionCounterName() + "_Reactions");
			reactionCounter->SetTimeComparer(G4MoleculeCounterTimeComparer::CreateWithFixedPrecision(timePrecision));
			reactionCounter->SetActiveLowerBound(activeLowerBound);
			reactionCounter->SetActiveUpperBound(activeUpperBound);
			G4MoleculeCounterManager::Instance()->RegisterCounter(std::move(reactionCounter));

			if (fVerbosity > 0)
				G4cout << "Chemistry `" << fName << "` registered fixed time precision reaction counter: " << GetDefaultFixedPrecisionCounterName() << "_Reactions" << G4endl;
		}
	}

	//
	// [2] Variable time precision (to facilitate longer simulation times without memory creep and slowdown)

	if (fPm->GetBooleanParameterOrDefault(GetFullParmName("Counters/DefaultWithVariableTimePrecision/Enabled"), true))
	{
		std::map<G4double, G4double> variableTimePrecision = {
			// TBD, suggest better ones please!
			{10 * ps, 5 * ps},
			{100 * ps, 50 * ps},
			{1000 * ps, 500 * ps},
			{10 * ns, 5 * ns},
			{1 * microsecond, 50 * ns},
		};
		if (fPm->ParameterExists(GetFullParmName("Counters/DefaultWithVariableTimePrecision/ResolutionStepsTimeHighEdges")) ||
			fPm->ParameterExists(GetFullParmName("Counters/DefaultWithVariableTimePrecision/ResolutionStepsTimeResolutions"))) {
			auto n = fPm->GetVectorLength(GetFullParmName("Counters/DefaultWithVariableTimePrecision/ResolutionStepsTimeHighEdges"));
			if (n != fPm->GetVectorLength(GetFullParmName("Counters/DefaultWithVariableTimePrecision/ResolutionStepsTimeResolutions"))) {
				G4cerr << "Topas is exiting due to a serious error." << G4endl;
				G4cerr << "The number of elements in `ResolutionStepsTimeHighEdges` and `ResolutionStepsTimeResolutions` must be the same!" << G4endl;
				fPm->AbortSession(1);
			}
			auto timeSteps = std::unique_ptr<G4double[]>(fPm->GetDoubleVector(GetFullParmName("Counters/DefaultWithVariableTimePrecision/ResolutionStepsTimeHighEdges"), "Time"));
			auto timeResolutions = std::unique_ptr<G4double[]>(fPm->GetDoubleVector(GetFullParmName("Counters/DefaultWithVariableTimePrecision/ResolutionStepsTimeResolutions"), "Time"));

			variableTimePrecision.clear();
			for (auto i = 0; i < n; ++i)
				variableTimePrecision.emplace(timeSteps[i], timeResolutions[i]);
		}

		auto activeLowerBound = 0 * ps;
		if (fPm->ParameterExists(GetFullParmName("Counters/DefaultWithFixedTimePrecision/ActiveAboveTime")))
			activeLowerBound = fPm->GetDoubleParameter(GetFullParmName("Counters/DefaultWithFixedTimePrecision/ActiveAboveTime"), "Time");
		auto activeUpperBound = DBL_MAX;
		if (fPm->ParameterExists(GetFullParmName("Counters/DefaultWithFixedTimePrecision/ActiveBelowTime")))
			activeUpperBound = fPm->GetDoubleParameter(GetFullParmName("Counters/DefaultWithFixedTimePrecision/ActiveAboveTime"), "Time");

		auto counter = std::make_unique<G4MoleculeCounter>(GetDefaultVariablePrecisionCounterName());
		counter->SetTimeComparer(G4MoleculeCounterTimeComparer::CreateWithVariablePrecision(variableTimePrecision));
		counter->SetActiveLowerBound(activeLowerBound);
		counter->SetActiveUpperBound(activeUpperBound);
		if (fPm->GetBooleanParameterOrDefault("Counters/DefaultWithVariableTimePrecision/IgnoreH2O", true))
			counter->IgnoreMolecule(G4H2O::Definition());

		G4MoleculeCounterManager::Instance()->RegisterCounter(std::move(counter));

		if (fVerbosity > 0)
			G4cout << "Chemistry `" << fName << "` registered variable time precision molecule counter: " << GetDefaultVariablePrecisionCounterName() << G4endl;

		if (fPm->GetBooleanParameterOrDefault("Counters/DefaultWithVariableTimePrecision/IncludeReactions", false)) {
			auto reactionCounter = std::make_unique<G4MoleculeReactionCounter>(GetDefaultVariablePrecisionCounterName() + "_Reactions");
			reactionCounter->SetTimeComparer(G4MoleculeCounterTimeComparer::CreateWithVariablePrecision(variableTimePrecision));
			reactionCounter->SetActiveLowerBound(activeLowerBound);
			reactionCounter->SetActiveUpperBound(activeUpperBound);
			G4MoleculeCounterManager::Instance()->RegisterCounter(std::move(reactionCounter));

			if (fVerbosity > 0)
				G4cout << "Chemistry `" << fName << "` registered variable time precision reaction counter: " << GetDefaultVariablePrecisionCounterName() << "_Reactions" << G4endl;
		}
	}

	//
	//
}

G4String TsChemistryManager::GetFullParmName(G4String suffix) const
{
	G4String fullParName = "Ch/" + fName + "/" + suffix;
	return fullParName;
}
