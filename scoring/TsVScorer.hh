//
// ********************************************************************
// *                                                                  *
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

#ifndef TsVScorer_hh
#define TsVScorer_hh

#include "TsParameterManager.hh"
#include "TsVFile.hh"
#include "TsVFilter.hh"
#include "TsVGeometryComponent.hh"

#include "G4VPrimitiveScorer.hh"
#include "G4VSolid.hh"
#include "G4SystemOfUnits.hh"

class TsMaterialManager;
class TsGeometryManager;
class TsScoringManager;
class TsExtensionManager;
class TsFilterByRTStructure;

class TsVScorer : public G4VPrimitiveScorer
{
public:
	TsVScorer(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
			  G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer);

	virtual ~TsVScorer();

	// Gives full parameter name given just last part
	G4String GetFullParmName(const char* parmName);

	// Same as above but returns lower case
	G4String GetFullParmNameLower(const char* parmName);

	// Gives quantity scored
	G4String GetQuantity() const { return fQuantity; }

	// Hooks for users who want to write scorers that accumulate on a non-per-hit basis
	virtual void UserHookForBeginOfTrack(const G4Track*) {}
	virtual void UserHookForEndOfTrack(const G4Track*) {}
	virtual void UserHookForEndOfIncidentParticle() {}
	virtual void UserHookForEndOfEvent() {}
	virtual void UserHookForEndOfRun() {}
	virtual void UserHookForPreTimeStepAction() {}
	virtual void UserHookForPostTimeStepAction() {}
    virtual void UserHookForChemicalReaction(const G4Track&, const G4Track&, const std::vector<G4Track*>*) {}
	virtual void UserHookForBeginOfChemicalTrack(const G4Track*) {}
    virtual void UserHookForEndOfChemicalTrack(const G4Track*) {}
	virtual void UserHookForChemicalStep(const G4Step*) {}
    
	// Handle time-dependent parameter changes
	virtual void UpdateForSpecificParameterChange(G4String parameter);

	// Energy and momentum when track first entered this scoring volume
	G4bool HaveIncidentParticle();
	G4double GetIncidentParticleEnergy();
	G4double GetIncidentParticleMomentum();
	const G4VProcess* GetIncidentParticleCreatorProcess();
	G4ParticleDefinition* GetIncidentParticleDefinition();
	G4int GetIncidentParticleCharge();
	G4int GetIncidentParticleParentID();

	// Cache any geometry pointers that are to be used by scorers.
	// This will be called automatically when relevant geometry has updated.
	virtual void CacheGeometryPointers();

protected:
	// This method is called for every hit in the sensitive detector volume.
	// Code in this method must be written as efficiently as possible.
	// Do not directly access parameters from here. Instead, access and cache them in
	// the method UpdateForParameterChange described above.
	// Do not do string comparisons if you can help it. They tend to be slow.
	virtual G4bool ProcessHits(G4Step*,G4TouchableHistory*) = 0;

	// Must be called before value of area, volume or material is used.
	// Failure to do so will result in incorrect results from divided or parameterized components.
	void ResolveSolid(G4Step*);

	// Following call to ResolveSolid, this will now point to the correct solid
	G4VSolid* fSolid;

	// Following call to ResolveSolid, this will compute the volume of the solid less any subvolumes
	G4double GetCubicVolume(G4Step*);

	// Gets index for divided or parameterized components.
	G4int GetIndex(G4Step*);

	// Helper method to find any of the three bin indices given the combined index
	G4int GetBin(G4int index, G4int iBin);

	// Gets material pointers
	G4Material* GetMaterial(G4String name);
	G4Material* GetMaterial(const char* name);

	// Pointer to parameter manager
	TsParameterManager *fPm;

	// Pointer to file (currently only used by ntuple scorers)
	// Check if NULL before using
	TsVFile *fOutFile;

	// To be called by scorers associated with the surface of a geometry component
	void SetSurfaceScorer();

	// To be called by scorers need surface area calculation
	void SetNeedsSurfaceAreaCalculation();

	// To be called by scorers that do not use the standard output file handling
	void SuppressStandardOutputHandling();

	// Gets current TOPAS time
	G4double GetTime();

	// Current run and event ID
	G4int GetRunID();
	G4int GetEventID();

	// Gets the current random number status
	const G4String& GetRandomNumberStatusForThisEvent();

	// Total number of original histories since scorer last refreshed
	G4long GetScoredHistories();

	// Used for gated scoring.
	G4bool fIsActive;
	G4long fSkippedWhileInactive;

	// Tells whether there is a hit on the active surface
	G4bool IsSelectedSurface(G4Step*);

	// Gets area of selected surface
	G4double GetAreaOfSelectedSurface(G4Step* aStep);

