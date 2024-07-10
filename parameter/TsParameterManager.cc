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

#include "TsParameterManager.hh"

#include "TsSequenceManager.hh"

#include "TsParameterFile.hh"
#include "TsTempParameter.hh"
#include "TsVParameter.hh"

#include "G4Run.hh"
#include "G4UIcommand.hh"
#include "G4UnitsTable.hh"
#include "G4IonTable.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"

#include "gdcmUIDGenerator.h"

#ifdef TOPAS_MT
#include "G4AutoLock.hh"

namespace {
	G4Mutex firstEventIsHereMutex = G4MUTEX_INITIALIZER;
}
#endif

TsParameterManager::TsParameterManager(G4int argc, char** argv, G4String topasVersion):
fTOPASVersion(topasVersion), fSqm(0), fAddParameterHasBeenCalled(false), fNowDoingParameterDump(false),
fUnableToCalculateForDump(false), fHasGeometryOverlap(false),
fNeedsTrackingAction(false), fNeedsSteppingAction(false), fNeedsChemistry(false),
fHandledFirstEvent(false), fIsInQt(false), fIsFindingSeed(false), fUseVarianceReduction(false), fAddedParameterFileCounter(1)
{
#ifdef TOPAS_MT
	fLastDirectAction.Put("");
	fLastDirectParameterName.Put("");
#else
	fLastDirectAction = "";
	fLastDirectParameterName = "";
#endif

	// Flag to tell if we are testing a new parameter file syntax
	if ( getenv( "TOPAS_Test_Mode" ) ) {
		G4String testNewSyntax = getenv( "TOPAS_Test_Mode" );
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(testNewSyntax);
#else
		testNewSyntax.toLower();
#endif
		fTestMode = (testNewSyntax=="t" || testNewSyntax=="true" || testNewSyntax=="1");
		if (fTestMode) {
			G4cout << "######### TOPAS detected environment variable TOPAS_Test_Mode set to True. #########" << G4endl;
			G4cout << "########## New features that have not yet been validated will be enabled. #########" << G4endl;
		}
	}

	// Parse the argument
	if (argc > 1) fTopParameterFileSpec = argv[1];
	else fTopParameterFileSpec = "TsUserParameters.txt";

	G4cout << "Loading parameters starting from: " << fTopParameterFileSpec << G4endl;

	fTimer.Start();

	CreateUnits();

	// Create the store for the parameter files.
	// We refer to this store from TsParameterFile to avoid creating the same parameter file twice.
	fParameterFileStore = new std::map<G4String, TsParameterFile*>;

	// First TsParameterFile object created is the special one to hold transient parameters.
	// That file will implicitly include and instantiate the file named in fTopParameterFileSpec.
	// From there, each file may instantiate other include files.
	G4String transientParameterFile = "TransientParameters";
	fParameterFile = new TsParameterFile(this, transientParameterFile, fTopParameterFileSpec, 0);
	RegisterParameterFile(fTopParameterFileSpec, fParameterFile);

	// Initialize values of time feature parameters
	SetCurrentTime(GetDoubleParameter("Tf/TimelineStart", "Time"));

	UpdateTimeFeatureStore();

	fIsRandomMode = GetBooleanParameter("Tf/RandomizeTimeDistribution");

	fFindSeedRun = GetIntegerParameter("Ts/FindSeedForRun");
	fFindSeedHistory = GetIntegerParameter("Ts/FindSeedForHistory");
	if (fFindSeedHistory > -1) {
		fIsFindingSeed = true;
		G4cout << "\nTs/FindSeedForHistory has been set." << G4endl;
		G4cout << "TOPAS will skip producing any primaries and disable other non-essential code." << G4endl;
		G4cout << "TOPAS will run until Run: " << fFindSeedRun << ", History: " << fFindSeedHistory << " has been reached," << G4endl;
		G4cout << "then will print out that history's random seed and terminate." << G4endl;
	}

	if (!fIsFindingSeed && ParameterExists("Vr/UseVarianceReduction") && GetBooleanParameter("Vr/UseVarianceReduction"))
		fUseVarianceReduction = true;

	// Define all colors
	fColorMap = new std::map<G4String,G4VisAttributes*>;

	G4String prefix = "Gr/Color/";
	std::vector<G4String>* values = new std::vector<G4String>;
	GetParameterNamesStartingWith(prefix, values);
	const size_t length = values->size();

	for (size_t iToken=0; iToken<length; iToken++) {
		G4String colorParmName = (*values)[iToken];
		G4String colorName = colorParmName.substr(9);
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(colorName);
#else
		colorName.toLower();
#endif

		G4int* colorValues = GetIntegerVector(colorParmName);
		G4double alpha;
		if (GetVectorLength(colorParmName) == 4)
			alpha = colorValues[3]/255.;
		else
			alpha = 1.;
		
		G4VisAttributes* color = new G4VisAttributes(G4Colour(colorValues[0]/255.,colorValues[1]/255.,colorValues[2]/255., alpha));

		if (colorName.substr(0,13) == "patienttissue")
			color->SetForceSolid(true);

		(*fColorMap)[colorName] = color;
	}

	fInvisible = new G4VisAttributes(false);

	fComponentTypeNames = new std::vector<G4String>;
	fScorerQuantityNames = new std::vector<G4String>;
	fFilterNames = new std::vector<G4String>;
	fAddedParameters = new std::map<G4String, G4String>;

	fTimer.Stop();
}


TsParameterManager::~TsParameterManager()
{
}


void TsParameterManager::UpdateTimeFeatureStore()
{
	fTimeFeatureStore = new std::vector<TsVParameter*>;
	fParameterFile->GetTimeFeatureParameters(fTimeFeatureStore);
	size_t tf_number = fTimeFeatureStore->size();
	for (size_t tf =0; tf < tf_number; ++tf) {
		(*fTimeFeatureStore)[tf]->InitializeTimeFeatureValue();
	}
}


G4bool TsParameterManager::ParameterExists(const char* c)
{
	G4String stringValue = c;
	return ParameterExists(stringValue);
}


G4bool TsParameterManager::ParameterExists(const G4String& stringValue)
{
	return fParameterFile->ParameterExists(stringValue);
}


G4int TsParameterManager::GetVectorLength(const char* c)
{
	G4String stringValue = c;
	return GetVectorLength(stringValue);
}


G4int TsParameterManager::GetVectorLength(const G4String& stringValue)
{
	return fParameterFile->GetVectorLength(stringValue);
}


G4double TsParameterManager::GetDoubleParameter(const char* c, const char* unitCategory)
{
	G4String stringValue = c;
	return GetDoubleParameter(stringValue, unitCategory);
}


G4double TsParameterManager::GetDoubleParameter(const G4String& stringValue, const char* unitCategory)
{
#ifdef TOPAS_MT
	fLastDirectAction.Put("GetDoubleParameter");
	fLastDirectParameterName.Put(stringValue);
#else
	fLastDirectAction = "GetDoubleParameter";
	fLastDirectParameterName = stringValue;
#endif

	G4double result = IGetDoubleParameter(stringValue);

	if (GetUnitCategoryOfParameter(stringValue) != unitCategory) {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "Expected the parameter named: " << stringValue << " to have unit category: " << unitCategory << G4endl;
		G4cerr << "But instead found unit category: " << GetUnitCategoryOfParameter(stringValue) << G4endl;
		AbortSession(1);
	}

	return result;
}


G4double TsParameterManager::IGetDoubleParameter(const G4String& stringValue)
{
	if (fSqm && IsChangeable(stringValue)) fSqm->NoteAnyUseOfChangeableParameters(stringValue);
	return fParameterFile->GetDoubleParameter(stringValue);
}


G4double TsParameterManager::GetUnitlessParameter(const char* c)
{
	G4String stringValue = c;
	return GetUnitlessParameter(stringValue);
}


G4double TsParameterManager::GetUnitlessParameter(const G4String& stringValue)
{
#ifdef TOPAS_MT
	fLastDirectAction.Put("GetUnitlessParameter");
	fLastDirectParameterName.Put(stringValue);
#else
	fLastDirectAction = "GetUnitlessParameter";
	fLastDirectParameterName = stringValue;
#endif
	return IGetUnitlessParameter(stringValue);
}


G4double TsParameterManager::IGetUnitlessParameter(const G4String& stringValue)
{
	if (fSqm && IsChangeable(stringValue)) fSqm->NoteAnyUseOfChangeableParameters(stringValue);
	return fParameterFile->GetUnitlessParameter(stringValue);
}


G4int TsParameterManager::GetIntegerParameter(const char* c)
{
	G4String stringValue = c;
	return GetIntegerParameter(stringValue);
}


G4int TsParameterManager::GetIntegerParameter(const G4String& stringValue)
{
#ifdef TOPAS_MT
	fLastDirectAction.Put("GetIntegerParameter");
	fLastDirectParameterName.Put(stringValue);
#else
	fLastDirectAction = "GetIntegerParameter";
	fLastDirectParameterName = stringValue;
#endif
	return IGetIntegerParameter(stringValue);
}


G4int TsParameterManager::IGetIntegerParameter(const G4String& stringValue)
{
	if (fSqm && IsChangeable(stringValue)) fSqm->NoteAnyUseOfChangeableParameters(stringValue);
	return fParameterFile->GetIntegerParameter(stringValue);
}


G4bool TsParameterManager::GetBooleanParameter(const char* c)
{
	G4String stringValue = c;
	return GetBooleanParameter(stringValue);
}


