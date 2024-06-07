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

#include "TsVParameter.hh"

#include "TsParameterFile.hh"
#include "TsTempParameter.hh"

TsVParameter::TsVParameter(TsParameterManager* pM, TsParameterFile* pf, TsTempParameter* tempParameter)
:fPm(pM), fUsed(false), fPf(pf), fTempParameter(tempParameter),
fName(tempParameter->GetName()), fIsChangeable(tempParameter->IsChangeable())
{;}


TsVParameter::~TsVParameter()
{;}


void TsVParameter::CheckRelativeParameterConsistency()
{;}


void TsVParameter::InitializeTimeFeatureValue()
{;}


G4bool TsVParameter::IsChangeable() {
	return fIsChangeable;
}


G4bool TsVParameter::ValueHasChanged() {
	return false;
}


G4String TsVParameter::GetUnit() {
	G4String unit = "";
	return unit;
}


G4int TsVParameter::GetVectorLength() {
	G4cerr << "Topas is exiting due to a serious error triggered by:" << G4endl;
	G4cerr << fPm->GetLastDirectAction() << " for parameter: " << fPm->GetLastDirectParameterName() << G4endl;
	G4cerr << "Attempt to get vector length from a non-vector parameter.";
	G4cerr << "Parameter name was: " << fName << " from parameter file: " << fPf->GetFileName() << G4endl;
	fPm->AbortSession(1);
	return 0;
}


G4double TsVParameter::GetDoubleValue() {
	WrongType("double");
	return -99999999.;
}


G4double TsVParameter::GetUnitlessValue() {
	WrongType("unitless");
	return -99999999.;
}


G4int TsVParameter::GetIntegerValue() {
	WrongType("integer");
	return -99999999;
}


G4bool TsVParameter::GetBooleanValue() {
	WrongType("boolean");
	return false;
}


G4String TsVParameter::GetStringValue() {
	WrongType("string");
	G4String sValue = "";
	return sValue;
}


G4double* TsVParameter::GetDoubleVector() {
	WrongType("double vector");
	G4double* dValue = new G4double[1];
	dValue[0] = -9999999.;
	return dValue;
}


G4double* TsVParameter::GetUnitlessVector() {
	WrongType("unitless vector");
	G4double* dValue = new G4double[1];
	dValue[0] = -9999999.;
	return dValue;
}



G4int* TsVParameter::GetIntegerVector() {
	WrongType("integer vector");
	G4int* iValue = new G4int[1];
	iValue[0] = -9999999;
	return iValue;
}


G4bool* TsVParameter::GetBooleanVector() {
	WrongType("boolean vector");
	G4bool* bValue = new G4bool[1];
	bValue[0] = false;
	return bValue;
}


G4String* TsVParameter::GetStringVector() {
	WrongType("string vector");
	G4String* sValue = new G4String[1];
	sValue[0] = "";
	return sValue;
}


G4bool TsVParameter::IsUsed() {
	return fUsed;
}


TsParameterFile* TsVParameter::GetParameterFile() {
	return fPf;
}


void TsVParameter::QuitIfMustBeAbsolute() {
	// If value was being evaluated just to write out to a parameter dump, no need to quit."
	if (fPm->NowDoingParameterDump()) {
		fPm->UnableToCalculateForDump();
		return;
	}

	if (fTempParameter->MustBeAbsolute()) {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "Parameter is set in two different include file chains." << G4endl;
		G4cerr << "This is only allowed if top file sets this parameter to an absolute value (not relative to any other parameters)." << G4endl;
		G4cerr << "Parameter name was: " << fName << " from parameter file: " << fPf->GetFileName() << G4endl;
		fPm->AbortSession(1);
	}
}


G4double TsVParameter::ConvertToDouble(const G4String& value) {
	if (!fPm->IsDouble(value)) WrongSyntax();
	return G4UIcommand::ConvertToDouble(value);
}


G4int TsVParameter::ConvertToInteger(const G4String& value) {
	if (!fPm->IsInteger(value,32)) WrongSyntax();
	return SafeConvertToInteger(value);
}


