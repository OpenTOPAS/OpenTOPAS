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

#include "TsVGeometryComponent.hh"

#include "TsParameterManager.hh"
#include "TsMaterialManager.hh"
#include "TsGeometryManager.hh"
#include "TsExtensionManager.hh"
#include "TsGeometryHub.hh"
#include "TsTopasConfig.hh"

#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVParameterised.hh"
#include "G4RegionStore.hh"
#include "G4VPVParameterisation.hh"
#include "TsVMagneticField.hh"
#include "TsMagneticFieldDipole.hh"
#include "TsMagneticFieldQuadrupole.hh"
#include "TsMagneticFieldMap.hh"
#include "TsVElectroMagneticField.hh"
#include "TsElectroMagneticFieldUniform.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4OpticalSurface.hh"
#include "G4VisAttributes.hh"
#include "G4PhysicalVolumeModel.hh"
#include "G4BoundingExtentScene.hh"
#include "G4UserLimits.hh"
#include "G4UIcommand.hh"
#include "G4Material.hh"
#include "G4Step.hh"


TsVGeometryComponent::TsVGeometryComponent(TsParameterManager* pM,  TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
										   TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name)
:fPm(pM), fEm(eM), fParentComponent(parentComponent), fParentVolume(parentVolume), fEnvelopePhys(0), fEnvelopeLog(0),
fName(name), fIsCopy(false), fOriginalComponent(0), fCopyId(""), fIsGroup(false), fIsDividable(false),
fCanCalculateSurfaceArea(false), fHasDifferentVolumePerDivision(false), fScoringVolume(0),
fMaximumNumberOfDetailedErrorReports(4), fNumberOfDetailedErrorReports(0),
fHasVariableMaterial(false), fImageIndex(0), fMm(mM), fGm(gM), fIsUsedByScorer(false), fHasPropagatingScorer(false),
fHasCopy(false), fIsParallel(false), fHasParallelWorldDescendents(false),
fNeedToRebuild(false), fNeedToUpdatePlacement(false), fSimplestCreateWasCalled(false),
fWorldName(""), fLogicalSkinSurface(0)
{
	fVerbosity = fPm->GetIntegerParameter("Ge/Verbosity");

	if (fPm->ParameterExists("Ts/MaximumNumberOfDetailedErrorReports"))
		fMaximumNumberOfDetailedErrorReports = fPm->GetIntegerParameter("Ts/MaximumNumberOfDetailedErrorReports");

	// Initialize division information to be used by scorers
	fDivisionNames[0] = "";
	fDivisionNames[1] = "";
	fDivisionNames[2] = "";
	fDivisionUnits[0] = "";
	fDivisionUnits[1] = "";
	fDivisionUnits[2] = "";
	fDivisionCounts[0] = 1;
	fDivisionCounts[1] = 1;
	fDivisionCounts[2] = 1;
	fFullWidths[0] = 1;
	fFullWidths[1] = 1;
	fFullWidths[2] = 1;

	fMaterialList = new std::vector<G4Material*>;
	fCurrentMaterialIndex = new std::vector<unsigned short>;

	if (fPm->ParameterExists(GetFullParmName("VoxelMaterials"))) {
		fHasVariableMaterial = true;
		G4String* voxelMaterials = fPm->GetStringVector(GetFullParmName("VoxelMaterials"));
		G4int numberOfVoxels = fPm->GetVectorLength(GetFullParmName("VoxelMaterials"));
		for (G4int i = 0; i < numberOfVoxels; i++) {
			G4int foundPosition = std::find(fMaterialList->begin(), fMaterialList->end(), GetMaterial(voxelMaterials[i])) - fMaterialList->begin();
			if (foundPosition >= (G4int)fMaterialList->size())
				fMaterialList->push_back(GetMaterial(voxelMaterials[i]));
			fCurrentMaterialIndex->push_back(foundPosition);
		}
	}
}


TsVGeometryComponent::~TsVGeometryComponent()
{
	if (fVerbosity>0)
		G4cout << "TsVGeometryComponent destructor called for component: " << GetNameWithCopyId() << G4endl;
	DeleteContents();
	G4String nameWithCopyId = GetNameWithCopyId();
	fGm->UnRegisterComponent(nameWithCopyId);
}


// Delete subcomponents and volumes from a component
void TsVGeometryComponent::DeleteContents() {
	// Delete any child components
	std::vector<TsVGeometryComponent*>::iterator ChildIter;
	for (ChildIter=fChildren.begin(); ChildIter!=fChildren.end(); ChildIter++)
		delete *ChildIter;
	fChildren.clear();

	// Delete any magnetic fields
	std::vector<TsVMagneticField*>::iterator MagneticFieldIter;
	for (MagneticFieldIter=fMagneticFields.begin(); MagneticFieldIter!=fMagneticFields.end(); MagneticFieldIter++)
		delete *MagneticFieldIter;
	fMagneticFields.clear();

	// Delete any electronmagnetic fields
	std::vector<TsVElectroMagneticField*>::iterator ElectroMagneticFieldIter;
	for (ElectroMagneticFieldIter=fElectroMagneticFields.begin(); ElectroMagneticFieldIter!=fElectroMagneticFields.end(); ElectroMagneticFieldIter++)
		delete *ElectroMagneticFieldIter;
	fMagneticFields.clear();

	if (fVerbosity>0)
		G4cout << "TsVGeometryComponent::DeleteContents for component: " << GetNameWithCopyId() << G4endl;
	if (!fIsGroup) {
		if (fLogicalSkinSurface) {
			delete fLogicalSkinSurface;
			fLogicalSkinSurface = 0;
		}

		std::vector<G4VPhysicalVolume*>::iterator PVIter;
		for (PVIter=fPhysicalVolumes.begin(); PVIter!=fPhysicalVolumes.end(); PVIter++)
			delete *PVIter;
		fPhysicalVolumes.clear();

		std::vector<G4RotationMatrix*>::iterator RIter;
		for (RIter=fRotations.begin(); RIter!=fRotations.end(); RIter++)
			delete *RIter;
		fRotations.clear();

		std::vector<G4ThreeVector*>::iterator TRIter;
		for (TRIter=fTranslations.begin(); TRIter!=fTranslations.end(); TRIter++)
			delete *TRIter;
		fTranslations.clear();

		std::vector<G4Transform3D*>::iterator TransformIter;
		for (TransformIter=fTransforms.begin(); TransformIter!=fTransforms.end(); TransformIter++)
			delete *TransformIter;
		fTransforms.clear();

		std::vector<G4VPVParameterisation*>::iterator PIter;
		for (PIter=fParameterizations.begin(); PIter!=fParameterizations.end(); PIter++)
			delete *PIter;
		fParameterizations.clear();

		std::vector<G4LogicalVolume*>::iterator LVIter;
		for (LVIter=fLogicalVolumes.begin(); LVIter!=fLogicalVolumes.end(); LVIter++) {
			fGm->RemoveFromReoptimizeList(*LVIter);
			delete *LVIter;
		}
		fLogicalVolumes.clear();

		for (LVIter=fLogicalVolumesToBeSensitive.begin(); LVIter!=fLogicalVolumesToBeSensitive.end(); LVIter++) {
			delete *LVIter;
		}
		fLogicalVolumesToBeSensitive.clear();

		std::vector<G4UserLimits*>::iterator LimitsIter;
		for (LimitsIter=fUserLimits.begin(); LimitsIter!=fUserLimits.end(); LimitsIter++)
			delete *LimitsIter;
		fUserLimits.clear();

		std::vector<G4VisAttributes*>::iterator AttIter;
		for (AttIter=fVisAtts.begin(); AttIter!=fVisAtts.end(); AttIter++)
			delete *AttIter;
		fVisAtts.clear();

		std::vector<G4VSolid*>::iterator SIter;
		for (SIter=fSolids.begin(); SIter!=fSolids.end(); SIter++)
			delete *SIter;
		fSolids.clear();

		delete fRotRelToWorld;
		delete fTransRelToWorld;
		delete fRotRelToLastNonGroupComponent;
		delete fTransRelToLastNonGroupComponent;

		// For simple placements, these are instead handled by UpdatePlacementIfNeeded's deletion of fRotations[0] and fTranslations[0]
		if ( (fParentComponent && fParentComponent->fIsGroup) || (fIsParallel && !fParentComponent->IsParallel()) ) {
			delete fRotRelToParentComponent;
			delete fTransRelToParentComponent;
		}
	}
}


// Instantiate and constuct all children of the current component
void TsVGeometryComponent::InstantiateChildren() {
	if (fVerbosity>0)
		G4cout << "InstantiateChildren called for: " << GetNameWithCopyId() << ", fIsCopy: " << fIsCopy << G4endl;
	fGm->SetCurrentComponent(0);

	// Copy components do not instantiate children
	if (fIsCopy) return;

	// Loop over all children of the current comopnent
	std::vector<G4String> childNames = fGm->GetChildComponentsOf(fName);
	std::vector<G4String>::iterator iter;
	for (iter = childNames.begin(); iter!=childNames.end(); iter++)
		InstantiateAndConstructChild(*iter);
}

