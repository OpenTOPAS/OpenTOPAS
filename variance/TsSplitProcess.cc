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

#include "TsSplitProcess.hh"
#include "TsParameterManager.hh"
#include "TsGeometryManager.hh"
#include "TsVGeometryComponent.hh"
#include "TsIStore.hh"

#include "G4GeometryCell.hh"
#include "G4PathFinder.hh"
#include "G4TransportationManager.hh"
#include "G4FieldTrackUpdator.hh"
#include "G4GeometryTolerance.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

TsSplitProcess::TsSplitProcess(TsParameterManager* pm, TsGeometryManager* gm, const G4String& aName, TsIStore* aistore)
:G4VProcess(aName), fPm(pm), fGm(gm), fAIStore(aistore), fParticleChange(new G4ParticleChange),
fGhostNavigator(0), fNavigatorID(-1), fFieldTrack('0'), fName(aName)
{
	ResolveParameters();
	G4VProcess::pParticleChange = fParticleChange;
	fGhostStep = new G4Step();
	fGhostPreStepPoint = fGhostStep->GetPreStepPoint();
	fGhostPostStepPoint = fGhostStep->GetPostStepPoint();
	fTransportationManager = G4TransportationManager::GetTransportationManager();
	fPathFinder = G4PathFinder::GetInstance();
	fKCarTolerance = G4GeometryTolerance::GetInstance()->GetSurfaceTolerance();
}


TsSplitProcess::~TsSplitProcess() {
	delete fParticleChange;
}


void TsSplitProcess::ResolveParameters(){
	if (fPm->UseVarianceReduction()) {
		G4String type = fPm->GetStringParameter(GetFullParmName("Type"));
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(type);
#else
		type.toLower();
#endif
		
		if ( type == "geometricalparticlesplit")  {
			G4String componentName = fPm->GetStringParameter(GetFullParmName("Component"));
			fMotherComponent = fGm->GetComponent( componentName );
			
			fRadius = fPm->GetDoubleParameter(GetFullParmName("RussianRoulette/ROIRadius"), "Length");
			
			fLimit = fPm->GetDoubleParameter(GetFullParmName("RussianRoulette/ROITrans"), "Length");
			
			fFactor = fPm->GetIntegerVector(GetFullParmName("SplitNumber"))[0];
			
			G4String axis = fPm->GetStringParameter(GetFullParmName("SplitAxis"));
			
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(axis);
#else
			axis.toLower();
#endif
			if ( axis == "zaxis" )
				fAxisVector = G4ThreeVector(0, 0, 1);
			else if ( axis == "yaxis" )
				fAxisVector = G4ThreeVector(0, 1, 0);
			else if ( axis == "xaxis" )
				fAxisVector = G4ThreeVector(1, 0, 0);
			
			fOnlySplitParticlesOfUnitaryWeight = false;
			if ( fPm->ParameterExists(GetFullParmName("OnlySplitParticlesOfUnitaryWeight")))
				fOnlySplitParticlesOfUnitaryWeight = fPm->ParameterExists(GetFullParmName("OnlySplitParticlesOfUnitaryWeight"));
			
		}
	}
}


G4String TsSplitProcess::GetFullParmName(G4String parmName){
	G4String fullName = "Vr/" + fName + "/" + parmName;
	return fullName;
}


void TsSplitProcess::SetParallelWorld(G4String parallelWorldName) {
	fGhostNavigator = fTransportationManager->GetNavigator(fTransportationManager->GetParallelWorld(parallelWorldName));
}


void TsSplitProcess::SetParallelWorld(G4VPhysicalVolume* parallelWorld) {
	fGhostNavigator = fTransportationManager->GetNavigator(parallelWorld);
}


void TsSplitProcess::StartTracking(G4Track* track) {
	fNavigatorID = fTransportationManager->ActivateNavigator(fGhostNavigator);
	fPathFinder->PrepareNewTrack(track->GetPosition(),track->GetMomentumDirection());
	G4TouchableHandle oldGhostTouchable = fPathFinder->CreateTouchableHandle(fNavigatorID);
	fGhostPreStepPoint->SetTouchableHandle(oldGhostTouchable);
	G4TouchableHandle newGhostTouchable = oldGhostTouchable;
	fGhostPostStepPoint->SetTouchableHandle(newGhostTouchable);
	fGhostSafety = -1.;
	fOnBoundary = false;
}