	// Gives name of selected surface
	G4String GetSurfaceName();

	// fFlux_In means particle entering component
	// fFlux_Out means particle leaving component
	G4int GetDirection();


// User classes should not use methods or properties below this point
public:
	virtual void RestoreResultsFromFile() = 0;
	virtual void AccumulateEvent() = 0;
	virtual void AbsorbResultsFromWorkerScorer(TsVScorer*) = 0;

	virtual void PostConstructor();
	virtual void UpdateForNewRun(G4bool rebuiltSomeComponents);
	virtual void UpdateForEndOfRun();
	void PostUpdateForEndOfRun();
	virtual G4bool HasUnsatisfiedLimits() { return false; }
	void Finalize();
	void PostFinalize();

	void ClearIncidentParticleInfo();
	void NoteIncidentParticleInfo(const G4Step* aStep);
	void NoteHitHadNoIncidentParticle();

	virtual void ResolveParameters();
	G4String GetNameWithSplitId();
	void SetSplitInfo(G4String splitParmValueName, G4String splitId, G4String splitFunction, G4String splitUnitLower, G4String splitUnitCategory,
					  G4String splitStringValue, G4bool splitBooleanValue, G4double splitLowerValue, G4double splitUpperValue);
	void SetRTStructureFilter(TsFilterByRTStructure* filter);
	void SetFilter(TsVFilter* filter);
	TsVGeometryComponent* GetComponent() { return fComponent; }

	virtual G4int CombineSubScorers() { return 0; };
	G4int CallCombineOnSubScorers();
	void LinkReferencedSubScorers();

	G4int fUID;

protected:
	virtual void GetAppropriatelyBinnedCopyOfComponent(G4String componentName);
	void InstantiateSubScorer(G4String quantityName, G4String outFileName, G4String identifier="", G4bool cloneParameters=true);
	TsVScorer* GetSubScorer(const G4String& key);
	void UpdateFileNameForUpcomingRun();
	virtual void Output() = 0;
	virtual void Clear() = 0;

	G4bool ExcludedByRTStructFilter(G4int idx);
	void OutOfRange(G4String parameterName, G4String requirement);

	TsScoringManager *fScm;
	TsGeometryManager *fGm;
	TsExtensionManager *fEm;

	G4long fScoredHistories;
	G4long fHitsWithNoIncidentParticle;
	G4long fUnscoredSteps;
	G4double fUnscoredEnergy;
	G4int fMissedSubScorerVoxels;

	G4String fOutFileType;
	G4String fOutFileMode;
	G4String fOutFileName;
	G4bool fOutputAfterRun;
	G4bool fOutputAfterRunShouldAccumulate;

	G4String fQuantity;

	TsVGeometryComponent* fComponent;
	G4MultiFunctionalDetector* fDetector;
	G4LogicalVolume* fSensitiveLogicalVolume;
	G4String fComponentName;
	G4int fNDivisions;

	G4int fVerbosity;
	G4int fTrackingVerbosity;
	G4bool fSuppressStandardOutputHandling;

	std::map<G4String, TsVScorer*> fSubScorers;
	std::map<G4String, G4String> fReferencedSubScorers;
	G4bool fIsSubScorer;
	G4bool fHasCombinedSubScorers;

	G4bool fHaveIncidentParticle;
	G4double fIncidentParticleEnergy;
	G4double fIncidentParticleMomentum;
	const G4VProcess* fIncidentParticleCreatorProcess;
	G4ParticleDefinition* fIncidentParticleDefinition;
	G4int fIncidentParticleCharge;
	G4int fIncidentParticleParentID;

	G4bool fHadParameterChangeSinceLastRun;

private:
	G4LogicalVolume* GetSensitiveLogicalVolume() = delete; // delete not implemented

	TsMaterialManager *fMm;

	G4String fSplitParmValueName;
	G4String fSplitId;
	G4String fSplitFunction;
	G4String fSplitUnitLower;
	G4String fSplitUnitCategory;
	G4String fSplitStringValue;
	G4bool fSplitBooleanValue;
	G4double fSplitLowerValue;
	G4double fSplitUpperValue;

	G4double fCachedCubicVolume;
	G4bool fPropagateToChildren;
	G4bool fIsSurfaceScorer;
	G4bool fNeedsSurfaceAreaCalculation;
	G4String fSurfaceName;
	TsVGeometryComponent::SurfaceType fSurfaceID;
	G4int fDirection;
	G4bool fOnlyIncludeParticlesGoingIn;
	G4bool fOnlyIncludeParticlesGoingOut;

	G4bool fSetBinToMinusOneIfNotInRTStructure;
	TsFilterByRTStructure* fRTStructureFilter;
};

#endif