// Instantiate one child of the current component
void TsVGeometryComponent::InstantiateAndConstructChild(G4String childName) {

	// Only include child if Include flag is missing or true
	G4String parmName = "Ge/"+childName+"/Include";
	if (!fPm->ParameterExists(parmName) || fPm->GetBooleanParameter(parmName)) {
		// Skip children that already exist
		// (such as slice thickness section TsBoxes that are instantiated directly from TsVPatient).
		if (!fGm->GetComponent(childName)) {
			// Forbid divided components from having children unless those children are in a parallel world
			parmName = "Ge/"+childName+"/IsParallel";
			G4bool childIsParallel = false;
			if (fPm->ParameterExists(parmName) && fPm->GetBooleanParameter(parmName))
				childIsParallel = true;

			G4int nDivisions = fDivisionCounts[0] * fDivisionCounts[1] * fDivisionCounts[2];
			if (nDivisions > 1 && (!childIsParallel || (childIsParallel && fIsParallel))) {
				G4cerr << "Topas is exiting due to a serious error." << G4endl;
				G4cerr << "Attempt to instantiate child: " << childName << " inside divided component: " << fName << G4endl;
				G4cerr << "This is only permitted if the child is in a parallel world and parent is not." << G4endl;
				fPm->AbortSession(1);
			} else {
				TsVGeometryComponent* child = InstantiateChild(childName);

				child->Construct();

				if (!(child->fIsGroup) && !(child->fEnvelopeLog)) {
					G4cerr << "Topas is exiting due to a serious error in geometry setup." << G4endl;
					G4cerr << "Something seems to be wrong in the C++ code used to build the component named: " << childName << G4endl;
					G4cerr << "The class has failed to fill the base class variable fEnvelopeLog." << G4endl;
					G4cerr << "A common cause of this is that the class has defined its own local copy of fEnvelopeLog." << G4endl;
					G4cerr << "fEnvelopeLog should instead just be used as already provided by TsVGeometryComponent." << G4endl;
					G4cerr << "This sort of error is referred to in C++ design as \"Shadowing a variable\"." << G4endl;
					fPm->AbortSession(1);
				}

				if (!(child->fEnvelopePhys)) {
					G4cerr << "Topas is exiting due to a serious error in geometry setup." << G4endl;
					G4cerr << "Something seems to be wrong in the C++ code used to build the component named: " << childName << G4endl;
					G4cerr << "The class has failed to fill the base class variable fEnvelopePhys." << G4endl;
					G4cerr << "A common cause of this is that the class has defined its own local copy of fEnvelopePhys." << G4endl;
					G4cerr << "fEnvelopePhys should instead just be used as already provided by TsVGeometryComponent." << G4endl;
					G4cerr << "This sort of error is referred to in C++ design as \"Shadowing a variable\"." << G4endl;
					fPm->AbortSession(1);
				}

				if (!(child->fIsGroup) && !(child->SimplestCreateWasCalled())) {
					G4cerr << "\nTopas is exiting due to a serious error in the geometry component class" << G4endl;
					G4cerr << "that instantiated the component: " << child->GetName() << G4endl;
					G4cerr << "This class created fEnvelopePhys in the wrong way." << G4endl;
					G4cerr << "fEnvelopePhys may only be created by a call to CreatePhysicalVolume, and it must" << G4endl;
					G4cerr << "use the simplest version of that method - the one that only takes one argument." << G4endl;
					fPm->AbortSession(1);
				}

				child->InstantiateFields();
				fGm->SetCurrentMagneticField(0);
				fGm->SetCurrentElectroMagneticField(0);
				child->AddOpticalSurfaces();
				fGm->SetCurrentComponent(0);

				// May need to create parallel world scoring copies of this component.
				// Loop over all scorer parameters to check whether they use this component.
				// Note that the actual scorer objects do not exist yet. They are only created later,
				// once geometry has been constructed and initialized.
				std::vector<G4String>* values = new std::vector<G4String>;
				G4String prefix = "Sc";
				G4String suffix = "/Quantity";
				G4String componentName;
				fPm->GetParameterNamesBracketedBy(prefix, suffix, values);
				G4int length = values->size();
				for (G4int iToken=0; iToken<length; iToken++) {
					parmName = (*values)[iToken];
#if GEANT4_VERSION_MAJOR >= 11
					G4String scorerParmNameBase = parmName.substr(0, parmName.size()-8);
#else
					G4String scorerParmNameBase = parmName(0,parmName.size()-8);
#endif

					G4String componentNameParm = scorerParmNameBase + "Component";
					if (fPm->ParameterExists(componentNameParm)) {
						componentName = fPm->GetStringParameter(componentNameParm);
					} else {
						G4String surfaceNameParm = scorerParmNameBase + "Surface";
						if (fPm->ParameterExists(surfaceNameParm)) {
							G4String componentPlusSurface = fPm->GetStringParameter(surfaceNameParm);
							size_t pos = componentPlusSurface.find_last_of("/");
							componentName = componentPlusSurface.substr(0,pos);
						} else {
							G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
							G4cerr << "Scorer: " << scorerParmNameBase << " does not specify Component or Surface." << G4endl;
							fPm->AbortSession(1);
						}
					}

					if (componentName == childName) {
						// If child is already used by another scorer, both scorers need to have
						// same value of PropagateToChildren
						parmName = scorerParmNameBase + "PropagateToChildren";

						if (child->IsUsedByScorer()) {
							if ((fPm->ParameterExists(parmName) && fPm->GetBooleanParameter(parmName)) !=
								child->HasPropagatingScorer()) {
								G4cerr << "\nTopas is exiting due to a serious error in scoring." << G4endl;
								G4cerr << "Scorer: " << scorerParmNameBase << " has different value for PropagateToChildren" << G4endl;
								G4cerr << "than another scorer that used the same component." << G4endl;
								fPm->AbortSession(1);
							}
						} else {
							child->SetUsedByScorer();
							if (fPm->ParameterExists(parmName) && fPm->GetBooleanParameter(parmName))
								child->SetHasPropagatingScorer();
						}

						// Will need a parallel scoring copy if the scorer's binning does not match the component's binning.
						G4bool scoreInCopy = false;
						G4int nBins[3];
						for (G4int i = 0; i < 3; i++) {
							parmName = scorerParmNameBase + child->GetDivisionName(i) + "Bins";
							if (fPm->ParameterExists(parmName)) {
								nBins[i] = fPm->GetIntegerParameter(parmName);
								if (nBins[i] != child->GetDivisionCount(i))
									scoreInCopy = true;
							} else
								nBins[i] = child->GetDivisionCount(i);
						}

						// Will need a parallel scoring copy if the component
						// has children that are used by scorers,
						// and the user wants this scorer to PropagateToChildren.
						if (child->HasChildrenUsedByScorers() && child->HasPropagatingScorer())
							scoreInCopy = true;
						
						if (scoreInCopy) {
							// Check whether we have already instantiated this copy
							G4String nameWithCopyId = childName + fGm->GetCopyIdFromBinning(nBins);
							if (!fGm->GetComponent(nameWithCopyId)) {
								// Instantiate the child. Declare it to be a parallel scoring division. Construct it.
								TsVGeometryComponent* copyComponent = InstantiateChild(childName);
								copyComponent->SetParallelScoringCopyDivisions(nBins);
								copyComponent->Construct();
								fGm->SetCurrentComponent(0);
								child->HasCopy();
							}
						}
					}
				}
			}
		}
	}
	return;
}


// Instantiate one child of the current component
TsVGeometryComponent* TsVGeometryComponent::InstantiateChild(G4String childName) {
	G4String parmString = "Ge/"+childName+"/Type";
	G4String childCompType = fPm->GetStringParameterWithoutMonitoring(parmString);
	TsVGeometryComponent* child = fGm->GetGeometryHub()->InstantiateComponent(fPm, fEm, fMm, fGm, this, fEnvelopePhys, childCompType, childName);
	fChildren.push_back(child);
	return child;
}



// Deprecated form. Instead use the version that takes no arguments
void TsVGeometryComponent::InstantiateChildren(G4VPhysicalVolume*) {
	InstantiateChildren();
}


void TsVGeometryComponent::InstantiateFields() {
	if (fPm->ParameterExists(GetFullParmName("Field"))) {
		G4String fieldType = fPm->GetStringParameter(GetFullParmName("Field"));
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(fieldType);
#else
		fieldType.toLower();
#endif

		TsVMagneticField* magneticField = 0;
		TsVElectroMagneticField* electroMagneticField = 0;

		magneticField = fEm->InstantiateMagneticField(fPm, fGm, this, fieldType);

		if (!magneticField) {
			// if didn't find this FieldType as a magnetic field, see if it is an electroMagnetic field
			electroMagneticField = fEm->InstantiateElectroMagneticField(fPm, fGm, this, fieldType);

			if (!electroMagneticField) {
				if ( fieldType == "dipolemagnet") {
					magneticField = new TsMagneticFieldDipole(fPm, fGm, this);
				} else if (fieldType == "quadrupolemagnet") {
					magneticField = new TsMagneticFieldQuadrupole(fPm, fGm, this);
				} else if ( fieldType == "mappedmagnet") {
					magneticField = new TsMagneticFieldMap(fPm, fGm, this);
				} else if ( fieldType == "uniformelectromagnetic") {
					electroMagneticField = new TsElectroMagneticFieldUniform(fPm, fGm, this);
				} else if (fieldType != "none") {
					G4cerr << "Topas is exiting due to a serious error." << G4endl;
					G4cerr << "Component: " << GetName() << " has unknown Field value: " << fieldType << G4endl;
					fPm->AbortSession(1);
				}
			}
		}
		if (magneticField) fMagneticFields.push_back(magneticField);
		if (electroMagneticField) fElectroMagneticFields.push_back(electroMagneticField);
	}
}


