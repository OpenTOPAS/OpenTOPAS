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

#include "TsMain.hh"

#include "TsParameterManager.hh"
#include "TsExtensionManager.hh"
#include "TsMaterialManager.hh"
#include "TsGeometryManager.hh"
#include "TsPhysicsManager.hh"
#include "TsVarianceManager.hh"
#include "TsFilterManager.hh"
#include "TsScoringManager.hh"
#include "TsGraphicsManager.hh"
#include "TsSequenceManager.hh"
#include "TsChemistryManager.hh"
#include "TsSourceManager.hh"
#include "TsTopasConfig.hh"

#include "G4UImanager.hh"

int setenv(const char*, const char*, int);
void setDataEnvVars(G4String);

TsMain::TsMain(int argc, char** argv)
{
	G4String topasVersion = G4UIcommand::ConvertToString(TOPAS_VERSION_MAJOR) + "." +
	G4UIcommand::ConvertToString(TOPAS_VERSION_MINOR);
	if (TOPAS_VERSION_PATCH > 0)
		topasVersion = topasVersion + ".p" + G4UIcommand::ConvertToString(TOPAS_VERSION_PATCH);

	// If any of the arguments are "--version" then print version string and exit
	if (argc >= 2) {
		for (int i=1; i<argc; ++i) {
			if (strcmp(argv[i], "--version") == 0) {
				std::cout << topasVersion << std::endl;
				exit(0);
			}
		}
	}

	// This is the one place in TOPAS that we use std::cout rather than G4cout (G4cout is not available yet).
	std::cout << std::endl;
	std::cout << "Welcome to TOPAS, Tool for Particle Simulation (Version " << topasVersion << ")" << std::endl;

	// Instantiate the parameter manager.
	TsParameterManager* parameterManager= new TsParameterManager(argc, argv, topasVersion);

	// Set data directories
	G4String dataDirectory = parameterManager->GetStringParameter("Ts/G4DataDirectory");
	if (dataDirectory!="") {
		G4cout << "\nGeant4 Data directory has been specified by the" << G4endl;
		G4cout << "Ts/G4DataDirectory parameter as " << dataDirectory << G4endl;
	} else if (getenv("TOPAS_G4_DATA_DIR")) {
		dataDirectory = getenv("TOPAS_G4_DATA_DIR");
		G4cout << "\nGeant4 Data directory has been specified by the" << G4endl;
		G4cout << "TOPAS_G4_DATA_DIR environment variable as " << dataDirectory << G4endl;
	}

	if (dataDirectory == "") {
		G4cout << "\nGeant4 Data directory has not been specified by the" << G4endl;
		G4cout << "Ts/G4DataDirectory parameter or the TOPAS_G4_DATA_DIR environment variable." << G4endl;
		G4cout << "We will assume you have explicitly set all of the data environment variables." << G4endl;
	} else {
		setDataEnvVars(dataDirectory);
	}

	// Initialize the random seed
	G4Random::setTheEngine(new CLHEP::RanecuEngine);
	G4Random::setTheSeed(parameterManager->GetIntegerParameter("Ts/Seed"));

	if (parameterManager->ParameterExists("Ts/SeedFile")) {
		G4cout << "" << G4endl;
		if (parameterManager->ParameterExists("Ts/SeedDirectory")) {
			G4cout << "TOPAS reading seed from directory: " << parameterManager->GetStringParameter("Ts/SeedDirectory") << G4endl;
			G4UImanager::GetUIpointer()->ApplyCommand("/random/setDirectoryName " + parameterManager->GetStringParameter("Ts/SeedDirectory"));
		}
		G4cout << "TOPAS reading seed from file: " << parameterManager->GetStringParameter("Ts/SeedFile") << G4endl;
		G4UImanager::GetUIpointer()->ApplyCommand("/random/resetEngineFrom " + parameterManager->GetStringParameter("Ts/SeedFile"));
	}

	// Instantiate the other managers.
	TsExtensionManager*	extensionManager = new TsExtensionManager(parameterManager);
	TsMaterialManager*	materialManager	 = new TsMaterialManager(parameterManager);
	TsGeometryManager*	geometryManager	 = new TsGeometryManager(parameterManager, extensionManager, materialManager);
	TsPhysicsManager*	physicsManager	 = new TsPhysicsManager(parameterManager, extensionManager, geometryManager);
	TsVarianceManager*	varianceManager	 = new TsVarianceManager(parameterManager, geometryManager, physicsManager);
	TsFilterManager*	filterManager	 = new TsFilterManager(parameterManager, extensionManager, materialManager, geometryManager);
	TsScoringManager*	scoringManager	 = new TsScoringManager(parameterManager, extensionManager, materialManager, geometryManager, filterManager);
	TsGraphicsManager*	graphicsManager	 = new TsGraphicsManager(parameterManager, geometryManager);
	TsChemistryManager*	chemistryManager = new TsChemistryManager(parameterManager);
	TsSourceManager*	sourceManager	 = new TsSourceManager(parameterManager, geometryManager, extensionManager);
	TsSequenceManager*	sequenceManager	 = new TsSequenceManager(parameterManager, extensionManager, materialManager, geometryManager, physicsManager,
																 varianceManager, filterManager, scoringManager, graphicsManager, chemistryManager,
																 sourceManager, argc, argv);

	// Delete the managers
	delete sequenceManager;
	delete graphicsManager;
	delete scoringManager;
	delete filterManager;
	delete physicsManager;
	delete varianceManager;
	delete materialManager;
	delete chemistryManager;
	delete sourceManager;
	// delete of geometryManager is handled automatically by Geant4
	delete extensionManager;
	delete parameterManager;
}

