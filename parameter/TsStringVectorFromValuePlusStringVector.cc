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

#include "TsStringVectorFromValuePlusStringVector.hh"

TsStringVectorFromValuePlusStringVector::TsStringVectorFromValuePlusStringVector(TsParameterManager* pM, TsParameterFile* pf, TsTempParameter* tempParameter,
										   G4int nValues, G4String* values, const G4String& nameOfOtherParameter)
:TsVParameter(pM, pf, tempParameter)
{
	QuitIfMustBeAbsolute();

	fLength = nValues;
	fValues = new G4String [nValues];
	fValuesIfParameterNames = new G4String[nValues];
	ConvertToStringVector(nValues, values,  fValues, fValuesIfParameterNames);

	fNameOfOtherParameter = ParseNameOfOtherParameter(nameOfOtherParameter, "sv");
}


TsStringVectorFromValuePlusStringVector::~TsStringVectorFromValuePlusStringVector()
{
	delete fValues;
	delete fValuesIfParameterNames;
}


void TsStringVectorFromValuePlusStringVector::CheckRelativeParameterConsistency()
{
	CheckVectorLengthAgreement(fNameOfOtherParameter);
}


G4String TsStringVectorFromValuePlusStringVector::GetType()
{
	G4String type = "sv";
	return type;
}


G4int TsStringVectorFromValuePlusStringVector::GetVectorLength()
{
	return fLength;
}


G4String* TsStringVectorFromValuePlusStringVector::GetStringVector()
{
	fUsed = true;
	G4String* values = new G4String[fLength];
	for (G4int iValue=0; iValue<fLength; iValue++)
		if (fValuesIfParameterNames[iValue]=="")
			values[iValue] = fValues[iValue] +
			EvaluateStringVector(fNameOfOtherParameter)[iValue];
		else
			values[iValue] = EvaluateStringParameter(fValuesIfParameterNames[iValue]) +
			EvaluateStringVector(fNameOfOtherParameter)[iValue];
	return values;
}


G4String TsStringVectorFromValuePlusStringVector::GetHTMLValue()
{
	G4String output;
	output += G4UIcommand::ConvertToString(fLength);
	for (G4int iValue=0; iValue<fLength; iValue++)
		if (fValuesIfParameterNames[iValue]=="")
			output+= " " + fValues[iValue];
		else
			output+= " " + EvaluateAsHTML(fValuesIfParameterNames[iValue]);
	output += " + ";
	output += EvaluateAsHTML(fNameOfOtherParameter);
	return output;
}