void TsVGeometryComponent::AddOpticalSurfaces() {
	// Check for Single Component Optical Surfaces
	if (fPm->ParameterExists(GetFullParmName("OpticalBehavior"))) {
		G4String surfaceName = fPm->GetStringParameter(GetFullParmName("OpticalBehavior"));
		fLogicalSkinSurface = new G4LogicalSkinSurface(surfaceName, fEnvelopeLog, fGm->GetOpticalSurface(surfaceName));
	}

	// Check for Shared Optical Surfaces.
	// The actual surface is only created later, once all components have been instantiated.
	G4String prefix = "Ge/" + fName + "/OpticalBehaviorTo";
	std::vector<G4String>* values = new std::vector<G4String>;
	fPm->GetParameterNamesStartingWith(prefix, values);
	G4int length = values->size();
	if (length > 0) {
		fGm->RegisterToIgnoreInUnusedComponentCheck(prefix.substr(3));
		for (G4int iToken=0; iToken<length; iToken++)
			fGm->RegisterBorderSurface((*values)[iToken]);
	}

	fGm->CheckForEffectOnBorderSurfaceDestinations(fName);
}


G4String TsVGeometryComponent::GetWorldName() {
	return fWorldName;
}


G4VPhysicalVolume* TsVGeometryComponent::GetEnvelopePhysicalVolume() {
	return fEnvelopePhys;
}


G4LogicalVolume* TsVGeometryComponent::GetEnvelopeLogicalVolume() {
	return fEnvelopeLog;
}


G4bool TsVGeometryComponent::IsCopy() {
	return fIsCopy;
}


TsVGeometryComponent* TsVGeometryComponent::OriginalComponent() {
	return fOriginalComponent;
}


void TsVGeometryComponent::SetOriginalComponent(TsVGeometryComponent* originalComponent) {
	fOriginalComponent = originalComponent;
}


G4bool TsVGeometryComponent::HasDifferentVolumePerDivision() {
	return fHasDifferentVolumePerDivision;
}


G4bool TsVGeometryComponent::IsParallel() {
	return fIsParallel;
}


G4bool TsVGeometryComponent::HasChildrenUsedByScorers() {
	std::vector<TsVGeometryComponent*>::iterator ChildIter;
	for (ChildIter=fChildren.begin(); ChildIter!=fChildren.end(); ChildIter++)
		if ((*ChildIter)->IsUsedByScorer() || (*ChildIter)->HasChildrenUsedByScorers())
			return true;

	return false;
}


G4bool TsVGeometryComponent::IsUsedByScorer() {
	return fIsUsedByScorer;
}


void TsVGeometryComponent::SetUsedByScorer() {
	fIsUsedByScorer = true;
}


G4bool TsVGeometryComponent::HasPropagatingScorer() {
	return fHasPropagatingScorer;
}


void TsVGeometryComponent::SetHasPropagatingScorer() {
	fHasPropagatingScorer = true;
}


void TsVGeometryComponent::HasCopy() {
	fHasCopy = true;
}


void TsVGeometryComponent::RegisterRotation(G4RotationMatrix* rot) {
	fRotations.push_back(rot);
}


void TsVGeometryComponent::RegisterTranslation(G4ThreeVector* translation) {
	fTranslations.push_back(translation);
}


void TsVGeometryComponent::RegisterTransform(G4Transform3D* transform) {
	fTransforms.push_back(transform);
}


G4String TsVGeometryComponent::GetFullParmName(const char* parmName) {
	G4String fullName = "Ge/"+fName+"/"+parmName;
	return fullName;
}


G4double TsVGeometryComponent::GetCubicVolume() {
	if (fIsGroup) {
		G4double cubicVolume = 0.;
		std::vector<TsVGeometryComponent*>::iterator ChildIter;
		for (ChildIter=fChildren.begin(); ChildIter!=fChildren.end(); ChildIter++)
			cubicVolume += (*ChildIter)->GetCubicVolume();
		return cubicVolume;
	}

	return GetEnvelopeLogicalVolume()->GetSolid()->GetCubicVolume();
}


G4String TsVGeometryComponent::GetFullParmName(const char* subComponentName, const char* parmName) {
	G4String nameString = subComponentName;
	return GetFullParmName(nameString, parmName);
}


G4String TsVGeometryComponent::GetFullParmName(G4String& subComponentName, const char* parmName) {
	G4String fullName;
	if (subComponentName=="")
		fullName = "Ge/"+ fName + "/" + parmName;
	else
		fullName = "Ge/"+ fName + "/" + subComponentName + "/" + parmName;
	return fullName;
}


G4String TsVGeometryComponent::GetFullParmNameLower(const char* parmName) {
	G4String fullName = GetFullParmName(parmName);
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(fullName);
#else
	fullName.toLower();
#endif
	return fullName;
}


G4String TsVGeometryComponent::GetBinParmName(G4int i) {
	G4String fullName = "Ge/" + fName + "/" + fDivisionNames[i] + "Bins";
	return fullName;
}


G4String TsVGeometryComponent::GetResolvedMaterialName() {
	G4String nameString = "";
	return GetResolvedMaterialName(nameString);
}


// Get material name of subcomponent
G4String TsVGeometryComponent::GetResolvedMaterialName(G4String& subComponentName, G4bool allowUndefined) {
	G4String materialName;

	G4String parmName = GetFullParmName(subComponentName, "Material");
	if (fIsCopy)
		materialName = "";
	else if (fPm->ParameterExists(parmName))
		materialName = fPm->GetStringParameter(parmName);
	else if (fIsGroup)
		materialName = "parent";
	else if (fIsParallel || allowUndefined)
		materialName = "";
	else if (fPm->ParameterExists(GetFullParmName(subComponentName, "ImagingToMaterialConverter")))
		materialName = "parent";
	else {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "Unable to find Parameter name: " << parmName << G4endl;
		fPm->AbortSession(1);
	}

	G4String lowerCaseName = materialName;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(lowerCaseName);
#else
	lowerCaseName.toLower();
#endif

	if (lowerCaseName == "parent")
		materialName = fParentComponent->GetResolvedMaterialName();
	else if (lowerCaseName == "world")
		materialName = fPm->GetStringParameter("Ge/World/Material");

	// Note any parallel worlds that have a volume with material
	if (fIsParallel && materialName!="")
		fGm->SetParallelWorldHasMaterial(fWorldName);

	return materialName;
}


G4LogicalVolume* TsVGeometryComponent::CreateLogicalVolume(G4VSolid* solid) {
	return CreateLogicalVolume("", solid);
}


G4LogicalVolume* TsVGeometryComponent::CreateLogicalVolume(const char* subComponentName, G4VSolid* solid) {
	G4String nameString = subComponentName;
	return CreateLogicalVolume(nameString, solid);
}

G4LogicalVolume* TsVGeometryComponent::CreateLogicalVolume(G4String& subComponentName, G4VSolid* solid) {
	G4String materialName = "";
	return CreateLogicalVolume(subComponentName, materialName, solid);
}


G4LogicalVolume* TsVGeometryComponent::CreateLogicalVolume(const char* subComponentName, G4String& materialName, G4VSolid* solid) {
	G4String nameString = subComponentName;
	return CreateLogicalVolume(nameString, materialName, solid);
}


