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

#include "TsSequenceManager.hh"

#include "TsExtensionManager.hh"
#include "TsGeometryManager.hh"
#include "TsPhysicsManager.hh"
#include "TsVarianceManager.hh"
#include "TsFilterManager.hh"
#include "TsSourceManager.hh"
#include "TsGeneratorManager.hh"
#include "TsScoringManager.hh"
#include "TsGraphicsManager.hh"
#include "TsChemistryManager.hh"

#include "TsGeometricalParticleSplit.hh"
#include "TsImportanceSampling.hh"
#include "TsVBiasingProcess.hh"

#include "TsActionInitialization.hh"
#include "TsSteppingAction.hh"
#include "TsVParameter.hh"

#ifdef G4UI_USE_QT
#include "G4UIQt.hh"
#include "TsQt.hh"
#endif

#ifdef TOPAS_MT
#include "G4MTRunManager.hh"
#include "G4AutoLock.hh"
#include "G4Threading.hh"
#endif

#include "G4Tokenizer.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"

#include "G4SystemOfUnits.hh"
#include "G4StateManager.hh"


#ifdef TOPAS_MT
namespace {
	G4Mutex noteAnyUseMutex = G4MUTEX_INITIALIZER;
	G4Mutex noteKilledTrackMutex = G4MUTEX_INITIALIZER;
	G4Mutex noteUnscoredHitMutex = G4MUTEX_INITIALIZER;
	G4Mutex noteParameterizationErrorMutex = G4MUTEX_INITIALIZER;
	G4Mutex noteIndexErrorMutex = G4MUTEX_INITIALIZER;
	G4Mutex noteInterruptedHistoryMutex = G4MUTEX_INITIALIZER;
	G4Mutex registerSteppingActionMutex = G4MUTEX_INITIALIZER;
}
#endif

TsSequenceManager::TsSequenceManager(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsPhysicsManager* phM, TsVarianceManager* vM,
									 TsFilterManager* fM, TsScoringManager* scM, TsGraphicsManager* grM, TsChemistryManager* chM, TsSourceManager* soM,
									 G4int argc, char** argv)
