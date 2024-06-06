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

#include "TsFilterByInteractedOrTraversed.hh"

#include "TsGeometryManager.hh"

#include "TsTrackInformation.hh"
#include "TsVGeometryComponent.hh"

#include "G4VPhysicalVolume.hh"
#include "G4VTouchable.hh"

TsFilterByInteractedOrTraversed::TsFilterByInteractedOrTraversed(G4String name, TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM,
																 TsFilterManager* fM, TsVGenerator* generator, TsVScorer* scorer, TsVFilter* parentFilter, G4bool invert,
																 G4bool includeAncestors, G4bool namesAreVolumes,
																 G4bool includeChildren, G4bool testTraversed, G4bool onlyConsiderLastVolume)
:TsVFilter(name, pM, mM, gM, fM, generator, scorer, parentFilter, invert),
fIncludeAncestors(includeAncestors), fNamesAreVolumes(namesAreVolumes),
fIncludeChildren(includeChildren), fTestTraversed(testTraversed), fOnlyConsiderLastVolume(onlyConsiderLastVolume) {
	ResolveParameters();

	pM->SetNeedsSteppingAction();

	if (fIncludeAncestors)
		pM->SetNeedsTrackingAction();
}


TsFilterByInteractedOrTraversed::~TsFilterByInteractedOrTraversed()
{;}


void TsFilterByInteractedOrTraversed::ResolveParameters() {
	if (fOnlyConsiderLastVolume) {
		fNames = new G4String[1];
		fNames[0] = fPm->GetStringParameter(GetFullParmName(GetName()));
		fNamesLength = 1;
	} else {
		fNames = fPm->GetStringVector(GetFullParmName(GetName()));
		fNamesLength = fPm->GetVectorLength(GetFullParmName(GetName()));
	}

	CacheGeometryPointers();
}


void TsFilterByInteractedOrTraversed::CacheGeometryPointers() {
	fVolumes.clear();

	for (G4int i = 0; i < fNamesLength; i++) {
		if (fNamesAreVolumes) {
			G4VPhysicalVolume* volume = GetPhysicalVolume(fNames[i]);
			if (volume) {
				fVolumes.push_back(volume);
			} else {
				G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
				G4cerr << GetName() << " = " << fNames[i] << " refers to an unknown Volume." << G4endl;
				fPm->AbortSession(1);
			}
		} else {
			TsVGeometryComponent* originComponent = GetComponent(fNames[i]);
			if (originComponent) {
				// Insert all volumes of Component
				std::vector<G4VPhysicalVolume*> physicalVolumes = originComponent->GetAllPhysicalVolumes();
				for ( size_t j = 0; j < physicalVolumes.size(); j++)
					fVolumes.push_back(physicalVolumes[j]);

				if (fIncludeChildren) {
					// Insert all volumes of all child Components
					std::vector<G4String> childNames = GetChildComponentsOf(fNames[i]);
					std::vector<G4String>::iterator iter;
					for (iter = childNames.begin(); iter!=childNames.end(); iter++) {
						G4String childName = *iter;
						TsVGeometryComponent* childComponent = GetComponent(childName);
						if (childComponent) {
							std::vector<G4VPhysicalVolume*> childVolumes = childComponent->GetAllPhysicalVolumes();
							for ( size_t j = 0; j < childVolumes.size(); j++)
								fVolumes.push_back(childVolumes[j]);
						}
					}
				}
			} else {
				G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
				G4cerr << GetName() << " = " << fNames[i] << " refers to an unknown Component." << G4endl;
				fPm->AbortSession(1);
			}
		}
	}
}


G4bool TsFilterByInteractedOrTraversed::Accept(const G4Step* aStep) const {
	if (fParentFilter && !fParentFilter->Accept(aStep)) return false;

	TsTrackInformation* parentInformation = (TsTrackInformation*)(aStep->GetTrack()->GetUserInformation());
	if (parentInformation) {
		std::vector<G4VPhysicalVolume*> volumes;
		if (fTestTraversed)
			volumes = parentInformation->GetTraversedVolumes();
		else
			volumes = parentInformation->GetInteractionVolumes();

		if (volumes.size() != 0) {
			if (fOnlyConsiderLastVolume) {
				for ( size_t j = 0; j < fVolumes.size(); j++) {
					if (volumes[volumes.size()-1] == fVolumes[j]) {
						if (fInvert) return false;
						else return true;
					}
				}
			} else {
				for (size_t i=0; i < volumes.size(); i++) {
					for ( size_t j = 0; j < fVolumes.size(); j++)
						if (volumes[i] == fVolumes[j]) {
							if (fInvert) return false;
							else return true;
						}
				}

				if (fIncludeAncestors) {
					if (fTestTraversed)
						volumes = parentInformation->GetAncestorTraversedVolumes();
					else
						volumes = parentInformation->GetAncestorInteractionVolumes();
					for (size_t i=0; i < volumes.size(); i++) {
						for ( size_t j = 0; j < fVolumes.size(); j++)
							if (volumes[i] == fVolumes[j]) {
								if (fInvert) return false;
								else return true;
							}
					}
				}
			}
		}
	}

	if (fInvert) return true;
	else return false;
}


G4bool TsFilterByInteractedOrTraversed::AcceptTrack(const G4Track*) const {
	G4cerr << "Topas is exiting due to a serious error in source setup." << G4endl;
	G4cerr << "Sources cannot be filtered by " << GetName() << G4endl;
	fPm->AbortSession(1);
	return false;
}