G4LogicalVolume* TsVGeometryComponent::CreateLogicalVolume(G4String& subComponentName, G4String& materialName, G4VSolid* solid) {
	// Resolve the material
	// Note that if GetResolvedMaterialName is called with subComponentName="", it returns name of the main component's material
	G4String resolvedMaterialName = materialName;
	if (materialName=="")
		resolvedMaterialName = GetResolvedMaterialName(subComponentName);

	G4Material* material;
	if (resolvedMaterialName=="")
		material = 0;
	else
		material = fMm->GetMaterial(resolvedMaterialName);

	// Create the logical volume name including subComponent and copy number
	G4String lVolName = fName + fCopyId;
	if (subComponentName!="") lVolName += "/" + subComponentName;

	// Register name so that it does not appear in list of unused components
	fGm->RegisterToIgnoreInUnusedComponentCheck(lVolName);

	// Create the logical volume
	G4LogicalVolume* lVol = new G4LogicalVolume(solid, material, lVolName);

	// Set visualization attributes
	G4VisAttributes* visAtt = GetVisAttributes(subComponentName);
	lVol->SetVisAttributes(visAtt);
	fVisAtts.push_back(visAtt);

	// If defined, set Max Step Size
	if (!fIsCopy && fPm->ParameterExists(GetFullParmName(subComponentName, "MaxStepSize"))) {
		if (fIsParallel) {
			G4cerr << "Topas is exiting due to a serious error in definition of geometry component: " << GetName() << G4endl;
			G4cerr << "Components in parallel worlds may not have MaxStepSize." << G4endl;
			fPm->AbortSession(1);
		}

		G4UserLimits* userLimit = new G4UserLimits(fPm->GetDoubleParameter(GetFullParmName(subComponentName, "MaxStepSize"), "Length"));
		lVol->SetUserLimits(userLimit);
		fUserLimits.push_back(userLimit);
	}

	// Register the solid and logical volume
	fSolids.push_back(solid);
	fLogicalVolumes.push_back(lVol);

	// Define region for the current component
	if (lVol->GetName() != "World") {
		if (fPm->ParameterExists(GetFullParmName(subComponentName, "AssignToRegionNamed"))) {
			G4String regionName = fPm->GetStringParameter(GetFullParmName(subComponentName, "AssignToRegionNamed"));
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(regionName);
#else
			regionName.toLower();
#endif
			if (regionName == "defaultregionfortheworld" )
				regionName = "DefaultRegionForTheWorld";
			
			G4Region* region = G4RegionStore::GetInstance()->FindOrCreateRegion(regionName);
			lVol->SetRegion(region);
			region->AddRootLogicalVolume(lVol);
		}
	}

	return lVol;
}


G4VPhysicalVolume* TsVGeometryComponent::CreatePhysicalVolume(G4LogicalVolume* lVol) {
	fSimplestCreateWasCalled = true;
	return CreatePhysicalVolume("", -1, false, lVol, GetRotForPlacement(), GetTransForPlacement(), fParentVolume);
}


G4VPhysicalVolume* TsVGeometryComponent::CreatePhysicalVolume(const char* subComponentName, G4LogicalVolume* lVol, G4VPhysicalVolume* parent) {
	return CreatePhysicalVolume(subComponentName, -1, false, lVol, 0, new G4ThreeVector(), parent);
}


G4VPhysicalVolume* TsVGeometryComponent::CreatePhysicalVolume(G4String& subComponentName, G4LogicalVolume* lVol, G4VPhysicalVolume* parent) {
	return CreatePhysicalVolume(subComponentName, -1, false, lVol, 0, new G4ThreeVector(), parent);
}


G4VPhysicalVolume* TsVGeometryComponent::CreatePhysicalVolume(const char* subComponentName, G4LogicalVolume* lVol,
															  G4RotationMatrix* rot, G4ThreeVector* trans, G4VPhysicalVolume* parent) {
	return CreatePhysicalVolume(subComponentName, -1, false, lVol, rot, trans, parent);
}


G4VPhysicalVolume* TsVGeometryComponent::CreatePhysicalVolume(G4String& subComponentName, G4LogicalVolume* lVol,
															  G4RotationMatrix* rot, G4ThreeVector* trans, G4VPhysicalVolume* parent) {
	return CreatePhysicalVolume(subComponentName, -1, false, lVol, rot, trans, parent);
}


G4VPhysicalVolume* TsVGeometryComponent::CreatePhysicalVolume(const char* subComponentName, G4int copy, G4bool reuseLogical, G4LogicalVolume* lVol,
															  G4RotationMatrix* rot, G4ThreeVector* trans, G4VPhysicalVolume* parent) {
	G4String nameString = subComponentName;
	return CreatePhysicalVolume(nameString, copy, reuseLogical, lVol, rot, trans, parent);
}


G4VPhysicalVolume* TsVGeometryComponent::CreatePhysicalVolume(G4String& subComponentName, G4int copy, G4bool reuseLogical, G4LogicalVolume* lVol,
															  G4RotationMatrix* rot, G4ThreeVector* trans, G4VPhysicalVolume* parent) {
	G4String pVolName = fName + fCopyId;
	if (subComponentName!="") pVolName += "/" + subComponentName;
	if (copy!=-1) pVolName += G4UIcommand::ConvertToString(copy);

	G4int replica;
	if (reuseLogical) replica = copy;
	else replica = 0;

	G4VPhysicalVolume* pvol = new G4PVPlacement(rot, *trans, pVolName, lVol, parent, false, replica, false);

	CheckForOverlaps(pvol);

	fRotations.push_back(rot);
	fTranslations.push_back(trans);
	fPhysicalVolumes.push_back(pvol);
	return pvol;
}


G4VPhysicalVolume* TsVGeometryComponent::CreatePhysicalVolume(const char* subComponentName, G4int copy, G4bool reuseLogical, G4LogicalVolume* lVol,
															  G4RotationMatrix* rot, G4ThreeVector* trans, G4LogicalVolume* parent) {
	G4String nameString = subComponentName;
	return CreatePhysicalVolume(nameString, copy, reuseLogical, lVol, rot, trans, parent);
}


G4VPhysicalVolume* TsVGeometryComponent::CreatePhysicalVolume(G4String& subComponentName, G4int copy, G4bool reuseLogical, G4LogicalVolume* lVol,
															  G4RotationMatrix* rot, G4ThreeVector* trans, G4LogicalVolume* parent) {
	G4String pVolName = fName + fCopyId;
	if (subComponentName!="") pVolName += "/" + subComponentName;
	if (copy!=-1) pVolName += G4UIcommand::ConvertToString(copy);

	G4int replica;
	if (reuseLogical) replica = copy;
	else replica = 0;

	G4VPhysicalVolume* pvol = new G4PVPlacement(rot, *trans, lVol, pVolName, parent, false, replica, false);

	CheckForOverlaps(pvol);

	fRotations.push_back(rot);
	fTranslations.push_back(trans);
	fPhysicalVolumes.push_back(pvol);
	return pvol;
}

G4VPhysicalVolume* TsVGeometryComponent::CreatePhysicalVolume(const char* subComponentName, G4int copy, G4bool reuseLogical, G4LogicalVolume* lVol,
															  G4Transform3D transform, G4VPhysicalVolume* parent) {
	G4String nameString = subComponentName;
	return CreatePhysicalVolume(nameString, copy, reuseLogical, lVol, transform, parent);
}


G4VPhysicalVolume* TsVGeometryComponent::CreatePhysicalVolume(G4String& subComponentName, G4int copy, G4bool reuseLogical, G4LogicalVolume* lVol,
															  G4Transform3D transform, G4VPhysicalVolume* parent) {
	G4String pVolName = fName + fCopyId;
	if (subComponentName!="") pVolName += "/" + subComponentName;
	if (copy!=-1) pVolName += G4UIcommand::ConvertToString(copy);
	
	G4int replica;
	if (reuseLogical) replica = copy;
	else replica = 0;
	
	G4VPhysicalVolume* pvol = new G4PVPlacement(transform, pVolName, lVol, parent, false, replica, false);
	
	CheckForOverlaps(pvol);
	
	fTransforms.push_back(new G4Transform3D(transform));
	fPhysicalVolumes.push_back(pvol);
	return pvol;
}


G4VPhysicalVolume* TsVGeometryComponent::CreatePhysicalVolume(const char* subComponentName, G4LogicalVolume* lVol, G4VPhysicalVolume* parent,
															  const EAxis pAxis, const G4int nReplicas, G4VPVParameterisation* pParam) {
	G4String nameString = subComponentName;
	return CreatePhysicalVolume(nameString,lVol,parent,pAxis,nReplicas,pParam);
}

G4VPhysicalVolume* TsVGeometryComponent::CreatePhysicalVolume(G4String& subComponentName, G4LogicalVolume* lVol, G4VPhysicalVolume* parent,
															  const EAxis pAxis, const G4int nReplicas, G4VPVParameterisation* pParam) {
	G4String pVolName = fName + fCopyId;
	G4String nameString = subComponentName;
	if (nameString!="") pVolName += "/" + nameString;
	G4VPhysicalVolume* pvol = new G4PVParameterised(pVolName, lVol, parent, pAxis, nReplicas, pParam, false);

	CheckForOverlaps(pvol);

	fParameterizations.push_back(pParam);
	fPhysicalVolumes.push_back(pvol);
	return pvol;
}

G4VPhysicalVolume *TsVGeometryComponent::CreatePhysicalVolume(const char *subComponentName, G4LogicalVolume *lVol, G4VPhysicalVolume *parent,
															  const EAxis pAxis, const G4int nReplicas, G4double width, G4double offset)
{
	G4String pVolName = fName + fCopyId;
	G4String nameString = subComponentName;
	if (nameString != "")
		pVolName += "/" + nameString;
	G4VPhysicalVolume *pvol = new G4PVReplica(pVolName, lVol, parent, pAxis, nReplicas, width, offset);
	fPhysicalVolumes.push_back(pvol);
	return pvol;
}


G4bool TsVGeometryComponent::IsParameterized() {
	return fParameterizations.size() > 0;
}


