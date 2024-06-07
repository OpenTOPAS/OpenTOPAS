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

#include "TsTempParameter.hh"
#include "TsDoubleFromTimeFeature.hh"
#include "G4Tokenizer.hh"
#include "TsTFLinear.hh"
#include "TsTFSqrt.hh"
#include "TsTFExponent.hh"

TsDoubleFromTimeFeature::TsDoubleFromTimeFeature(TsParameterManager* pM, TsParameterFile* pf, TsTempParameter* tempParameter,
										const G4String& nameOfBaseParameter, const G4String& unit)
	:TsVParameter(pM, pf, tempParameter), fUnit(unit), fNameOfBaseParameter(nameOfBaseParameter), fLatestUpdatedTime(-1)
{
	QuitIfMustBeAbsolute();

	CheckUnit(unit);

	G4String initvalue = tempParameter->GetValue();
	fCurrentValue = (G4UIcommand::ConvertToDouble(initvalue)) * fPm->GetUnitValue(fUnit);
	fRange = DBL_MAX;
}


TsDoubleFromTimeFeature::~TsDoubleFromTimeFeature()
{
}


G4String TsDoubleFromTimeFeature::GetType()
{
	G4String type = "d";
	return type;
}


G4String TsDoubleFromTimeFeature::GetUnit() {
	return fUnit;
}

G4double TsDoubleFromTimeFeature::GetDoubleValue()
{
	fUsed = true;
	G4double t = fPm->GetCurrentSequenceTime();
	while(t >= fRange ) { t -= fRange; }
	if (t == fLatestUpdatedTime) { return fCurrentValue;}
	return ((*fFunction)(t));
}


G4String TsDoubleFromTimeFeature::GetHTMLValue()
{
	G4String output  = G4UIcommand::ConvertToString( GetDoubleValue() );
	output += " ";
	output += fUnit;
	return output;
}

void TsDoubleFromTimeFeature::InitializeTimeFeatureValue() {
	G4String funcValue = fPm->GetStringParameter(G4String(fNameOfBaseParameter+"/Function"));
	G4Tokenizer next(funcValue);
	G4String funcname = next(); 
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(funcname);
#else
	funcname.toLower();
#endif
	G4String category = fPm->GetUnitCategory(fUnit) + "/Time";
	G4double rate       = fPm->GetDoubleParameter(G4String(fNameOfBaseParameter+"/Rate"), category);
	G4double startvalue = fPm->GetDoubleParameter(G4String(fNameOfBaseParameter+"/StartValue"), fPm->GetUnitCategory(fUnit));

	if (fPm->ParameterExists(G4String(fNameOfBaseParameter+"/RepetitionInterval")))
		fRange = fPm->GetDoubleParameter(G4String(fNameOfBaseParameter+"/RepetitionInterval"), "Time");

	if ( funcname == "linear") {
		fFunction = new TsTFLinear(rate, startvalue);
	} else if (funcname == "sqrt") {
		//sqrt supports both; double and unitless
		fFunction = new TsTFSqrt(rate, startvalue);
	} else if (funcname == "exponent") {
		//exponent supports both; double and unitless
		fFunction = new TsTFExponent(rate, startvalue);
	} else if (funcname == "sine" || funcname == "cosine") {
		G4cerr<<"[TimeFeature for double] Requires a unit of " << funcname <<" TimeFunction"<<G4endl;
		fPm->AbortSession(1);
	} else {
		G4cerr<<"[TimeFeature for double] Can't find TimeFunction for given " << funcname <<" TimeFunction"<<G4endl;
		fPm->AbortSession(1);
	}
}

G4bool TsDoubleFromTimeFeature::ValueHasChanged() {
	G4double previousValue = fCurrentValue;
	G4double newValue = GetDoubleValue();
	if ( newValue != previousValue) {
		fCurrentValue = newValue;
		fLatestUpdatedTime = fPm->GetCurrentSequenceTime();
		return true;
	} else {
		return false;
	}
}