G4bool TsVParameter::ConvertToBoolean(const G4String& value) {
	if (value.substr(0,1)!="\"") WrongSyntax();

	const auto secondQuotePos = value.find('"',1);
	if (secondQuotePos==G4String::npos) WrongSyntax();

	G4String trimmedValue = value.substr(1,secondQuotePos-1);
	if (!fPm->IsBoolean(trimmedValue)) WrongSyntax();

	return G4UIcommand::ConvertToBool(trimmedValue);
}


void TsVParameter::ConvertToDoubleVector(G4int nValues, G4String* values, G4double* fValues, G4String* fValuesIfParameterNames) {
	if (nValues < 1) VectorCountTooLow();

	for (G4int iValue=0; iValue<nValues; iValue++) {
		if (fPm->IsDouble(values[iValue])) {
			fValues[iValue] = G4UIcommand::ConvertToDouble(values[iValue]);
			fValuesIfParameterNames[iValue] = "";
		} else {
			fValuesIfParameterNames[iValue] = values[iValue];
		}
	}
}


void TsVParameter::ConvertToIntegerVector(G4int nValues, G4String* values, G4int* fValues, G4String* fValuesIfParameterNames) {
	if (nValues < 1) VectorCountTooLow();

	for (G4int iValue=0; iValue<nValues; iValue++) {
		if (fPm->IsInteger(values[iValue],32)) {
			fValues[iValue] = SafeConvertToInteger(values[iValue]);
			fValuesIfParameterNames[iValue] = "";
		} else if (fPm->IsDouble(values[iValue])) {
			WrongSyntax();
		} else {
			fValuesIfParameterNames[iValue] = values[iValue];
		}
	}
}


void TsVParameter::ConvertToBooleanVector(G4int nValues, G4String* values, G4bool* fValues, G4String* fValuesIfParameterNames) {
	if (nValues < 1) VectorCountTooLow();

	for (G4int iValue=0; iValue<nValues; iValue++) {
		if (values[iValue].substr(0,1)=="\"") {
			const auto secondQuotePos = values[iValue].find('"',1);
			if (secondQuotePos==G4String::npos) WrongSyntax();
			values[iValue] = values[iValue].substr(1,secondQuotePos-1);
			if (!fPm->IsBoolean(values[iValue])) WrongSyntax();
			fValues[iValue] = G4UIcommand::ConvertToBool(values[iValue]);
			fValuesIfParameterNames[iValue] = "";
		} else {
			fValuesIfParameterNames[iValue] = values[iValue];
		}
	}
}


void TsVParameter::ConvertToStringVector(G4int nValues, G4String* values, G4String* fValues, G4String* fValuesIfParameterNames) {
	if (nValues < 1) VectorCountTooLow();

	for (G4int iValue=0; iValue<nValues; iValue++) {
		if (values[iValue].substr(0,1)=="\"") {
			const auto secondQuotePos = values[iValue].find('"',1);
			if (secondQuotePos==G4String::npos) WrongSyntax();
			values[iValue] = values[iValue].substr(1,secondQuotePos-1);
			fValues[iValue] = values[iValue];
			fValuesIfParameterNames[iValue] = "";
		} else {
			fValuesIfParameterNames[iValue] = values[iValue];
		}
	}
}


G4int TsVParameter::SafeConvertToInteger(const char* value) {
	G4int val;
	std::istringstream is(value);
	is >> val;
	if (is.fail()) {
		G4cerr << "Topas is exiting due to a serious error triggered by:" << G4endl;
		G4cerr << fPm->GetLastDirectAction() << " for parameter: " << fPm->GetLastDirectParameterName() << G4endl;
		G4cerr << "Parameter name: " << fTempParameter->GetType() << ":" << fName << G4endl;
		G4cerr << "has out-of-bounds value: " << fTempParameter->GetValue() << G4endl;
		G4cerr << "Integers must be between " << INT_MIN << " and " << INT_MAX << G4endl;
		fPm->AbortSession(1);
	}
	return val;
}