: fPm(pM), fEm(eM), fMm(mM), fGm(gM), fPhm(phM), fVm(vM), fFm(fM), fScm(scM), fGrm(grM), fSom(soM), fChm(chM), fUseQt(false), fTsQt(0), fRunID(-1),
fKilledTrackEnergy(0.), fKilledTrackCount(0), fUnscoredHitEnergy(0.), fUnscoredHitCount(0),
fParameterizationErrorEnergy(0.), fParameterizationErrorCount(0), fIndexErrorEnergy(0.), fIndexErrorCount(0), fInterruptedHistoryCount(0),
fIsExecutingSequence(false)
{
	// Instantiate G4UIExecutive at start if a session is going to be needed so that G4cout, etc., can be captured.
	G4UIExecutive* ui = 0;

	// Adjust Qt request based on limitations of some operating systems.
	if (getenv("TOPAS_HEADLESS_MODE") || fPm->IsFindingSeed())
		fUseQt = false;
	else
		fUseQt = fPm->GetBooleanParameter("Ts/UseQt");

#if defined(G4UI_USE_QT) && !defined(G4VIS_USE_OPENGLX)
	if (!fUseQt && fGrm->UsingOpenGL()) {
		G4cout << "TOPAS is activating the Qt graphical user interface even though" << G4endl;
		G4cout << "you did not turn this feature on, as this is the only way to provide" << G4endl;
		G4cout << "your requested OpenGL graphics on your current operating system."<< G4endl;
		fUseQt = true;
	}
#endif

#if !defined(G4UI_USE_QT)
	if (fUseQt) {
		G4cout << "TOPAS is ignoring your request to use the Qt graphical user interface," << G4endl;
		G4cout << "as Qt is not available for the TOPAS build on your current operating system." << G4endl;
		fUseQt = false;
	}
#endif

	if (fUseQt)
		ui = new G4UIExecutive(argc, argv);
	else if (fPm->GetBooleanParameter("Ts/PauseBeforeInit") ||
			 fPm->GetBooleanParameter("Ts/PauseBeforeSequence") ||
			 fPm->GetBooleanParameter("Ts/PauseBeforeQuit"))
		ui = new G4UIExecutive(argc, argv, "tcsh");

	fVerbosity = fPm->GetIntegerParameter("Ts/SequenceVerbosity");

	G4int numberOfThreads = fPm->GetIntegerParameter("Ts/NumberOfThreads");
#ifdef TOPAS_MT
	if (numberOfThreads <= 0)
		numberOfThreads = G4Threading::G4GetNumberOfCores() + numberOfThreads;

	if (numberOfThreads > 1 && fPm->IsRandomMode()) {
		G4cerr << "TOPAS can not run in Random Time Mode with more than one thread." << G4endl;
		G4cerr << "Either set Tf/RandomizeTimeDistribution = \"False\" or set Ts/NumberOfThreads = 1" << G4endl;
		exit(1);
	}

	G4cout << "TOPAS is in MT mode, setting number of threads to: " << numberOfThreads << "\n" << G4endl;
	SetNumberOfThreads(numberOfThreads);
#else
	if (numberOfThreads != 1) {
		G4cout << "TOPAS is not in MT mode - ignoring attempt to set number of threads to: " << numberOfThreads << "\n" << G4endl;
	}
#endif

	// ParameterManager needs pointer back so it can call SequenceManager::NoteAnyUseOfChangeableParameters
	fPm->SetSequenceManager(this);

	// Read limits for various anomalies
	fMaxStepNumber = fPm->GetIntegerParameter("Ts/MaxStepNumber");
	fKilledTrackMaxEnergy = fPm->GetDoubleParameter("Ts/KilledTrackMaxEnergy", "Energy");
	fKilledTrackMaxCount = fPm->GetIntegerParameter("Ts/KilledTrackMaxCount");
	fKilledTrackMaxReports = fPm->GetIntegerParameter("Ts/KilledTrackMaxReports");

	fUnscoredHitMaxEnergy = fPm->GetDoubleParameter("Ts/UnscoredHitMaxEnergy", "Energy");
	fUnscoredHitMaxCount = fPm->GetIntegerParameter("Ts/UnscoredHitMaxCount");
	fUnscoredHitMaxReports = fPm->GetIntegerParameter("Ts/UnscoredHitMaxReports");

	fParameterizationErrorMaxEnergy = fPm->GetDoubleParameter("Ts/ParameterizationErrorMaxEnergy", "Energy");
	fParameterizationErrorMaxCount = fPm->GetIntegerParameter("Ts/ParameterizationErrorMaxCount");
	fParameterizationErrorMaxReports = fPm->GetIntegerParameter("Ts/ParameterizationErrorMaxReports");

	fIndexErrorMaxEnergy = fPm->GetDoubleParameter("Ts/IndexErrorMaxEnergy", "Energy");
	fIndexErrorMaxCount = fPm->GetIntegerParameter("Ts/IndexErrorMaxCount");
	fIndexErrorMaxReports = fPm->GetIntegerParameter("Ts/IndexErrorMaxReports");

	fInterruptedHistoryMaxCount = fPm->GetIntegerParameter("Ts/MaxInterruptedHistories");
	fInterruptedHistoryMaxReports = fPm->GetIntegerParameter("Ts/InterruptedHistoryMaxReports");

	// Timers accumulate total CPU time used at various stages of the job (init, execute, finalize).
	fTimer[0].Start();

	// Check that user has not linked against inappropriate copy of Geant4.
	G4Tokenizer next(GetVersionString());
	next();
	next();
	next();
	G4String geant4VersionString = next();
	if (geant4VersionString != fPm->GetStringParameter("Ts/Geant4VersionString")) {
		G4cerr << "TOPAS quitting with serious error." << G4endl;
		G4cerr << "This version of TOPAS expects the Geant4 version string to be: " << fPm->GetStringParameter("Ts/Geant4VersionString") << G4endl;
		G4cerr << "But the Geant4 version you have linked against has version string:" << geant4VersionString << G4endl;
		exit(1);
	}

	if (fPm->GetBooleanParameter("Ts/LimitConsoleToOneThread"))
		G4UImanager::GetUIpointer()->ApplyCommand("/control/cout/ignoreThreadsExcept 1");

	// User hook for start of session
	fEm->BeginSession(fPm);

	// Initialize geometry
	SetUserInitialization(fGm);
	InitializeGeometry();
	fGm->Initialize();

	std::vector<TsGeometrySampler*> vpgs;
	if (fPm->UseVarianceReduction()) {
		fVm->Configure();
		G4int index = -1;
		if ( fVm->BiasingProcessExists("geometricalparticlesplit", index) ) {
			vpgs = dynamic_cast<TsGeometricalParticleSplit*>(fVm->GetBiasingProcessFromList(index))->GetGeometrySampler();
			fPhm->AddBiasing(vpgs);
		}
	}

	// Initialize physics
	SetUserInitialization(fPhm->GetPhysicsList());

	// Optionally allow Geant4 commands before Init
	if (!fPm->GetBooleanParameter("Ts/UseQt") && fPm->GetBooleanParameter("Ts/PauseBeforeInit") && !getenv("TOPAS_HEADLESS_MODE")) {
		G4cout << G4endl;
		G4cout << "Pausing at Geant4 command line before initializing Geant4 physics." << G4endl;
		G4cout << "This pause is occurring because Ts/PauseBeforeInit = \"True\"" << G4endl;
		G4cout << "Enter any desired Geant4 commands and then return to TOPAS by typing \"exit\"" << G4endl;
		G4cout << "This feature should only be used by experts. Changes you make in Geant4 at this command line" << G4endl;
		G4cout << "are outside of TOPAS control and can result in unreliable behavior." << G4endl;
		ui->SessionStart();
	}

	// Initialize the physics
	InitializePhysics();
	initializedAtLeastOnce = true;

	// Starting with release 10.3, initializing physics causes G4State to be set to Init.
	// We need to set it back to Idle for next few actions to work.
	G4StateManager::GetStateManager()->SetNewState(G4ApplicationState::G4State_Idle);

	TsActionInitialization* actionInitialization = new TsActionInitialization(fPm, fEm, fGm, fSom, fFm, fChm, fVm, this);
#ifdef TOPAS_MT
	SetUserInitialization(actionInitialization);
	G4UImanager::GetUIpointer()->ApplyCommand("/particle/createAllIon");
#else
	actionInitialization->Build();
#endif

	// Initialize the graphics.
	// Must come after physics is initialized, since that moves the Geant4 state from PreInit to Idle,
	// and some commands used by graphics view are only availalble in Idle state.
	fGrm->Initialize();

	// Optionally allow Geant4 commands before Sequence
	if (fUseQt) {
#ifdef G4UI_USE_QT
		G4UIQt* UIQt = static_cast<G4UIQt*> (G4UImanager::GetUIpointer()->GetG4UIWindow());
		if (UIQt) fTsQt = new TsQt(fPm, fEm, fMm, fGm, fScm, this, fGrm, fSom);
		G4cout << "Pausing at Geant4 command line before starting run sequence." << G4endl;
		G4cout << "Use Qt widgets to adjust TOPAS Parameters, Components, Scorers, etc." << G4endl;
		G4cout << "Then hit the right arrow on the right side of the main menu to execute the TOPAS Run Sequence." << G4endl;
		G4cout << "You can also enter Geant4 commands on this command line, but note that changes made" << G4endl;
		G4cout << "this way are outside of TOPAS control and can result in unreliable behavior." << G4endl;
		ui->SessionStart();
#endif
	} else if (fPm->GetBooleanParameter("Ts/PauseBeforeSequence") && !getenv("TOPAS_HEADLESS_MODE")) {
		G4cout << "This pause is occurring because Ts/PauseBeforeSequence = \"True\"" << G4endl;
		G4cout << "Enter any desired Geant4 commands and then return to TOPAS by typing \"exit\"" << G4endl;
		G4cout << "This feature should only be used by experts. Changes you make in Geant4 at this command line" << G4endl;
		G4cout << "are outside of TOPAS control and can result in unreliable behavior." << G4endl;
		ui->SessionStart();
	}

	// Qt users trigger the Sequence by hitting the appropriate widget in the Qt GUI
	if (!fUseQt) {
		Sequence();

		if (fPm->ParameterExists("Ts/ExtraSequenceFiles")) {
			G4String* extraSequenceFileSpecs = fPm->GetStringVector("Ts/ExtraSequenceFiles");
			for (G4int iFile=0; iFile < fPm->GetVectorLength("Ts/ExtraSequenceFiles"); ++iFile)
				ExtraSequence(extraSequenceFileSpecs[iFile]);
		}
	}

	// User hook for end of session
	fEm->EndSession(fPm);

	// Optionally allow Geant4 commands before Quit
	if (!fUseQt && fPm->GetBooleanParameter("Ts/PauseBeforeQuit") && !getenv("TOPAS_HEADLESS_MODE")) {
		G4cout << G4endl;
		G4cout << "Pausing at Geant4 command line because Ts/PauseBeforeQuit = \"True\"" << G4endl;
		G4cout << "Enter any desired Geant4 commands and then end the session by typing \"exit\"" << G4endl;
		ui->SessionStart();
	}

	// Delete session
	delete ui;
}


TsSequenceManager::~TsSequenceManager()
{}