G4double TsSplitProcess::PostStepGetPhysicalInteractionLength(const G4Track&, G4double, G4ForceCondition* condition) {
	*condition = Forced;
	return DBL_MAX;
}


G4VParticleChange* TsSplitProcess::PostStepDoIt( const G4Track& track, const G4Step& step ) {
	fParticleChange->Initialize(track);
	
	G4TouchableHandle oldGhostTouchable = fGhostPostStepPoint->GetTouchableHandle();
	CopyStep(step);
	
	G4TouchableHandle newGhostTouchable;
	if (fOnBoundary)
		newGhostTouchable = fPathFinder->CreateTouchableHandle(fNavigatorID);
	else
		newGhostTouchable = oldGhostTouchable;
	
	fGhostPreStepPoint->SetTouchableHandle(oldGhostTouchable);
	fGhostPostStepPoint->SetTouchableHandle(newGhostTouchable);
	
	if ( (fGhostPostStepPoint->GetStepStatus() == fGeomBoundary ) && (step.GetStepLength() > fKCarTolerance)) {
		if ( fOnlySplitParticlesOfUnitaryWeight && 1 != track.GetWeight() )
			return fParticleChange;
		
		G4RotationMatrix* rotationangle = new G4RotationMatrix(fMotherComponent->GetRotRelToWorld()->inverse());
		G4Point3D* tr = fMotherComponent->GetTransRelToWorld();
		G4double rx = rotationangle->zx();
		G4double ry = rotationangle->zy();
		G4double rz = rotationangle->zz();
		fTransVector = G4ThreeVector(tr->x(), tr->y(), tr->z());
		fAxisVector1 = G4ThreeVector(rx, -ry, rz);
		
		if ( AcceptTrack(track, fRadius, fLimit) == true ) {
			G4GeometryCell prekey(*(fGhostPreStepPoint->GetPhysicalVolume()),
								  fGhostPreStepPoint->GetTouchable()->GetReplicaNumber());
			G4GeometryCell postkey(*(fGhostPostStepPoint->GetPhysicalVolume()),
								   fGhostPostStepPoint->GetTouchable()->GetReplicaNumber());
			
			// If applicable Russian roulette in current subcomponent
			if ( fAIStore->GetPropertyValue(postkey,1) == 1)
				if ( AcceptTrack(track, fRadius, fLimit) == false )
					RussianRoulette(fParticleChange, fFactor);
			
			// Otherwise split particles
			G4int nsplit = CalculateNSplit(fAIStore->GetPropertyValue(prekey, -1),
										   fAIStore->GetPropertyValue(postkey, -1));
			
			if ( nsplit < 1 ) {
				RussianRoulette(fParticleChange, fFactor);
				return fParticleChange;
			}
			
			// Is symmetric the region
			G4int option = fAIStore->GetPropertyValue(postkey,0);
			
			Split(fParticleChange, track, nsplit, option);
			
		} else {
			RussianRoulette(fParticleChange, fFactor);
		}
	}
	
	return fParticleChange;
}


G4double TsSplitProcess::AlongStepGetPhysicalInteractionLength( const G4Track& track, G4double previousStepSize,
															   G4double currentMinimumStep,
															   G4double& proposedSafety, G4GPILSelection* selection) {
	static G4FieldTrack endTrack('0');
	static ELimited eLimited;
	
	*selection = NotCandidateForSelection;
	G4double returnedStep = DBL_MAX;
	
	if (previousStepSize > 0.)
		fGhostSafety -= previousStepSize;
	
	if (fGhostSafety < 0.)
		fGhostSafety = 0.0;
	
	if (currentMinimumStep <= fGhostSafety && currentMinimumStep > 0.) {
		returnedStep = currentMinimumStep;
		fOnBoundary = false;
		proposedSafety = fGhostSafety - currentMinimumStep;
	} else {
		G4FieldTrackUpdator::Update(&fFieldTrack,&track);
		
		returnedStep = fPathFinder->ComputeStep(fFieldTrack,currentMinimumStep,fNavigatorID,
												track.GetCurrentStepNumber(),fGhostSafety,eLimited,
												endTrack,track.GetVolume());
		
		if(eLimited == kDoNot) {
			fOnBoundary = false;
			fGhostSafety = fGhostNavigator->ComputeSafety(endTrack.GetPosition());
		} else {
			fOnBoundary = true;
		}
		
		proposedSafety = fGhostSafety;
		
		if (eLimited == kUnique || eLimited == kSharedOther)
			*selection = CandidateForSelection;
		else if (eLimited == kSharedTransport)
			returnedStep *= (1.0 + 1.0e-9);
	}
	
	return returnedStep;
}