G4bool TsParameterManager::GetBooleanParameter(const G4String& stringValue)
{
#ifdef TOPAS_MT
	fLastDirectAction.Put("GetBooleanParameter");
	fLastDirectParameterName.Put(stringValue);
#else
	fLastDirectAction = "GetBooleanParameter";
	fLastDirectParameterName = stringValue;
#endif
	return IGetBooleanParameter(stringValue);
}


G4bool TsParameterManager::IGetBooleanParameter(const G4String& stringValue)
{
	if (fSqm && IsChangeable(stringValue)) fSqm->NoteAnyUseOfChangeableParameters(stringValue);
	return fParameterFile->GetBooleanParameter(stringValue);
}


G4String TsParameterManager::GetStringParameter(const char* c)
{
	G4String stringValue = c;
	return GetStringParameter(stringValue);
}


G4String TsParameterManager::GetStringParameter(const G4String& stringValue)
{
#ifdef TOPAS_MT
	fLastDirectAction.Put("GetStringParameter");
	fLastDirectParameterName.Put(stringValue);
#else
	fLastDirectAction = "GetStringParameter";
	fLastDirectParameterName = stringValue;
#endif
	return IGetStringParameter(stringValue);
}


G4String TsParameterManager::IGetStringParameter(const G4String& stringValue)
{
	if (fSqm && IsChangeable(stringValue)) fSqm->NoteAnyUseOfChangeableParameters(stringValue);
	return fParameterFile->GetStringParameter(stringValue);
}


G4String TsParameterManager::IGetStringParameter(const char* stringValue)
{
	return IGetStringParameter(G4String(stringValue));
}


G4double* TsParameterManager::GetDoubleVector(const char* c, const char* unitCategory)
{
	G4String stringValue = c;
	return GetDoubleVector(stringValue, unitCategory);
}


G4double* TsParameterManager::GetDoubleVector(const G4String& stringValue, const char* unitCategory)
{
#ifdef TOPAS_MT
	fLastDirectAction.Put("GetDoubleVector");
	fLastDirectParameterName.Put(stringValue);
#else
	fLastDirectAction = "GetDoubleVector";
	fLastDirectParameterName = stringValue;
#endif

	G4double* result = IGetDoubleVector(stringValue);

	if (GetUnitCategoryOfParameter(stringValue) != unitCategory) {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "Expected the parameter named: " << stringValue << " to have unit category: " << unitCategory << G4endl;
		G4cerr << "But instead found unit category: " << GetUnitCategoryOfParameter(stringValue) << G4endl;
		AbortSession(1);
	}

	return result;
}


G4double* TsParameterManager::IGetDoubleVector(const G4String& stringValue)
{
	if (fSqm && IsChangeable(stringValue)) fSqm->NoteAnyUseOfChangeableParameters(stringValue);
	return fParameterFile->GetDoubleVector(stringValue);
}


G4double* TsParameterManager::GetUnitlessVector(const char* c)
{
	G4String stringValue = c;
	return GetUnitlessVector(stringValue);
}


G4double* TsParameterManager::GetUnitlessVector(const G4String& stringValue)
{
#ifdef TOPAS_MT
	fLastDirectAction.Put("GetUnitlessVector");
	fLastDirectParameterName.Put(stringValue);
#else
	fLastDirectAction = "GetUnitlessVector";
	fLastDirectParameterName = stringValue;
#endif
	return IGetUnitlessVector(stringValue);
}


G4double* TsParameterManager::IGetUnitlessVector(const G4String& stringValue)
{
	if (fSqm && IsChangeable(stringValue)) fSqm->NoteAnyUseOfChangeableParameters(stringValue);
	return fParameterFile->GetUnitlessVector(stringValue);
}


G4int* TsParameterManager::GetIntegerVector(const char* c)
{
	G4String stringValue = c;
	return GetIntegerVector(stringValue);
}


G4int* TsParameterManager::GetIntegerVector(const G4String& stringValue)
{
#ifdef TOPAS_MT
	fLastDirectAction.Put("GetIntegerVector");
	fLastDirectParameterName.Put(stringValue);
#else
	fLastDirectAction = "GetIntegerVector";
	fLastDirectParameterName = stringValue;
#endif
	return IGetIntegerVector(stringValue);
}


G4int* TsParameterManager::IGetIntegerVector(const G4String& stringValue)
{
	if (fSqm && IsChangeable(stringValue)) fSqm->NoteAnyUseOfChangeableParameters(stringValue);
	return fParameterFile->GetIntegerVector(stringValue);
}


G4bool* TsParameterManager::GetBooleanVector(const char* c)
{
	G4String stringValue = c;
	return GetBooleanVector(stringValue);
}


G4bool* TsParameterManager::GetBooleanVector(const G4String& stringValue)
{
#ifdef TOPAS_MT
	fLastDirectAction.Put("GetBooleanVector");
	fLastDirectParameterName.Put(stringValue);
#else
	fLastDirectAction = "GetBooleanVector";
	fLastDirectParameterName = stringValue;
#endif
	return IGetBooleanVector(stringValue);
}


G4bool* TsParameterManager::IGetBooleanVector(const G4String& stringValue)
{
	if (fSqm && IsChangeable(stringValue)) fSqm->NoteAnyUseOfChangeableParameters(stringValue);
	return fParameterFile->GetBooleanVector(stringValue);
}


G4String* TsParameterManager::GetStringVector(const char* c)
{
	G4String stringValue = c;
	return GetStringVector(stringValue);
}


G4String* TsParameterManager::GetStringVector(const G4String& stringValue)
{
#ifdef TOPAS_MT
	fLastDirectAction.Put("GetStringVector");
	fLastDirectParameterName.Put(stringValue);
#else
	fLastDirectAction = "GetStringVector";
	fLastDirectParameterName = stringValue;
#endif
	return IGetStringVector(stringValue);
}


G4String* TsParameterManager::IGetStringVector(const G4String& stringValue)
{
	if (fSqm && IsChangeable(stringValue)) fSqm->NoteAnyUseOfChangeableParameters(stringValue);
	return fParameterFile->GetStringVector(stringValue);
}


G4String TsParameterManager::GetStringParameterWithoutMonitoring(const char* c)
{
	G4String stringValue = c;
	return GetStringParameterWithoutMonitoring(stringValue);
}


G4String TsParameterManager::GetStringParameterWithoutMonitoring(const G4String& stringValue)
{
	return fParameterFile->GetStringParameter(stringValue);
}


G4String TsParameterManager::GetTypeOfParameter(const G4String& stringValue)
{
	return fParameterFile->GetTypeOfParameter(stringValue);
}


G4String TsParameterManager::GetUnitOfParameter(const G4String& stringValue)
{
	return fParameterFile->GetUnitOfParameter(stringValue);
}


G4TwoVector TsParameterManager::GetTwoVectorParameter(const char* c, const char* unitCategory)
{
	G4String stringValue = c;
	return GetTwoVectorParameter(stringValue, unitCategory);
}


G4TwoVector TsParameterManager::GetTwoVectorParameter(const G4String& stringValue, const char* unitCategory)
{
#ifdef TOPAS_MT
	fLastDirectAction.Put("GetTwoVectorParameter");
	fLastDirectParameterName.Put(stringValue);
#else
	fLastDirectAction = "GetTwoVectorParameter";
	fLastDirectParameterName = stringValue;
#endif
	if (fSqm && IsChangeable(stringValue)) fSqm->NoteAnyUseOfChangeableParameters(stringValue);
	G4double* values = fParameterFile->GetDoubleVector(stringValue);

	if (fParameterFile->GetVectorLength(stringValue) !=2) {
		G4cerr << "Topas is exiting due to request for a 2Vector from a parameter that does not have appropriate form." << G4endl;
		G4cerr << "Parameter name: " << stringValue << G4endl;
		AbortSession(1);
	}

	if (GetUnitCategoryOfParameter(stringValue) != unitCategory) {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "Expected the parameter named: " << stringValue << " to have unit category: " << unitCategory << G4endl;
		G4cerr << "But instead found unit category: " << GetUnitCategoryOfParameter(stringValue) << G4endl;
		AbortSession(1);
	}

	return G4TwoVector(values[0],values[1]);
}


G4ThreeVector TsParameterManager::GetThreeVectorParameter(const char* c, const char* unitCategory)
{
	G4String stringValue = c;
	return GetThreeVectorParameter(stringValue, unitCategory);
}


G4ThreeVector TsParameterManager::GetThreeVectorParameter(const G4String& stringValue, const char* unitCategory)
{
#ifdef TOPAS_MT
	fLastDirectAction.Put("GetThreeVectorParameter");
	fLastDirectParameterName.Put(stringValue);
#else
	fLastDirectAction = "GetThreeVectorParameter";
	fLastDirectParameterName = stringValue;
#endif
	if (fSqm && IsChangeable(stringValue)) fSqm->NoteAnyUseOfChangeableParameters(stringValue);
	G4double* values = fParameterFile->GetDoubleVector(stringValue);

	if (fParameterFile->GetVectorLength(stringValue) !=3) {
		G4cerr << "Topas is exiting due to request for a 3Vector from a parameter that does not have appropriate form." << G4endl;
		G4cerr << "Parameter name: " << stringValue << G4endl;
		AbortSession(1);
	}

	if (GetUnitCategoryOfParameter(stringValue) != unitCategory) {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "Expected the parameter named: " << stringValue << " to have unit category: " << unitCategory << G4endl;
		G4cerr << "But instead found unit category: " << GetUnitCategoryOfParameter(stringValue) << G4endl;
		AbortSession(1);
	}

	return G4ThreeVector(values[0],values[1],values[2]);
}


