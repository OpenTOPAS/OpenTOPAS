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

#ifndef TsVParameter_hh
#define TsVParameter_hh

#include "TsParameterManager.hh"

#include "G4UIcommand.hh"

class TsTempParameter;

class TsVParameter
{
public:
	TsVParameter(TsParameterManager* pM, TsParameterFile* pf, TsTempParameter* tempParameter);

	~TsVParameter();

	virtual void CheckRelativeParameterConsistency();
	virtual void InitializeTimeFeatureValue();

	G4bool IsChangeable();
	virtual G4bool ValueHasChanged();

	G4String GetName() {return fName;};
	virtual G4String GetType() = 0;
	virtual G4String GetUnit();

	virtual G4int GetVectorLength();

	virtual G4double GetDoubleValue();
	virtual G4double GetUnitlessValue();
	virtual G4int GetIntegerValue();
	virtual G4bool GetBooleanValue();
	virtual G4String GetStringValue();
	virtual G4double* GetDoubleVector();
	virtual G4double* GetUnitlessVector();
	virtual G4int* GetIntegerVector();
	virtual G4bool* GetBooleanVector();
	virtual G4String* GetStringVector();

	virtual G4String GetHTMLValue() = 0;

	G4bool IsUsed();

	TsParameterFile* GetParameterFile();

protected:
	void QuitIfMustBeAbsolute();

	G4double ConvertToDouble(const G4String& value);
	G4int ConvertToInteger(const G4String& value);
	G4bool ConvertToBoolean(const G4String& value);
	void ConvertToDoubleVector(G4int nValues, G4String* values, G4double* fValues, G4String* fValuesIfParameterNames);
	void ConvertToIntegerVector(G4int nValues, G4String* values, G4int* fValues, G4String* fValuesIfParameterNames);
	void ConvertToBooleanVector(G4int nValues, G4String* values, G4bool* fValues, G4String* fValuesIfParameterNames);
	void ConvertToStringVector(G4int nValues, G4String* values, G4String* fValues, G4String* fValuesIfParameterNames);
	G4int SafeConvertToInteger(const char* value);

	G4String ParseNameOfOtherParameter(const G4String& nameOfOtherParameter, const char* type);
	G4int GetLengthOfOtherParameter(const G4String& otherParameterName);

	void CheckUnit(const G4String& unit);
	void CheckUnitAgreement(const G4String& nameOfOtherParameter);
	void CheckVectorLengthAgreement(const G4String& nameOfOtherParameter);

	G4double EvaluateDoubleParameter(const G4String& nameOfOtherParameter);
	G4double EvaluateUnitlessParameter(const G4String& nameOfOtherParameter);
	G4int EvaluateIntegerParameter(const G4String& nameOfOtherParameter);
	G4bool EvaluateBooleanParameter(const G4String& nameOfOtherParameter);
	G4String EvaluateStringParameter(const G4String& nameOfOtherParameter);
	G4double* EvaluateDoubleVector(const G4String& nameOfOtherParameter);
	G4double* EvaluateUnitlessVector(const G4String& nameOfOtherParameter);
	G4int* EvaluateIntegerVector(const G4String& nameOfOtherParameter);
	G4bool* EvaluateBooleanVector(const G4String& nameOfOtherParameter);
	G4String* EvaluateStringVector(const G4String& nameOfOtherParameter);

	G4String EvaluateAsHTML(const G4String& nameOfOtherParameter);

	void OtherParameterMissing(const char* nameOfOtherParameter);
	void WrongType(const char* type);
	void WrongUnitCategory();
	void MissingUnit();
	void UnrecognizedUnit(const G4String& unit);
	void VectorCountTooLow();
	void WrongSyntax();

	TsParameterManager* fPm;

	G4bool fUsed;

private:
	TsParameterFile* fPf;
	TsTempParameter* fTempParameter;
	G4String fName;
	G4bool fIsChangeable;
};

#endif