void TsSequenceManager::Sequence() {
	// Do this part only once. The rest may happen more than once if sequence is triggered by Qt
	if (fRunID == -1) {
		// Initialize the particle sources.
		fSom->Initialize(this);

		// Initialize the scoring.
		// Must come after physics is initialized, since needs pointers to specific particles and processes.
		fScm->Initialize();

		// Only advance runID after scorers are initialized, so that runID of -1 indicates not yet running
		fRunID++;

		// If restoring results from file, skip all of the actual Geant4 run work
		if (fPm->GetBooleanParameter("Ts/RestoreResultsFromFile")) {
			fTimer[0].Stop();
			G4cout << "\nRestoring results from file rather than performing simulation run." << G4endl;
			fScm->RestoreResultsFromFile();
			BeamOn(0); // Empty run to avoid segmentation fault
		} else {
			// Initialize the variance reduction:
			if (fPm->UseVarianceReduction())
					fVm->Initialize();

			// Show the list of physics processes:
			if (fPm->ParameterExists("Ph/ListProcesses") && fPm->GetBooleanParameter("Ph/ListProcesses") ) {
				G4cout << "\nRegistered Physics Processes:" << G4endl;
				G4UImanager::GetUIpointer()->ApplyCommand("/process/list");
				G4cout << "" << G4endl;
			}

			// Set thread buffering option
			if(fPm->GetBooleanParameter("Ts/BufferThreadOutput"))
				G4UImanager::GetUIpointer()->ApplyCommand("/control/cout/useBuffer true");

			// Finished with all initialization
			fTimer[0].Stop();
		}
	}

	if (fGrm->UsingRayTracer()) {
		G4cout << "At least one Graphics View has been set to type \"RayTracer\" " << G4endl;
		G4cout << "This graphics mode is intended only for drawing Geometry." << G4endl;
		G4cout << "Therefore, no actual particles will be generated." << G4endl;
	}

	fTimer[1].Start();

	if (!fPm->GetBooleanParameter("Ts/RestoreResultsFromFile") && !fGrm->UsingRayTracer()) {
		// Begin the run sequence, consisting of one or more BeamOn with different time feature values

		fIsExecutingSequence = true;
		std::vector<TsGeneratorManager*>::iterator iter;
		for (iter=fGeneratorManagers.begin(); iter!=fGeneratorManagers.end(); iter++)
			(*iter)->SetIsExecutingSequence(fIsExecutingSequence);

		// Verbosity
		G4UImanager::GetUIpointer()->ApplyCommand("/run/verbose " +
												  G4UIcommand::ConvertToString(fPm->GetIntegerParameter("Ts/RunVerbosity")));
		G4UImanager::GetUIpointer()->ApplyCommand("/event/verbose " +
												  G4UIcommand::ConvertToString(fPm->GetIntegerParameter("Ts/EventVerbosity")));
		G4UImanager::GetUIpointer()->ApplyCommand("/tracking/verbose " +
												  G4UIcommand::ConvertToString(fPm->GetIntegerParameter("Ts/TrackingVerbosity")));
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/verbose " +
												  G4UIcommand::ConvertToString(fPm->GetIntegerParameter("Gr/Verbosity")));
		if (fPm->GetIntegerParameter("Tf/Verbosity") > 0 ) fBCMFile.open("NbParticlesInTime.txt");

		// We must handle situations in which there are multiple sources, and it may be that different sources have
		// different numbers of events. To handle this, we execute BeamOn for the maximum number of events needed by any source.
		// GeneratorManager will call every source generator's GeneratePrimaries for every event. Any individual gneerator
		// that reaches its owm required number of events can ignore subsequent GeneratePrimaries calls.
		G4double timelineStart = fPm->GetDoubleParameter("Tf/TimelineStart", "Time");
		G4double timelineEnd = fPm->GetDoubleParameter("Tf/TimelineEnd", "Time");
		G4double timelineTotal = timelineEnd - timelineStart;

		if (fPm->GetBooleanParameter("Ts/ListUnusedParameters"))
			fPm->ListUnusedParameters();

		// If desired, will repeat the Run process until a given accuracy target is reached.
		G4bool hasUnsatisfiedLimits = true;
		while (hasUnsatisfiedLimits) {
			// Perform the runs, either random mode or sequential mode.
			if (fPm->IsRandomMode()) {
				// Random Time Mode: keep doing runs of one event per run until one of the geenrators reaches its target number of histories.
				if ( timelineTotal <= 0. ) {
					G4cerr << "Topas quitting. Total time interval has been set less than or equal to zero in Random Time Mode." << G4endl;
					exit(1);
				}
				while ( fSom->RandomModeNeedsMoreRuns() )
					Run(timelineStart + timelineTotal * G4UniformRand());

			} else {
				// Sequential Time Mode: have one run for each sequential time.
				G4int numTimes = fPm->GetIntegerParameter("Tf/NumberOfSequentialTimes");
				if ( numTimes < 1 ) {
					G4cerr << "Topas quitting. NumberOfSequentialTimes has been set less than 1 in Fixed or Sequential Time Mode." << G4endl;
					exit(1);
				} else if (numTimes == 1 ) {
					Run(timelineStart);
				} else {
					if ( timelineTotal < 0. ) {
						G4cerr << "Topas quitting. Total time interval has been set less than zero in Sequential Time Mode." << G4endl;
						exit(1);
					}
					G4double timelineInterval = timelineTotal / numTimes;
					for ( G4int steps =0; steps < numTimes; ++steps )
						Run(timelineStart + steps*timelineInterval);
				}
			}
			hasUnsatisfiedLimits = fScm->HasUnsatisfiedLimits();
		}

	    if (fPm->UseVarianceReduction())
	        fVm->Clear();

		G4cout << "\nTOPAS run sequence complete." << G4endl;
		if ( fPm->GetIntegerParameter("Tf/Verbosity") > 0 ) fBCMFile.close();
	}

	fTimer[1].Stop();

	fIsExecutingSequence = false;
	std::vector<TsGeneratorManager*>::iterator gIter;
	for (gIter=fGeneratorManagers.begin(); gIter!=fGeneratorManagers.end(); gIter++)
		(*gIter)->SetIsExecutingSequence(fIsExecutingSequence);

	// Finalize the sources (so they can report on any skipped particles) and the scorers (so they can output).
	fTimer[2].Start();
	fSom->Finalize();
	fScm->Finalize();
	fTimer[2].Stop();

	if (fKilledTrackCount > 0) {
		G4cerr << "\nNote that TOPAS killed one or more tracks during this session because they appeared to be stuck." << G4endl;
		G4cerr << "Specifically, their number of steps exceeded the value of " << fPm->GetIntegerParameter("Ts/MaxStepNumber") <<
		" set in the parameter Ts/MaxStepNumber." << G4endl;
		G4cerr << "The total number of affected tracks was: " << fKilledTrackCount << G4endl;
		G4cerr << "The total energy of affected tracks was: " << fKilledTrackEnergy << " MeV" << G4endl;
		G4cerr << "For details on each stuck track, see the console log for lines starting with: \"Topas killed a stuck track\"" << G4endl;
		G4cerr << "If you have only a small number of stuck tracks, there is generally no cause for alarm." << G4endl;
		G4cerr << "If you have a large number of stuck tracks, this may be due to invalid geometry, specifically overlaps." << G4endl;
		G4cerr << "In that case, be sure you have the parameter Ge/CheckForOverlaps set to true (its default value)," << G4endl;
		G4cerr << "and review the overlap report at the start of the TOPAS session." << G4endl;
		G4cerr << "You can set TOPAS to limit the number of messages about killed tracks by setting" << G4endl;
		G4cerr << "the parameter Ts/KilledTrackMaxReports" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given number of killed tracks by setting" << G4endl;
		G4cerr << "the parameter Ts/KilledTrackMaxCount" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given total energy from killed tracks by setting" << G4endl;
		G4cerr << "the parameter Ts/KilledTrackMaxEnergy" << G4endl;
	}

	if (fUnscoredHitCount > 0) {
		G4cerr << "\nNote that TOPAS omitted scoring one or more hits during this session because" << G4endl;
		G4cerr << "appropriate stopping power information was lacking for the given energy." << G4endl;
		G4cerr << "The total number of affected hits was: " << fUnscoredHitCount << G4endl;
		G4cerr << "The total energy of affected hits was: " << fUnscoredHitEnergy << " MeV" << G4endl;
		G4cerr << "For details on each unscored hit, see the console log for lines starting with: \"Topas omitted scoring a hit\"" << G4endl;
		G4cerr << "If you have only a small number of unscored hits, there is generally no cause for alarm." << G4endl;
		G4cerr << "If you have a large number of unscored hits, this may be due to invalid physics settings." << G4endl;
		G4cerr << "You can set TOPAS to limit the number of messages about unscored hits by setting" << G4endl;
		G4cerr << "the parameter Ts/UnscoredHitMaxReports" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given number of unscored hits by setting" << G4endl;
		G4cerr << "the parameter Ts/UnscoredHitMaxCount" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given total energy from unscored hits by setting" << G4endl;
		G4cerr << "the parameter Ts/UnscoredHitMaxEnergy" << G4endl;
	}

	if (fParameterizationErrorCount > 0) {
		G4cerr << "\nNote that TOPAS omitted scoring one or more hits during this session because" << G4endl;
		G4cerr << "Geant4 navigation seemed to be at the wrong level of the parameterization for this scorer." << G4endl;
		G4cerr << "The total number of affected hits was: " << fParameterizationErrorCount << G4endl;
		G4cerr << "The total energy of affected hits was: " << fParameterizationErrorEnergy << " MeV" << G4endl;
		G4cerr << "For details on each unscored hit, see the console log for lines starting with:" << G4endl;
		G4cerr << "\"Topas experienced a potentially serious error in scoring\"" << G4endl;
		G4cerr << "If you have only a small number of unscored hits, there is generally no cause for alarm." << G4endl;
		G4cerr << "If you have a large number of unscored hits, this may be due to a problem in Geant4 navigation." << G4endl;
		G4cerr << "You can set TOPAS to limit the number of messages about this parameterization error by setting" << G4endl;
		G4cerr << "the parameter Ts/ParameterizationErrorMaxReports" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given number of unscored hits by setting" << G4endl;
		G4cerr << "the parameter Ts/ParameterizationErrorMaxCount" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given total energy from unscored hits by setting" << G4endl;
		G4cerr << "the parameter Ts/ParameterizationErrorMaxEnergy" << G4endl;
	}

	if (fInterruptedHistoryCount > 0) {
		G4cerr << "\nNote that TOPAS interrupted one or more histories during this session" << G4endl;
		G4cerr << "due to errors reported by Geant4." << G4endl;
		G4cerr << "The total number of interrupted histories was: " << fInterruptedHistoryCount << G4endl;
		G4cerr << "For details on each of these errors, see the console log for \"G4Exception\"" << G4endl;
		G4cerr << "If only a very small proportion of your histories were affected," << G4endl;
		G4cerr << "you may be able to still use the overall result," << G4endl;
		G4cerr << "but this is a judgement call you need to make for yourself." << G4endl;
		G4cerr << "You can set TOPAS to limit the number of messages about interrupted histories by setting" << G4endl;
		G4cerr << "the parameter Ts/InterruptedHistoryMaxReports" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given number of interrupted histories by setting" << G4endl;
		G4cerr << "the parmameter Ts/MaxInterruptedHistories" << G4endl;
	}

	// Print time use summary
	if (fPm->ParameterExists("Ts/ShowCPUTime") && fPm->GetBooleanParameter("Ts/ShowCPUTime")) {
		G4double TotalRealTime;
		G4double TotalUserTime;
		G4double TotalSysTime;
		G4Timer PMTimer = fPm->GetTimer();
		G4cout<<G4endl;
		G4cout<<"Elapsed times:" << G4endl;
		G4cout<<std::setw(20)<<"Parameter Reading : " << PMTimer  << G4endl;
		G4cout<<std::setw(20)<<"Initialization: "     << fTimer[0]<< G4endl;
		G4cout<<std::setw(20)<<"Execution: "          << fTimer[1]<< G4endl;
		G4cout<<std::setw(20)<<"Finalization: "       << fTimer[2]<< G4endl;
		TotalUserTime   = PMTimer.GetUserElapsed()
		+ fTimer[0].GetUserElapsed()
		+ fTimer[1].GetUserElapsed()
		+ fTimer[2].GetUserElapsed();
		TotalRealTime   = PMTimer.GetRealElapsed()
		+ fTimer[0].GetRealElapsed()
		+ fTimer[1].GetRealElapsed()
		+ fTimer[2].GetRealElapsed();
		TotalSysTime = PMTimer.GetSystemElapsed()
		+ fTimer[0].GetSystemElapsed()
		+ fTimer[1].GetSystemElapsed()
		+ fTimer[2].GetSystemElapsed();
		G4cout<<std::setw(20)<<"Total: " <<"User="<< TotalUserTime<<"s "
		<<"Real="<< TotalRealTime<<"s "
		<<"Sys="<< TotalSysTime<<"s" << G4endl;
	}

	if (fPm->HasGeometryOverlap()) {
		G4cerr << "\nThis TOPAS session found geometry overlap issues." << G4endl;
		G4cerr << "Details are shown earlier in this console (near start of session)." << G4endl;
		G4cerr << "While normally TOPAS will quit in such situations," << G4endl;
		G4cerr << "in this case it continued  because you set the parameter Ge/QuitIfOverlapDetected to False." << G4endl;
		G4cerr << "Be advised that simulation results can not be trusted when any overlap exists." << G4endl;
	}
}