void TsParameterManager::AddParameter(const G4String& name, const G4String& value, G4bool test, G4bool permissive)
{
	if (!test)
		fAddParameterHasBeenCalled = true;

	TsTempParameter* tempParameter = fParameterFile->AddTempParameter(name, value);

	if (!test && !permissive && ParameterExists(tempParameter->GetName()) && !tempParameter->IsChangeable()) {
		// Need this until we rework Schneider handling to more flexibly handle multiple dicoms
		if (tempParameter->GetName().substr(0,16)=="Ma/PatientTissue") return;

		G4cerr << "Topas is exiting due to C++ code calling AddParameter for an unchangeable parameter that already exists." << G4endl;
		G4cerr << "Parameter name: " << name << G4endl;
		G4cerr << "Parameter value: " << value << G4endl;
		AbortSession(1);
	}

	fParameterFile->ProcessTempParameters(test);

	if (!test)
		(*fAddedParameters)[name] = value;
}


void TsParameterManager::CloneParameter(const G4String& oldName, const G4String& newName)
{
	G4String type = GetTypeOfParameter(oldName);
	if (type == "d" || type == "dv") {
		G4String unit = fParameterFile->GetUnitOfParameter(oldName);
		AddParameter(type + ":" + newName, oldName + " " + unit);
	} else{
		AddParameter(type + ":" + newName, oldName);
	}
}


G4String TsParameterManager::GetPartAfterLastSlash(const G4String& name) {
	const auto lastSlashPos = name.find_last_of( "/" );
	G4String lastPart = name.substr(lastSlashPos+1);
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(lastPart);
#else
	lastPart.toLower();
#endif
	return lastPart;
}


void TsParameterManager::AbortSession(G4int exitCode) {
	if (fSqm)
		fSqm->AbortSession(exitCode);
	else
		exit(exitCode);
}


G4bool TsParameterManager::AddParameterHasBeenCalled() {
	return fAddParameterHasBeenCalled;
}


G4bool TsParameterManager::IsChangeable(const G4String& stringValue)
{
	return fParameterFile->IsChangeable(stringValue);
}


void TsParameterManager::SetNeedsTrackingAction() {
	fNeedsTrackingAction = true;
}


G4bool TsParameterManager::NeedsTrackingAction() {
	return fNeedsTrackingAction;
}


void TsParameterManager::SetNeedsSteppingAction() {
	fNeedsSteppingAction = true;
}


G4bool TsParameterManager::NeedsSteppingAction() {
	return fNeedsSteppingAction;
}


void TsParameterManager::SetNeedsChemistry() {
	fNeedsChemistry = true;
}


G4bool TsParameterManager::NeedsChemistry() {
	return fNeedsChemistry;
}


void TsParameterManager::GetParameterNamesStartingWith(const G4String& stringValue, std::vector<G4String>* parameterNames)
{
	fParameterFile->GetParameterNamesStartingWith(stringValue, parameterNames);
}


void TsParameterManager::GetParameterNamesBracketedBy(const G4String& prefix, const G4String& suffix, std::vector<G4String>* parameterNames)
{
	fParameterFile->GetParameterNamesBracketedBy(prefix, suffix, parameterNames);
}


void TsParameterManager::CheckFilterParameterNamesStartingWith(const G4String& prefix, std::vector<G4String>* filterNames)
{
	fParameterFile->CheckFilterParameterNamesStartingWith(prefix, filterNames);
}


G4String TsParameterManager::GetUnitCategoryOfParameter(const G4String& parameterName)
{
	return (GetUnitCategory(fParameterFile->GetUnitOfParameter(parameterName)));
}


// Wraps standard method to add a few more units
G4String TsParameterManager::GetUnitCategory(const G4String& unitString) {
	G4String category;

	if (unitString == "g/mole")   category = "mass";
	else if (unitString == "tesla/ms") category = "Magnetic flux density/Time";
	else if (unitString == "tesla/s") category = "Magnetic flux density/Time";
	else if (unitString == "tesla/mm") category = "magnetic field gradient";
	else if (unitString == "tesla/cm") category = "magnetic field gradient";
	else if (unitString == "tesla/m") category = "magnetic field gradient";
	else if (unitString == "V/mm") category = "electric field strength";
	else if (unitString == "V/cm") category = "electric field strength";
	else if (unitString == "V/m") category = "electric field strength";
	else if (unitString == "kV/mm") category = "electric field strength";
	else if (unitString == "kV/cm") category = "electric field strength";
	else if (unitString == "kV/m") category = "electric field strength";
	else if (unitString == "MeV/mm2") category = "energy fluence";
	else if (unitString == "MeV/mm/(g/cm3)") category = "force per density";
	else if (unitString == "MeV2/mm/(g/cm3)") category = "energy force per density";
	else if (unitString == "MeV/mm") category = "force";
	else if (unitString == "mm/MeV") category = "perForce";
	else if (unitString == "mm2/MeV2") category = "perForceSquare";
	else if (unitString == "/mm2") category = "fluence";
	else if (unitString == "/cm2") category = "fluence";
	else if (unitString == "/Gy") category = "perDose";
	else if (unitString == "/Gy2") category = "perDoseSquare";
    else if (unitString == "/MeV") category = "perEnergy";
	else if (unitString == "mm/MeV/Gy") category = "perForce perDose";
	else if (unitString == "Sv") category = "dose";
	else if (unitString == "Sv*mm2") category = "dose fluence";
	else if (unitString == "M") category = "molar concentration";
	else if (unitString == "/s") category = "perTime";
	else if (unitString == "/M/s") category = "perMolarConcentration perTime";
	else if (unitString == "m2/s") category = "surface perTime";
	else if (unitString == "nm2/s") category = "surface perTime";
    else if (unitString == "um3") category = "Volume";
    else if (unitString == "nm3") category = "Volume";
	else if (!G4UnitDefinition::IsUnitDefined(unitString)) category = "None";
	else category = G4UIcommand::CategoryOf(unitString);

	return category;
}


// Wraps standard method to add a few more units
G4double TsParameterManager::GetUnitValue(const G4String& unitString) {
	G4double value;

	if (unitString == "g/mole")   value = g/mole;
	else if (unitString == "tesla/s") value = tesla/s;
	else if (unitString == "tesla/ms") value = tesla/ms;
	else if (unitString == "tesla/mm") value = tesla/mm;
	else if (unitString == "tesla/cm") value = tesla/cm;
	else if (unitString == "tesla/m") value = tesla/m;
	else if (unitString == "V/mm") value = volt/mm;
	else if (unitString == "V/cm") value = volt/mm;
	else if (unitString == "V/m") value = volt/m;
	else if (unitString == "kV/mm") value = kilovolt/mm;
	else if (unitString == "kV/cm") value = kilovolt/cm;
	else if (unitString == "kV/m") value = kilovolt/m;
	else if (unitString == "MeV/mm2") value = MeV/mm/mm;
	else if (unitString == "MeV/mm/(g/cm3)") value = MeV/mm/(g/cm/cm/cm);
	else if (unitString == "MeV2/mm/(g/cm3)") value = MeV*MeV/mm/(g/cm/cm/cm);
	else if (unitString == "MeV/mm") value = MeV/mm;
	else if (unitString == "mm/MeV") value = mm/MeV;
	else if (unitString == "mm2/MeV2") value = mm*mm/MeV/MeV;
	else if (unitString == "/mm2") value = 1./mm/mm;
	else if (unitString == "/cm2") value = 1./cm/cm;
	else if (unitString == "/Gy") value = 1./gray;
	else if (unitString == "/Gy2") value = 1./gray/gray;
    else if (unitString == "/MeV") value = 1./MeV;
	else if (unitString == "mm/MeV/Gy") value = mm/MeV/gray;
	else if (unitString == "Sv") value = gray;
	else if (unitString == "Sv*mm2") value = gray*mm*mm;
	else if (unitString == "M") value = 1.0e3*mole/(m*m*m);
	else if (unitString == "/s") value = 1./s;
	else if (unitString == "/M/s") value = 1.0e-3*m*m*m/(mole*s);
	else if (unitString == "m2/s") value = m*m/s;
	else if (unitString == "nm2/s") value = nm*nm/s;
    else if (unitString == "um3") value = um*um*um;
    else if (unitString == "nm3") value = nm*nm*nm;
	else value = G4UIcommand::ValueOf(unitString);

	return value;
}


void TsParameterManager::SetLastDirectAction(G4String action) {
#ifdef TOPAS_MT
	fLastDirectAction.Put(action);
#else
	fLastDirectAction = action;
#endif
}


void TsParameterManager::SetLastDirectParameterName(G4String name) {
#ifdef TOPAS_MT
	fLastDirectParameterName.Put(name);
#else
	fLastDirectAction = name;
#endif
}


G4String TsParameterManager::GetLastDirectAction() {
#ifdef TOPAS_MT
	return fLastDirectAction.Get();
#else
	return fLastDirectAction;
#endif
}


G4String TsParameterManager::GetLastDirectParameterName() {
#ifdef TOPAS_MT
	return fLastDirectParameterName.Get();
#else
	return fLastDirectParameterName;
#endif
}


void TsParameterManager::RegisterParameterFile(G4String fileName, TsParameterFile* parameterFile) {
	(*fParameterFileStore)[fileName] = parameterFile;
}


TsParameterFile* TsParameterManager::GetParameterFile(G4String fileName) {
	return (*fParameterFileStore)[fileName];
}