void TsVParameter::CheckUnit(const G4String& unit) {
	// Check that this is a known unit
	if (unit.empty()) MissingUnit();
	G4String unitCategory = fPm->GetUnitCategory(unit);
	if (unitCategory =="None") UnrecognizedUnit(unit);

	// If this parameter exists in the parent file, make sure the unit matches
	G4String parentUnit = fPf->GetUnitOfParameter(fName);
	if (parentUnit!="" && fPm->GetUnitCategory(parentUnit) != unitCategory)
		WrongUnitCategory();
}


G4String TsVParameter::ParseNameOfOtherParameter(const G4String& nameOfOtherParameter, const char* type) {
	G4String output;

	if (nameOfOtherParameter == fName) {
		output = "";
	} else {
		G4String testName = nameOfOtherParameter;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(testName);
#else
		testName.toLower();
#endif

		if (testName == "inheritedvalue") {
			output = "";
		} else {
			G4String otherParameterType = fPf->GetTempTypeOfParameter(nameOfOtherParameter);
			if (otherParameterType=="") OtherParameterMissing(nameOfOtherParameter);
			if (otherParameterType!= type) WrongSyntax();
			output = nameOfOtherParameter;
		}
	}
	return output;
}


G4int TsVParameter::GetLengthOfOtherParameter(const G4String& otherParameterName) {
	return fPf->GetVectorLength(otherParameterName);
}


void TsVParameter::CheckUnitAgreement(const G4String& otherParameterName) {
	if ( otherParameterName!="" &&
		( fPm->GetUnitCategory(fPf->GetUnitOfParameter(fName)) != fPm->GetUnitCategory(fPf->GetUnitOfParameter(otherParameterName)))) {

		// If value was being evaluated just to write out to a parameter dump, no need to quit."
		if (fPm->NowDoingParameterDump()) {
			fPm->UnableToCalculateForDump();
			return;
		}

		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "Parameter name: " << fName << " expects units of: " << fPm->GetUnitCategory(fPf->GetUnitOfParameter(fName)) << G4endl;
		G4cerr << "But its value is computed from parameter name: " << otherParameterName <<
		" which has units of: " << fPm->GetUnitCategory(fPf->GetUnitOfParameter(otherParameterName)) << G4endl;
		fPm->AbortSession(1);
	}
}


void TsVParameter::CheckVectorLengthAgreement(const G4String& otherParameterName) {
	if ( fPf->GetVectorLength(fName) != fPf->GetVectorLength(otherParameterName)) {

		// If value was being evaluated just to write out to a parameter dump, no need to quit."
		if (fPm->NowDoingParameterDump()) {
			fPm->UnableToCalculateForDump();
			return;
		}

		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "Parameter name: " << fName << " has vector length of: " << fPf->GetVectorLength(fName) << G4endl;
		G4cerr << "But is being computed from parameter name: " << otherParameterName <<
		" which has vector length of: " << fPf->GetVectorLength(otherParameterName) << G4endl;
		fPm->AbortSession(1);
	}
}


G4double TsVParameter::EvaluateDoubleParameter(const G4String& nameOfOtherParameter) {
	if (nameOfOtherParameter=="")
		return fPf->GetParentFile()->GetDoubleParameter(fName);
	else
		return fPm->IGetDoubleParameter(nameOfOtherParameter);
}


G4double TsVParameter::EvaluateUnitlessParameter(const G4String& nameOfOtherParameter) {
	if (nameOfOtherParameter=="")
		return fPf->GetParentFile()->GetUnitlessParameter(fName);
	else
		return fPm->IGetUnitlessParameter(nameOfOtherParameter);
}


G4int TsVParameter::EvaluateIntegerParameter(const G4String& nameOfOtherParameter) {
	if (nameOfOtherParameter=="")
		return fPf->GetParentFile()->GetIntegerParameter(fName);
	else
		return fPm->IGetIntegerParameter(nameOfOtherParameter);
}