G4bool TsSequenceManager::IsExecutingSequence() {
	return fIsExecutingSequence;
}


void TsSequenceManager::ExtraSequence(G4String extraSequenceFileSpec) {
	// Loop to check for existance of this file on disk.
	// If it is not present, sleep and then repeat the loop.
	G4double sleepInterval = fPm->GetDoubleParameter("Ts/ExtraSequenceSleepInterval", "Time");
	G4double sleepLimit = fPm->GetDoubleParameter("Ts/ExtraSequenceSleepLimit", "Time");
	G4double sleepTime = 0.;
	std::ifstream infile(extraSequenceFileSpec, std::ios::in & std::ios::binary);

	G4cout << "SleepInterval (seconds): " << sleepInterval / s << G4endl;
	G4cout << "SleepLimit (seconds): " << sleepLimit / s << G4endl;

	while (!infile && sleepTime < sleepLimit) {
		G4cout << "TOPAS was unable to open ExtraSequenceFile: " << extraSequenceFileSpec << G4endl;
		G4cout << "Will sleep " << sleepInterval / s << " seconds and then try again." << G4endl;
		sleep(sleepInterval / s);
		sleepTime += sleepInterval;
		infile.open(extraSequenceFileSpec, std::ios::in & std::ios::binary);
	}

	if (!infile) {
		G4cerr << "Topas quitting. Unable to find ExtraSequence file." << G4endl;
		G4cerr << "Exceeded allowed sleep limit of " << sleepLimit / s << " seconds." << G4endl;
		exit(1);
	}

	G4cout << "\nPerforming Extra Sequence: " << extraSequenceFileSpec << G4endl;
	std::vector<G4String>* names = new std::vector<G4String>;
	std::vector<G4String>* values = new std::vector<G4String>;
	fPm->ReadFile(extraSequenceFileSpec, infile, names, values);

	G4int length = names->size();
	for (G4int iToken=0; iToken<length; iToken++) {
		G4String name = (*names)[iToken];
		G4String nameLower = name;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(nameLower);
#else
		nameLower.toLower();
#endif
		if (nameLower == "includefile") {
			G4cerr << "Topas quitting. ExtraSequence file contains IncludeFile." << G4endl;
			G4cerr << "This is not permitted in an ExtraSequence file." << G4endl;
			exit(1);
		}

		G4String value = (*values)[iToken];
		G4cout << "ExtraSequence updating parameter: " << name << " to value: " << value << G4endl;
		fPm->AddParameter(name, value);
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(name);
#else
		name.toLower();
#endif
		G4int colonPos = name.find( ":" );
		UpdateForSpecificParameterChange(name.substr(colonPos+1));
	}
	delete names;
	delete values;

	fPm->UpdateTimeFeatureStore();
	UpdateForNewRunOrQtChange();
	ClearGenerators();
	Sequence();
}