G4VisAttributes* TsParameterManager::GetColor(G4String name)
{
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(name);
#else
	name.toLower();
#endif
	std::map<G4String, G4VisAttributes*>::const_iterator iter = fColorMap->find(name);
	if (iter == fColorMap->end()) {
		G4cout << "Color not found: " << name << G4endl;
		return 0;
	}
	return iter->second;
}


G4VisAttributes* TsParameterManager::GetColor(const char* c)
{
	G4String stringValue = c;
	return GetColor(stringValue);
}


std::vector<G4String> TsParameterManager::GetColorNames()
{
	std::vector<G4String>* names = new std::vector<G4String>;
	std::map<G4String, G4VisAttributes*>::const_iterator iter;
	for (iter = fColorMap->begin(); iter != fColorMap->end(); ++iter)
		names->push_back(iter->first);
	return *names;
}


G4VisAttributes* TsParameterManager::GetInvisible()
{
	return fInvisible;
}


// Evaluate a particle definition string
// If "100001002", ask Geant4 for "deuteron"  - Geant4 does not accept PDG code for light ions
// If "100001003", ask Geant4 for "triton"    - Geant4 does not accept PDG code for light ions
// If "100002003", ask Geant4 for "He3"       - Geant4 does not accept PDG code for light ions
// If "100002004", ask Geant4 for "alpha"     - Geant4 does not accept PDG code for light ions
// If a number that has 10 digits and starts with 100, ask Geant4 for GenericIon,
// digits 4-6 give Z, digts 7-9 give A, digit 10 gives Isomer Level (not used)
// If any other number, ask Geant4 based on this PDG code
// If "He3" in any case, ask Geant4 for "He3" - Geant4 requires exact case here
// If other string, change to lower case      - Geant4 requires lower case for all particles other than He3
TsParticleDefinition TsParameterManager::GetParticleDefinition(G4String name) {
	TsParticleDefinition p;
	p.particleDefinition = 0;
	p.isOpticalPhoton = false;
	p.isGenericIon = false;
	p.ionZ = -1;
	p.ionA = -1;
	p.ionCharge = 999;

	if (name == "1000010020") {
		p.particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle("deuteron");
		return p;
	}

	if (name == "1000010030") {
		p.particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle("triton");
		return p;
	}

	if (name == "1000020030") {
		p.particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle("He3");
		return p;
	}

	if (name == "1000020040") {
		p.particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle("alpha");
		return p;
	}

	if (name == "0") {
		p.particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle("opticalphoton");
		p.isOpticalPhoton = true;
		return p;
	}

	if (IsInteger(name, 32)) {
		G4int pdgCode = G4UIcommand::ConvertToInt(name);
		if ( pdgCode > 999999999 && name.substr(0,3)=="100" ) {
			G4String someDigits = name.substr(3,3);
			p.ionZ = G4UIcommand::ConvertToInt(someDigits);
			someDigits = name.substr(6,3);
			p.ionA = G4UIcommand::ConvertToInt(someDigits);
			someDigits = name.substr(9,1);

			if ((someDigits != "0") && !GetBooleanParameter("Ts/TreatExcitedIonsAsGroundState")) {
				G4cerr << "A phase space input file or filter parameter is using a PDG" << G4endl;
				G4cerr << "particle code that corresponds to an ion in an excited state." << G4endl;
				G4cerr << "This is any ten digit PDG code that does not end in a zero." << G4endl;
				G4cerr << "The PDG code seen here was: " << name << G4endl;
				G4cerr << "TOPAS can only handle such ions by treating them as ground state." << G4endl;
				G4cerr << "To accept this compromise, set" << G4endl;
				G4cerr << "Ts/TreatExcitedIonsAsGroundState to True." << G4endl;
				AbortSession(1);
			}
		} else {
			p.particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle(pdgCode);
			return p;
		}
	} else {
		G4String nameLower = name;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(nameLower);
#else
		nameLower.toLower();
#endif
		if (nameLower == "he3") {
			p.particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle("He3");
			return p;
		}

		if (strstr(nameLower,"genericion")) {
			G4String ionSpecification = nameLower.substr(11,nameLower.length()-1);
			size_t prev = 0;
			size_t next = ionSpecification.find_first_of(",", prev);
			if (next == std::string::npos) {
				G4cerr << "Trying to use GenericIon with wrong syntax: " << name << G4endl;
				G4cerr << "Should be in format GenericIon(Z,A) or GenericIon(Z,A,Charge)" << G4endl;
				AbortSession(1);
			}

			G4String token = ionSpecification.substr(prev, next - prev);
			if (token=="*") token = "-1";
			p.ionZ = G4UIcommand::ConvertToInt(token);

			prev = next + 1;
			next = ionSpecification.find_first_of(",", prev);
			if (next == std::string::npos) {
				// Assume user has not supplied third argument, Charge
				next = ionSpecification.find_first_of(")", prev);
				if (next == std::string::npos) {
					G4cerr << "Trying to use GenericIon with wrong syntax: " << name << G4endl;
					G4cerr << "Should be in format GenericIon(Z,A) or GenericIon(Z,A,Charge)" << G4endl;
					AbortSession(1);
				}
				token = ionSpecification.substr(prev, next - prev);
				if (token=="*") token = "-1";
				p.ionA = G4UIcommand::ConvertToInt(token);
			} else {
				// Assume user has supplied third argument, Charge
				token = ionSpecification.substr(prev, next - prev);
				if (token=="*") token = "-1";
				p.ionA = G4UIcommand::ConvertToInt(token);

				prev = next + 1;
				next = ionSpecification.find_first_of(")", prev);
				if (next == std::string::npos) {
					G4cerr << "Trying to use GenericIon with wrong syntax: " << name << G4endl;
					G4cerr << "Should be in format GenericIon(Z,A) or GenericIon(Z,A,Charge)" << G4endl;
					AbortSession(1);
				}

				token = ionSpecification.substr(prev, next - prev);
				if (token!="*")
					p.ionCharge = G4UIcommand::ConvertToInt(token);
			}
		} else {
			p.particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle(nameLower);
			if (nameLower == "opticalphoton" ) p.isOpticalPhoton = true;
			return p;
		}
	}

	p.isGenericIon = true;

	if(p.ionZ > 999) {
		G4cerr << "Trying to use GenericIon with atomic mass greater than 999: " << name << G4endl;
		AbortSession(1);
	}

	if (p.ionA != -1 && p.ionZ > p.ionA) {
		G4cerr << "Trying to use GenericIon with Z greater than A: " << name << G4endl;
		AbortSession(1);
	}
	if (p.ionZ != -1 && p.ionCharge != 999 && abs(p.ionCharge) > p.ionZ) {
		G4cerr << "Trying to use GenericIon with abs(charge) greater than Z: " << name << G4endl;
		AbortSession(1);
	}

	if (p.ionZ != -1 && p.ionA != -1)
		p.particleDefinition = G4ParticleTable::GetParticleTable()->GetIonTable()->GetIon(p.ionZ, p.ionA);
	return p;
}


void TsParameterManager::RegisterComponentTypeName(G4String typeName) {
	fComponentTypeNames->push_back(typeName);
}


std::vector<G4String> TsParameterManager::GetComponentTypeNames()
{
	return *fComponentTypeNames;
}


void TsParameterManager::RegisterScorerQuantityName(G4String typeName) {
	fScorerQuantityNames->push_back(typeName);
}


std::vector<G4String> TsParameterManager::GetScorerQuantityNames()
{
	return *fScorerQuantityNames;
}


void TsParameterManager::RegisterFilterName(G4String name) {
	if (std::find(fFilterNames->begin(), fFilterNames->end(), name) == fFilterNames->end())
		fFilterNames->push_back(name);
}


std::vector<G4String>* TsParameterManager::GetFilterNames() {
	return fFilterNames;
}


std::vector<TsVParameter*>* TsParameterManager::GetTimeFeatureStore(G4double currentTime) {
	SetCurrentTime(currentTime);
	return fTimeFeatureStore;
}


void TsParameterManager::SetSequenceManager(TsSequenceManager* sequenceManager) {
	fSqm = sequenceManager;
}


TsSequenceManager* TsParameterManager::GetSequenceManager() {
	return fSqm;
}


G4double TsParameterManager::GetCurrentTime() {
	return fSequenceTime;
}


G4int TsParameterManager::GetRunID() {
	if (fSqm)
		return fSqm->GetRunID();
	else
		return -1;
}


void TsParameterManager::HandleFirstEvent() {
#ifdef TOPAS_MT
	G4AutoLock l(&firstEventIsHereMutex);
#endif
	if (!fHandledFirstEvent) {
		fSqm->HandleFirstEvent();
		fHandledFirstEvent = true;
	}
}


void TsParameterManager::GetChangeableParameters(std::vector<G4String>* parameterNames, std::vector<G4String>* parameterValues) {
	std::map<G4String, TsVParameter*>* parameterMap = new std::map<G4String, TsVParameter*>;
	fParameterFile->GetAllParameters(parameterMap);
	std::map<G4String, TsVParameter*>::const_iterator iter;
	for (iter = parameterMap->begin(); iter != parameterMap->end(); ++iter) {

		TsVParameter* parameter = iter->second;
		if (parameter->IsChangeable()) {
			G4String parameterName = parameter->GetName();
			G4String parameterType = parameter->GetType();
			G4String parameterValue = GetParameterValueAsString(parameterType, parameterName);
			G4String nameWithType = parameterType + "c:" + parameterName;
			parameterNames->push_back(nameWithType);
			parameterValues->push_back(parameterValue);
		}
	}
}