G4bool TsVParameter::EvaluateBooleanParameter(const G4String& nameOfOtherParameter) {
	if (nameOfOtherParameter=="")
		return fPf->GetParentFile()->GetBooleanParameter(fName);
	else
		return fPm->IGetBooleanParameter(nameOfOtherParameter);
}


G4String TsVParameter::EvaluateStringParameter(const G4String& nameOfOtherParameter) {
	if (nameOfOtherParameter=="")
		return fPf->GetParentFile()->GetStringParameter(fName);
	else
		return fPm->IGetStringParameter(nameOfOtherParameter);
}


G4double* TsVParameter::EvaluateDoubleVector(const G4String& nameOfOtherParameter) {
	if (nameOfOtherParameter=="")
		return fPf->GetParentFile()->GetDoubleVector(fName);
	else
		return fPm->IGetDoubleVector(nameOfOtherParameter);
}


G4double* TsVParameter::EvaluateUnitlessVector(const G4String& nameOfOtherParameter) {
	if (nameOfOtherParameter=="")
		return fPf->GetParentFile()->GetUnitlessVector(fName);
	else
		return fPm->IGetUnitlessVector(nameOfOtherParameter);
}


G4int* TsVParameter::EvaluateIntegerVector(const G4String& nameOfOtherParameter) {
	if (nameOfOtherParameter=="")
		return fPf->GetParentFile()->GetIntegerVector(fName);
	else
		return fPm->IGetIntegerVector(nameOfOtherParameter);
}


G4bool* TsVParameter::EvaluateBooleanVector(const G4String& nameOfOtherParameter) {
	if (nameOfOtherParameter=="")
		return fPf->GetParentFile()->GetBooleanVector(fName);
	else
		return fPm->IGetBooleanVector(nameOfOtherParameter);
}


G4String* TsVParameter::EvaluateStringVector(const G4String& nameOfOtherParameter) {
	if (nameOfOtherParameter=="")
		return fPf->GetParentFile()->GetStringVector(fName);
	else
		return fPm->IGetStringVector(nameOfOtherParameter);
}


G4String TsVParameter::EvaluateAsHTML(const G4String& nameOfOtherParameter) {
	G4String output;

	if (nameOfOtherParameter=="")
		output = "<a href='#" + fName + "'>" + fName + "</a>";
	else {
		G4String testName = nameOfOtherParameter;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(testName);
#else
		testName.toLower();
#endif
		if (testName.substr(0,7)=="parent/")
			output = "nameOfOtherParameter";
		else
			output = "<a href='#" + nameOfOtherParameter + "'>" + nameOfOtherParameter + "</a>";
	}
	return output;
}


void TsVParameter::OtherParameterMissing(const char* nameOfOtherParameter) {
	// If value was being evaluated just to write out to a parameter dump, no need to quit."
	if (fPm->NowDoingParameterDump()) {
		fPm->UnableToCalculateForDump();
		return;
	}

	G4cerr << "Topas is exiting due to a serious error triggered by:" << G4endl;
	G4cerr << fPm->GetLastDirectAction() << " for parameter: " << fPm->GetLastDirectParameterName() << G4endl;
	G4cerr << "Parameter name was: " << fName << " from parameter file: " << fPf->GetFileName() << G4endl;

	if (strcmp(nameOfOtherParameter, " "))
		G4cerr << "has nothing or an unknown parameter name on right side of equals sign." << G4endl;
	else
		G4cerr << "uses an unknown other parameter: " << nameOfOtherParameter << G4endl;
	fPm->AbortSession(1);
}


void TsVParameter::WrongType(const char* type) {
	// If value was being evaluated just to write out to a parameter dump, no need to quit."
	if (fPm->NowDoingParameterDump()) {
		fPm->UnableToCalculateForDump();
		return;
	}

	G4cerr << "Topas is exiting due to a serious error triggered by:" << G4endl;
	G4cerr << fPm->GetLastDirectAction() << " for parameter: " << fPm->GetLastDirectParameterName() << G4endl;
	G4cerr << "Attempt to get " << type << " value from a parameter that does not have appropriate type." << G4endl;
	G4cerr << "Parameter name was: " << fName << " from parameter file: " << fPf->GetFileName() << G4endl;
	fPm->AbortSession(1);
}