// During initialization of managers, note their use any changeable parameters
void TsSequenceManager::NoteAnyUseOfChangeableParameters(const G4String& parameter)
{
#ifdef TOPAS_MT
	G4AutoLock l(&noteAnyUseMutex);
#endif

	fGm->NoteAnyUseOfChangeableParameters(parameter);
	fSom->NoteAnyUseOfChangeableParameters(parameter);
	fScm->NoteAnyUseOfChangeableParameters(parameter);
	fFm->NoteAnyUseOfChangeableParameters(parameter);
	fGrm->NoteAnyUseOfChangeableParameters(parameter);

	std::vector<TsGeneratorManager*>::iterator gIter;
	for (gIter=fGeneratorManagers.begin(); gIter!=fGeneratorManagers.end(); gIter++)
		(*gIter)->NoteAnyUseOfChangeableParameters(parameter);
}


void TsSequenceManager::UpdateForSpecificParameterChange(const G4String& parameterName)
{
	if (fVerbosity > 0)
		G4cout << "TsSequenceManager::UpdateForSpecificParameterChange called for parameterName: " << parameterName << G4endl;
	
	fGm ->UpdateForSpecificParameterChange(parameterName);
	fSom->UpdateForSpecificParameterChange(parameterName);
	fFm ->UpdateForSpecificParameterChange(parameterName);
	fScm->UpdateForSpecificParameterChange(parameterName);
	fGrm->UpdateForSpecificParameterChange(parameterName);

	std::vector<TsGeneratorManager*>::iterator gIter;
	for (gIter=fGeneratorManagers.begin(); gIter!=fGeneratorManagers.end(); gIter++)
		(*gIter)->UpdateForSpecificParameterChange(parameterName);
}


void TsSequenceManager::UpdateForNewRunOrQtChange() {
	// Will force update of placement if this is the first run and any components added transient parameters, since these may affect
	// placement of other components that were already added.
	G4bool forceUpdatePlacement = fPm->AddParameterHasBeenCalled() && G4RunManager::GetRunManager()->GetCurrentRun() == 0;
	if (fVerbosity > 0 && forceUpdatePlacement)
		G4cout << "Setting forceUpdatePlacement for first run since AddParameterHasBeenCalled" << G4endl;

	G4bool rebuiltSomeComponents = fGm->UpdateForNewRun(this, forceUpdatePlacement);

	// Call the rest of the updatable managers, telling them whether any components were rebuilt.
	fSom->UpdateForNewRun(rebuiltSomeComponents);

	if (fPm->IsFindingSeed())
		return;

	fFm ->UpdateForNewRun(rebuiltSomeComponents);
	fScm->UpdateForNewRun(rebuiltSomeComponents);
	fGrm->UpdateForNewRun(rebuiltSomeComponents);
	fVm ->UpdateForNewRun(rebuiltSomeComponents);

	std::vector<TsGeneratorManager*>::iterator gIter;
	for (gIter=fGeneratorManagers.begin(); gIter!=fGeneratorManagers.end(); gIter++)
		(*gIter)->UpdateForNewRun(rebuiltSomeComponents);

	// The following call will cause ConstructSDandField to be called on all the workers.
	// This is needed to reattach scorers to any rebuilt components.
	// Tell it not to reoptimize the full geometry (we elsewhere tell it which subsections
	// we explicitly want to reoptimize).
	if (fRunID > 0 && rebuiltSomeComponents) {
		SetGeometryToBeOptimized(false);
		ReinitializeGeometry();
	}
}


void TsSequenceManager::ClearGenerators() {
	std::vector<TsGeneratorManager*>::iterator gIter;
	for (gIter=fGeneratorManagers.begin(); gIter!=fGeneratorManagers.end(); gIter++)
		(*gIter)->ClearGenerators();
}

void TsSequenceManager::Run(G4double currentTime) {
	// User hook for begin of run
	fEm->BeginRun(fPm);

	fTime = currentTime;

	// For each run, loop over time features, advising all of the updatable managers of any changed time features.
	// Each manager will pay attention only to those features it noted above.
	G4String name;
	G4bool found;
	std::set<G4String> alreadyHandled;
	std::set<G4String>::const_iterator iter;

	std::vector<TsVParameter*>* timeFeatureStore = fPm->GetTimeFeatureStore(currentTime);
	size_t tf_number = timeFeatureStore->size();
	for (size_t tf =0; tf < tf_number; ++tf) {
		TsVParameter* timeFeatureParameter = (*timeFeatureStore)[tf];

	    // If the time feature occurs more than once in the hierarchy, only need to handle top instance
	    name = timeFeatureParameter->GetName();
	    found = false;
	    for (iter=alreadyHandled.begin(); iter!=alreadyHandled.end() && !found; iter++)
	        if (*iter == name)
	            found = true;

	    if (!found) {
	        alreadyHandled.insert(name);

	        if ( timeFeatureParameter->ValueHasChanged() ) {

	            if ( fPm->GetIntegerParameter("Tf/Verbosity")>0 ) {
	                G4String tf_type = timeFeatureParameter->GetType();
	                G4String value_string;
	                if ( tf_type == "b") {
	                    G4bool myvalue = timeFeatureParameter->GetBooleanValue();
	                    if (myvalue)
	                        value_string = "\"true\"";
	                    else
	                        value_string = "\"false\"";
	                } else if ( tf_type == "d" ) {
	                    G4String unit_string = timeFeatureParameter->GetUnit();
	                    G4double myvalue = timeFeatureParameter->GetDoubleValue()/fPm->GetUnitValue(unit_string);
	                    value_string = G4UIcommand::ConvertToString(myvalue) + " " + unit_string;
	                } else if ( tf_type == "i" ) {
	                    G4int myvalue = timeFeatureParameter->GetIntegerValue();
	                    value_string = G4UIcommand::ConvertToString(myvalue);
	                } else if ( tf_type == "s" ) {
	                    value_string = "\"" + timeFeatureParameter->GetStringValue() +"\"";
	                } else {
	                    G4double myvalue = timeFeatureParameter->GetUnitlessValue();
	                    value_string = G4UIcommand::ConvertToString(myvalue);
	                }
	                G4cout<<"\nTimeFeature updated: "<< timeFeatureParameter->GetName() << " = " << value_string << " at: "<< currentTime/ms <<" (ms)" << G4endl;
	            }

	            G4String parameterName = timeFeatureParameter->GetName();
	            UpdateForSpecificParameterChange(parameterName);
	        }
	    }
	}

	// Advise all of the updateable managers that a new run is starting.
	if ( fPm->GetIntegerParameter("Tf/Verbosity") > 1 ) G4cout << "Updating Time Features for time interval:" << currentTime/ms << G4endl;

	UpdateForNewRunOrQtChange();

	// Have to wait until after fSom update to retrieve the current number of histories in run.
	G4int nEvents;
	if (fPm->IsRandomMode())
		nEvents = 1;
	else
		nEvents = fSom->GetNumberOfHistoriesInRun();

	if ( fPm->GetIntegerParameter("Tf/Verbosity") > 0 ) fBCMFile << currentTime/ms <<" "<< nEvents <<std::endl;

	// Dump parameters to file if requested.
	if (fPm->GetBooleanParameter("Ts/DumpParameters"))
		fPm->DumpParameters(fTime, true);
	if (fPm->GetBooleanParameter("Ts/DumpNonDefaultParameters"))
		fPm->DumpParameters(fTime, false);
	if (fPm->ParameterExists("Ts/DumpParametersToSimpleFile"))
		fPm->DumpParametersToSimpleFile(fTime);
	if (fPm->ParameterExists("Ts/DumpParametersToSemicolonSeparatedFile"))
		fPm->DumpParametersToSemicolonSeparatedFile(fTime);

#ifdef TOPAS_MT
	if (G4MTRunManager::GetMasterRunManager()->GetNumberOfThreads() > 1) {
		// Hand set the Geant4 "eventModulo". This is the number of events each worker should be given at any one time.
		// Our setting is actually the same as the default that Geant4 calculates by default in G4MTRunManager,
		// but we set it explicitly here so that if Geant4 later changes the algorithm, we don't end up with a wrong assumption
		// (we need this number during phase space reading and there is no way to ask Geant4 for this number).
		eventModulo = int(std::sqrt(double(nEvents / G4MTRunManager::GetMasterRunManager()->GetNumberOfThreads())));
		if (eventModulo < 1) eventModulo = 1;
		G4UImanager::GetUIpointer()->ApplyCommand("/run/eventModulo " + G4UIcommand::ConvertToString(eventModulo));
	}
#endif

	// Run the events
	BeamOn(nEvents);

	// Advise scoring manager and geometry manager that the run is over.
	fSom->UpdateForEndOfRun();
	fScm->UpdateForEndOfRun();
	fGrm->UpdateForEndOfRun();

	std::vector<TsGeneratorManager*>::iterator gIter;
	for (gIter=fGeneratorManagers.begin(); gIter!=fGeneratorManagers.end(); gIter++)
		(*gIter)->Finalize();

	// User hook for end of run
	fEm->EndRun(fPm);

	fRunID++;
}