G4String TsParameterManager::GetParameterValueAsString(G4String parameterType, G4String parameterName) {
	fNowDoingParameterDump = true;
	G4String parameterValue = "Undefined";

	if (parameterType=="d") {
		G4double aDouble = IGetDoubleParameter(parameterName)/ GetUnitValue(fParameterFile->GetUnitOfParameter(parameterName));
		if (!fUnableToCalculateForDump)
			parameterValue = G4UIcommand::ConvertToString(aDouble) + " " + fParameterFile->GetUnitOfParameter(parameterName);
	} else if (parameterType=="u") {
		G4double aDouble = GetUnitlessParameter(parameterName);
		if (!fUnableToCalculateForDump)
			parameterValue = G4UIcommand::ConvertToString(aDouble);
	} else if (parameterType=="i") {
		G4int anInt = GetIntegerParameter(parameterName);
		if (!fUnableToCalculateForDump)
			parameterValue = G4UIcommand::ConvertToString(anInt);
	} else if (parameterType=="b") {
		G4bool aBool = GetBooleanParameter(parameterName);
		if (!fUnableToCalculateForDump)
			parameterValue = G4UIcommand::ConvertToString(aBool);
	} else if (parameterType=="s") {
		parameterValue = "\"" + GetStringParameter(parameterName) + "\"";
	} else if (parameterType=="dv") {
		G4double* values = IGetDoubleVector(parameterName);
		if (!fUnableToCalculateForDump) {
			G4int valuesLength = GetVectorLength(parameterName);
			parameterValue = G4UIcommand::ConvertToString(valuesLength) + " ";
			G4double unitValue = GetUnitValue(fParameterFile->GetUnitOfParameter(parameterName));
			for (G4int iValue=0; iValue<valuesLength; iValue++) {
				if (iValue > 0) parameterValue += " ";
				parameterValue += G4UIcommand::ConvertToString(values[iValue]/unitValue);
			}
			parameterValue += fParameterFile->GetUnitOfParameter(parameterName);
		}
	} else if (parameterType=="uv") {
		G4double* values = GetUnitlessVector(parameterName);
		if (!fUnableToCalculateForDump) {
			G4int valuesLength = GetVectorLength(parameterName);
			parameterValue = G4UIcommand::ConvertToString(valuesLength) + " ";
			for (G4int iValue=0; iValue<valuesLength; iValue++) {
				if (iValue > 0) parameterValue += " ";
				parameterValue += G4UIcommand::ConvertToString(values[iValue]);
			}
		}
	} else if (parameterType=="iv") {
		G4int* values = GetIntegerVector(parameterName);
		if (!fUnableToCalculateForDump) {
			G4int valuesLength = GetVectorLength(parameterName);
			parameterValue = G4UIcommand::ConvertToString(valuesLength) + " ";
			for (G4int iValue=0; iValue<valuesLength; iValue++) {
				if (iValue > 0) parameterValue += " ";
				parameterValue += G4UIcommand::ConvertToString(values[iValue]);
			}
		}
	} else if (parameterType=="bv") {
		G4bool* values = GetBooleanVector(parameterName);
		if (!fUnableToCalculateForDump) {
			G4int valuesLength = GetVectorLength(parameterName);
			parameterValue = G4UIcommand::ConvertToString(valuesLength) + " ";
			for (G4int iValue=0; iValue<valuesLength; iValue++) {
				if (iValue > 0) parameterValue += " ";
				parameterValue += G4UIcommand::ConvertToString(values[iValue]);
			}
		}
	} else if (parameterType=="sv") {
		G4String* values = GetStringVector(parameterName);
		if (!fUnableToCalculateForDump) {
			G4int valuesLength = GetVectorLength(parameterName);
			parameterValue = G4UIcommand::ConvertToString(valuesLength) + " ";
			for (G4int iValue=0; iValue<valuesLength; iValue++) {
				if (iValue > 0) parameterValue += " ";
				parameterValue += "\"" + values[iValue] + "\"";
			}
		}
	}

	fNowDoingParameterDump = false;
	return parameterValue;
}


void TsParameterManager::DumpParameters(G4double currentTime, G4bool includeDefaults) {
	G4String runID_padding = "";
	G4int runID = GetRunID();
	G4int nPadding = GetIntegerParameter("Ts/RunIDPadding");
	for (G4int index = 1; index < nPadding; index++)
		if (runID < pow(10,index)) runID_padding += "0";
	G4String filespec = "TopasParameterDump_Run_" + runID_padding + G4UIcommand::ConvertToString(runID) + ".html";
	std::ofstream htmlFile(filespec);
	if (!htmlFile) {
		G4cerr << "ERROR: Failed to open file " << filespec << G4endl;
		AbortSession(1);
	}

	fNowDoingParameterDump = true;

	htmlFile << "Run: " << runID << ", TOPAS Time: " << currentTime / ms << " ms" << G4endl;

	htmlFile << "<table border='1' >" << G4endl;

	G4int linesSinceLastHeader = 19;

	// Loop over all found parameters
	std::map<G4String, TsVParameter*>* parameterMap = new std::map<G4String, TsVParameter*>;
	fParameterFile->GetAllParameters(parameterMap);
	std::map<G4String, TsVParameter*>::const_iterator iter;
	for (iter = parameterMap->begin(); iter != parameterMap->end(); ++iter) {

		TsVParameter* parameter = iter->second;
		if (includeDefaults || ( parameter->GetParameterFile()->GetParentFile() != 0)) {

			linesSinceLastHeader++;
			if (linesSinceLastHeader==20) {
				htmlFile << "<tr>" << G4endl;
				htmlFile << "<th> ParameterName </th>" << G4endl;
				htmlFile << "<th> Type </th>" << G4endl;
				htmlFile << "<th> Current Value </th>" << G4endl;
				// Loop over chain of parameter files, writing out file name
				TsParameterFile* aFile = fParameterFile;
				while (aFile) {
					htmlFile << "<th>" << aFile->GetFileName() << "</th>" << G4endl;
					aFile = aFile->GetParentFile();
				}
				htmlFile << "</tr>" << G4endl;
				linesSinceLastHeader=0;
			}

			G4String parameterName = parameter->GetName();

			htmlFile << "<tr>" << G4endl;
			htmlFile << "<td> <a name='" << parameterName << "'>" << parameterName << "</a> </td>" << G4endl;

			G4String parameterType = parameter->GetType();
			htmlFile << "<td>" << parameterType << "</td>" << G4endl;

			if (parameterType=="d") {
				G4double aDouble = IGetDoubleParameter(parameterName)/ GetUnitValue(fParameterFile->GetUnitOfParameter(parameterName));
				if (fUnableToCalculateForDump)
					htmlFile << "<td>Undefined</td>" << G4endl;
				else
					htmlFile << "<td>" << aDouble << " " << fParameterFile->GetUnitOfParameter(parameterName) << "</td>" << G4endl;
			} else if (parameterType=="u") {
				G4double aDouble = GetUnitlessParameter(parameterName);
				if (fUnableToCalculateForDump)
					htmlFile << "<td>Undefined</td>" << G4endl;
				else
					htmlFile << "<td>" << aDouble << "</td>" << G4endl;
			} else if (parameterType=="i") {
				G4int anInt = GetIntegerParameter(parameterName);
				if (fUnableToCalculateForDump)
					htmlFile << "<td>Undefined</td>" << G4endl;
				else
					htmlFile << "<td>" << anInt << "</td>" << G4endl;
			} else if (parameterType=="b") {
				G4bool aBool = GetBooleanParameter(parameterName);
				if (fUnableToCalculateForDump)
					htmlFile << "<td>Undefined</td>" << G4endl;
				else
					htmlFile << "<td>" << aBool << "</td>" << G4endl;
			} else if (parameterType=="s") {
				htmlFile << "<td>" << GetStringParameter(parameterName) << "</td>" << G4endl;
			} else if (parameterType=="dv") {
				htmlFile << "<td>" << G4endl;
				G4double* values = IGetDoubleVector(parameterName);
				if (fUnableToCalculateForDump)
					htmlFile << "<td>Undefined</td>" << G4endl;
				else {
					G4int length = GetVectorLength(parameterName);
					htmlFile << length << " " << G4endl;
					G4double unitValue = GetUnitValue(fParameterFile->GetUnitOfParameter(parameterName));
					for (G4int iToken=0; iToken<length; iToken++)
						htmlFile << values[iToken]/unitValue << " " << G4endl;
					htmlFile << fParameterFile->GetUnitOfParameter(parameterName) << "</td>" << G4endl;
				}
			} else if (parameterType=="uv") {
				htmlFile << "<td>" << G4endl;
				G4double* values = GetUnitlessVector(parameterName);
				if (fUnableToCalculateForDump)
					htmlFile << "<td>Undefined</td>" << G4endl;
				else {
					G4int length = GetVectorLength(parameterName);
					htmlFile << length << " " << G4endl;
					for (G4int iToken=0; iToken<length; iToken++)
						htmlFile << values[iToken] << " " << G4endl;
					htmlFile << "</td>" << G4endl;
				}
			} else if (parameterType=="iv") {
				htmlFile << "<td>" << G4endl;
				G4int* values = GetIntegerVector(parameterName);
				if (fUnableToCalculateForDump)
					htmlFile << "<td>Undefined</td>" << G4endl;
				else {
					G4int length = GetVectorLength(parameterName);
					htmlFile << length << " " << G4endl;
					for (G4int iToken=0; iToken<length; iToken++)
						htmlFile << values[iToken] << " " << G4endl;
					htmlFile << "</td>" << G4endl;
				}
			} else if (parameterType=="bv") {
				htmlFile << "<td>" << G4endl;
				G4bool* values = GetBooleanVector(parameterName);
				if (fUnableToCalculateForDump)
					htmlFile << "<td>Undefined</td>" << G4endl;
				else {
					G4int length = GetVectorLength(parameterName);
					htmlFile << length << " " << G4endl;
					for (G4int iToken=0; iToken<length; iToken++)
						htmlFile << values[iToken] << " " << G4endl;
					htmlFile << "</td>" << G4endl;
				}
			} else if (parameterType=="sv") {
				htmlFile << "<td>" << G4endl;
				G4String* values = GetStringVector(parameterName);
				if (fUnableToCalculateForDump)
					htmlFile << "<td>Undefined</td>" << G4endl;
				else {
					G4int length = GetVectorLength(parameterName);
					htmlFile << length << " " << G4endl;
					for (G4int iToken=0; iToken<length; iToken++)
						htmlFile << values[iToken] << " " << G4endl;
					htmlFile << "</td>" << G4endl;
				}
			} else {
				htmlFile << "<td> Invalid Type </td>" << G4endl;
			}

			// Loop over chain of parameter files, writing out this params value in each file
			TsParameterFile* aFile = fParameterFile;
			while (aFile) {
				htmlFile << "<td>" << aFile->GetHTMLValueOfParameter(parameterName) << "</td>" << G4endl;
				aFile = aFile->GetParentFile();
			}

			htmlFile << "</tr>" << G4endl;

			fUnableToCalculateForDump = false;
		}
	}

	htmlFile << "</table>" << G4endl;
	htmlFile.close();

	fNowDoingParameterDump = false;

	G4cout << "Parameters have been dumped to the file " << filespec << G4endl;
}


