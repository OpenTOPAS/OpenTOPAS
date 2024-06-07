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
#include "TsParameterFile.hh"
#include "TsDoubleFromTimeFeatureStep.hh"

TsDoubleFromTimeFeatureStep::TsDoubleFromTimeFeatureStep(TsParameterManager* pM, TsParameterFile* pf, TsTempParameter* tempParameter,
										const G4String& nameOfBaseParameter, G4int nValues, G4String* values ,const G4String& unit)
	:TsVParameter(pM, pf, tempParameter), fUnit(unit), fNameOfBaseParameter(nameOfBaseParameter), fLatestUpdatedTime(-1)
{
	QuitIfMustBeAbsolute();

	CheckUnit(unit);

	fValues = new G4double [nValues];
	fValuesIfParameterNames = new G4String[nValues];
	ConvertToDoubleVector(nValues, values, fValues, fValuesIfParameterNames);
	G4String initvalue = tempParameter->GetValue();
	fCurrentValue = (G4UIcommand::ConvertToDouble(initvalue)) * fPm->GetUnitValue(fUnit);
	fRange = 0;
	fTimes = new std::map<G4double, G4int>;
}


TsDoubleFromTimeFeatureStep::~TsDoubleFromTimeFeatureStep()
{
}


G4String TsDoubleFromTimeFeatureStep::GetType()
{
	G4String type = "d";
	return type;
}


G4String TsDoubleFromTimeFeatureStep::GetUnit() {
	return fUnit;
}

G4double TsDoubleFromTimeFeatureStep::GetDoubleValue()
{
	fUsed = true;
	G4double t = fPm->GetCurrentSequenceTime();
	while(t >= fRange ) { t -= fRange; }
	if (t == fLatestUpdatedTime) { return fCurrentValue;}
	std::map<G4double, G4int>::const_iterator pos = fTimes->upper_bound(t);
	G4int idx = pos->second;
	if ( fValuesIfParameterNames[idx] =="" ) {
		return fValues[idx]*fPm->GetUnitValue(fUnit);
	} else {
		return EvaluateDoubleParameter(fValuesIfParameterNames[idx]);
	}
}

G4String TsDoubleFromTimeFeatureStep::GetHTMLValue() {
	G4String output  = G4UIcommand::ConvertToString( GetDoubleValue() );
	output += " ";
	output += fUnit;
	return output;
}

void TsDoubleFromTimeFeatureStep::InitializeTimeFeatureValue() {
	G4int ntimes   = fPm->GetVectorLength(fNameOfBaseParameter+"/Times" );
	G4int nbvalues = fPm->GetVectorLength(fNameOfBaseParameter+"/Values");
	if (ntimes != nbvalues ) {
		G4cerr<<"ERROR: # of Values must be same # of Times :"<< GetName() <<G4endl;
		fPm->AbortSession(1);
	}

	G4double* times = fPm->GetDoubleVector(fNameOfBaseParameter+"/Times", "Time");
	fRange = times[ntimes-1];
	for (int i=0; i < ntimes; ++i) {
		fTimes->insert(std::make_pair(times[i],i));
	}
}

G4bool TsDoubleFromTimeFeatureStep::ValueHasChanged() {
	G4double previousValue = fCurrentValue;
	G4double newValue      = GetDoubleValue();
	if ( newValue != previousValue) {
		fCurrentValue = newValue;
		fLatestUpdatedTime = fPm->GetCurrentSequenceTime();
		return true;
	} else {
		return false;
	}
}
