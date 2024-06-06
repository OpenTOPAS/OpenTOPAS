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

#ifndef TsParameterFile_hh
#define TsParameterFile_hh

#include "globals.hh"

#include <vector>
#include <map>

class TsVParameter;
class TsParameterManager;
class TsTempParameter;

class TsParameterFile
{
public:
	TsParameterFile(TsParameterManager* pM, const G4String& fileSpec, const G4String& topFileSpec, TsParameterFile* transientFile);

	~TsParameterFile();

	TsTempParameter* AddTempParameter(const G4String& typeAndName, const G4String& value);
	void ProcessTempParameters(G4bool test = false);
	void ProtectAgainstControlByDifferentArms(G4String paramName, TsParameterFile* includeFile1, G4String prefix, G4String suffix);

	G4bool ParameterExists(const G4String& parameterName);

	G4int GetVectorLength(const G4String& parameterName);

	// Double values are returned in Geant4's internal units
	G4double GetDoubleParameter(const G4String& parameterName);
	G4double GetUnitlessParameter(const G4String& parameterName);
	G4int GetIntegerParameter(const G4String& parameterName);
	G4bool GetBooleanParameter(const G4String& parameterName);
	G4String GetStringParameter(const G4String& parameterName);

	G4double* GetDoubleVector(const G4String& parameterName);
	G4double* GetUnitlessVector(const G4String& parameterName);
	G4int* GetIntegerVector(const G4String& parameterName);
	G4bool* GetBooleanVector(const G4String& parameterName);
	G4String* GetStringVector(const G4String& parameterName);

	G4String GetHTMLValueOfParameter(const G4String& parameterName);

	G4String GetTypeOfParameter(const G4String& parameterName);
	G4String GetUnitOfParameter(const G4String& parameterName);
	G4bool IsChangeable(const G4String& parameterName);

	G4String GetTempTypeOfParameter(const G4String& parameterName);

	TsParameterFile* GetParentFile();

	void GetAllParameters(std::map<G4String, TsVParameter*>* parameterMap);
	void GetAllParametersBeforeLinearized(std::map<G4String, TsVParameter*>* parameterMap);
	void GetParameterNamesStartingWith(const G4String& value, std::vector<G4String>*);
	void GetParametersOperatingOn(const G4String& value, std::vector<TsVParameter*>*);
	void GetParameterNamesBracketedBy(const G4String& prefix, const G4String& suffix, std::vector<G4String>*);
	void CheckFilterParameterNamesStartingWith(const G4String& prefix, std::vector<G4String>*);
	void GetTimeFeatureParameters(std::vector<TsVParameter*>* );
	void BuildVectorOfParameterFiles(std::vector<TsParameterFile*>* parameterFiles);

	G4String GetFileName();

private:
	TsVParameter* ConvertTempToRealParameter(G4String, TsTempParameter*) = delete; // delete not implemented
	void CheckChainsForConflicts();

	TsVParameter* GetParameter(const G4String& parameterName);
	TsVParameter* GetParameterBeforeLinearized(const G4String& parameterName);

	void ResetParentFileTo(TsParameterFile* parent);

	void Quit(TsTempParameter* tempParameter, const char* message);
	void Quit(const G4String& parameterName, const char* message);
	void Undefined(const G4String& parameterName);

	TsParameterManager* fPm;
	G4String fFileSpec;
	G4String fDefaultFileSpec;
	G4String fIncludeFileSpecs;
	TsParameterFile* fParentFile;
	TsParameterFile* fTransientFile;

	std::vector<TsParameterFile*>* fIncludeFiles;
	std::map<G4String,TsTempParameter*>* fTempParameters;
	std::map<G4String,TsVParameter*>* fRealParameters;
	std::vector<TsVParameter*>* fTimeFeatureParameters;
};

#endif