void TsParameterManager::DumpParametersToSimpleFile(G4double currentTime) {
	G4String runID_padding = "";
	G4int runID = GetRunID();
	G4int nPadding = GetIntegerParameter("Ts/RunIDPadding");
	for (G4int index = 1; index < nPadding; index++)
		if (runID < pow(10,index)) runID_padding += "0";
	G4String filespec = "TopasParameterDump_Run_" + runID_padding + G4UIcommand::ConvertToString(runID) + ".txt";
	std::ofstream outFile(filespec);
	if (!outFile) {
		G4cerr << "ERROR: Failed to open file " << filespec << G4endl;
		AbortSession(1);
	}

	fNowDoingParameterDump = true;

	outFile << "Run = " << runID << G4endl;
	outFile << "Time = " << currentTime / ms << " ms" << G4endl;

	G4String* parameters = GetStringVector("Ts/DumpParametersToSimpleFile");
	G4int length = GetVectorLength("Ts/DumpParametersToSimpleFile");

	for (G4int iToken=0; iToken<length; iToken++) {
		outFile << parameters[iToken] << " = ";

		G4String parameterType = GetTypeOfParameter(parameters[iToken]);

		if (parameterType=="d") {
			G4double aDouble = IGetDoubleParameter(parameters[iToken])/ GetUnitValue(fParameterFile->GetUnitOfParameter(parameters[iToken]));
			if (fUnableToCalculateForDump)
				outFile << "Undefined";
			else
				outFile << aDouble << " " << fParameterFile->GetUnitOfParameter(parameters[iToken]);
		} else if (parameterType=="u") {
			G4double aDouble = GetUnitlessParameter(parameters[iToken]);
			if (fUnableToCalculateForDump)
				outFile << "Undefined";
			else
				outFile << aDouble;
		} else if (parameterType=="i") {
			G4int anInt = GetIntegerParameter(parameters[iToken]);
			if (fUnableToCalculateForDump)
				outFile << "Undefined";
			else
				outFile << anInt;
		} else if (parameterType=="b") {
			G4bool aBool = GetBooleanParameter(parameters[iToken]);
			if (fUnableToCalculateForDump)
				outFile << "Undefined";
			else
				outFile << aBool;
		} else if (parameterType=="s") {
			outFile << GetStringParameter(parameters[iToken]);
		} else if (parameterType=="dv") {
			G4double* values = IGetDoubleVector(parameters[iToken]);
			if (fUnableToCalculateForDump)
				outFile << "Undefined";
			else {
				G4int valuesLength = GetVectorLength(parameters[iToken]);
				outFile << valuesLength << " ";
				G4double unitValue = GetUnitValue(fParameterFile->GetUnitOfParameter(parameters[iToken]));
				for (G4int iValue=0; iValue<valuesLength; iValue++) {
					if (iValue > 0) outFile << " ";
					outFile << values[iValue]/unitValue;
				}
				outFile << fParameterFile->GetUnitOfParameter(parameters[iToken]);
			}
		} else if (parameterType=="uv") {
			G4double* values = GetUnitlessVector(parameters[iToken]);
			if (fUnableToCalculateForDump)
				outFile << "Undefined";
			else {
				G4int valuesLength = GetVectorLength(parameters[iToken]);
				outFile << valuesLength << " ";
				for (G4int iValue=0; iValue<valuesLength; iValue++) {
					if (iValue > 0) outFile << " ";
					outFile << values[iValue];
				}
			}
		} else if (parameterType=="iv") {
			G4int* values = GetIntegerVector(parameters[iToken]);
			if (fUnableToCalculateForDump)
				outFile << "Undefined";
			else {
				G4int valuesLength = GetVectorLength(parameters[iToken]);
				outFile << valuesLength << " ";
				for (G4int iValue=0; iValue<valuesLength; iValue++) {
					if (iValue > 0) outFile << " ";
					outFile << values[iValue];
				}
			}
		} else if (parameterType=="bv") {
			G4bool* values = GetBooleanVector(parameters[iToken]);
			if (fUnableToCalculateForDump)
				outFile << "Undefined";
			else {
				G4int valuesLength = GetVectorLength(parameters[iToken]);
				outFile << valuesLength << " ";
				for (G4int iValue=0; iValue<valuesLength; iValue++) {
					if (iValue > 0) outFile << " ";
					outFile << values[iValue];
				}
			}
		} else if (parameterType=="sv") {
			G4String* values = GetStringVector(parameters[iToken]);
			if (fUnableToCalculateForDump)
				outFile << "Undefined";
			else {
				G4int valuesLength = GetVectorLength(parameters[iToken]);
				outFile << valuesLength << " ";
				for (G4int iValue=0; iValue<valuesLength; iValue++) {
					if (iValue > 0) outFile << " ";
					outFile << values[iValue];
				}
			}
		} else {
			outFile << "Undefined";
		}

		outFile << G4endl;
	}

	outFile.close();

	fNowDoingParameterDump = false;

	G4cout << "Parameters have been dumped to the file " << filespec << G4endl;
}