G4double TsSplitProcess::AtRestGetPhysicalInteractionLength(const G4Track&, G4ForceCondition*) {
	return -1.0;
}


G4VParticleChange* TsSplitProcess::AtRestDoIt(const G4Track&, const G4Step& ) {
	return 0;
}


G4VParticleChange* TsSplitProcess::AlongStepDoIt(const G4Track& track, const G4Step& ) {
	pParticleChange->Initialize(track);
	return pParticleChange;
}


void TsSplitProcess::CopyStep(const G4Step & step) {
	fGhostStep->SetTrack(step.GetTrack());
	fGhostStep->SetStepLength(step.GetStepLength());
	fGhostStep->SetTotalEnergyDeposit(step.GetTotalEnergyDeposit());
	fGhostStep->SetControlFlag(step.GetControlFlag());
	
	*fGhostPreStepPoint = *(step.GetPreStepPoint());
	*fGhostPostStepPoint = *(step.GetPostStepPoint());
	
	if(fOnBoundary)
		fGhostPostStepPoint->SetStepStatus(fGeomBoundary);
	else if(fGhostPostStepPoint->GetStepStatus()==fGeomBoundary)
		fGhostPostStepPoint->SetStepStatus(fPostStepDoItProc);
}


void TsSplitProcess::Split(G4ParticleChange* particleChange, const G4Track& track, G4int n, G4int option) {
	G4int i(0);
	G4double newWeight = track.GetWeight()/n;
	particleChange->ProposeWeight(newWeight);
	particleChange->SetNumberOfSecondaries(n-1);
	
	G4ThreeVector InitPos = track.GetPosition();
	G4ThreeVector InitDir = track.GetMomentumDirection();
	
	for ( i = 1; i < n; i++) {
		G4Track* symTrack = new G4Track(track);
		symTrack->SetWeight(newWeight);
		G4double randomAngle = G4UniformRand()*twopi*option;
		G4ThreeVector newPos = InitPos.rotate(fAxisVector1,randomAngle);
		G4ThreeVector newDir = InitDir.rotate(fAxisVector1,randomAngle);
		symTrack->SetMomentumDirection(newDir);
		symTrack->SetPosition(newPos);
		particleChange->AddSecondary(symTrack);
	}
	
	particleChange->SetSecondaryWeightByProcess(true);
}


G4bool TsSplitProcess::AcceptTrack(const G4Track& track, G4double radius, G4double limit) {
	G4ThreeVector r = track.GetPosition();
	G4ThreeVector mom = track.GetMomentumDirection();
	
	G4ThreeVector position = (limit+1.0e-11)*fAxisVector1;
	G4ThreeVector normal = -1.0 * G4ThreeVector(position.unit());
	G4double sVal = -normal.dot( (r - position) )/ normal.dot(mom);
	
	if ( sVal < 0 )
		return false;
	
	G4ThreeVector ip = r + sVal * mom - position;
	
	G4double ri = ip.x()*ip.x() + ip.y()*ip.y();
	
	if ( ri < radius*radius )
		return true;
	
	return false;
}


G4int TsSplitProcess::CalculateNSplit(G4int pre, G4int post) {
	double postOverPre = (double)post/pre;
        G4int splitNumber = static_cast<G4int>(postOverPre);
	return splitNumber; 
}


void TsSplitProcess::RussianRoulette(G4ParticleChange* particleChange, G4int n) {
	if ( G4UniformRand() < 1. - 1./n )
		particleChange->ProposeTrackStatus(fStopAndKill);
	else
		particleChange->ProposeWeight(1.);
	return;
}
