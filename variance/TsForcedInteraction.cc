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

#include "TsForcedInteraction.hh"
#include "TsGeometryManager.hh"
#include "TsParameterManager.hh"

#include "G4UImanager.hh"
#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"

#include "globals.hh"
#include <vector>

TsForcedInteraction::TsForcedInteraction(G4String name, TsParameterManager* pM, TsGeometryManager* gM)
: TsVBiasingProcess(name, pM, gM), fPm(pM), fName(name)
{
	ResolveParameters();
}


TsForcedInteraction::~TsForcedInteraction()
{;}


void TsForcedInteraction::ResolveParameters() {
	G4String prefix = GetFullParmName("ForRegion");
	G4String subfix = "processesNamed";
	std::vector<G4String>* regionNames = new std::vector<G4String>;
	G4bool found;

	fPm->GetParameterNamesBracketedBy(prefix, subfix, regionNames);
	G4int numberOfRegions = regionNames->size();
	if ( numberOfRegions > 0 ) {
		for ( int i = 0; i < numberOfRegions; i++ ) {
			G4String aRegionName = (*regionNames)[i];
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(aRegionName);
#else
			aRegionName.toLower();
#endif
			if ( aRegionName == "defaultregionfortheworld" )
				aRegionName = "DefaultRegionForTheWorld";
			
			found = false;
			for ( int j = i + 1; j < numberOfRegions; j++ ) {
				G4String tempRegName = (*regionNames)[j];
#if GEANT4_VERSION_MAJOR >= 11
				G4StrUtil::to_lower(tempRegName);
#else
				tempRegName.toLower();
#endif
				if ( tempRegName == "defaultregionfortheworld")
					tempRegName = "DefaultRegionForTheWorld";
				
				if ( aRegionName == tempRegName )
					found = true;
			}
			
			if (!found) {
				aRegionName = aRegionName.substr(0,aRegionName.length()-subfix.length()-1);
				aRegionName = aRegionName.substr(prefix.length()+1);
				fRegionName.push_back(aRegionName);
				
				G4String parmName = "forregion/"+aRegionName+"/";
				G4int procNameSize = fPm->GetVectorLength(GetFullParmName(parmName+"ProcessesNamed"));
				
				if ( procNameSize != fPm->GetVectorLength(GetFullParmName(parmName+"ForcedDistances")) ) {
					Quit(GetFullParmName(parmName+"ProcessesNamed"),
						 "size does not match with size of parameter named: "
						 + GetFullParmName(parmName + "ForcedDistances"));
				}
			}
		}
	}
}


void TsForcedInteraction::Initialize() {
	SetForcedInteraction();
	return;
}


void TsForcedInteraction::SetForcedInteraction() {
	for ( int i = 0; i < int(fRegionName.size()); i++ ) {
		G4String parmName = "ForRegion/"+fRegionName[i]+"/";
		G4String* procName = fPm->GetStringVector(GetFullParmName(parmName+"ProcessesNamed"));
		G4int procNameSize = fPm->GetVectorLength(GetFullParmName(parmName+"ProcessesNamed"));
		G4double* forcedDistances = fPm->GetDoubleVector(GetFullParmName(parmName+"ForcedDistances"), "Length");
		G4bool correctByWeight = false;
		if ( fPm->ParameterExists(GetFullParmName(parmName+"CorrectByWeight")))
			correctByWeight = fPm->GetBooleanParameter(GetFullParmName(parmName+"CorrectByWeight"));
		
		if ( fRegionName[i] == "defaultregionfortheworld" )
			fRegionName[i] = "DefaultRegionForTheWorld";
		
		G4String sCorrectByWeight = !correctByWeight ? " false " : " true ";
		for ( int j = 0; j < procNameSize; j++ ) {
			G4String command ="/process/em/setForcedInteraction " + procName[j] + " " + fRegionName[i] + " " +
			G4UIcommand::ConvertToString(forcedDistances[j]/mm, "mm") + sCorrectByWeight;
			G4UImanager::GetUIpointer()->ApplyCommand(command);
			G4cout << "-- Forced interaction will be corrected by weight: "  + sCorrectByWeight << G4endl;
		}
	}
}


void TsForcedInteraction::Clear() {
	return;
}