void TsVGeometryComponent::CheckForOverlaps(G4VPhysicalVolume* pvol) {
	if (fPm->GetBooleanParameter("Ge/CheckForOverlaps") &&
		(!fEnvelopePhys || (pvol == fEnvelopePhys) || fPm->GetBooleanParameter("Ge/CheckInsideEnvelopesForOverlaps"))) {
		// The overlap check will at least trigger a warning.
		// Then, will quit or not depending on state of the Ts/QuitIfOverlapDetected parameter.
		G4int resolution;
		if (fPm->ParameterExists(GetFullParmName("CheckForOverlapsResolution")))
			resolution = fPm->GetIntegerParameter(GetFullParmName("CheckForOverlapsResolution"));
		else
			resolution = fPm->GetIntegerParameter("Ge/CheckForOverlapsResolution");

		G4double tolerance;
		if (fPm->ParameterExists(GetFullParmName("CheckForOverlapsTolerance")))
			tolerance = fPm->GetDoubleParameter(GetFullParmName("CheckForOverlapsTolerance"), "Length");
		else
			tolerance = fPm->GetDoubleParameter("Ge/CheckForOverlapsTolerance", "Length");

		if (pvol->CheckOverlaps(resolution, tolerance)) {
			if (!fPm->GetBooleanParameter("Ts/UseQt") && fPm->GetBooleanParameter("Ge/QuitIfOverlapDetected")) {
				G4cerr << "Topas is quitting due to the above geometry overlap problem." << G4endl;
				G4cerr << "Simulation results can not be trusted when any overlap exists." << G4endl;
				G4cerr << "If you still want the TOPAS session to continue" << G4endl;
				G4cerr << "(such as to use visualization to study the overlap),"  << G4endl;
				G4cerr << "Set the parameter Ge/QuitIfOverlapDetected to False" << G4endl;
				fPm->AbortSession(1);
			} else {
				G4cerr << "Since you have set the parameter Ge/QuitIfOverlapDetected to False" << G4endl;
				G4cerr << "Topas is continuing despite the above geometry overlap problem." << G4endl;
				G4cerr << "However simulation results can not be trusted when any overlap exists." << G4endl;
				G4cerr << "In general, the parameter Ge/QuitIfOverlapDetected should be left at its default value of True." << G4endl;
				fPm->NoteGeometryOverlap(true);
			}
		}
	}
}


// Explicitly fill a vector of logical volumes to be sensitive for scoring for sub-components
void TsVGeometryComponent::SetLogicalVolumeToBeSensitive(G4LogicalVolume* lvol){
	fLogicalVolumesToBeSensitive.push_back(lvol);
}


// Component is a parallel scoring copy. Set number of divisions, overriding values that came from the parameter file
void TsVGeometryComponent::SetParallelScoringCopyDivisions(G4int* nDivisions) {
	if (!fIsDividable && (nDivisions[0]!=1 || nDivisions[1]!=1 || nDivisions[2]!=1)) {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "A scorer is attempting to divide a component that is not dividable: " << fName << G4endl;
		fPm->AbortSession(1);
	}

	for (G4int i = 0; i < 3; i++)
		fDivisionCounts[i] = nDivisions[i];

	fIsParallel = true;
	fIsCopy = true;
	fHasVariableMaterial = false;
	fCopyId = fGm->GetCopyIdFromBinning(nDivisions);
}


void TsVGeometryComponent::BeginConstruction() {
	if (fVerbosity>0)
		G4cout << "\nTsVGeometryComponent::BeginConstruction for component: " << GetNameWithCopyId() << G4endl;
	// Tell GeometryManager the name of the component we are currently building.
	fGm -> SetCurrentComponent(this);

	// Issue optional message associated with construction of this component
	if (fPm->ParameterExists(GetFullParmName("Message")))
		G4cout << fPm->GetStringParameter(GetFullParmName("Message")) << G4endl;

	// Some components also contain an additional list of required parameters.
	G4String type = fPm->GetStringParameter(GetFullParmName("Type"));
	G4String requiredParametersName = "Ge/Params/" + type;

	if (fPm->ParameterExists(requiredParametersName)) {
		G4int length = fPm->GetVectorLength(requiredParametersName);
		G4String* values = fPm->GetStringVector(requiredParametersName);
		for (G4int iValue = 0; iValue < length; iValue++)
			if (!fPm->ParameterExists(GetFullParmName(values[iValue])))
				Quit(GetFullParmName(values[iValue]), "Unable to find this parameter needed by a Geometry Component");
	}

	// See if parameter system says this component should be in a parallel world
	if (fPm->ParameterExists(GetFullParmName("IsParallel")) && fPm->GetBooleanParameter(GetFullParmName("IsParallel")))
		fIsParallel = true;

	// Components in mass world are forbidden to have parent components in a parallel world
	if ( !fIsParallel && fParentComponent && fParentComponent->IsParallel() ) {
		G4cerr << "Topas is exiting due to a serious error attempting to construct component: " << fName << G4endl;
		G4cerr << "Components in mass world are forbidden to have parent components in a parallel world" << G4endl;
		fPm->AbortSession(1);
	}

	// If component is in parallel world but its parent is in mass world, create a new parallel world to hold component
	// All copy components also have their own parallel world.
	if ( fIsParallel && !fParentComponent->IsParallel() ) {
		// if is not a copy component, allow user to optionally specify the parallel world name in a parameter
		fWorldName = GetNameWithCopyId();
		if (!fIsCopy && fPm->ParameterExists(GetFullParmName("ParallelWorldName")))
			fWorldName = fPm->GetStringParameter(GetFullParmName("ParallelWorldName"));

		G4VPhysicalVolume* parallelWorldVolume = fGm->CreateParallelWorld(fWorldName, fIsGroup);

		parallelWorldVolume->GetLogicalVolume()->SetVisAttributes(fPm->GetInvisible());
		fParentVolume = parallelWorldVolume;
		fParentComponent->MarkAsHavingParallelWorldDescendent();
	} else if (fParentComponent) {
		fWorldName = fParentComponent->GetWorldName();
	} else {
		fWorldName = "World";
	}

	// Calculate placement information for the component before creating this first volume.
	CalculatePlacement();

	fNeedToRebuild = false;
}


void TsVGeometryComponent::MarkAsHavingParallelWorldDescendent() {
	if (!fHasParallelWorldDescendents) {
		fHasParallelWorldDescendents = true;
		if (fParentComponent)
			fParentComponent->MarkAsHavingParallelWorldDescendent();
	}
}


// Store cummulative translation and rotation information
void TsVGeometryComponent::CalculatePlacement() {
	if (fVerbosity>0)
		G4cout << "CalculatePlacement for: " << GetNameWithCopyId() << G4endl;
	if (fParentVolume) {
		// This is not the top component.
		// Start with its placement relative to parent component.
		fRotRelToParentComponent = GetRotRelToParentComponent();
		fTransRelToParentComponent = GetTransRelToParentComponent();

		// Get rotation relative to world by incorporating parent component's rotation relative to world
		fRotRelToWorld = new G4RotationMatrix(*fRotRelToParentComponent * *(fParentComponent->GetRotRelToWorld()));

		// Get translation relative to world by incorporating parent component's translation relative to world
		fTransRelToWorld = new G4Point3D(*(fParentComponent->GetTransRelToWorld()));
		G4RotationMatrix* invMatrix1 = new G4RotationMatrix(fParentComponent->GetRotRelToWorld()->inverse());
		*fTransRelToWorld = G4Translate3D(*invMatrix1 * *fTransRelToParentComponent) * (*fTransRelToWorld);
		delete invMatrix1;

		if (fIsGroup) {
			// For group components, accumulate total rotation and translation since last non-group component
			// Get rotation since last non-group component by incorporating parent component's rotation since last non-group component
			G4RotationMatrix* rot;
			G4Point3D* trans;
			if (fIsParallel && !fParentComponent->IsParallel()) {
				rot = fParentComponent->fRotRelToWorld;
				trans = fParentComponent->fTransRelToWorld;
			} else {
				rot = fParentComponent->fRotRelToLastNonGroupComponent;
				trans = fParentComponent->fTransRelToLastNonGroupComponent;
			}

			fRotRelToLastNonGroupComponent = new G4RotationMatrix(*fRotRelToParentComponent * *rot);

			// Get translation since last non-group component by incorporating parent component's translation since last non-group component
			fTransRelToLastNonGroupComponent = new G4Point3D(*trans);
			G4RotationMatrix* invMatrix2 = new G4RotationMatrix(rot->inverse());
			*fTransRelToLastNonGroupComponent = G4Translate3D(*invMatrix2 * *fTransRelToParentComponent) * (*fTransRelToLastNonGroupComponent);
			delete invMatrix2;
		} else {
			// For non-group components, this information is zero
			fRotRelToLastNonGroupComponent = new G4RotationMatrix();
			fTransRelToLastNonGroupComponent = new G4Point3D();
		}
	} else {
		// For top volume, cumulative information is all zero
		fRotRelToWorld = new G4RotationMatrix();
		fTransRelToWorld = new G4Point3D();
		fRotRelToLastNonGroupComponent = new G4RotationMatrix();
		fTransRelToLastNonGroupComponent = new G4Point3D();
		fRotRelToParentComponent = new G4RotationMatrix();
		fTransRelToParentComponent = new G4ThreeVector();
	}

	fNeedToUpdatePlacement = false;
}