TsMain::~TsMain()
{
}

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
int setenv(const char* name, const char* value, int overwrite)
{
	int errcode = 0;
	if (!overwrite) {
		size_t envsize = 0;
		errcode = getenv_s(&envsize, NULL, 0, name);
		if (errcode || envsize) return errcode;
	}
	return _putenv_s(name, value);
}
#endif

void setDataEnvVars(G4String dataDirectory)
{
	std::vector<std::tuple<G4String, G4String>> dataVars;
#if GEANT4_VERSION_MAJOR == 10 && GEANT4_VERSION_MINOR == 6
	dataVars.push_back({"G4LEDATA", "G4EMLOW7.9.1"});
	dataVars.push_back({"G4NEUTRONHPDATA", "G4NDL4.6"});
	dataVars.push_back({"G4LEVELGAMMADATA", "PhotonEvaporation5.5"});
	dataVars.push_back({"G4RADIOACTIVEDATA", "RadioactiveDecay5.4"});
	dataVars.push_back({"G4SAIDXSDATA", "G4SAIDDATA2.0"});
	dataVars.push_back({"G4PARTICLEXSDATA", "G4PARTICLEXS2.1"});
	dataVars.push_back({"G4PIIDATA", "G4PII1.3"});
	dataVars.push_back({"G4REALSURFACEDATA", "RealSurface2.1.1"});
	dataVars.push_back({"G4ABLADATA", "G4ABLA3.1"});
	dataVars.push_back({"G4INCLDATA", "G4INCL1.0"});
	dataVars.push_back({"G4ENSDFSTATEDATA", "G4ENSDFSTATE2.2"});
	dataVars.push_back({"G4TENDLDATA", "G4TENDL1.3.2"});
	dataVars.push_back({"G4PROTONHPDATA", "G4TENDL1.3.2/Proton"});
#elif GEANT4_VERSION_MAJOR == 10 && GEANT4_VERSION_MINOR == 7
	dataVars.push_back({"G4LEDATA", "G4EMLOW7.13"});
	dataVars.push_back({"G4NEUTRONHPDATA", "G4NDL4.6"});
	dataVars.push_back({"G4LEVELGAMMADATA", "PhotonEvaporation5.7"});
	dataVars.push_back({"G4RADIOACTIVEDATA", "RadioactiveDecay5.6"});
	dataVars.push_back({"G4SAIDXSDATA", "G4SAIDDATA2.0"});
	dataVars.push_back({"G4PARTICLEXSDATA", "G4PARTICLEXS3.1.1"});
	dataVars.push_back({"G4PIIDATA", "G4PII1.3"});
	dataVars.push_back({"G4REALSURFACEDATA", "RealSurface2.2"});
	dataVars.push_back({"G4ABLADATA", "G4ABLA3.1"});
	dataVars.push_back({"G4INCLDATA", "G4INCL1.0"});
	dataVars.push_back({"G4ENSDFSTATEDATA", "G4ENSDFSTATE2.3"});
	dataVars.push_back({"G4TENDLDATA", "G4TENDL1.3.2"});
	dataVars.push_back({"G4PROTONHPDATA", "G4TENDL1.3.2/Proton"});
#elif GEANT4_VERSION_MAJOR == 11
	dataVars.push_back({"G4LEDATA", "G4EMLOW8.2"});
	dataVars.push_back({"G4NEUTRONHPDATA", "G4NDL4.7"});
	dataVars.push_back({"G4LEVELGAMMADATA", "PhotonEvaporation5.7"});
	dataVars.push_back({"G4RADIOACTIVEDATA", "RadioactiveDecay5.6"});
	dataVars.push_back({"G4SAIDXSDATA", "G4SAIDDATA2.0"});
	dataVars.push_back({"G4PARTICLEXSDATA", "G4PARTICLEXS4.0"});
	dataVars.push_back({"G4PIIDATA", "G4PII1.3"});
	dataVars.push_back({"G4REALSURFACEDATA", "RealSurface2.2"});
	dataVars.push_back({"G4ABLADATA", "G4ABLA3.1"});
	dataVars.push_back({"G4INCLDATA", "G4INCL1.0"});
	dataVars.push_back({"G4ENSDFSTATEDATA", "G4ENSDFSTATE2.3"});
	dataVars.push_back({"G4TENDLDATA", "G4TENDL1.4"});
	dataVars.push_back({"G4PROTONHPDATA", "G4TENDL1.4/Proton"});
	dataVars.push_back({"G4LENDDATA", "LEND_GND1.3_ENDF.BVII.1"});

#else
	G4cout << "\nUnable to use Ts/G4DataDirectory parameter or TOPAS_G4_DATA_DIR environment variable" << G4endl;
	G4cout << "as TOPAS does not know the set of data files needed for this Geant4 release." << G4endl;
	G4cout << "You should instead set all data file environment variables individually." << G4endl;
	exit(1);
#endif
	
	G4String varval;
	for (auto const& pair : dataVars)
	{
		varval = dataDirectory + "/" + std::get<1>(pair);
		setenv(std::get<0>(pair), varval, 0);
	}
}
