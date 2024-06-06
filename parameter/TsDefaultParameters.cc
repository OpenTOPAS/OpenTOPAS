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

#include "TsDefaultParameters.hh"
#include "TsParameterFile.hh"
#include "TsTopasConfig.hh"

#include <sstream>
#include <iomanip>

void TsDefaultParameters::SetDefaults(TsParameterFile* file)
{
	std::ostringstream ss;
	ss << "geant4-" << std::setw(2) << std::setfill('0') << GEANT4_VERSION_MAJOR
	   << "-" << std::setw(2) << std::setfill('0') << GEANT4_VERSION_MINOR;
	if (GEANT4_VERSION_PATCH > 0)
		ss << "-patch-" << std::setw(2) << std::setfill('0') << GEANT4_VERSION_PATCH;
	file->AddTempParameter("s:Ts/Geant4VersionString", "\"" + ss.str() + "\"");

	file->AddTempParameter("i:Ts/Seed", "1");
	file->AddTempParameter("i:Ts/MaxStepNumber", "1000000");
	file->AddTempParameter("b:Ts/DumpParameters", "\"False\"");
	file->AddTempParameter("b:Ts/DumpNonDefaultParameters", "\"False\"");
	file->AddTempParameter("b:Ts/ListUnusedParameters", "\"False\"");
	file->AddTempParameter("b:Ts/LimitConsoleToOneThread", "\"False\"");
	file->AddTempParameter("i:Ts/ShowHistoryCountAtInterval", "1");
	file->AddTempParameter("i:Ts/MaxShowHistoryCountInterval", "2147483647");
	file->AddTempParameter("b:Ts/ShowHistoryCountOnSingleLine", "\"False\"");
	file->AddTempParameter("b:Ts/IncludeTimeInHistoryCount", "\"False\"");
	file->AddTempParameter("b:Ts/ShowHistoryCountLessFrequentlyAsSimulationProgresses", "\"False\"");
	file->AddTempParameter("i:Ts/RunIDPadding", "4");
	file->AddTempParameter("b:Ts/PauseBeforeInit", "\"False\"");
	file->AddTempParameter("b:Ts/PauseBeforeSequence", "\"False\"");
	file->AddTempParameter("b:Ts/PauseBeforeQuit", "\"False\"");
	file->AddTempParameter("b:Ts/IncludeDefaultGeant4QtWidgets", "\"False\"");
	file->AddTempParameter("i:Ts/RunVerbosity", "0");
	file->AddTempParameter("i:Ts/EventVerbosity", "0");
	file->AddTempParameter("i:Ts/TrackingVerbosity", "0");
	file->AddTempParameter("i:Ts/SequenceVerbosity", "0");
	file->AddTempParameter("b:Ts/FullRebuildTestMode","\"False\"");
	file->AddTempParameter("b:Ts/DisableReoptimizeTestMode","\"False\"");
	file->AddTempParameter("b:Ts/RestoreResultsFromFile","\"False\"");
	file->AddTempParameter("i:Ts/FindSeedForRun", "0");
	file->AddTempParameter("i:Ts/FindSeedForHistory", "-1");
	file->AddTempParameter("i:Ts/NumberOfThreads", "1");
	file->AddTempParameter("b:Ts/BufferThreadOutput", "\"False\"");
	file->AddTempParameter("b:Ts/TreatExcitedIonsAsGroundState", "\"False\"");
	file->AddTempParameter("s:Ts/G4DataDirectory", "\"\"");
	file->AddTempParameter("b:Ts/UseQt", "\"False\"");
	file->AddTempParameter("d:Ts/ExtraSequenceSleepInterval", "10 s");
	file->AddTempParameter("d:Ts/ExtraSequenceSleepLimit", "36000 s");
	file->AddTempParameter("b:Ts/QuitIfManyHistoriesSeemAnomalous","\"True\"");
	file->AddTempParameter("i:Ts/NumberOfAnomalousHistoriesToAllowInARow","10000");
	file->AddTempParameter("d:Ts/KilledTrackMaxEnergy", "0. MeV");
	file->AddTempParameter("i:Ts/KilledTrackMaxCount","0");
	file->AddTempParameter("i:Ts/KilledTrackMaxReports","0");
	file->AddTempParameter("d:Ts/UnscoredHitMaxEnergy", "0. MeV");
	file->AddTempParameter("i:Ts/UnscoredHitMaxCount","0");
	file->AddTempParameter("i:Ts/UnscoredHitMaxReports","0");
	file->AddTempParameter("d:Ts/ParameterizationErrorMaxEnergy", "0. MeV");
	file->AddTempParameter("i:Ts/ParameterizationErrorMaxCount","0");
	file->AddTempParameter("i:Ts/ParameterizationErrorMaxReports","0");
	file->AddTempParameter("d:Ts/IndexErrorMaxEnergy", "0. MeV");
	file->AddTempParameter("i:Ts/IndexErrorMaxCount","0");
	file->AddTempParameter("i:Ts/IndexErrorMaxReports","0");
	file->AddTempParameter("i:Ts/MaxInterruptedHistories","0");
	file->AddTempParameter("i:Ts/InterruptedHistoryMaxReports","0");

	file->AddTempParameter("s:Ph/ListName", "\"Default\"");
	file->AddTempParameter("b:Ph/ListProcesses", "\"False\"");
	file->AddTempParameter("b:Ph/SetNeutronToStable", "\"False\"");
	file->AddTempParameter("s:Ph/Default/Type", "\"Geant4_Modular\"");
	file->AddTempParameter("sv:Ph/Default/Modules", "6 \"g4em-standard_opt4\" \"g4h-phy_QGSP_BIC_HP\" \"g4decay\" \"g4ion-binarycascade\" \"g4h-elastic_HP\" \"g4stopping\"");
	file->AddTempParameter("d:Ph/Default/EMRangeMin", "100. eV");
	file->AddTempParameter("d:Ph/Default/EMRangeMax", "500. MeV");

	file->AddTempParameter("b:Tf/RandomizeTimeDistribution", "\"False\"");
	file->AddTempParameter("d:Tf/TimelineStart", "0. s");
	file->AddTempParameter("d:Tf/TimelineEnd", "Tf/TimelineStart s");
	file->AddTempParameter("i:Tf/NumberOfSequentialTimes", "1");
	file->AddTempParameter("i:Tf/Verbosity", "0");

	file->AddTempParameter("i:Ge/Verbosity", "0");
	file->AddTempParameter("b:Ge/CheckForOverlaps", "\"True\"");
	file->AddTempParameter("b:Ge/CheckInsideEnvelopesForOverlaps", "\"False\"");
	file->AddTempParameter("i:Ge/CheckForOverlapsResolution", "1000");
	file->AddTempParameter("d:Ge/CheckForOverlapsTolerance", "0. mm");
	file->AddTempParameter("b:Ge/QuitIfOverlapDetected", "\"True\"");
	file->AddTempParameter("i:Ge/NumberOfPointsPerOverlapCheck", "100");
	file->AddTempParameter("b:Ge/CheckForUnusedComponents", "\"True\"");

// Leave this line in place until finish deprecating this parameter at next major release.
// This is required in case some existing parameter file is currently setting this without specifying a parameter type.
	file->AddTempParameter("b:Ge/AllowDividedCylinderOrSphereWithParallelWorld", "\"False\"");

	file->AddTempParameter("b:Ge/ForceUseOfCoupledTransportation", "\"False\"");
	file->AddTempParameter("b:Ge/CacheMaterialMapForEachTimeSlice", "\"True\"");
	file->AddTempParameter("s:Ge/World/Type", "\"TsBox\"");
	file->AddTempParameter("s:Ge/World/Material", "\"Air\"");
	file->AddTempParameter("d:Ge/World/HLX", "5. m");
	file->AddTempParameter("d:Ge/World/HLY", "5. m");
	file->AddTempParameter("d:Ge/World/HLZ", "5. m");
	file->AddTempParameter("d:Ge/World/TransX", "0. m");
	file->AddTempParameter("d:Ge/World/TransY", "0. m");
	file->AddTempParameter("d:Ge/World/TransZ", "0. m");
	file->AddTempParameter("d:Ge/World/RotX", "0. deg");
	file->AddTempParameter("d:Ge/World/RotY", "0. deg");
	file->AddTempParameter("d:Ge/World/RotZ", "0. deg");

	file->AddTempParameter("s:Ge/BeamPosition/Parent", "\"World\"");
	file->AddTempParameter("s:Ge/BeamPosition/Type", "\"Group\"");
	file->AddTempParameter("d:Ge/BeamPosition/TransX", "0. m");
	file->AddTempParameter("d:Ge/BeamPosition/TransY", "0. m");
	file->AddTempParameter("d:Ge/BeamPosition/TransZ", "Ge/World/HLZ m");
	file->AddTempParameter("d:Ge/BeamPosition/RotX", "180. deg");
	file->AddTempParameter("d:Ge/BeamPosition/RotY", "0. deg");
	file->AddTempParameter("d:Ge/BeamPosition/RotZ", "0. deg");

	file->AddTempParameter("i:So/Verbosity", "0");
	file->AddTempParameter("s:So/Demo/Type", "\"Beam\"");
	file->AddTempParameter("s:So/Demo/Component", "\"BeamPosition\"");
	file->AddTempParameter("s:So/Demo/BeamParticle", "\"proton\"");
	file->AddTempParameter("d:So/Demo/BeamEnergy", "169.23 MeV");
	file->AddTempParameter("u:So/Demo/BeamEnergySpread", "0.757504");
	file->AddTempParameter("s:So/Demo/BeamPositionDistribution", "\"Gaussian\"");
	file->AddTempParameter("s:So/Demo/BeamPositionCutoffShape", "\"Ellipse\"");
	file->AddTempParameter("d:So/Demo/BeamPositionCutoffX", "10. cm");
	file->AddTempParameter("d:So/Demo/BeamPositionCutoffY", "10. cm");
	file->AddTempParameter("d:So/Demo/BeamPositionSpreadX", "0.65 cm");
	file->AddTempParameter("d:So/Demo/BeamPositionSpreadY", "0.65 cm");
	file->AddTempParameter("s:So/Demo/BeamAngularDistribution", "\"Gaussian\"");
	file->AddTempParameter("d:So/Demo/BeamAngularCutoffX", "90. deg");
	file->AddTempParameter("d:So/Demo/BeamAngularCutoffY", "90. deg");
	file->AddTempParameter("d:So/Demo/BeamAngularSpreadX", "0.0032 rad");
	file->AddTempParameter("d:So/Demo/BeamAngularSpreadY", "0.0032 rad");
	file->AddTempParameter("i:So/Demo/NumberOfHistoriesInRun", "0");
	file->AddTempParameter("i:So/Demo/NumberOfHistoriesInRandomJob", "0");

	file->AddTempParameter("s:El/Hydrogen/Symbol", "\"H\"");
	file->AddTempParameter("s:El/Helium/Symbol", "\"He\"");
	file->AddTempParameter("s:El/Lithium/Symbol", "\"Li\"");
	file->AddTempParameter("s:El/Beryllium/Symbol", "\"Be\"");
	file->AddTempParameter("s:El/Boron/Symbol", "\"B\"");
	file->AddTempParameter("s:El/Carbon/Symbol", "\"C\"");
	file->AddTempParameter("s:El/Nitrogen/Symbol", "\"N\"");
	file->AddTempParameter("s:El/Oxygen/Symbol", "\"O\"");
	file->AddTempParameter("s:El/Fluorine/Symbol", "\"F\"");
	file->AddTempParameter("s:El/Neon/Symbol", "\"Ne\"");
	file->AddTempParameter("s:El/Sodium/Symbol", "\"Na\"");
	file->AddTempParameter("s:El/Magnesium/Symbol", "\"Mg\"");
	file->AddTempParameter("s:El/Aluminum/Symbol", "\"Al\"");
	file->AddTempParameter("s:El/Silicon/Symbol", "\"Si\"");
	file->AddTempParameter("s:El/Phosphorus/Symbol", "\"P\"");
	file->AddTempParameter("s:El/Sulfur/Symbol", "\"S\"");
	file->AddTempParameter("s:El/Chlorine/Symbol", "\"Cl\"");
	file->AddTempParameter("s:El/Argon/Symbol", "\"Ar\"");
	file->AddTempParameter("s:El/Potassium/Symbol", "\"K\"");
	file->AddTempParameter("s:El/Calcium/Symbol", "\"Ca\"");
	file->AddTempParameter("s:El/Scandium/Symbol", "\"Sc\"");
	file->AddTempParameter("s:El/Titanium/Symbol", "\"Ti\"");
	file->AddTempParameter("s:El/Vanadium/Symbol", "\"V\"");
	file->AddTempParameter("s:El/Chromium/Symbol", "\"Cr\"");
	file->AddTempParameter("s:El/Manganese/Symbol", "\"Mn\"");
	file->AddTempParameter("s:El/Iron/Symbol", "\"Fe\"");
	file->AddTempParameter("s:El/Cobalt/Symbol", "\"Co\"");
	file->AddTempParameter("s:El/Nickel/Symbol", "\"Ni\"");
	file->AddTempParameter("s:El/Copper/Symbol", "\"Cu\"");
	file->AddTempParameter("s:El/Zinc/Symbol", "\"Zn\"");
	file->AddTempParameter("s:El/Gallium/Symbol", "\"Ga\"");
	file->AddTempParameter("s:El/Germanium/Symbol", "\"Ge\"");
	file->AddTempParameter("s:El/Arsenic/Symbol", "\"As\"");
	file->AddTempParameter("s:El/Selenium/Symbol", "\"Se\"");
	file->AddTempParameter("s:El/Bromine/Symbol", "\"Br\"");
	file->AddTempParameter("s:El/Krypton/Symbol", "\"Kr\"");
	file->AddTempParameter("s:El/Rubidium/Symbol", "\"Rb\"");
	file->AddTempParameter("s:El/Strontium/Symbol", "\"Sr\"");
	file->AddTempParameter("s:El/Yttrium/Symbol", "\"Y\"");
	file->AddTempParameter("s:El/Zirconium/Symbol", "\"Zr\"");
	file->AddTempParameter("s:El/Niobium/Symbol", "\"Nb\"");
	file->AddTempParameter("s:El/Molybdenum/Symbol", "\"Mo\"");
	file->AddTempParameter("s:El/Technetium/Symbol", "\"Tc\"");
	file->AddTempParameter("s:El/Ruthenium/Symbol", "\"Ru\"");
	file->AddTempParameter("s:El/Rhodium/Symbol", "\"Rh\"");
	file->AddTempParameter("s:El/Palladium/Symbol", "\"Pd\"");
	file->AddTempParameter("s:El/Silver/Symbol", "\"Ag\"");
	file->AddTempParameter("s:El/Cadmium/Symbol", "\"Cd\"");
	file->AddTempParameter("s:El/Indium/Symbol", "\"In\"");
	file->AddTempParameter("s:El/Tin/Symbol", "\"Sn\"");
	file->AddTempParameter("s:El/Antimony/Symbol", "\"Sb\"");
	file->AddTempParameter("s:El/Tellurium/Symbol", "\"Te\"");
	file->AddTempParameter("s:El/Iodine/Symbol", "\"I\"");
	file->AddTempParameter("s:El/Xenon/Symbol", "\"Xe\"");
	file->AddTempParameter("s:El/Caesium/Symbol", "\"Cs\"");
	file->AddTempParameter("s:El/Barium/Symbol", "\"Ba\"");
	file->AddTempParameter("s:El/Lanthanum/Symbol", "\"La\"");
	file->AddTempParameter("s:El/Cerium/Symbol", "\"Ce\"");
	file->AddTempParameter("s:El/Praseodymium/Symbol", "\"Pr\"");
	file->AddTempParameter("s:El/Neodymium/Symbol", "\"Nd\"");
	file->AddTempParameter("s:El/Promethium/Symbol", "\"Pm\"");
	file->AddTempParameter("s:El/Samarium/Symbol", "\"Sm\"");
	file->AddTempParameter("s:El/Europium/Symbol", "\"Eu\"");
	file->AddTempParameter("s:El/Gadolinium/Symbol", "\"Gd\"");
	file->AddTempParameter("s:El/Terbium/Symbol", "\"Tb\"");
	file->AddTempParameter("s:El/Dysprosium/Symbol", "\"Dy\"");
	file->AddTempParameter("s:El/Holmium/Symbol", "\"Ho\"");
	file->AddTempParameter("s:El/Erbium/Symbol", "\"Er\"");
	file->AddTempParameter("s:El/Thulium/Symbol", "\"Tm\"");
	file->AddTempParameter("s:El/Ytterbium/Symbol", "\"Yb\"");
	file->AddTempParameter("s:El/Lutetium/Symbol", "\"Lu\"");
	file->AddTempParameter("s:El/Hafnium/Symbol", "\"Hf\"");
	file->AddTempParameter("s:El/Tantalum/Symbol", "\"Ta\"");
	file->AddTempParameter("s:El/Tungsten/Symbol", "\"W\"");
	file->AddTempParameter("s:El/Rhenium/Symbol", "\"Re\"");
	file->AddTempParameter("s:El/Osmium/Symbol", "\"Os\"");
	file->AddTempParameter("s:El/Iridium/Symbol", "\"Ir\"");
	file->AddTempParameter("s:El/Platinum/Symbol", "\"Pt\"");
	file->AddTempParameter("s:El/Gold/Symbol", "\"Au\"");
	file->AddTempParameter("s:El/Mercury/Symbol", "\"Hg\"");
	file->AddTempParameter("s:El/Thallium/Symbol", "\"Tl\"");
	file->AddTempParameter("s:El/Lead/Symbol", "\"Pb\"");
	file->AddTempParameter("s:El/Bismuth/Symbol", "\"Bi\"");
	file->AddTempParameter("s:El/Polonium/Symbol", "\"Po\"");
	file->AddTempParameter("s:El/Astatine/Symbol", "\"At\"");
	file->AddTempParameter("s:El/Radon/Symbol", "\"Rn\"");
	file->AddTempParameter("s:El/Francium/Symbol", "\"Fr\"");
	file->AddTempParameter("s:El/Radium/Symbol", "\"Ra\"");

	file->AddTempParameter("s:Ma/DefaultColor", "\"white\"");
	file->AddTempParameter("i:Ma/Verbosity", "0");
	file->AddTempParameter("b:Ma/SetOpticalPropertiesForPatientMaterials", "\"False\"");
	file->AddTempParameter("sv:Ma/ExtraMaterialNamesForQtMenu", "10 \"G4_WATER\" \"G4_He\" \"G4_Au\" \"G4_W\" \"G4_ADIPOSE_TISSUE_ICRP\" \"G4_BONE_COMPACT_ICRU\" \"G4_BONE_CORTICAL_ICRP\" \"G4_MUSCLE_SKELETAL_ICRP\" \"G4_SKIN_ICRP\" \"G4_TISSUE_SOFT_ICRP\"");

	file->AddTempParameter("sv:Ma/Vacuum/Components", "4 \"Carbon\" \"Nitrogen\" \"Oxygen\" \"Argon\"");
	file->AddTempParameter("uv:Ma/Vacuum/Fractions", "4 0.000124 0.755268 0.231781 0.012827");
	file->AddTempParameter("d:Ma/Vacuum/Density", "1.0E-25 g/cm3");
	file->AddTempParameter("s:Ma/Vacuum/State", "\"Gas\"");
	file->AddTempParameter("d:Ma/Vacuum/Temperature", "2.73 kelvin");
	file->AddTempParameter("d:Ma/Vacuum/Pressure", "3.0E-18 pascal");
	file->AddTempParameter("s:Ma/Vacuum/DefaultColor", "\"skyblue\"");

	file->AddTempParameter("sv:Ma/Carbon/Components", "1 \"Carbon\"");
	file->AddTempParameter("uv:Ma/Carbon/Fractions", "1 1.0");
	file->AddTempParameter("d:Ma/Carbon/Density", "1.867 g/cm3");
	file->AddTempParameter("d:Ma/Carbon/MeanExcitationEnergy", "78 eV");
	file->AddTempParameter("s:Ma/Carbon/DefaultColor", "\"green\"");

	file->AddTempParameter("sv:Ma/Aluminum/Components", "1 \"Aluminum\"");
	file->AddTempParameter("uv:Ma/Aluminum/Fractions", "1 1.0");
	file->AddTempParameter("d:Ma/Aluminum/Density", "2.6989 g/cm3");
	file->AddTempParameter("s:Ma/Aluminum/DefaultColor", "\"skyblue\"");
	file->AddTempParameter("i:Ma/Aluminum/AtomicNumber", "13");
	file->AddTempParameter("d:Ma/Aluminum/AtomicMass", "26.98154 g/mole");

	file->AddTempParameter("sv:Ma/Nickel/Components", "1 \"Nickel\"");
	file->AddTempParameter("uv:Ma/Nickel/Fractions", "1 1.0");
	file->AddTempParameter("d:Ma/Nickel/Density", "8.902 g/cm3");
	file->AddTempParameter("s:Ma/Nickel/DefaultColor", "\"indigo\"");

	file->AddTempParameter("sv:Ma/Copper/Components", "1 \"Copper\"");
	file->AddTempParameter("uv:Ma/Copper/Fractions", "1 1.0");
	file->AddTempParameter("d:Ma/Copper/Density", "8.96 g/cm3");
	file->AddTempParameter("s:Ma/Copper/DefaultColor", "\"orange\"");

	file->AddTempParameter("sv:Ma/Iron/Components", "1 \"Iron\"");
	file->AddTempParameter("uv:Ma/Iron/Fractions", "1 1.0");
	file->AddTempParameter("d:Ma/Iron/Density", "7.87 g/cm3");
	file->AddTempParameter("s:Ma/Iron/DefaultColor", "\"skyblue\"");

	file->AddTempParameter("sv:Ma/Tantalum/Components", "1 \"Tantalum\"");
	file->AddTempParameter("uv:Ma/Tantalum/Fractions", "1 1.0");
	file->AddTempParameter("d:Ma/Tantalum/Density", "16.654 g/cm3");
	file->AddTempParameter("s:Ma/Tantalum/DefaultColor", "\"indigo\"");

	file->AddTempParameter("sv:Ma/Lead/Components", "1 \"Lead\"");
	file->AddTempParameter("uv:Ma/Lead/Fractions", "1 1.0");
	file->AddTempParameter("d:Ma/Lead/Density", "11.35 g/cm3");
	file->AddTempParameter("i:Ma/Lead/AtomicNumber", "82");
	file->AddTempParameter("d:Ma/Lead/AtomicMass", "207.19 g/mole");
	file->AddTempParameter("d:Ma/Lead/MeanExcitationEnergy", "823 eV");
	file->AddTempParameter("s:Ma/Lead/DefaultColor", "\"brown\"");

	file->AddTempParameter("sv:Ma/Air/Components", "4 \"Carbon\" \"Nitrogen\" \"Oxygen\" \"Argon\"");
	file->AddTempParameter("uv:Ma/Air/Fractions", "4 0.000124 0.755268 0.231781 0.012827");
	file->AddTempParameter("d:Ma/Air/Density", "1.20484 mg/cm3");
	file->AddTempParameter("d:Ma/Air/MeanExcitationEnergy", "85.7 eV");
	file->AddTempParameter("s:Ma/Air/DefaultColor", "\"lightblue\"");

	file->AddTempParameter("sv:Ma/Brass/Components", "2  \"Copper\" \"Zinc\"");
	file->AddTempParameter("uv:Ma/Brass/Fractions", "2 0.7 0.3");
	file->AddTempParameter("d:Ma/Brass/Density", "8.550 g/cm3");
	file->AddTempParameter("d:Ma/Brass/MeanExcitationEnergy", "324.4 eV");
	file->AddTempParameter("s:Ma/Brass/DefaultColor", "\"grass\"");

	file->AddTempParameter("sv:Ma/Lexan/Components", "3  \"Hydrogen\" \"Carbon\" \"Oxygen\"");
	file->AddTempParameter("uv:Ma/Lexan/Fractions", "3 0.055491 0.755751 0.188758");
	file->AddTempParameter("d:Ma/Lexan/Density", "1.2 g/cm3");
	file->AddTempParameter("d:Ma/Lexan/MeanExcitationEnergy", "73.1 eV");
	file->AddTempParameter("s:Ma/Lexan/DefaultColor", "\"grey\"");

	file->AddTempParameter("sv:Ma/Lucite/Components", "3 \"Hydrogen\" \"Carbon\" \"Oxygen\"");
	file->AddTempParameter("uv:Ma/Lucite/Fractions", "3 0.080538 0.599848 0.319614");
	file->AddTempParameter("d:Ma/Lucite/Density", "1.190 g/cm3");
	file->AddTempParameter("d:Ma/Lucite/MeanExcitationEnergy", "74.0 eV");
	file->AddTempParameter("s:Ma/Lucite/DefaultColor", "\"grey\"");

	file->AddTempParameter("sv:Ma/Mylar/Components", "3 \"Hydrogen\" \"Carbon\" \"Oxygen\"");
	file->AddTempParameter("uv:Ma/Mylar/Fractions", "3 0.041959 0.625017 0.333025");
	file->AddTempParameter("d:Ma/Mylar/Density", "1.40 g/cm3");
	file->AddTempParameter("s:Ma/Mylar/DefaultColor", "\"red\"");

	file->AddTempParameter("sv:Ma/Mylon/Components", "4 \"Hydrogen\" \"Carbon\" \"Nitrogen\" \"Oxygen\"");
	file->AddTempParameter("uv:Ma/Mylon/Fractions", "4 0.097976 0.636856 0.123779 0.141389");
	file->AddTempParameter("d:Ma/Mylon/Density", "1.140 g/cm3");
	file->AddTempParameter("s:Ma/Mylon/DefaultColor", "\"purple\"");

	file->AddTempParameter("sv:Ma/Kapton/Components", "4 \"Hydrogen\" \"Carbon\" \"Nitrogen\" \"Oxygen\"");
	file->AddTempParameter("uv:Ma/Kapton/Fractions", "4 0.026362 0.691133 0.073270 0.209235");
	file->AddTempParameter("d:Ma/Kapton/Density", "1.420 g/cm3");
	file->AddTempParameter("s:Ma/Kapton/DefaultColor", "\"purple\"");

	file->AddTempParameter("sv:Ma/Water_75eV/Components", "2 \"Hydrogen\" \"Oxygen\"");
	file->AddTempParameter("uv:Ma/Water_75eV/Fractions", "2 0.111894 0.888106");
	file->AddTempParameter("d:Ma/Water_75eV/Density", "1.0 g/cm3");
	file->AddTempParameter("d:Ma/Water_75eV/MeanExcitationEnergy", "75.0 eV");
	file->AddTempParameter("s:Ma/Water_75eV/DefaultColor", "\"blue\"");

	file->AddTempParameter("sv:Ma/Titanium/Components", "1 \"Titanium\"");
	file->AddTempParameter("uv:Ma/Titanium/Fractions", "1 1.0");
	file->AddTempParameter("d:Ma/Titanium/Density", "4.54 g/cm3");
	file->AddTempParameter("s:Ma/Titanium/DefaultColor", "\"blue\"");

	file->AddTempParameter("sv:Ma/Steel/Components", "8 \"Carbon\" \"Silicon\" \"Phosphorus\" \"Sulfur\" \"Chromium\" \"Manganese\" \"Iron\" \"Nickel\"");
	file->AddTempParameter("uv:Ma/Steel/Fractions", "8 0.0015 0.01 0.00045 0.0003 0.19 0.02 0.67775 0.1");
	file->AddTempParameter("d:Ma/Steel/Density", "8.027 g/cm3");
	file->AddTempParameter("s:Ma/Steel/DefaultColor", "\"lightblue\"");

	file->AddTempParameter("i:Sc/Verbosity", "0");
	file->AddTempParameter("b:Sc/AddUnitEvenIfItIsOne", "\"False\"");
	file->AddTempParameter("s:Sc/RootFileName", "\"topas\"");
	file->AddTempParameter("s:Sc/XmlFileName", "\"topas\"");

	// 16 Standard colors from HTML4.01 specification as shown at https://en.wikipedia.org/wiki/Web_colors
	file->AddTempParameter("iv:Gr/Color/White"	, "	3 255 255 255");
	file->AddTempParameter("iv:Gr/Color/Silver"	, "	3 191 191 191");
	file->AddTempParameter("iv:Gr/Color/Gray"	, "	3 127 127 127");
	file->AddTempParameter("iv:Gr/Color/Grey"	, "	3 127 127 127");
	file->AddTempParameter("iv:Gr/Color/Black"	, "	3   0   0   0");
	file->AddTempParameter("iv:Gr/Color/Red"	, "	3 255   0   0");
	file->AddTempParameter("iv:Gr/Color/Maroon"	, "	3 127   0   0");
	file->AddTempParameter("iv:Gr/Color/Yellow"	, "	3 255 255   0");
	file->AddTempParameter("iv:Gr/Color/Olive"	, "	3 127 127   0");
	file->AddTempParameter("iv:Gr/Color/Lime"	, "	3   0 255   0");
	file->AddTempParameter("iv:Gr/Color/Green"	, "	3   0 127   0");
	file->AddTempParameter("iv:Gr/Color/Aqua"	, "	3   0 255 255");
	file->AddTempParameter("iv:Gr/Color/Teal"	, "	3   0 127 127");
	file->AddTempParameter("iv:Gr/Color/Blue"	, "	3   0   0 255");
	file->AddTempParameter("iv:Gr/Color/Navy"	, "	3   0   0 127");
	file->AddTempParameter("iv:Gr/Color/Fuchsia", "	3 255   0 255");
	file->AddTempParameter("iv:Gr/Color/Purple"	, "	3 127   0 127");

	// Other colors from the original PAGA code
	file->AddTempParameter("iv:Gr/Color/Lightblue", "	3 175 255 255");
	file->AddTempParameter("iv:Gr/Color/Skyblue", "	3 175 124 255");
	file->AddTempParameter("iv:Gr/Color/Magenta", "	3 255   0 255");
	file->AddTempParameter("iv:Gr/Color/Violet", "	3 224   0 255");
	file->AddTempParameter("iv:Gr/Color/Pink", "	3 255   0 222");
	file->AddTempParameter("iv:Gr/Color/Indigo", "	3   0   0 190");
	file->AddTempParameter("iv:Gr/Color/Grass", "	3   0 239   0");
	file->AddTempParameter("iv:Gr/Color/Orange", "	3 241 224   0");
	file->AddTempParameter("iv:Gr/Color/Brown", "	3 225 126  66");

	// Grey scale
	file->AddTempParameter("iv:Gr/Color/Grey020", "3  20  20  20");
	file->AddTempParameter("iv:Gr/Color/Grey040", "3  40  40  40");
	file->AddTempParameter("iv:Gr/Color/Grey060", "3  60  60  60");
	file->AddTempParameter("iv:Gr/Color/Grey080", "3  80  80  80");
	file->AddTempParameter("iv:Gr/Color/Grey100", "3 100 100 100");
	file->AddTempParameter("iv:Gr/Color/Grey120", "3 120 120 120");
	file->AddTempParameter("iv:Gr/Color/Grey140", "3 140 140 140");
	file->AddTempParameter("iv:Gr/Color/Grey160", "3 160 160 160");
	file->AddTempParameter("iv:Gr/Color/Grey180", "3 180 180 180");
	file->AddTempParameter("iv:Gr/Color/Grey200", "3 200 200 200");
	file->AddTempParameter("iv:Gr/Color/Grey220", "3 220 220 220");
	file->AddTempParameter("iv:Gr/Color/Grey240", "3 240 240 240");

	file->AddTempParameter("b:Gr/Enable", "\"True\"");
	file->AddTempParameter("i:Gr/Verbosity", "0");
	file->AddTempParameter("s:Gr/RefreshEvery", "\"Run\"");
	file->AddTempParameter("i:Gr/ShowOnlyOutlineIfVoxelCountExceeds", "8000");
	file->AddTempParameter("i:Gr/SwitchOGLtoOGLIifVoxelCountExceeds", "70000000");

	file->AddTempParameter("sv:Ge/Params/G4CutTubs", "4 \"RMax\" \"HL\" \"LowNorm\" \"HighNorm\"");
	file->AddTempParameter("sv:Ge/Params/G4Cons", "3 \"RMax1\" \"RMax2\" \"HL\"");
	file->AddTempParameter("sv:Ge/Params/G4Para", "6 \"HLX\" \"HLY\" \"HLZ\" \"Alpha\" \"Theta\" \"Phi\"");
	file->AddTempParameter("sv:Ge/Params/G4Trd", "5 \"HLX1\" \"HLX2\" \"HLY1\" \"HLY2\" \"HLZ\"");
	file->AddTempParameter("sv:Ge/Params/G4RTrap", "4 \"LZ\" \"LY\" \"LX\" \"LTX\"");
	file->AddTempParameter("sv:Ge/Params/G4GTrap", "11 \"HLZ\" \"Theta\" \"Phi\" \"HLY1\" \"HLX1\" \"HLX2\" \"Alp1\" \"HLY2\" \"HLX3\" \"HLX4\" \"Alp2\"");
	file->AddTempParameter("sv:Ge/Params/G4Orb", "1 \"R\"");
	file->AddTempParameter("sv:Ge/Params/G4Torus", "2 \"RMax\" \"RTor\"");
	file->AddTempParameter("sv:Ge/Params/G4HPolycone", "3 \"Z\" \"RInner\" \"ROuter\"");
	file->AddTempParameter("sv:Ge/Params/G4SPolycone", "2 \"R\" \"Z\"");
	file->AddTempParameter("sv:Ge/Params/G4GenericPolycone", "2 \"R\" \"Z\"");
	file->AddTempParameter("sv:Ge/Params/G4HPolyhedra", "4 \"NSides\" \"Z\" \"RInner\" \"ROuter\"");
	file->AddTempParameter("sv:Ge/Params/G4SPolyhedra", "3 \"NSides\" \"R\" \"Z\"");
	file->AddTempParameter("sv:Ge/Params/G4EllipticalTube", "3 \"HLX\" \"HLY\" \"HLZ\"");
	file->AddTempParameter("sv:Ge/Params/G4Ellipsoid", "3 \"HLX\" \"HLY\" \"HLZ\"");
	file->AddTempParameter("sv:Ge/Params/G4EllipticalCone", "3 \"HLX\" \"HLY\" \"ZMax\"");
	file->AddTempParameter("sv:Ge/Params/G4Paraboloid", "3 \"HLZ\" \"R1\" \"R2\"");
	file->AddTempParameter("sv:Ge/Params/G4Hype", "3 \"OR\" \"OS\" \"HLZ\"");
	file->AddTempParameter("sv:Ge/Params/G4Tet", "4 \"Anchor\" \"P2\" \"P3\" \"P4\"");
	file->AddTempParameter("sv:Ge/Params/G4ExtrudedSolid", "6 \"Polygons\" \"HLZ\" \"Off1\" \"Scale1\" \"Off2\" \"Scale2\"");
	file->AddTempParameter("sv:Ge/Params/G4TwistedBox", "4 \"Twist\" \"HLX\" \"HLY\" \"HLZ\"");
	file->AddTempParameter("sv:Ge/Params/G4RTwistedTrap", "5 \"Twist\" \"HLX1\" \"HLX2\" \"HLY\" \"HLZ\"");
	file->AddTempParameter("sv:Ge/Params/G4GTwistedTrap", "11 \"Twist\" \"HLZ\" \"Theta\" \"Phi\" \"HLY1\" \"HLX1\" \"HLX2\" \"HLY2\" \"HLX3\" \"HLX4\" \"Alpha\"");
	file->AddTempParameter("sv:Ge/Params/G4TwistedTrd", "6 \"HLX1\" \"HLX2\" \"HLY1\" \"HLY2\" \"HLZ\" \"Twist\"");
	file->AddTempParameter("sv:Ge/Params/G4GenericTrap", "2 \"HLZ\" \"Vertices\"");
	file->AddTempParameter("sv:Ge/Params/G4TwistedTubs", "5 \"Twist\" \"EndInnerRad\" \"EndOuterRad\" \"HLZ\" \"Phi\"");
}