void TsParameterManager::DumpParametersToSemicolonSeparatedFile(G4double currentTime) {
	G4String runID_padding = "";
	G4int runID = GetRunID();
	G4int nPadding = GetIntegerParameter("Ts/RunIDPadding");
	for (G4int index = 1; index < nPadding; index++)
		if (runID < pow(10,index)) runID_padding += "0";
	G4String filespec = "TopasParameterDumpSSF_Run_" + runID_padding + G4UIcommand::ConvertToString(runID) + ".txt";
	std::ofstream outFile(filespec);
	if (!outFile) {
		G4cerr << "ERROR: Failed to open file " << filespec << G4endl;
		AbortSession(1);
	}

	fNowDoingParameterDump = true;

	outFile << "i; Run; ; 1;" << runID << ";" << G4endl;
	outFile << "d; Time; ms; 1;" << currentTime / ms << ";" << G4endl;

	G4String* parameters = GetStringVector("Ts/DumpParametersToSemicolonSeparatedFile");
	G4int length = GetVectorLength("Ts/DumpParametersToSemicolonSeparatedFile");

	for (G4int iToken=0; iToken<length; iToken++) {
		G4String parameterType = GetTypeOfParameter(parameters[iToken]);

		if (parameterType=="d") {
			outFile << parameterType << "; " << parameters[iToken] << "; ";
			outFile << fParameterFile->GetUnitOfParameter(parameters[iToken]) << "; ";
			G4double aDouble = IGetDoubleParameter(parameters[iToken])/ GetUnitValue(fParameterFile->GetUnitOfParameter(parameters[iToken]));
			if (fUnableToCalculateForDump)
				outFile << "1; Undefined;";
			else
				outFile << "1; " << aDouble << ";";
		} else if (parameterType=="u") {
			outFile << parameterType << "; " << parameters[iToken] << "; ";
			G4double aDouble = GetUnitlessParameter(parameters[iToken]);
			if (fUnableToCalculateForDump)
				outFile << "; 1; Undefined;";
			else
				outFile << "; 1; " << aDouble << ";";
		} else if (parameterType=="i") {
			outFile << parameterType << "; " << parameters[iToken] << "; ";
			G4int anInt = GetIntegerParameter(parameters[iToken]);
			if (fUnableToCalculateForDump)
				outFile << "; 1; Undefined;";
			else
				outFile << "; 1; " << anInt << ";";
		} else if (parameterType=="b") {
			outFile << parameterType << "; " << parameters[iToken] << "; ";
			G4bool aBool = GetBooleanParameter(parameters[iToken]);
			if (fUnableToCalculateForDump)
				outFile << "; 1; Undefined;";
			else
				outFile << "; 1; " << aBool << ";";
		} else if (parameterType=="s") {
			outFile << parameterType << "; " << parameters[iToken] << "; ";
			outFile << "; 1; " << GetStringParameter(parameters[iToken]) << ";";
		} else if (parameterType=="dv") {
			outFile << parameterType << "; " << parameters[iToken] << "; ";
			outFile << fParameterFile->GetUnitOfParameter(parameters[iToken]) << "; ";
			G4double* values = IGetDoubleVector(parameters[iToken]);
			if (fUnableToCalculateForDump)
				outFile << "1; Undefined;";
			else {
				G4int valuesLength = GetVectorLength(parameters[iToken]);
				outFile << valuesLength << ";";
				G4double unitValue = GetUnitValue(fParameterFile->GetUnitOfParameter(parameters[iToken]));
				for (G4int iValue=0; iValue<valuesLength; iValue++)
					outFile << " " << values[iValue]/unitValue << ";";
			}
		} else if (parameterType=="uv") {
			outFile << parameterType << "; " << parameters[iToken] << "; ";
			G4double* values = GetUnitlessVector(parameters[iToken]);
			if (fUnableToCalculateForDump)
				outFile << "; 1; Undefined;";
			else {
				G4int valuesLength = GetVectorLength(parameters[iToken]);
				outFile << "; " << valuesLength << ";";
				for (G4int iValue=0; iValue<valuesLength; iValue++)
					outFile << " " << values[iValue] << ";";
			}
		} else if (parameterType=="iv") {
			outFile << parameterType << "; " << parameters[iToken] << "; ";
			G4int* values = GetIntegerVector(parameters[iToken]);
			if (fUnableToCalculateForDump)
				outFile << "; 1; Undefined;";
			else {
				G4int valuesLength = GetVectorLength(parameters[iToken]);
				outFile << "; " << valuesLength << ";";
				for (G4int iValue=0; iValue<valuesLength; iValue++)
					outFile << " " << values[iValue] << ";";
			}
		} else if (parameterType=="bv") {
			outFile << parameterType << "; " << parameters[iToken] << "; ";
			G4bool* values = GetBooleanVector(parameters[iToken]);
			if (fUnableToCalculateForDump)
				outFile << "; 1; Undefined;";
			else {
				G4int valuesLength = GetVectorLength(parameters[iToken]);
				outFile << "; " << valuesLength << ";";
				for (G4int iValue=0; iValue<valuesLength; iValue++)
					outFile << " " << values[iValue] << ";";
			}
		} else if (parameterType=="sv") {
			outFile << parameterType << "; " << parameters[iToken] << "; ";
			G4String* values = GetStringVector(parameters[iToken]);
			G4int valuesLength = GetVectorLength(parameters[iToken]);
			outFile << "; " << valuesLength << ";";
			for (G4int iValue=0; iValue<valuesLength; iValue++)
				outFile << " " << values[iValue] << ";";
		} else {
			outFile << "?; " << parameters[iToken] << "; ; 1; Undefined; ";
		}

		outFile << G4endl;
	}

	outFile.close();

	fNowDoingParameterDump = false;

	G4cout << "Parameters have been dumped to the file " << filespec << G4endl;
}


void TsParameterManager::DumpAddedParameters() {
	G4String filespec = "ChangedParameters_"
						+ G4UIcommand::ConvertToString(fAddedParameterFileCounter++) + ".txt";
	std::ofstream outFile(filespec);
	if (!outFile) {
		G4cerr << "ERROR: Failed to open file " << filespec << G4endl;
		AbortSession(1);
	}

	std::time_t currentTime = std::time(0);
	outFile << "# To restore TOPAS to state GUI had on " << std::asctime(std::localtime(&currentTime)) << G4endl;

	outFile << "includeFile = " << fTopParameterFileSpec << "\n" << G4endl;

	std::map<G4String, G4String>::const_iterator iter;
	for (iter = fAddedParameters->begin(); iter != fAddedParameters->end(); ++iter)
		outFile << iter->first << " = " << iter->second << G4endl;

	outFile.close();

	G4cout << "Changed parameters have been saved to: " << filespec << G4endl;
}


void TsParameterManager::ListUnusedParameters() {
	fNowDoingParameterDump = true;

	G4cout << "\nBegin list of unused parameters:" << G4endl;
	// Loop over all found parameters
	std::map<G4String, TsVParameter*>* parameterMap = new std::map<G4String, TsVParameter*>;
	fParameterFile->GetAllParameters(parameterMap);
	std::map<G4String, TsVParameter*>::const_iterator iter;
	for (iter = parameterMap->begin(); iter != parameterMap->end(); ++iter) {
		TsVParameter* parameter = iter->second;
		if (!parameter->IsUsed())
			G4cout << parameter->GetName() << G4endl;
	}
	G4cout << "End list of unused parameters." << G4endl;
}


G4bool TsParameterManager::NowDoingParameterDump() {
	return fNowDoingParameterDump;
}


void TsParameterManager::UnableToCalculateForDump() {
	fUnableToCalculateForDump = true;
}


void TsParameterManager::NoteGeometryOverlap(G4bool state) {
	fHasGeometryOverlap = state;
}


G4bool TsParameterManager::HasGeometryOverlap() {
	return fHasGeometryOverlap;
}


void TsParameterManager::SetInQtSession() {
	fIsInQt = true;
}


G4bool TsParameterManager::IsInQtSession() {
	return fIsInQt;
}


G4bool TsParameterManager::IsFindingSeed() {
	return fIsFindingSeed;
}


G4int TsParameterManager::FindSeedRun() {
	return fFindSeedRun;
}


G4int TsParameterManager::FindSeedHistory() {
	return fFindSeedHistory;
}


G4bool TsParameterManager::UseVarianceReduction() {
	return fUseVarianceReduction;
}


G4Timer TsParameterManager::GetTimer() {
	return fTimer;
}


G4String TsParameterManager::GetDicomRootUID() {
	// TOPAS-specific root UID
	return "1.2.826.0.1.3680043.9.5871";
}


G4String TsParameterManager::GetStudyInstanceUID() {
	if (fStudyInstanceUID.empty()) {
		gdcm::UIDGenerator genUID;
		genUID.SetRoot(GetDicomRootUID());
		fStudyInstanceUID = genUID.Generate();
	}
	return fStudyInstanceUID;
}


G4String TsParameterManager::GetWorldFrameOfReferenceUID() {
	if (fWorldFrameOfReferenceUID.empty()) {
		gdcm::UIDGenerator genUID;
		genUID.SetRoot(GetDicomRootUID());
		fWorldFrameOfReferenceUID = genUID.Generate();
	}
	return fWorldFrameOfReferenceUID;
}


G4String TsParameterManager::GetTOPASVersion() {
	return fTOPASVersion;
}


G4String TsParameterManager::GetTopParameterFileSpec() {
	return fTopParameterFileSpec;
}


void TsParameterManager::ReadFile(G4String fileSpec, std::ifstream& infile,
								  std::vector<G4String>* names, std::vector<G4String>* values) {
	const std::string& delchar  = "=";
	const std::string& comchar   = "#";
	static const char forbiddeninName[] = "=+-*\"'`# \t\n\r";
	static const char forbiddeninValue[] = "='`??????#\r";

	// Since a comment may cover multiple lines,
	// we defer instantiating each parameter until we see the beginning of next parameter (or we reach the end of file).
	std::string nextline = "";
	G4String name = "";
	G4String value = "";

	while( infile || nextline.length() > 0 )
	{
		// Read an entire line at a time
		std::string line;
		if ( nextline.length() > 0 )
		{
			line = nextline;  // we read ahead; use it now
			nextline = "";
		} else {
			GetLineWithoutNewlineAndCarriageReturnIssues( infile, line );
		}

		// Ignore comments
		line = line.substr( 0, line.find(comchar) );

		// Replace smart single or double quotes with regular double quotes
		ReplaceStringInLine(line, "\u2018","\u0022");
		ReplaceStringInLine(line, "\u2019","\u0022");
		ReplaceStringInLine(line, "\u201C","\u0022");
		ReplaceStringInLine(line, "\u201D","\u0022");

		// Replace various unicode hyphen category characters with minus sign
		ReplaceStringInLine(line, "\u2010","\u002D");
		ReplaceStringInLine(line, "\u2011","\u002D");
		ReplaceStringInLine(line, "\u2012","\u002D");
		ReplaceStringInLine(line, "\u2013","\u002D");
		ReplaceStringInLine(line, "\u2014","\u002D");
		ReplaceStringInLine(line, "\u2015","\u002D");

		// Parse the line if it contains the delimiter
		std::string::size_type delcharPos = line.find( delchar );
		if ( delcharPos < std::string::npos )
		{
			// Extract the key
			std::string key = line.substr( 0, delcharPos );
			line.replace( 0, delcharPos+1, "" );

			// See if value continues on the next line.
			// Stop at blank line, next line with a key or end of stream.
			G4bool doneWithLine = false;
			while( !doneWithLine && infile )
			{
				GetLineWithoutNewlineAndCarriageReturnIssues( infile, nextline );
				doneWithLine = true;

				std::string copyOfNextLine = nextline;
				Trim(copyOfNextLine);
				if ( copyOfNextLine == "" ) continue;

				nextline = nextline.substr( 0, nextline.find(comchar) );
				if ( nextline.find(delchar) != std::string::npos ) continue;

				copyOfNextLine = nextline;
				Trim(copyOfNextLine);

				if ( copyOfNextLine != "" ) line += "\n";
				line += nextline;
				doneWithLine = false;
			}

			Trim(key);
			Trim(line);

			// A new parameter has been started.  Time to instantiate the previous parameter.
			if (name!="") {
				// Protect against reserved characters in name
				if (name.find_first_of(forbiddeninName) < name.size()) {
					char badChar = name[name.find_first_of(forbiddeninName)];
#if GEANT4_VERSION_MAJOR >= 11
					G4String badString(1,badChar);
#else
					G4String badString = badChar;
#endif
					if (badChar=='\"') badString = "Double Quotes";
					else if (badChar==' ') badString = "Space";
					else if (badChar=='\t') badString = "Tab";
					else if (badChar=='\n') badString = "Line Feed";
					else if (badChar=='\r') badString = "Carriage Return";
					G4cerr << "Topas quitting, parameter name: " << name << " in parameter file:" << fileSpec <<
					" uses the following reserved character in its name: \"" << badString << "\"" << G4endl;
					AbortSession(1);
				}

				// Protect against reserved characters in value
 				if (value.find_first_of(forbiddeninValue) < value.size()) {
					char badChar = value[value.find_first_of(forbiddeninValue)];
#if GEANT4_VERSION_MAJOR >= 11
					G4String badString(1,badChar);
#else
					G4String badString = badChar;
#endif
					if (badChar=='\r') badString = "Carriage Return";
					G4cerr << "Topas quitting, parameter name: " << name << " in parameter file:" << fileSpec <<
					" uses the following reserved character in its value: \"" << badString << "\"" << G4endl;
					AbortSession(1);
				}

				names->push_back(name);
				values->push_back(value);
			}

			name = key;
			value = line;
		}
	}

	// Instantiate the last parameter
	names->push_back(name);
	values->push_back(value);

	return;
}


