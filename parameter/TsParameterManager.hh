//
// ********************************************************************
// *                                                                  *
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

#ifndef TsParameterManager_hh
#define TsParameterManager_hh

#include <map>
#include <vector>

#include "TsTopasConfig.hh"

#ifdef TOPAS_MT
#include "G4Cache.hh"
#endif

#include "G4TwoVector.hh"
#include "G4ThreeVector.hh"
#include "G4Timer.hh"

class TsSequenceManager;
class TsParameterFile;
class TsVParameter;
class G4VisAttributes;
class G4ParticleDefinition;

struct TsParticleDefinition
{
	G4ParticleDefinition* particleDefinition;
	G4bool isOpticalPhoton;
	G4bool isGenericIon;
	G4int ionZ;
	G4int ionA;
	G4int ionCharge;
};

class TsParameterManager
{
public:
	TsParameterManager(G4int argc, char** argv, G4String topasVersion);
	~TsParameterManager();

	// See if parameter exists
	G4bool ParameterExists(const G4String& parameterName);
	G4bool ParameterExists(const char* parameterName);

	// Get number of values in a vector parameter
	G4int GetVectorLength(const G4String& parameterName);
	G4int GetVectorLength(const char* parameterName);

	// Get double value of parameter in Geant4's internal units
	G4double GetDoubleParameter(const G4String& parameterName, const char* unitCategory);
	G4double GetDoubleParameter(const char* parameterName, const char* unitCategory);

	// Get double value of a unitless parameter
	G4double GetUnitlessParameter(const G4String& parameterName);
	G4double GetUnitlessParameter(const char* parameterName);

	// Get integer value of parameter
	G4int GetIntegerParameter(const G4String& parameterName);
	G4int GetIntegerParameter(const char* parameterName);

	// Get Boolean value of parameter
	G4bool GetBooleanParameter(const G4String& parameterName);
	G4bool GetBooleanParameter(const char* parameterName);

	// Get string value of parameter (whether it is actually a string parameter of not)
	G4String GetStringParameter(const G4String& parameterName);
	G4String GetStringParameter(const char* parameterName);

	// Get vector of double values of parameter in Geant4's internal units
	G4double* GetDoubleVector(const G4String& parameterName, const char* unitCategory);
	G4double* GetDoubleVector(const char* parameterName, const char* unitCategory);

	// Get vector of double values of a unitless parameter
	G4double* GetUnitlessVector(const G4String& parameterName);
	G4double* GetUnitlessVector(const char* parameterName);

	// Get vector of integer values of parameter
	G4int* GetIntegerVector(const G4String& parameterName);
	G4int* GetIntegerVector(const char* parameterName);

	// Get vector of Boolean values of parameter
	G4bool* GetBooleanVector(const G4String& parameterName);
	G4bool* GetBooleanVector(const char* parameterName);

	// Get vector of string values of parameter
	G4String* GetStringVector(const G4String& parameterName);
	G4String* GetStringVector(const char* parameterName);

	// Get TwoVector of double values of parameter in Geant4's internal units
	G4TwoVector GetTwoVectorParameter(const G4String& parameterName, const char* unitCategory);
	G4TwoVector GetTwoVectorParameter(const char* parameterName, const char* unitCategory);

	// Get ThreeVector of double values of parameter in Geant4's internal units
	G4ThreeVector GetThreeVectorParameter(const G4String& parameterName, const char* unitCategory);
	G4ThreeVector GetThreeVectorParameter(const char* parameterName, const char* unitCategory);

	// Get defined color
	G4VisAttributes* GetColor(G4String name);
	G4VisAttributes* GetColor(const char* name);
	G4VisAttributes* GetInvisible();

	// Evaluate a particle definition string
	TsParticleDefinition GetParticleDefinition(G4String name);

	// Register name of a filter
	void RegisterFilterName(G4String filterName);

	// Call if your scorer or filter is going to need TsTrackInformation
	void SetNeedsTrackingAction();

	// Call if your scorer or filter is going to need traversed geometry information
	void SetNeedsSteppingAction();

	// Call if your physics module requires G4DNA chemistry
	void SetNeedsChemistry();

	// Add a parameter
	void AddParameter(const G4String& name, const G4String& value, G4bool test = false, G4bool permissive = false);

	// Get the part of a string after the last slash, in lower case
	G4String GetPartAfterLastSlash(const G4String& name);

	// Aborts the session in a way that can be trapped by various GUIs
	void AbortSession(G4int exitCode);

	// User classes should not access any methods or data beyond this point

	void UpdateTimeFeatureStore();
	void CloneParameter(const G4String& oldName, const G4String& newName);

	std::vector<G4String>* GetFilterNames();

	G4bool NeedsTrackingAction();
	G4bool NeedsSteppingAction();
	G4bool NeedsChemistry();

	G4double GetCurrentTime();

	G4int GetRunID();

	void HandleFirstEvent();

	G4bool IsChangeable(const G4String& parameterName);

	void GetParameterNamesStartingWith(const G4String& s, std::vector<G4String>* parameterNames);
	void GetParameterNamesBracketedBy(const G4String& prefix, const G4String& suffix, std::vector<G4String>* parameterNames);
	void CheckFilterParameterNamesStartingWith(const G4String& prefix, std::vector<G4String>* filterNames);