G4RotationMatrix* TsVGeometryComponent::GetRotRelToWorld() {
	return fRotRelToWorld;
}


G4Point3D* TsVGeometryComponent::GetTransRelToWorld() {
	return fTransRelToWorld;
}


// Get rotation matrix of this component relative to mother component
G4RotationMatrix* TsVGeometryComponent::GetRotRelToParentComponent() {
	G4double rotX, rotY, rotZ;

	if (fPm->ParameterExists(GetFullParmName("RotX")))
		rotX = fPm->GetDoubleParameter(GetFullParmName("RotX"), "Angle");
	else
		rotX = 0.;

	if (fPm->ParameterExists(GetFullParmName("RotY")))
		rotY = fPm->GetDoubleParameter(GetFullParmName("RotY"), "Angle");
	else
		rotY = 0.;

	if (fPm->ParameterExists(GetFullParmName("RotZ")))
		rotZ = fPm->GetDoubleParameter(GetFullParmName("RotZ"), "Angle");
	else
		rotZ = 0.;

	G4RotationMatrix* rm = new G4RotationMatrix();
	rm->rotateX(rotX);
	rm->rotateY(rotY);
	rm->rotateZ(rotZ);
	return rm;
}


// Get translation vector of this component relative to mother component
G4ThreeVector* TsVGeometryComponent::GetTransRelToParentComponent() {
	G4double transX, transY, transZ;

	if (fPm->ParameterExists(GetFullParmName("TransX")))
		transX = fPm->GetDoubleParameter(GetFullParmName("TransX"), "Length");
	else
		transX = 0.;

	if (fPm->ParameterExists(GetFullParmName("TransY")))
		transY = fPm->GetDoubleParameter(GetFullParmName("TransY"), "Length");
	else
		transY = 0.;

	if (fPm->ParameterExists(GetFullParmName("TransZ")))
		transZ = fPm->GetDoubleParameter(GetFullParmName("TransZ"), "Length");
	else
		transZ = 0.;

	return new G4ThreeVector(transX, transY, transZ);
}


// Get rotation matrix to use in placing this component in its mother physical volume.
// If parent component is not a group component, this is just rotation relative to parent component.
// If parent component is a group component, this will also include rotation of parent group since last non-group component.
G4RotationMatrix* TsVGeometryComponent::GetRotForPlacement() {
	if (fParentComponent && fParentComponent->fIsGroup) {
		// Current component is child of a group component.
		// Rotation must include current component's rotation plus rotation of the group.
		return new G4RotationMatrix(*fRotRelToParentComponent * *(fParentComponent->fRotRelToLastNonGroupComponent));
	} else if (fIsParallel && !fParentComponent->IsParallel()) {
		// Current component is in parallel world with parent component in mass world.
		// Rotation must include current component's rotation plus world coordinate rotation of parent component.
		return new G4RotationMatrix(*fRotRelToParentComponent * *(fParentComponent->GetRotRelToWorld()));
	} else {
		// Just return rotation relative to parent component.
		return fRotRelToParentComponent;
	}
}


// Get translation vector to use in placing this component in its mother physical volume.
// If parent component is not a group component, this is just translation relative to parent component.
// If parent component is a group component, this will also include translation of parent group since last non-group component.
G4ThreeVector* TsVGeometryComponent::GetTransForPlacement() {
	if (fParentComponent && fParentComponent->fIsGroup) {
		// Current component is child of a group component.
		// Translation must include current component's translation plus translation of the group.
		G4Point3D* lastPoint = new G4Point3D(*(fParentComponent->fTransRelToLastNonGroupComponent));
		G4Point3D* newPoint = new G4Point3D();
		G4RotationMatrix* invMatrix = new G4RotationMatrix(fParentComponent->fRotRelToLastNonGroupComponent->inverse());
		*newPoint = G4Translate3D(*invMatrix * *fTransRelToParentComponent) * *newPoint;
		G4ThreeVector* result = new G4ThreeVector(lastPoint->x()+newPoint->x(), lastPoint->y()+newPoint->y(), lastPoint->z()+newPoint->z());
		delete invMatrix;
		delete newPoint;
		delete lastPoint;
		return result;
	} else if (fIsParallel && !fParentComponent->IsParallel()) {
		// Current component is in parallel world with parent component in mass world.
		// Translation must include current component's translation plus world coordinate translation of parent component.
		G4Point3D* lastPoint = new G4Point3D(*(fParentComponent->fTransRelToWorld));
		G4Point3D* newPoint = new G4Point3D();
		G4RotationMatrix* invMatrix = new G4RotationMatrix(fParentComponent->fRotRelToWorld->inverse());
		*newPoint = G4Translate3D(*invMatrix * *fTransRelToParentComponent) * *newPoint;
		G4ThreeVector* result = new G4ThreeVector(lastPoint->x()+newPoint->x(), lastPoint->y()+newPoint->y(), lastPoint->z()+newPoint->z());
		delete invMatrix;
		delete newPoint;
		delete lastPoint;
		return result;
	} else {
		// Just return translation relative to parent component.
		return fTransRelToParentComponent;
	}
}


G4VisAttributes* TsVGeometryComponent::GetVisAttributes(G4String subComponentName) {

	if (fPm->ParameterExists(GetFullParmName(subComponentName, "Invisible")) &&
		fPm->GetBooleanParameter(GetFullParmName(subComponentName, "Invisible")))
		return new G4VisAttributes(false);

	G4VisAttributes* visAtt	= new G4VisAttributes();

	// If color is explicitly specified, use it, otherwise use the color specified for this material,
	// and if no color is specified for the material, use Grey.
	if (fPm->ParameterExists(GetFullParmName(subComponentName, "Color")))
		visAtt->SetColor( fPm->GetColor(fPm->GetStringParameter(GetFullParmName(subComponentName, "Color")))->GetColor() );
	else {
		G4String resolvedMaterialName = GetResolvedMaterialName(subComponentName, true);
		if (resolvedMaterialName!="")
			visAtt->SetColor(fPm->GetColor(fMm->GetDefaultMaterialColor(resolvedMaterialName))->GetColor());
		else
			visAtt->SetColor( fPm->GetColor("grey")->GetColor() );
	}

	if ( fPm->ParameterExists(GetFullParmName(subComponentName, "DrawingStyle")) ) {
		G4String style = fPm->GetStringParameter(GetFullParmName(subComponentName, "DrawingStyle"));
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(style);
#else
		style.toLower();
#endif
		if ( style == "solid" )
			visAtt->SetForceSolid(true);
		else if ( style == "wireframe")
			visAtt->SetForceWireframe(true);
		else if ( style == "fullwireframe")
			visAtt->SetForceAuxEdgeVisible(true);
		else if ( style == "cloud") {
			visAtt->SetForceCloud(true);
			if ( fPm->ParameterExists(GetFullParmName(subComponentName, "NumberOfCloudPoints")) )
				visAtt->SetForceNumberOfCloudPoints(fPm->GetIntegerParameter(GetFullParmName(subComponentName, "NumberOfCloudPoints")));
		}
		else {
			G4cerr << "Topas is exiting due to a serious error attempting to construct component: " << fName << G4endl;
			G4cerr << "DrawingStyle has unknown value: " << fPm->GetStringParameter(GetFullParmName(subComponentName, "DrawingStyle")) << G4endl;
			fPm->AbortSession(1);
		}
	} else {
		visAtt->SetForceAuxEdgeVisible(true);
	}

	if (fPm->ParameterExists(GetFullParmName(subComponentName, "VisSegsPerCircle")))
		visAtt->SetForceLineSegmentsPerCircle(fPm->GetIntegerParameter(GetFullParmName(subComponentName, "VisSegsPerCircle")));

	return visAtt;
}


G4String TsVGeometryComponent::GetName() {
	return fName;
}


G4String TsVGeometryComponent::GetNameWithCopyId() {
	G4String nameWithCopyId = fName + fCopyId;
	return nameWithCopyId;
}


TsVGeometryComponent* TsVGeometryComponent::GetOriginalComponent() {
	G4String originalComponentName = GetName();
	return fGm->GetComponent(originalComponentName);
}


G4bool TsVGeometryComponent::SimplestCreateWasCalled() {
	return fSimplestCreateWasCalled;
}


G4VPhysicalVolume* TsVGeometryComponent::GetParentVolume() {
	return fParentVolume;
}


G4VPhysicalVolume* TsVGeometryComponent::GetPhysicalVolume(G4String& name)
{
	std::vector<G4VPhysicalVolume*>::iterator iter;
	for (iter=fPhysicalVolumes.begin(); iter!=fPhysicalVolumes.end(); iter++)
		if ((*iter)->GetName() == name)
			return *iter;
	return 0;
}


std::vector<G4VPhysicalVolume*> TsVGeometryComponent::GetAllPhysicalVolumes(G4bool recursivelyIncludeSubcomponents)
{
	if (recursivelyIncludeSubcomponents) {
		std::vector<G4VPhysicalVolume*>* volumes = new std::vector<G4VPhysicalVolume*>;
		RecursivelyAddVolumes(volumes);
		return *volumes;
	} else {
		return fPhysicalVolumes;
	}
}


