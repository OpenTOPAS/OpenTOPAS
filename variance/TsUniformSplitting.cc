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

#include "TsUniformSplitting.hh"
#include "TsGeometryManager.hh"
#include "TsParameterManager.hh"

#include "G4UImanager.hh"
#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include "globals.hh"
#include <vector>

TsUniformSplitting::TsUniformSplitting(G4String name, TsParameterManager* pM, TsGeometryManager* gM)
	: TsVBiasingProcess(name, pM, gM), fPm(pM), fName(name), fUseDirectionalSplitting(false),
	  fTransX(0), fTransY(0), fTransZ(0), fRMax(0)
{
}

TsUniformSplitting::~TsUniformSplitting()
{;}


void TsUniformSplitting::ResolveParameters() {
	fType = fPm->GetStringParameter(GetFullParmName("Type"));
	G4String prefix = GetFullParmName("ForRegion");
	G4String subfix = "processesNamed";
	std::vector<G4String>* regionNames = new std::vector<G4String>;
	fPm->GetParameterNamesBracketedBy(prefix, subfix, regionNames);
	G4int numberOfRegions = regionNames->size();
	G4bool found;
	if ( numberOfRegions > 0 ) {
		for ( int i = 0; i < numberOfRegions; i++ ) {
			G4String aRegionName = (*regionNames)[i];
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(aRegionName);
#else
			aRegionName.toLower();
#endif
			found = false;
			for ( int j = i + 1; j < numberOfRegions; j++ ) {
				G4String tempRegName = (*regionNames)[j];
#if GEANT4_VERSION_MAJOR >= 11
				G4StrUtil::to_lower(tempRegName);
#else
				tempRegName.toLower();
#endif
				if ( aRegionName == tempRegName )
					found = true;
			}
			
			if (!found) {
				aRegionName = aRegionName.substr(0,aRegionName.length()-subfix.length()-1);
				aRegionName = aRegionName.substr(prefix.length()+1);
				fRegionName.push_back(aRegionName);
				
				G4String parmName = "forregion/"+aRegionName+"/";
				G4int procNameSize = fPm->GetVectorLength(GetFullParmName(parmName+"ProcessesNamed"));
				G4int splSize = fPm->GetVectorLength(GetFullParmName(parmName+"SplitNumber"));
				G4int maxEnergiesSize = fPm->GetVectorLength(GetFullParmName(parmName+"MaximumEnergies"));
				
				if ( procNameSize != splSize ) {
					Quit(GetFullParmName(parmName+"SplitNumber"),
						 "size does not match with size of parameter named: " +
						 GetFullParmName(parmName+"ProcessesNamed"));
				} else if ( procNameSize != maxEnergiesSize ) {
					Quit(GetFullParmName(parmName+"SplitNumber"),
						 "size does not match with size of parameter named: " +
						 GetFullParmName(parmName+"MaximumEnergies"));
				}
				
				G4String* procName = fPm->GetStringVector(GetFullParmName(parmName + "ProcessesNamed"));
				G4double* splitNumber = fPm->GetUnitlessVector(GetFullParmName(parmName + "SplitNumber"));
				G4double* maxEnergies = fPm->GetDoubleVector(GetFullParmName(parmName + "MaximumEnergies"), "Energy");
				for ( int j = 0; j < procNameSize; j++ ) {
					fProcesses.push_back(procName[j]);
					fSplitNumber.push_back(splitNumber[j]);
					fLowerEnergyLimitForRussianRoulette.push_back(maxEnergies[j]);
				}
			}
		}
	}
	
	if ( fPm->ParameterExists(GetFullParmName("UseDirectionalSplitting")) ) {
		fUseDirectionalSplitting = fPm->GetBooleanParameter(GetFullParmName("UseDirectionalSplitting"));
		fTransX = fPm->GetDoubleParameter(GetFullParmName("TransX"), "Length");
		fTransY = fPm->GetDoubleParameter(GetFullParmName("TransY"), "Length");
		fTransZ = fPm->GetDoubleParameter(GetFullParmName("TransZ"), "Length");
		fRMax = fPm->GetDoubleParameter(GetFullParmName("RMax"), "Length");
	}
}


void TsUniformSplitting::Initialize() {
	Clear();
	ResolveParameters();
	SetUniformSplitting();
}


void TsUniformSplitting::Clear() {
	fProcesses.clear();
	fSplitNumber.clear();
	fLowerEnergyLimitForRussianRoulette.clear();
	fRegionName.clear();
}


void TsUniformSplitting::SetUniformSplitting() {
	G4String command = "None";
	for ( int j = 0; j < (G4int)fProcesses.size(); j++ ) {
		for ( int i = 0; i < int(fRegionName.size()); i++ ) {
			command ="/process/em/setSecBiasing " + fProcesses[j] + " " + fRegionName[i] + " " +
			G4UIcommand::ConvertToString(fSplitNumber[j]) + " " +
			G4UIcommand::ConvertToString(fLowerEnergyLimitForRussianRoulette[j], "MeV");
			G4UImanager::GetUIpointer()->ApplyCommand(command);
			G4cout << " commamd " << command << G4endl;
		}
	}

	if ( fUseDirectionalSplitting ) {
		G4cout << "-Using Geant4 directional splitting" << G4endl;
		command = "/process/em/setDirectionalSplitting true";
		G4UImanager::GetUIpointer()->ApplyCommand(command);

		command = "/process/em/setDirectionalSplittingTarget " +
		G4UIcommand::ConvertToString(G4ThreeVector(fTransX, fTransY, fTransZ), "cm");
		G4UImanager::GetUIpointer()->ApplyCommand(command);
		
		command = "/process/em/setDirectionalSplittingRadius " +
		G4UIcommand::ConvertToString(fRMax, "cm");
		G4UImanager::GetUIpointer()->ApplyCommand(command);
	} else {
		command = "/process/em/setDirectionalSplitting false";
		G4UImanager::GetUIpointer()->ApplyCommand(command);
	}
}