void TsParameterManager::Trim(std::string& str) {
	// Remove leading and trailing whitespace
	static const char whitespace[] = " \n\t\v\r\f";
	str.erase( 0, str.find_first_not_of(whitespace) );
	str.erase( str.find_last_not_of(whitespace) + 1U );
}


// This method is taken from:
// http://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf
void TsParameterManager::GetLineWithoutNewlineAndCarriageReturnIssues(std::istream& is, std::string& t) {
	t.clear();

	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.

	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

	for(;;) {
		int c = sb->sbumpc();
		switch (c) {
			case '\n':
				return;
			case '\r':
				if(sb->sgetc() == '\n')
					sb->sbumpc();
				return;
			case EOF:
				// Also handle the case when the last line has no line ending
				if(t.empty())
					is.setstate(std::ios::eofbit);
				return;
			default:
				t += (char)c;
		}
	}
}


void TsParameterManager::ReplaceStringInLine(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if(start_pos != std::string::npos)
		str.replace(start_pos, from.length(), to);
}


void TsParameterManager::CreateUnits() {
	G4UnitsTable& pUnitTable = G4UnitDefinition::GetUnitsTable();

	if ( pUnitTable.size() == 0 )
		G4UnitDefinition::BuildUnitsTable();
	new G4UnitDefinition("unitless", "unitless", "unitless", 1.0);
	//Followings are all avialable unit categories in Geant4.9.p01.
	//Comment out for unit categories you whant to have time dependent units
	//and Specify numbers of those categories
	G4String time_dependent_unitcategories[] = {
		"Length",
		//	  "Surface",
		//	  "Volume",
		"Angle",
		//	  "Solid angle",
		//        "Frequency",
		//	  "Electric charge",
		"Energy",
		//	  "Energy/Length",
		//	  "Mass",
		//	  "Volumic Mass",
		//	  "Mass/Surface",
		//	  "Surface/Mass",
		//	  "Energy*Surface/Mass",
		//	  "Power",
		//	  "Force",
		//	  "Pressure",
		//	  "Electric current",
		//	  "Electric potential",
		//	  "Electric field",
		//	  "Magnetic flux",
		"Magnetic flux density",
		//	  "Temperature",
		//	  "Amount of substance",
		//	  "Activity",
		//	  "Dose", //uncommented because scoring as function of time.
		"unitless"
	};

	const size_t total_categories = 4;

	G4UnitsContainer* time_category = 0;
	size_t ntime_category =0;
	for ( size_t i =0; i < pUnitTable.size(); ++i )
		if ( pUnitTable[i]->GetName() == "Time" ) {
			time_category = &(pUnitTable[i]->GetUnitsList() );
			ntime_category = time_category->size();
		}

	for ( size_t i =0; i < pUnitTable.size(); ++i) {
		G4String cName = pUnitTable[i]->GetName();

		for ( size_t j=0; j < total_categories; ++j ) {
			if ( cName != time_dependent_unitcategories[j] )
				continue;

			//G4cout<<"Making time-dependent unit: "<< time_dependent_unitcategories[j] << G4endl;
			G4UnitsContainer& unitsInCategories = pUnitTable[i]->GetUnitsList();
			G4String u_cate_name = cName +"/Time";
			const size_t je = unitsInCategories.size();
			for ( size_t jIndex = 0; jIndex < je; ++jIndex) {
				for ( size_t k = 0; k < ntime_category; ++k) {
					G4String u_name = unitsInCategories[jIndex]->GetName()
					+ "/" + (*time_category)[k]->GetName();
					G4String u_symbol = unitsInCategories[jIndex]->GetSymbol()
					+"/" + (*time_category)[k]->GetSymbol();
					G4double new_val = unitsInCategories[jIndex]->GetValue()/(*time_category)[k]->GetValue();
					new G4UnitDefinition( u_name, u_symbol, u_cate_name, new_val );
				}
			}//for
		}//total_categoriy
	}//Unit table
	new G4UnitDefinition("ps_1" , "1/ps" , "/Time", 1.0/picosecond );
	new G4UnitDefinition("ns_1" , "1/ns" , "/Time", 1.0/nanosecond );
	new G4UnitDefinition("mus_1", "1/mus", "/Time", 1.0/microsecond );
	new G4UnitDefinition("ms_1" , "1/ms" , "/Time", 1.0/millisecond );
	new G4UnitDefinition("s_1"  , "1/s"  , "/Time", 1.0/second );
}


// This method was copied from G4UIcommand.  Would have used it directly from there, but it is a private method.
G4int TsParameterManager::IsDouble(const char* buf)
{
	const char* p= buf;
	switch( *p) {
		case '+':  case '-': ++p;
			if ( isdigit(*p) ) {
				while( isdigit( (G4int)(*p) )) { ++p; }
				switch ( *p ) {
					case '\0':	return 1;
						// break;
					case 'E':  case 'e':
						return ExpectExponent(++p );
						// break;
					case '.':  ++p;
						if ( *p == '\0' )  return 1;
						if ( *p == 'e' || *p =='E' ) return ExpectExponent(++p );
						if ( isdigit(*p) ) {
							while( isdigit( (G4int)(*p) )) { ++p; }
							if ( *p == '\0' )  return 1;
							if ( *p == 'e' || *p =='E') return ExpectExponent(++p);
						} else return 0;   break;
					default: return 0;
				}
			}
			if ( *p == '.' ) { ++p;
				if ( isdigit(*p) ) {
					while( isdigit( (G4int)(*p) )) { ++p; }
					if ( *p == '\0' )  return 1;
					if ( *p == 'e' || *p =='E')  return ExpectExponent(++p);
				}
			}
			break;
		case '.':  ++p;
			if ( isdigit(*p) ) {
				while( isdigit( (G4int)(*p) )) { ++p; }
				if ( *p == '\0' )  return 1;
				if ( *p == 'e' || *p =='E' )  return ExpectExponent(++p);
			}	break;
		default: // digit is expected
			if ( isdigit(*p) ) {
				while( isdigit( (G4int)(*p) )) { ++p; }
				if ( *p == '\0' )  return 1;
				if ( *p == 'e' || *p =='E')  return ExpectExponent(++p);
				if ( *p == '.' ) { ++p;
					if ( *p == '\0' )  return 1;
					if ( *p == 'e' || *p =='E')  return ExpectExponent(++p);
					if ( isdigit(*p) ) {
						while( isdigit( (G4int)(*p) )) { ++p; }
						if ( *p == '\0' )  return 1;
						if ( *p == 'e' || *p =='E') return ExpectExponent(++p);
					}
				}
			}
	}
	return 0;
}


// This method was copied from G4UIcommand.  Would have used it directly from there, but it is a private method.
G4int TsParameterManager::ExpectExponent(const char* str)   // used only by IsDouble()
{
	G4int maxExplength;
	if ( IsInteger( str, maxExplength=7 )) return 1;
	else return 0;
}


// This method was copied from G4UIcommand.  Would have used it directly from there, but it is a private method.
G4int TsParameterManager::IsInteger(const char* buf, short maxDigits)
{
	const char* p= buf;
	G4int length=0;
	if ( *p == '+' || *p == '-') { ++p; }
	if ( isdigit( (G4int)(*p) )) {
		while( isdigit( (G4int)(*p) )) { ++p;  ++length; }
		if ( *p == '\0' ) {
			if ( length > maxDigits) {
				G4cerr <<"digit length exceeds"<<G4endl;
				return 0;
			}
			return 1;
		} else {
			// G4cerr <<"illegal character after int:"<<buf<<G4endl;
		}
	} else {
		// G4cerr <<"illegal int:"<<buf<<G4endl;
	}
	return 0;
}


G4int TsParameterManager::IsBoolean(const char* buf)
{
	G4String testString = buf;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(testString);
#else
	testString.toLower();
#endif
	if (testString=="t" || testString=="true" || testString=="1" || testString=="f" || testString=="false" || testString=="0")
		return 1;
	else
		return 0;
}


G4bool TsParameterManager::IsRandomMode() {
	return fIsRandomMode;
}