void TsVGeometryComponent::RecursivelyAddVolumes(std::vector<G4VPhysicalVolume*>* volumes) {
	std::vector<G4VPhysicalVolume*>::iterator PVIter;
	for (PVIter=fPhysicalVolumes.begin(); PVIter!=fPhysicalVolumes.end(); PVIter++)
		volumes->push_back((*PVIter));

	std::vector<TsVGeometryComponent*>::iterator ChildIter;
	for (ChildIter=fChildren.begin(); ChildIter!=fChildren.end(); ChildIter++)
		(*ChildIter)->RecursivelyAddVolumes(volumes);
}


std::vector<G4LogicalVolume*> TsVGeometryComponent::GetLogicalVolumesToBeSensitive()
{
	return fLogicalVolumesToBeSensitive;
}


void TsVGeometryComponent::SetVisAttribute(G4VisAttributes* visAtt) {
	fEnvelopeLog->SetVisAttributes(visAtt);
	delete(fVisAtts[0]);
	fVisAtts[0] = visAtt;
}


void TsVGeometryComponent::UpdateForSpecificParameterChange(G4String parameter) {
	if (fVerbosity>0)
		G4cout << "TsVGeometryComponent::Update called for parameter: " << parameter << " for component: " << GetNameWithCopyId() << G4endl;
	if (!fPm->GetBooleanParameter("Ts/FullRebuildTestMode") &&
		(parameter == GetFullParmNameLower("TransX") || parameter == GetFullParmNameLower("TransY") || parameter == GetFullParmNameLower("TransZ") ||
		 parameter == GetFullParmNameLower("RotX") || parameter == GetFullParmNameLower("RotY") || parameter == GetFullParmNameLower("RotZ"))) {
			MarkAsNeedToUpdatePlacement();
		} else if (parameter == GetFullParmNameLower("Material") &&
			fDivisionCounts[0]==1 && fDivisionCounts[1]==1 && fDivisionCounts[2]==1) {
			G4String materialName = fPm->GetStringParameter(parameter);
			fEnvelopeLog->SetMaterial(fMm->GetMaterial(materialName));
		} else if ((!fPm->GetBooleanParameter("Ts/FullRebuildTestMode")) &&
				   ( (parameter == GetFullParmNameLower("Color")) || (parameter == GetFullParmNameLower("DrawingStyle")) ||
					 (parameter == GetFullParmNameLower("VisSegsPerCircle")) || (parameter == GetFullParmNameLower("Invisible")) ) ) {
			G4VisAttributes* visAtt = GetVisAttributes("");
			fEnvelopeLog->SetVisAttributes(visAtt);

			// Need to do update attributes for any subvolumes as well
			std::vector<G4LogicalVolume*>::iterator LVIter;
			for (LVIter=fLogicalVolumes.begin(); LVIter!=fLogicalVolumes.end(); LVIter++)
				(*LVIter)->SetVisAttributes(visAtt);

			delete(fVisAtts[0]);
			fVisAtts[0] = visAtt;
		} else {
			if (fVerbosity>0)
				G4cout << "Setting fNeedToRebuild true" << G4endl;
			fNeedToRebuild = true;
		}
}


void TsVGeometryComponent::UpdateForSpecificMagneticFieldParameterChange(G4String parameter) {
	if (fVerbosity>0)
		G4cout << "TsVGeometryComponent::Update called for magnetic field parameter: " << parameter << " for component: " << GetNameWithCopyId() << G4endl;
	std::vector<TsVMagneticField*>::iterator MagneticFieldIter;
	for (MagneticFieldIter=fMagneticFields.begin(); MagneticFieldIter!=fMagneticFields.end(); MagneticFieldIter++)
		(*MagneticFieldIter)->UpdateForSpecificParameterChange(parameter);
}


void TsVGeometryComponent::UpdateForSpecificElectroMagneticFieldParameterChange(G4String parameter) {
	if (fVerbosity>0)
		G4cout << "TsVGeometryComponent::Update called for electric field parameter: " << parameter << " for component: " << GetNameWithCopyId() << G4endl;
	std::vector<TsVElectroMagneticField*>::iterator ElectroMagneticFieldIter;
	for (ElectroMagneticFieldIter=fElectroMagneticFields.begin(); ElectroMagneticFieldIter!=fElectroMagneticFields.end(); ElectroMagneticFieldIter++)
		(*ElectroMagneticFieldIter)->UpdateForSpecificParameterChange(parameter);
}


void TsVGeometryComponent::MarkAsNeedToUpdatePlacement() {
	if (fVerbosity>0)
		G4cout << "TsVGeometryComponent::MarkAsNeedToUpdatePlacement Setting fNeedToUpdatePlacement true for : " << GetNameWithCopyId() << G4endl;
	fNeedToUpdatePlacement = true;

	// There are three cases where movement must be propagated down the tree.
	// If parent is a group, children need to update so they can take into account placement of the parent.
	// If child is parallel and parent is not, child needs to update to take into account placement of the parent.
	// If child is not parallel, but has parallel descendents, child needs to update so update placement can propagate
	// down to the parallel descendent.
	std::vector<TsVGeometryComponent*>::iterator ChildIter;
	for (ChildIter=fChildren.begin(); ChildIter!=fChildren.end(); ChildIter++)
		if (fIsGroup || (!fIsParallel && (*ChildIter)->IsParallel()) || (*ChildIter)->HasParallelWorldDescendents())
			(*ChildIter)->MarkAsNeedToUpdatePlacement();
}


G4bool TsVGeometryComponent::HasParallelWorldDescendents() {
	return fHasParallelWorldDescendents;
}


G4bool TsVGeometryComponent::RebuildIfNeeded() {
	G4bool rebuiltSomeComponents = false;

	if (fNeedToRebuild) {
		if (fVerbosity>0)
			G4cout << "Rebuilding: " << GetNameWithCopyId() << G4endl;
		DeleteContents();
		Construct();
		InstantiateFields();
		AddOpticalSurfaces();
		fGm->SetCurrentComponent(0);
		MarkAllVolumesForReoptimize();
		rebuiltSomeComponents = true;
	} else {
		std::vector<TsVGeometryComponent*>::iterator ChildIter;
		for (ChildIter=fChildren.begin(); ChildIter!=fChildren.end(); ChildIter++)
			if ((*ChildIter)->RebuildIfNeeded())
				rebuiltSomeComponents = true;
	}

	return rebuiltSomeComponents;
}


void TsVGeometryComponent::UpdateForNewRun(G4bool force) {
	if (fVerbosity>0)
		G4cout << "TsVGeometryComponent::UpdateForNewRun called for component: " << GetNameWithCopyId() << " with need: " << fNeedToUpdatePlacement << G4endl;

	// Never update the World component even if in force mode
	if (fParentComponent && ( fNeedToUpdatePlacement || force ) ) {
		CalculatePlacement();
		fExtent = G4VisExtent::GetNullExtent();
		if (!fIsGroup) {
			delete(fRotations[0]);
			delete(fTranslations[0]);
			fRotations[0] = GetRotForPlacement();
			fTranslations[0] = GetTransForPlacement();
			fEnvelopePhys->SetRotation(fRotations[0]);
			fEnvelopePhys->SetTranslation(*fTranslations[0]);
			if (fVerbosity>0)
				G4cout << "AddToReoptimizeList: " << fParentVolume->GetLogicalVolume()->GetName() << G4endl;
			fGm->AddToReoptimizeList(fParentVolume->GetLogicalVolume());
		}
	}

	std::vector<TsVMagneticField*>::iterator MagneticFieldIter;
	for (MagneticFieldIter=fMagneticFields.begin(); MagneticFieldIter!=fMagneticFields.end(); MagneticFieldIter++)
		(*MagneticFieldIter)->UpdateForNewRun();

	std::vector<TsVElectroMagneticField*>::iterator ElectroMagneticFieldIter;
	for (ElectroMagneticFieldIter=fElectroMagneticFields.begin(); ElectroMagneticFieldIter!=fElectroMagneticFields.end(); ElectroMagneticFieldIter++)
		(*ElectroMagneticFieldIter)->UpdateForNewRun();

	std::vector<TsVGeometryComponent*>::iterator ChildIter;
	for (ChildIter=fChildren.begin(); ChildIter!=fChildren.end(); ChildIter++)
		(*ChildIter)->UpdateForNewRun(force);
}


void TsVGeometryComponent::MarkAllVolumesForReoptimize() {
	// Mark all volumes to reoptimize
	std::vector<G4LogicalVolume*>::iterator LVIter;
	for (LVIter=fLogicalVolumes.begin(); LVIter!=fLogicalVolumes.end(); LVIter++)
		fGm->AddToReoptimizeList(*LVIter);

	// Call this method recursively for all child volumes
	std::vector<TsVGeometryComponent*>::iterator ChildIter;
	for (ChildIter=fChildren.begin(); ChildIter!=fChildren.end(); ChildIter++)
		(*ChildIter)->MarkAllVolumesForReoptimize();
}