void TsVParameter::WrongUnitCategory() {
	// If value was being evaluated just to write out to a parameter dump, no need to quit."
	if (fPm->NowDoingParameterDump()) {
		fPm->UnableToCalculateForDump();
		return;
	}

	G4cerr << "Topas is exiting due to a serious error triggered by:" << G4endl;
	G4cerr << fPm->GetLastDirectAction() << " for parameter: " << fPm->GetLastDirectParameterName() << G4endl;
	G4cerr << "Unit category does not match unit of parent file." << G4endl;
	G4cerr << "Parameter name was: " << fName << " from parameter file: " << fPf->GetFileName() << G4endl;
	fPm->AbortSession(1);
}


void TsVParameter::MissingUnit() {
	// If value was being evaluated just to write out to a parameter dump, no need to quit."
	if (fPm->NowDoingParameterDump()) {
		fPm->UnableToCalculateForDump();
		return;
	}

	G4cerr << "Topas is exiting due to a serious error triggered by:" << G4endl;
	G4cerr << fPm->GetLastDirectAction() << " for parameter: " << fPm->GetLastDirectParameterName() << G4endl;
	G4cerr << "Unable to find a unit for dimensioned double parameter." << G4endl;
	G4cerr << "Parameter name was: " << fName << " from parameter file: " << fPf->GetFileName() << G4endl;
	fPm->AbortSession(1);
}


void TsVParameter::UnrecognizedUnit(const G4String& unit) {
	// If value was being evaluated just to write out to a parameter dump, no need to quit."
	if (fPm->NowDoingParameterDump()) {
		fPm->UnableToCalculateForDump();
		return;
	}

	G4cerr << "Topas is exiting due to a serious error triggered by:" << G4endl;
	G4cerr << fPm->GetLastDirectAction() << " for parameter: " << fPm->GetLastDirectParameterName() << G4endl;
	G4cerr << "Unrecognized unit: " << unit << G4endl;
	G4cerr << "Parameter name was: " << fName << " from parameter file: " << fPf->GetFileName() << G4endl;
	fPm->AbortSession(1);
}


void TsVParameter::VectorCountTooLow() {
	// If value was being evaluated just to write out to a parameter dump, no need to quit."
	if (fPm->NowDoingParameterDump()) {
		fPm->UnableToCalculateForDump();
		return;
	}

	G4cerr << "Topas is exiting due to a serious error triggered by:" << G4endl;
	G4cerr << fPm->GetLastDirectAction() << " for parameter: " << fPm->GetLastDirectParameterName() << G4endl;
	G4cerr << "The first token in a vector parameter was less than 1." << G4endl;
	G4cerr << "Parameter name was: " << fName << " from parameter file: " << fPf->GetFileName() << G4endl;
	fPm->AbortSession(1);
}