G4int TsSequenceManager::GetRunID() {
	return fRunID;
}


G4double TsSequenceManager::GetTime() {
	return fTime;
}


G4bool TsSequenceManager::UsingRayTracer() {
	return fGrm->UsingRayTracer();
}


void TsSequenceManager::NoteKilledTrack(G4double energy, G4String particleName, G4String processName, G4String volumeName) {
#ifdef TOPAS_MT
	G4AutoLock l(&noteKilledTrackMutex);
#endif

	fKilledTrackCount++;
	fKilledTrackEnergy += energy;

	if ((fKilledTrackMaxReports == 0) || (fKilledTrackCount <= fKilledTrackMaxReports)) {
		G4cerr << "\nTopas killed a stuck track, defined as that the number of steps has exceeded value of " << fMaxStepNumber << G4endl;
		G4cerr << "which is set in parameter Ts/MaxStepNumber." << G4endl;
		G4cerr << "Run: " << fPm->GetRunID() << ", History: " << G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID() << G4endl;
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
		G4cerr << "Seed: " << SeedPart1 << ", " << SeedPart2 << ", " << SeedPart3 << ", " << SeedPart4 << G4endl;
		G4cerr << "TOPAS Time (the value used in time features): " << GetTime() / s << " s" << G4endl;
		G4cerr << "Particle type: " << particleName << G4endl;
		G4cerr << "Origin process: " << processName << G4endl;
		G4cerr << "Current volume: " << volumeName << G4endl;
		G4cerr << "Track will be killed, and scoring will not include the remaining kinetic energy: " << energy << " MeV\n" << G4endl;

		if (fKilledTrackCount == fKilledTrackMaxReports) {
			G4cerr << "The session has hit the limit for the number of times this issue will be reported." << G4endl;
			G4cerr << "To change this reporting limit, adjust the parameter Ts/KilledTrackMaxReports" << G4endl;
		}
	}

	if ((fKilledTrackMaxCount != 0) && (fKilledTrackCount > fKilledTrackMaxCount)) {
		G4cerr << "\nTOPAS is exiting because the number of killed tracks in this session has exceeded: " << fKilledTrackMaxCount << G4endl;
		G4cerr << "These tracks were killed because they appeared to be stuck." << G4endl;
		G4cerr << "Specifically, their number of steps exceeded the value of " << fMaxStepNumber << G4endl;
		G4cerr << "which is set in parameter Ts/MaxStepNumber." << G4endl;
		G4cerr << "The total number of killed tracks was: " << fKilledTrackCount << G4endl;
		G4cerr << "The total energy of killed tracks was: " << fKilledTrackEnergy << " MeV" << G4endl;
		G4cerr << "For details on each killed track, see the console log for lines starting with: \"Topas killed a track\"" << G4endl;
		G4cerr << "If you have only a small number of killed tracks, there is generally no cause for alarm." << G4endl;
		G4cerr << "If you have a large number of killed tracks, this may be due to ." << G4endl;
		G4cerr << "You can set TOPAS to limit the number of messages about killed tracks by setting" << G4endl;
		G4cerr << "the parameter Ts/KilledTrackMaxReports" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given number of killed tracks by setting" << G4endl;
		G4cerr << "the parameter Ts/KilledTrackMaxCount" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given total energy from killed tracks by setting" << G4endl;
		G4cerr << "the parameter Ts/KilledTrackMaxEnergy" << G4endl;
		exit(1);
	}

	if ((fKilledTrackMaxEnergy != 0.) && (fKilledTrackEnergy > fKilledTrackMaxEnergy)){
		G4cerr << "\nTOPAS is exiting because the total energy of killed tracks in this session has exceeded: " << fKilledTrackMaxEnergy << " MeV" << G4endl;
		G4cerr << "These tracks were killed because they appeared to be stuck." << G4endl;
		G4cerr << "Specifically, their number of steps exceeded the value of " << fMaxStepNumber << G4endl;
		G4cerr << "which is set in parameter Ts/MaxStepNumber." << G4endl;
		G4cerr << "The total number of killed tracks was: " << fKilledTrackCount << G4endl;
		G4cerr << "The total energy of killed tracks was: " << fKilledTrackEnergy << " MeV" << G4endl;
		G4cerr << "For details on each killed track, see the console log for lines starting with: \"Topas killed a track\"" << G4endl;
		G4cerr << "If you have only a small number of killed tracks, there is generally no cause for alarm." << G4endl;
		G4cerr << "If you have a large number of killed tracks, this may be due to ." << G4endl;
		G4cerr << "You can set TOPAS to limit the number of messages about killed tracks by setting" << G4endl;
		G4cerr << "the parameter Ts/KilledTrackaxReports" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given number of killed tracks by setting" << G4endl;
		G4cerr << "the parameter Ts/KilledTrackMaxCount" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given total energy from killed tracks by setting" << G4endl;
		G4cerr << "the parameter Ts/KilledTrackMaxEnergy" << G4endl;
		exit(1);
	}
}