TsVGeometryComponent::SurfaceType TsVGeometryComponent::GetSurfaceID(G4String surfaceName) {
	G4String surfaceNameLower = surfaceName;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(surfaceNameLower);
#else
	surfaceNameLower.toLower();
#endif
	if (surfaceNameLower!="anysurface") {
		G4cerr << "Topas is exiting due to a serious error." << G4endl;
		G4cerr << "TsVGeometryComponent::GetSurfaceID called for surface: " << surfaceName <<
		" of a component that does not have any specific surfaces designated: " << fName << G4endl;
		fPm->AbortSession(1);
	}
	return AnySurface;
}


// Return the scoring volume for the given component.
G4LogicalVolume* TsVGeometryComponent::GetScoringVolume()	{

	// If component has specifically set a scoring volume, use it.
	// This will typically be something like the individual voxels within a TsBox.
	if (fScoringVolume)
		return fScoringVolume;

	// Otherwise, return the top logical volume
	return fEnvelopeLog;
}


G4int TsVGeometryComponent::GetIndex(G4Step*) {
	return 0;
}


G4int TsVGeometryComponent::GetIndex(G4int, G4int, G4int) {
	return 0;
}


G4int TsVGeometryComponent::GetBin(G4int, G4int) {
	return 0;
}


G4bool TsVGeometryComponent::IsOnBoundary(G4ThreeVector, G4VSolid*, SurfaceType) {
	return true;
}


G4double TsVGeometryComponent::GetAreaOfSelectedSurface(G4VSolid*, SurfaceType, G4int, G4int, G4int) {
	G4cerr << "Topas is exiting due to a serious error." << G4endl;
	G4cerr << "TsVGeometryComponent::GetAreaOfSelectedSurface called for component that does not handle surface scoring: " << fName << G4endl;
	fPm->AbortSession(1);
	return 0.;
}


G4int TsVGeometryComponent::GetDivisionCount(G4int index) {
	return fDivisionCounts[index];
}


G4String TsVGeometryComponent::GetDivisionName(G4int index) {
	return fDivisionNames[index];
}


G4double TsVGeometryComponent::GetFullWidth(G4int index) {
	return fFullWidths[index];
}


// Formats header lines such as: "X in 4 bins of 0.5 cm" or  "Phi in 18 bins of 10. deg"
G4String TsVGeometryComponent::GetBinHeader(G4int index, G4int nBins) {
	G4String header = fDivisionNames[index];

	if (nBins == 1)
		header += " in 1 bin  of ";
	else
		header += " in " +  G4UIcommand::ConvertToString(nBins) + " bins of ";

	G4double binWidth = fFullWidths[index] /  G4UIcommand::ValueOf(fDivisionUnits[index]) / nBins;
	header += G4UIcommand::ConvertToString(binWidth) + " " + fDivisionUnits[index];

	return header;
}


void TsVGeometryComponent::SetMaterialList(std::vector<G4Material*>* materialList) {
	fHasVariableMaterial = true;
	fMaterialList = materialList;
}


void TsVGeometryComponent::SetMaterialIndex(std::vector<unsigned short>* materialIndex) {
	fCurrentMaterialIndex = materialIndex;
}


// Methods handled for the parameterization
G4int TsVGeometryComponent::GetNumberOfMaterials() const {
	return fMaterialList->size();
}


G4Material* TsVGeometryComponent::GetMaterialInVoxel(G4int idx) const {
	return (*fMaterialList)[idx];
}


G4Material* TsVGeometryComponent::ComputeMaterial(const G4int, G4VPhysicalVolume* pvol, const G4VTouchable*) {
	return pvol->GetLogicalVolume()->GetMaterial();
}


void TsVGeometryComponent::ComputeTransformation(const G4int, G4VPhysicalVolume*) const {
}

void TsVGeometryComponent::ComputeDimensions(G4Tubs&, const G4int, const G4VPhysicalVolume*) const {
}

void TsVGeometryComponent::ComputeDimensions(G4Sphere&, const G4int, const G4VPhysicalVolume*) const {
}


void TsVGeometryComponent::RegisterVisAtt(G4VisAttributes* attribute) {
	fVisAtts.push_back(attribute);
}


G4int TsVGeometryComponent::GetStructureID(G4String structureName) {
	if (fOriginalComponent)
		return OriginalComponent()->GetStructureID(structureName);

	for (G4int i=0; i < (int)fStructureNames.size(); i++) {
		if (structureName==fStructureNames[i])
			return i;
	}
	return -1;
}


G4bool TsVGeometryComponent::IsInNamedStructure(G4int structureID, const G4Step* aStep) {
	if (fOriginalComponent) {
		// Excude cases where corresponding step in mass world comes from the Y replica rather than
		// the Z level parameterization.
		if (aStep->GetTrack()->GetStep()->GetPreStepPoint()->GetTouchable()->GetVolume()->IsParameterised())
			return fOriginalComponent->IsInNamedStructure(structureID, fOriginalComponent->GetIndex((G4Step*)(aStep->GetTrack()->GetStep())));
		else
			return false;
	}
	return IsInNamedStructure(structureID, GetIndex((G4Step*)aStep));
}


G4bool TsVGeometryComponent::IsInNamedStructure(G4int structureID, G4int index) {
	if (fOriginalComponent) {
		G4cerr << "Topas is exiting due to a serious error in scoring or filtering." << G4endl;
		G4cerr << "TsVGeometryComponent::IsInNamedStructure(G4int structureID, G4int index)" << G4endl;
		G4cerr << "has been called for a component that is a parallel scoring copy." << G4endl;
		fPm->AbortSession(1);
	}
	return fIsInNamedStructure[fImageIndex][structureID][index];
}


void TsVGeometryComponent::OutOfRange(G4String parameterName, G4String requirement) {
	G4cerr << "Topas is exiting due to a serious error." << G4endl;
	G4cerr << parameterName << " " << requirement << G4endl;
	fPm->AbortSession(1);
}


void  TsVGeometryComponent::SetTooComplexForOGLS() {
	fGm->SetTooComplexForOGLS();
}

void  TsVGeometryComponent::SetHasDividedCylinderOrSphere() {
	fGm->SetHasDividedCylinderOrSphere();
}

std::vector<G4double> TsVGeometryComponent::Logspace(G4double startValue, G4double endValue, G4int num, G4double base)
{
	if (num == 0)
		return std::vector<G4double>();
	std::vector<G4double> vals(num);

	const G4double arg_start = startValue == 0 ? 0.0 : log10(startValue) / log10(base);
	const G4double arg_end = log10(endValue) / log10(base);
	const G4double arg_delta = (arg_end - arg_start) / (num - 1);

	vals[0] = startValue;
	for (auto i = 1; i < num - 1; ++i)
		vals[i] = pow(base, arg_start + arg_delta * i);
	vals[num - 1] = endValue;

	return vals;
}

void TsVGeometryComponent::AddToReoptimizeList(G4LogicalVolume* volume) {
	fGm->AddToReoptimizeList(volume);
}

G4Material* TsVGeometryComponent::GetMaterial(G4String name) {
	return fMm->GetMaterial(name);
}

G4Material* TsVGeometryComponent::GetMaterial(const char* c) {
	return fMm->GetMaterial(c);
}


const G4VisExtent& TsVGeometryComponent::GetExtent()
{
	// We need to lock this because it may be accessed from multiple threads
	static G4Mutex mutex = G4MUTEX_INITIALIZER;
	G4AutoLock al(&mutex);
	if (fExtent == G4VisExtent::GetNullExtent()) {
		G4BoundingExtentScene tmpScene;
		AddVolumesToScene(tmpScene);
		fExtent = tmpScene.GetExtent();
	}
	return fExtent;
}


G4String TsVGeometryComponent::GetDefaultMaterialColor(G4String name) {
	return fMm->GetDefaultMaterialColor(name);
}


G4String TsVGeometryComponent::GetDefaultMaterialColor(const char* c) {
	return fMm->GetDefaultMaterialColor(c);
}


void TsVGeometryComponent::AddVolumesToScene(G4BoundingExtentScene& scene) {
	if (fIsGroup) {
		std::vector<TsVGeometryComponent*>::iterator ChildIter;
		for (ChildIter=fChildren.begin(); ChildIter!=fChildren.end(); ChildIter++)
			(*ChildIter)->AddVolumesToScene(scene);
	} else {
		const G4Transform3D& transform3D = G4Transform3D(fRotRelToWorld->inverse(), *fTransRelToWorld);
		G4PhysicalVolumeModel pvModel(fEnvelopePhys, 0, transform3D, 0, true);
#if GEANT4_VERSION_MAJOR >= 11
		const G4VisExtent& thisExtent = pvModel.GetExtent();
#else
		const G4VisExtent& thisExtent = pvModel.GetTransformedExtent();
#endif
		scene.AccrueBoundingExtent(thisExtent);
	}
}


G4bool TsVGeometryComponent::CanCalculateSurfaceArea() {
	return fCanCalculateSurfaceArea;
}


void TsVGeometryComponent::Quit(const G4String& name, const char* message) {
	G4cerr << "Topas is exiting due to a serious error in geometry setup." << G4endl;
	G4cerr << "Parameter name: " << name << G4endl;
	G4cerr << message << G4endl;
	fPm->AbortSession(1);
}
