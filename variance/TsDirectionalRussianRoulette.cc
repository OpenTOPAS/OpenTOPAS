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

#include "TsDirectionalRussianRoulette.hh"
#include "TsGeometryManager.hh"
#include "TsVGeometryComponent.hh"
#include "TsParameterManager.hh"

#include "G4RegionStore.hh"
#include "G4Region.hh"
#include "G4VProcess.hh"
#include "G4ClassificationOfNewTrack.hh"

#include "globals.hh"
#include <vector>

TsDirectionalRussianRoulette::TsDirectionalRussianRoulette(G4String name, TsParameterManager* pM, TsGeometryManager* gM)
	: TsVBiasingProcess(name, pM, gM), fPm(pM), fGm(gM), fSourceComp(nullptr), fName(name)
{
}


TsDirectionalRussianRoulette::~TsDirectionalRussianRoulette()
{
}

void TsDirectionalRussianRoulette::ResolveParameters() {
	fRadius.clear();
	fLimit.clear();
	fProcesses.clear();
	fRegion.clear();
	
	std::vector<G4String>* regionNames = new std::vector<G4String>;
	fPm->GetParameterNamesBracketedBy("Vr/" + fName, "DirectionalSplitLimits", regionNames);
	if (  regionNames->size() > 0 ) {
		if ( !fPm->ParameterExists(GetFullParmName("ReferenceComponent") ))
			Quit(GetFullParmName("ReferenceComponent"),"Does not exists");
		
		G4String compName = fPm->GetStringParameter(GetFullParmName("ReferenceComponent"));
		fSourceComp = fGm->GetComponent( compName );
		if ( !fSourceComp )
			Quit(GetFullParmName("ReferenceComponent"),"Refers to an inexistent component");
		
		for ( int i = 0; i < int(regionNames->size()); i++ ) {
			G4String parmName = (*regionNames)[i];
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(parmName);
#else
			parmName.toLower();
#endif
 			parmName = parmName.substr(0, parmName.size()-22 );
			 
			G4String* processNames = fPm->GetStringVector(parmName + "processesNamed");
			G4double* limit = fPm->GetDoubleVector(parmName + "DirectionalSplitLimits", "Length");
			G4double* radius = fPm->GetDoubleVector(parmName + "DirectionalSplitRadius", "Length");
			G4int lengthOfProcessNames = fPm->GetVectorLength(parmName + "processesNamed");
			G4int lengthOfLimit = fPm->GetVectorLength(parmName + "DirectionalSplitLimits");
			G4int lengthOfRadius = fPm->GetVectorLength(parmName + "DirectionalSplitRadius");
	
			if ( lengthOfLimit != lengthOfProcessNames )
				Quit(parmName + "DirectionalSplitLimits", "The number of elements must matches with " +
					 parmName + "processesNamed");
	
			if ( lengthOfRadius != lengthOfProcessNames )
				Quit(parmName + "DirectionalSplitRadius", "The number of elements must matches with " +
					 parmName + "processesNamed");
	
			G4int prefixLength = 14 + fName.size();
			G4String regionName = parmName.substr( prefixLength, parmName.size());
			regionName = regionName.substr(0, regionName.size()-1);
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(regionName);
#else
			regionName.toLower();
#endif
			G4Region* aRegion = G4RegionStore::GetInstance()->FindOrCreateRegion(regionName);
			fRegion.push_back(aRegion);
	
			G4int length = fPm->GetVectorLength(parmName + "DirectionalSplitLimits");
			
			for ( int j = 0; j < length; j++ ) {
				fRadius[aRegion].push_back(radius[j]);
				fLimit[aRegion].push_back(limit[j]);
				fProcesses[aRegion].push_back(processNames[i]);
			}
		}
	}
}


void TsDirectionalRussianRoulette::Initialize() {
	Clear();
	ResolveParameters();
}


G4ClassificationOfNewTrack TsDirectionalRussianRoulette::Apply(const G4Track* aTrack) {
	for ( int i = 0; i < int(fRegion.size()); i++ ) {
		if ( aTrack->GetVolume()->GetLogicalVolume()->GetRegion() == fRegion[i] ) {
			for ( int j = 0; j < int(fLimit[fRegion[i]].size()); j++ ) {
				G4bool apply = false;
	
				for ( size_t k = 0; k < fProcesses[fRegion[i]].size() && !apply; k++ ) {
					if ( aTrack->GetCreatorProcess()->GetProcessName() == fProcesses[fRegion[i]][k] )
						apply = true;
				}
	
				if ( apply ) {
					G4RotationMatrix* dfRot = new G4RotationMatrix(fSourceComp->GetRotRelToWorld()->inverse());
					fAxisVector = G4ThreeVector(dfRot->zx(), -dfRot->zy(), dfRot->zz());
					G4Point3D* dfTrans = fSourceComp->GetTransRelToWorld();
					fTransVector = G4ThreeVector(dfTrans->x(), dfTrans->y(), dfTrans->z());
					delete dfRot;
	
					if ( !AcceptTrackDirection(aTrack, fRadius[fRegion[i]][j], fLimit[fRegion[i]][j]) ) {
						return fKill;
					}
				}
			}
		}
	}
	
	return fUrgent;
}
		
		
G4bool TsDirectionalRussianRoulette::AcceptTrackDirection(const G4Track* aTrack, G4double radius, G4double limit) {
	G4ThreeVector trackPosition = aTrack->GetPosition();
	G4ThreeVector trackMomentum = aTrack->GetMomentumDirection();
	
	G4ThreeVector roiPosition = limit*fAxisVector + fTransVector;
	G4ThreeVector normalRoiPosition = -1.0 * G4ThreeVector(roiPosition.unit());
	
	G4double sValue = -normalRoiPosition.dot( (trackPosition - roiPosition) )/ normalRoiPosition.dot(trackMomentum);
	
	if ( sValue < 0 )
		return false;
	
	G4ThreeVector ip = trackPosition + sValue * trackMomentum - roiPosition;
	
	if (  radius >= sqrt(ip.x()*ip.x() + ip.y()*ip.y()) )
		return true;
	
	return false;
}

		
void TsDirectionalRussianRoulette::Clear() {
	fRadius.clear();
	fLimit.clear();
	fProcesses.clear();
	fRegion.clear();
	return;
}