void TsSequenceManager::NoteUnscoredHit(G4double energy, G4String scorerName) {
#ifdef TOPAS_MT
	G4AutoLock l(&noteUnscoredHitMutex);
#endif

	fUnscoredHitCount++;
	fUnscoredHitEnergy += energy;

	if ((fUnscoredHitMaxReports == 0) || (fUnscoredHitCount <= fUnscoredHitMaxReports)) {
		G4cout << "\nScorer " << scorerName << " has omitted a hit that had energy: " << energy << " MeV." << G4endl;
		G4cout << "This hit could not be counted since the relevant material stopping power was unknown." << G4endl;
		G4cerr << "We think this may sometimes happen because Geant4 navigation gets into a corrupted state." << G4endl;

		if (fUnscoredHitCount == fUnscoredHitMaxReports) {
			G4cerr << "The session has hit the limit for the number of times this issue will be reported." << G4endl;
			G4cerr << "To change this reporting limit, adjust the parameter Ts/UnscoredHitMaxReports" << G4endl;
		}
	}

	if ((fUnscoredHitMaxCount != 0) && (fUnscoredHitCount > fUnscoredHitMaxCount)){
		G4cerr << "\nTOPAS is exiting because the number of unscored hits in this session has exceeded: " << fUnscoredHitMaxCount << G4endl;
		G4cout << "This hits could not be counted since the relevant material stopping power was unknown." << G4endl;
		G4cerr << "We think this may sometimes happen because Geant4 navigation gets into a corrupted state." << G4endl;
		G4cerr << "The total number of affected hits was: " << fUnscoredHitCount << G4endl;
		G4cerr << "The total energy of affected hits was: " << fUnscoredHitEnergy << " MeV" << G4endl;
		G4cerr << "For details on each unscored hit, see the console log for lines starting with: \"Topas omitted scoring a hit\"" << G4endl;
		G4cerr << "If you have only a small number of unscored hits, there is generally no cause for alarm." << G4endl;
		G4cerr << "If you have a large number of unscored hits, this may be due to invalid physics settings." << G4endl;
		G4cerr << "You can set TOPAS to limit the number of messages about unscored hits by setting" << G4endl;
		G4cerr << "the parameter Ts/UnscoredHitMaxReports" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given number of unscored hits by setting" << G4endl;
		G4cerr << "the parameter Ts/UnscoredHitMaxCount" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given total energy from unscored hits by setting" << G4endl;
		G4cerr << "the parameter Ts/UnscoredHitMaxEnergy" << G4endl;
		exit(1);
	}

	if ((fUnscoredHitMaxEnergy != 0.) && (fUnscoredHitEnergy > fUnscoredHitMaxEnergy)){
		G4cerr << "\nTOPAS is exiting because the total energy of unscored hits in this session has exceeded: " << fUnscoredHitMaxEnergy << " MeV" << G4endl;
		G4cout << "This hits could not be counted since the relevant material stopping power was unknown." << G4endl;
		G4cerr << "We think this may sometimes happen because Geant4 navigation gets into a corrupted state." << G4endl;
		G4cerr << "The total number of affected hits was: " << fUnscoredHitCount << G4endl;
		G4cerr << "The total energy of affected hits was: " << fUnscoredHitEnergy << " MeV" << G4endl;
		G4cerr << "For details on each unscored hit, see the console log for lines starting with: \"Topas omitted scoring a hit\"" << G4endl;
		G4cerr << "If you have only a small number of unscored hits, there is generally no cause for alarm." << G4endl;
		G4cerr << "If you have a large number of unscored hits, this may be due to invalid physics settings." << G4endl;
		G4cerr << "You can set TOPAS to limit the number of messages about unscored hits by setting" << G4endl;
		G4cerr << "the parameter Ts/UnscoredHitMaxReports" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given number of unscored hits by setting" << G4endl;
		G4cerr << "the parameter Ts/UnscoredHitMaxCount" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given total energy from unscored hits by setting" << G4endl;
		G4cerr << "the parameter Ts/UnscoredHitMaxEnergy" << G4endl;
		exit(1);
	}
}


void TsSequenceManager::NoteParameterizationError(G4double energy, G4String componentName, G4String volumeName) {
#ifdef TOPAS_MT
	G4AutoLock l(&noteParameterizationErrorMutex);
#endif

	fParameterizationErrorCount++;
	fParameterizationErrorEnergy += energy;

	if ((fParameterizationErrorMaxReports == 0) || (fParameterizationErrorCount <= fParameterizationErrorMaxReports)) {
		G4cerr << "\nTopas experienced an error in scoring." << G4endl;
		G4cerr << "A scorer in the Component: \"" << componentName << "\"" << G4endl;
		G4cerr << "has been called for a hit in the non-parameterized volume named: \"" << volumeName << "\"" << G4endl;
		G4cerr << "But this deos not make sense, as this scorer should only get called for hits" << G4endl;
		G4cerr << "in a parameterized volume (volume name would include the string \"_Division\")." << G4endl;
		G4cerr << "We think this may sometimes happen because Geant4 navigation gets into a corrupted state." << G4endl;
		G4cerr << "This hit will be omitted from scoring." << G4endl;
		G4cerr << "A note at end of session will show total number of unscored steps and the total unscored energy." << G4endl;
		G4cerr << "In some cases the unscored energy is small enough that you can ignore this issue." << G4endl;
		G4cerr << "This hit's energy deposit would have been: " << energy << " MeV" << G4endl;

		if (fParameterizationErrorCount == fParameterizationErrorMaxReports) {
			G4cerr << "The session has hit the limit for the number of times this issue will be reported." << G4endl;
			G4cerr << "To change this reporting limit, adjust the parameter Ts/ParameterizationErrorMaxReports" << G4endl;
		}
	}

	if ((fParameterizationErrorMaxCount != 0) && (fParameterizationErrorCount > fParameterizationErrorMaxCount)){
		G4cerr << "\nTOPAS is exiting because the number of hits with parameterized volume anomalies has exceeded: "
			<< fParameterizationErrorMaxCount << G4endl;
		G4cerr << "The total number of affected hits was: " << fParameterizationErrorCount << G4endl;
		G4cerr << "The total energy of affected hits was: " << fParameterizationErrorEnergy << " MeV" << G4endl;
		G4cerr << "For details on each such hit, see the console log for lines starting with:" << G4endl;
		G4cerr << "\"Topas experienced a potentially serious error in scoring\"" << G4endl;
		G4cerr << "If you have only a small number of such hits, there is generally no cause for alarm." << G4endl;
		G4cerr << "You can set TOPAS to limit the number of messages about such hits by setting" << G4endl;
		G4cerr << "the parameter Ts/ParameterizationErrorMaxReports" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given number of such hits by setting" << G4endl;
		G4cerr << "the parameter Ts/ParameterizationErrorMaxCount" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given total energy from such hits by setting" << G4endl;
		G4cerr << "the parameter Ts/ParameterizationErrorMaxEnergy" << G4endl;
		exit(1);
	}

	if ((fParameterizationErrorMaxEnergy != 0.) && (fParameterizationErrorEnergy > fParameterizationErrorMaxEnergy)){
		G4cerr << "\nTOPAS is exiting because the total energy of hits with parameterized volume anomalies has exceeded: " << fParameterizationErrorMaxEnergy << " MeV" << G4endl;
		G4cerr << "The total number of affected hits was: " << fParameterizationErrorCount << G4endl;
		G4cerr << "The total energy of affected hits was: " << fParameterizationErrorEnergy << " MeV" << G4endl;
		G4cerr << "For details on each such hit, see the console log for lines starting with:" << G4endl;
		G4cerr << "\"Topas experienced a potentially serious error in scoring\"" << G4endl;
		G4cerr << "If you have only a small number of such hits, there is generally no cause for alarm." << G4endl;
		G4cerr << "You can set TOPAS to limit the number of messages about unscored hits by setting" << G4endl;
		G4cerr << "the parameter Ts/ParameterizationErrorMaxReports" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given number of unscored hits by setting" << G4endl;
		G4cerr << "the parameter Ts/ParameterizationErrorMaxCount" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given total energy from unscored hits by setting" << G4endl;
		G4cerr << "the parameter Ts/ParameterizationErrorMaxEnergy" << G4endl;
		exit(1);
	}
}