	G4double IGetDoubleParameter(const G4String& parameterName);
	G4double IGetUnitlessParameter(const G4String& parameterName);
	G4int IGetIntegerParameter(const G4String& parameterName);
	G4bool IGetBooleanParameter(const G4String& parameterName);
	G4String IGetStringParameter(const G4String& parameterName);
	G4String IGetStringParameter(const char* parameterName);
	G4double* IGetDoubleVector(const G4String& parameterName);
	G4double* IGetUnitlessVector(const G4String& parameterName);
	G4int* IGetIntegerVector(const G4String& parameterName);
	G4bool* IGetBooleanVector(const G4String& parameterName);
	G4String* IGetStringVector(const G4String& parameterName);

	G4String GetStringParameterWithoutMonitoring(const G4String& parameterName);
	G4String GetStringParameterWithoutMonitoring(const char* parameterName);

	G4String GetTypeOfParameter(const G4String& parameterName);
	G4String GetUnitOfParameter(const G4String& parameterName);

	G4String GetUnitCategoryOfParameter(const G4String& parameterName);
	G4String GetUnitCategory(const G4String& unitString);
	G4double GetUnitValue(const G4String& unitString);

	void SetLastDirectAction(G4String action);
	void SetLastDirectParameterName(G4String name);
	G4String GetLastDirectAction();
	G4String GetLastDirectParameterName();

	std::vector<TsVParameter*>* GetTimeFeatureStore(G4double currentTime);

	G4double GetCurrentSequenceTime() {return fSequenceTime;}

	void SetSequenceManager(TsSequenceManager* sqM);
	TsSequenceManager* GetSequenceManager();

	G4bool AddParameterHasBeenCalled();

	void GetChangeableParameters(std::vector<G4String>* names, std::vector<G4String>* values);
	G4String GetParameterValueAsString(G4String parameterType, G4String parameterName);
	void DumpParameters(G4double currentTime, G4bool includeDefaults);
	void DumpParametersToSimpleFile(G4double currentTime);
	void DumpParametersToSemicolonSeparatedFile(G4double currentTime);
	void DumpAddedParameters();
	void ListUnusedParameters();
	G4bool NowDoingParameterDump();
	void UnableToCalculateForDump();

	void NoteGeometryOverlap(G4bool state);
	G4bool HasGeometryOverlap();

	void SetInQtSession();
	G4bool IsInQtSession();

	G4bool IsFindingSeed();
	G4int FindSeedRun();
	G4int FindSeedHistory();

	G4bool UseVarianceReduction();

	void RegisterParameterFile(G4String fileName, TsParameterFile* parameterFile);
	TsParameterFile* GetParameterFile(G4String fileName);

	void RegisterComponentTypeName(G4String typeName);
	std::vector<G4String> GetComponentTypeNames();

	void RegisterScorerQuantityName(G4String quantityName);
	std::vector<G4String> GetScorerQuantityNames();

	std::vector<G4String> GetColorNames();

	G4Timer GetTimer();

	G4String GetDicomRootUID();
	G4String GetStudyInstanceUID();
	G4String GetWorldFrameOfReferenceUID();
	G4String GetTOPASVersion();
	G4String GetTopParameterFileSpec();

	void ReadFile(G4String fileSpec, std::ifstream& inputStream, std::vector<G4String>* names, std::vector<G4String>* values);
	void Trim(std::string& inputString);
	void GetLineWithoutNewlineAndCarriageReturnIssues(std::istream& is, std::string& t);
	void ReplaceStringInLine(std::string& str, const std::string& from, const std::string& to);

	// These two methods were copied from G4UIcommand.
	// Would have used them directly from there, but they are private methods.
	G4int IsDouble(const char* str);
	G4int IsInteger(const char* str, short maxLength);
	G4int IsBoolean(const char* str);

	G4bool IsRandomMode();

	G4bool fTestMode;

private:
	void CreateUnits();

	void SetCurrentTime(G4double t) { fSequenceTime=t;}

	G4int ExpectExponent(const char* str);

	G4String fStudyInstanceUID;
	G4String fWorldFrameOfReferenceUID;
	G4String fTOPASVersion;

	G4String fTopParameterFileSpec;
	TsParameterFile* fParameterFile;
	TsSequenceManager* fSqm;
	G4Timer	fTimer;
	G4double fSequenceTime;
	G4bool fIsRandomMode;

#ifdef TOPAS_MT
	G4Cache<G4String> fLastDirectAction;
	G4Cache<G4String> fLastDirectParameterName;
#else
	G4String fLastDirectAction;
	G4String fLastDirectParameterName;
#endif

	G4bool fAddParameterHasBeenCalled;

	G4bool fNowDoingParameterDump;
	G4bool fUnableToCalculateForDump;

	G4bool fHasGeometryOverlap;

	G4bool fNeedsTrackingAction;
	G4bool fNeedsSteppingAction;
	G4bool fNeedsChemistry;

	G4bool fHandledFirstEvent;
	G4bool fIsInQt;

	G4bool fIsFindingSeed;
	G4int fFindSeedRun;
	G4int fFindSeedHistory;

	G4bool fUseVarianceReduction;

	G4VisAttributes* fInvisible;

	std::map<G4String,G4VisAttributes*>* fColorMap;
	std::map<G4String, TsParameterFile*>* fParameterFileStore;
	std::vector<TsVParameter*>* fTimeFeatureStore;
	std::vector<G4String>* fComponentTypeNames;
	std::vector<G4String>* fScorerQuantityNames;
	std::vector<G4String>* fFilterNames;

	std::map<G4String, G4String>* fAddedParameters;
	G4int fAddedParameterFileCounter;
};
#endif