void TsVParameter::WrongSyntax() {
	// If value was being evaluated just to write out to a parameter dump, no need to quit."
	if (fPm->NowDoingParameterDump()) {
		fPm->UnableToCalculateForDump();
		return;
	}

	G4cerr << "" << G4endl;
	G4cerr << "Topas is exiting due to a serious error triggered by:" << G4endl;
	G4cerr << fPm->GetLastDirectAction() << " for parameter: " << fPm->GetLastDirectParameterName() << G4endl;
	G4cerr << "Parameter name: " << fTempParameter->GetType() << ":" << fName << G4endl;
	G4cerr << "has unsupported value: " << fTempParameter->GetValue() << G4endl;

	G4String type = GetType();
	if (type=="d") {
		G4cerr << "Double parameters may have values of the following forms (or one of these plus # comment)" << G4endl;
		G4cerr << "   number unit" << G4endl;
		G4cerr << "   number unit + name_of_double_parameter" << G4endl;
		G4cerr << "   number unit - name_of_double_parameter" << G4endl;
		G4cerr << "   number unit * name_of_unitless_or_integer_parameter" << G4endl;
		G4cerr << "   number * name_of_double_parameter unit" << G4endl;
		G4cerr << "   name_of_double_parameter unit" << G4endl;
		G4cerr << "   name_of_double_parameter unit * number" << G4endl;
		G4cerr << "   name_of_double_parameter unit * name_of_unitless_or_integer_parameter" << G4endl;
		G4cerr << "   name_of_unitless_or_integer_parameter * number unit" << G4endl;
		G4cerr << "   name_of_double_parameter + number unit" << G4endl;
		G4cerr << "   name_of_double_parameter - number unit" << G4endl;
		G4cerr << "   name_of_double_parameter + name_of_double_parameter unit" << G4endl;
		G4cerr << "   name_of_double_parameter - name_of_double_parameter unit" << G4endl;
	} else if (type=="u") {
		G4cerr << "Unitless parameters may have values of the following forms (or one of these plus # comment)" << G4endl;
		G4cerr << "   number" << G4endl;
		G4cerr << "   number + name_of_unitless_or_integer_parameter" << G4endl;
		G4cerr << "   number - name_of_unitless_or_integer_parameter" << G4endl;
		G4cerr << "   number * name_of_unitless_or_integer_parameter" << G4endl;
		G4cerr << "   name_of_unitless_or_integer_parameter" << G4endl;
		G4cerr << "   name_of_unitless_or_integer_parameter + number" << G4endl;
		G4cerr << "   name_of_unitless_or_integer_parameter - number" << G4endl;
		G4cerr << "   name_of_unitless_or_integer_parameter * number" << G4endl;
		G4cerr << "   name_of_unitless_or_integer_parameter + name_of_unitless_or_integer_parameter" << G4endl;
		G4cerr << "   name_of_unitless_or_integer_parameter - name_of_unitless_or_integer_parameter" << G4endl;
		G4cerr << "   name_of_unitless_or_integer_parameter * name_of_unitless_or_integer_parameter" << G4endl;
	} else if (type=="i") {
		G4cerr << "Integer parameters may have values of the following forms (or one of these plus # comment)" << G4endl;
		G4cerr << "   integer" << G4endl;
		G4cerr << "   integer + name_of_integer_parameter" << G4endl;
		G4cerr << "   integer - name_of_integer_parameter" << G4endl;
		G4cerr << "   integer * name_of_integer_parameter" << G4endl;
		G4cerr << "   integer * name_of_unitless_parameter" << G4endl;
		G4cerr << "   name_of_integer_parameter" << G4endl;
		G4cerr << "   name_of_unitless_parameter" << G4endl;
		G4cerr << "   name_of_integer_parameter + integer" << G4endl;
		G4cerr << "   name_of_integer_parameter - integer" << G4endl;
		G4cerr << "   name_of_integer_parameter * integer" << G4endl;
		G4cerr << "   name_of_integer_parameter + name_of_integer_parameter" << G4endl;
		G4cerr << "   name_of_integer_parameter - name_of_integer_parameter" << G4endl;
		G4cerr << "   name_of_integer_parameter * name_of_integer_parameter" << G4endl;
		G4cerr << "   name_of_integer_parameter * name_of_unitless_parameter" << G4endl;
	} else if (type=="b") {
		G4cerr << "Boolean parameters may have values of the following forms (or one of these plus # comment)" << G4endl;
		G4cerr << "   \"True\"" << G4endl;
		G4cerr << "   \"False\"" << G4endl;
		G4cerr << "   \"T\"" << G4endl;
		G4cerr << "   \"F\"" << G4endl;
		G4cerr << "   name_of_boolean_parameter" << G4endl;
		G4cerr << "   name_of_boolean_parameter * name_of_boolean_parameter" << G4endl;
	} else if (type=="s") {
		G4cerr << "String parameters may have values of the following forms (or one of these plus # comment)" << G4endl;
		G4cerr << "   string" << G4endl;
		G4cerr << "   string + name_of_integer_or_string_parameter" << G4endl;
		G4cerr << "   name_of_integer_or_string_parameter" << G4endl;
		G4cerr << "   name_of_integer_or_string_parameter + string" << G4endl;
		G4cerr << "   name_of_integer_or_string_parameter + name_of_integer_or_string_parameter" << G4endl;
	} else if (type=="dv") {
		G4cerr << "Double vector parameters may have values of the following forms (or one of these plus # comment)" << G4endl;
		G4cerr << "   number * name_of_double_vector_parameter unit" << G4endl;
		G4cerr << "   number_of_values value1 value2 ... valueN unit" << G4endl;
		G4cerr << "   number_of_values value1 value2 ... valueN unit + name_of_double_or_double_vector_parameter" << G4endl;
		G4cerr << "   number_of_values value1 value2 ... valueN unit - name_of_double_or_double_vector_parameter" << G4endl;
		G4cerr << "   number_of_values value1 value2 ... valueN unit * name_of_unitless_or_integer_or_unitless_vector_or_integer_vector_parameter" << G4endl;
		G4cerr << "   number_of_values value1 value2 ... valueN * name_of_double_or_double_vector_parameter unit" << G4endl;
		G4cerr << "   name_of_double_vector_parameter unit" << G4endl;
		G4cerr << "   name_of_unitless_or_integer_parameter * name_of_double_vector_parameter unit" << G4endl;
	} else if (type=="uv") {
		G4cerr << "Unitless vector parameters may have values of the following forms (or one of these plus # comment)" << G4endl;
		G4cerr << "   umber * name_of_unitless_vector_parameter" << G4endl;
		G4cerr << "   number_of_values value1 value2 ... valueN" << G4endl;
		G4cerr << "   number_of_values value1 value2 ... valueN + name_of_unitless_or_integer_or_unitless_vector_or_integer_vector_parameter" << G4endl;
		G4cerr << "   number_of_values value1 value2 ... valueN - name_of_unitless_or_integer_or_unitless_vector_or_integer_vector_parameter" << G4endl;
		G4cerr << "   number_of_values value1 value2 ... valueN * name_of_unitless_or_integer_or_unitless_vector_or_integer_vector_parameter" << G4endl;
		G4cerr << "   name_of_unitless_vector_parameter" << G4endl;
		G4cerr << "   name_of_unitless_or_integer_parameter * name_of_unitless_vector_parameter" << G4endl;
	} else if (type=="iv") {
		G4cerr << "Integer vector parameters may have values of the following forms (or one of these plus # comment)" << G4endl;
		G4cerr << "   integer * name_of_integer_vector_parameter" << G4endl;
		G4cerr << "   number_of_values value1 value2 ... valueN" << G4endl;
		G4cerr << "   number_of_values value1 value2 ... valueN + name_of_integer_or_integer_vector_parameter" << G4endl;
		G4cerr << "   number_of_values value1 value2 ... valueN - name_of_integer_or_integer_vector_parameter" << G4endl;
		G4cerr << "   number_of_values value1 value2 ... valueN * name_of_integer_or_integer_vector_parameter" << G4endl;
		G4cerr << "   name_of_integer_vector_parameter" << G4endl;
		G4cerr << "   name_of_integer_parameter * name_of_integer_vector_parameter" << G4endl;
	} else if (type=="bv") {
		G4cerr << "Boolean vector parameters may have values of the following forms (or one of these plus # comment)" << G4endl;
		G4cerr << "   number_of_values value1 value2 ... valueN" << G4endl;
		G4cerr << "   name_of_boolean_vector_parameter" << G4endl;
	} else if (type=="sv") {
		G4cerr << "String vector parameters may have values of the following forms (or one of these plus # comment)" << G4endl;
		G4cerr << "   number_of_values value1 value2 ... valueN" << G4endl;
		G4cerr << "   number_of_values value1 value2 ... valueN + name_of_integer_or_string_or_integer_vector_or_string_vector_parameter" << G4endl;
		G4cerr << "   name_of_string_vector_parameter" << G4endl;
	}

	fPm->AbortSession(1);
}