void TsSequenceManager::NoteIndexError(G4double energy, G4String componentName,
									   G4String coordinate, G4int value, G4int limit) {
#ifdef TOPAS_MT
	G4AutoLock l(&noteIndexErrorMutex);
#endif

	fIndexErrorCount++;
	fIndexErrorEnergy += energy;

	if ((fIndexErrorMaxReports == 0) || (fIndexErrorCount <= fIndexErrorMaxReports)) {
		G4cerr << "\nTopas experienced an error in scoring." << G4endl;
		G4cerr << "A scorer in the Component: \"" << componentName << "\"" << G4endl;
		G4cerr << "returned " << coordinate << " index: " << value << " outside of the valid range of 0 to " << limit << G4endl;
		G4cerr << "We think this may sometimes happen because Geant4 navigation gets into a corrupted state." << G4endl;
		G4cerr << "This hit will be omitted from scoring." << G4endl;
		G4cerr << "A note at end of session will show total number of unscored steps and the total unscored energy." << G4endl;
		G4cerr << "In some cases the unscored energy is small enough that you can ignore this issue." << G4endl;
		G4cerr << "This hit's energy deposit would have been: " << energy << " MeV" << G4endl;

		if (fIndexErrorCount == fIndexErrorMaxReports) {
			G4cerr << "The session has hit the limit for the number of times this issue will be reported." << G4endl;
			G4cerr << "To change this reporting limit, adjust the parameter Ts/IndexErrorMaxReports" << G4endl;
		}
	}

	if ((fIndexErrorMaxCount != 0) && (fIndexErrorCount > fIndexErrorMaxCount)){
		G4cerr << "\nTOPAS is exiting because the number of hits with parameterized volume anomalies has exceeded: "
			<< fIndexErrorMaxCount << G4endl;
		G4cerr << "The total number of affected hits was: " << fIndexErrorCount << G4endl;
		G4cerr << "The total energy of affected hits was: " << fIndexErrorEnergy << " MeV" << G4endl;
		G4cerr << "For details on each such hit, see the console log for lines starting with:" << G4endl;
		G4cerr << "\"Topas experienced a potentially serious error in scoring\"" << G4endl;
		G4cerr << "If you have only a small number of such hits, there is generally no cause for alarm." << G4endl;
		G4cerr << "You can set TOPAS to limit the number of messages about such hits by setting" << G4endl;
		G4cerr << "the parameter Ts/IndexErrorMaxReports" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given number of such hits by setting" << G4endl;
		G4cerr << "the parameter Ts/IndexErrorMaxCount" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given total energy from such hits by setting" << G4endl;
		G4cerr << "the parameter Ts/IndexErrorMaxEnergy" << G4endl;
		exit(1);
	}

	if ((fIndexErrorMaxEnergy != 0.) && (fIndexErrorEnergy > fIndexErrorMaxEnergy)){
		G4cerr << "\nTOPAS is exiting because the total energy of hits with parameterized volume anomalies has exceeded: " << fIndexErrorMaxEnergy << " MeV" << G4endl;
		G4cerr << "The total number of affected hits was: " << fIndexErrorCount << G4endl;
		G4cerr << "The total energy of affected hits was: " << fIndexErrorEnergy << " MeV" << G4endl;
		G4cerr << "For details on each such hit, see the console log for lines starting with:" << G4endl;
		G4cerr << "\"Topas experienced a potentially serious error in scoring\"" << G4endl;
		G4cerr << "If you have only a small number of such hits, there is generally no cause for alarm." << G4endl;
		G4cerr << "You can set TOPAS to limit the number of messages about such hits by setting" << G4endl;
		G4cerr << "the parameter Ts/IndexErrorMaxReports" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given number of such hits by setting" << G4endl;
		G4cerr << "the parameter Ts/IndexErrorMaxCount" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given total energy from such hits by setting" << G4endl;
		G4cerr << "the parameter Ts/IndexErrorMaxEnergy" << G4endl;
		exit(1);
	}
}


void TsSequenceManager::NoteInterruptedHistory() {
#ifdef TOPAS_MT
	G4AutoLock l(&noteInterruptedHistoryMutex);
#endif

	fInterruptedHistoryCount++;

	if ((fInterruptedHistoryMaxCount != 0) && (fInterruptedHistoryCount > fInterruptedHistoryMaxCount)){
		G4cerr << "\nTOPAS is exiting because the number of interrupted histories in this session has reached: " << fInterruptedHistoryCount << G4endl;
		G4cerr << "These histories were interrupted due errors reported by Geant4." << G4endl;
		G4cerr << "For details on each of these errors, see the console log for \"G4Exception\"" << G4endl;
		G4cerr << "If only a very small proportion of your histories were affected," << G4endl;
		G4cerr << "you may want to adjust the limit set in Ts/MaxInterruptedHistories" << G4endl;
		G4cerr << "but this is a judgement call you need to make for yourself." << G4endl;
		G4cerr << "You can set TOPAS to limit the number of messages about interrupted histories by setting" << G4endl;
		G4cerr << "the parameter Ts/InterruptedHistoryMaxReports" << G4endl;
		G4cerr << "You can set TOPAS to exit if it exceeds a given number of interrupted histories by setting" << G4endl;
		G4cerr << "the parmameter Ts/MaxInterruptedHistories" << G4endl;
		exit(1);
	}
}


void TsSequenceManager::RegisterGeneratorManager(TsGeneratorManager* pgM) {
	fGeneratorManagers.push_back(pgM);
}


void TsSequenceManager::RegisterSteppingAction(TsSteppingAction* steppingAction) {
#ifdef TOPAS_MT
	G4AutoLock l(&registerSteppingActionMutex);
#endif
	fSteppingActions.push_back(steppingAction);
}


void TsSequenceManager::HandleFirstEvent() {
	// Delaying instantiation until this point allows the thread to populate the physics tables
	// that are needed for some filters
	fScm->InstantiateFilters();

	// Check that all source and scoring filter parameters refer to known filters
	fPm->CheckFilterParameterNamesStartingWith("Source", fPm->GetFilterNames());
	fPm->CheckFilterParameterNamesStartingWith("Scoring", fPm->GetFilterNames());

	// Now that filters have been instantiated, stepping actions can tell if they are needed
	std::vector<TsSteppingAction*>::iterator iter;
	for (iter=fSteppingActions.begin(); iter!=fSteppingActions.end(); iter++)
		(*iter)->CacheNeedsSteppingAction();
}


void TsSequenceManager::AbortSession(G4int exitCode) {
	if (fUseQt) {
#ifdef G4UI_USE_QT
		if (fPm->IsInQtSession())
			fTsQt->AbortSession();
		else
			exit(exitCode);
#endif
	} else {
		exit(exitCode);
	}
}
