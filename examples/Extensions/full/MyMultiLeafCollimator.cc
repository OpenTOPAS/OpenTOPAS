// Component for MyMultiLeafCollimator
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

#include "MyMultiLeafCollimator.hh"

#include "TsParameterManager.hh"

#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"

MyMultiLeafCollimator::MyMultiLeafCollimator(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
								 TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name)
	: TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name)
{;}

MyMultiLeafCollimator::~MyMultiLeafCollimator()
{;}


void MyMultiLeafCollimator::UpdateForSpecificParameterChange(G4String parameter)
{
	if (parameter == GetFullParmNameLower("XPlusLeavesOpen") || parameter == GetFullParmNameLower("XMinusLeavesOpen")) {
		XPlusLeavesOpen  = fPm->GetDoubleVector(GetFullParmName("XPlusLeavesOpen"), "Length");
		XMinusLeavesOpen = fPm->GetDoubleVector(GetFullParmName("XMinusLeavesOpen"), "Length");
		G4ThreeVector xpl;
		G4ThreeVector xml;
		for (int i= 0; i < NbOfLeavesPerSide; ++i) {
			if ( (XPlusLeavesOpen[i] - XMinusLeavesOpen[i]) < 0.0 ) {
				G4cerr << "Detected Leaf collision at "<< i <<" th leaf: XPlusLeaf "
				<< XPlusLeavesOpen[i]/cm << " (cm), XMinusLeaf "
				<< XMinusLeavesOpen[i]/cm << " (cm) "<< G4endl;
				exit(1);
			}

			xpl = (XPlusLeaves[i])->GetTranslation();
			xml = (XMinusLeaves[i])->GetTranslation();
			xpl.setX(LeafHalfLength + XPlusLeavesOpen[i]);
			xml.setX((-1)*LeafHalfLength + XMinusLeavesOpen[i]);
			(XPlusLeaves[i])->SetTranslation(xpl);
			(XMinusLeaves[i])->SetTranslation(xml);
		}
		AddToReoptimizeList(fEnvelopeLog);
	} else {
		// For any other parameters, fall back to the base class Update method
		TsVGeometryComponent::UpdateForSpecificParameterChange(parameter);
	}
}


G4VPhysicalVolume* MyMultiLeafCollimator::Construct()
{
	BeginConstruction();
	XPlusLeaves.clear();
	XMinusLeaves.clear();
	//Count leaves
	NbOfLeavesPerSide = fPm->GetVectorLength(GetFullParmName("Widths"));
	const G4int n_xl = fPm->GetVectorLength(GetFullParmName("XPlusLeavesOpen"));
	const G4int n_xr = fPm->GetVectorLength(GetFullParmName("XMinusLeavesOpen"));
	if (NbOfLeavesPerSide != n_xl) {
		G4cerr << "Number of Width does not Match to XPlusLeaves: Widths = " << NbOfLeavesPerSide << ", XPlusLeavesOpen = "<< n_xl << G4endl;
		exit(1);
	} else {
		if (NbOfLeavesPerSide != n_xr) {
				G4cerr << "Number of Width does not Match to XMinusLeaves: Widths = " << NbOfLeavesPerSide << ", XPlusLeavesOpen = "<< n_xr << G4endl;
				exit(1);
		}
	}

	//LeafHalfLength: x of leaf, leaf_thickness: z of leaf, and vt: various y of leaf
	LeafHalfLength          = (0.5)*fPm->GetDoubleParameter(GetFullParmName("Length"), "Length"); //x
	G4double leaf_thickness = fPm->GetDoubleParameter(GetFullParmName("Thickness"), "Length"); //z
	G4double*     vt        = fPm->GetDoubleVector(GetFullParmName("Widths"), "Length");  //y
	if ( fPm->ParameterExists(GetFullParmName("MaximumLeafOpen")) ) {
	MaximumLeafOpen = fPm->GetDoubleParameter(GetFullParmName("MaximumLeafOpen"), "Length");
	} else {
	MaximumLeafOpen = 2.0*LeafHalfLength;
	}

	XPlusLeavesOpen  = fPm->GetDoubleVector(GetFullParmName("XPlusLeavesOpen"), "Length");
	XMinusLeavesOpen = fPm->GetDoubleVector(GetFullParmName("XMinusLeavesOpen"), "Length");

	G4double y_sum = 0.0;
	for (G4int i = 0; i < NbOfLeavesPerSide; ++i) {
	y_sum += vt[i];
	//Prevent leaf open over its limit.
	if ( fabs(XPlusLeavesOpen[i]) > MaximumLeafOpen || fabs(XMinusLeavesOpen[i]) > MaximumLeafOpen) {
			G4cerr << i << " th Leaf tried to open more than the limit: " << MaximumLeafOpen/cm << " (cm)" << G4endl;
			exit(1);
	}
	//Leaf collision detection.
	if ( (XPlusLeavesOpen[i] - XMinusLeavesOpen[i]) < 0.0 ) {
			G4cerr << "Detected Leaf collision at "<< i <<" th leaf: XPlusLeaf "
						 << XPlusLeavesOpen[i]/cm << " (cm), XMinusLeaf "
						 << XMinusLeavesOpen[i]/cm << " (cm) "<< G4endl;
			exit(1);
	}
	}

	G4String envelopeMaterialName = fParentComponent->GetResolvedMaterialName();
	G4Box* svWholeBox = new G4Box(fName, LeafHalfLength*2 + MaximumLeafOpen, 0.5*y_sum, 0.5*leaf_thickness);
	fEnvelopeLog = CreateLogicalVolume(fName, envelopeMaterialName, svWholeBox);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

	G4double y_cpos = (0.5*y_sum);
	for (G4int i = 0; i < NbOfLeavesPerSide; ++i)
	{
	if (i == 0) y_cpos = y_cpos - 0.5 * vt[i];
	else        y_cpos = y_cpos - 0.5 * (vt[i-1] + vt[i]);

	G4String id_string = G4UIcommand::ConvertToString(i);
	G4Box* svLeaf = new G4Box("gLeaf", LeafHalfLength, 0.5 * vt[i], leaf_thickness*0.5);
	G4LogicalVolume* lLeaf  = CreateLogicalVolume(svLeaf);
		G4String volName = "X+Leaf"+id_string;
		G4ThreeVector* threeVecPlus = new G4ThreeVector(LeafHalfLength + XPlusLeavesOpen[i] , y_cpos, 0.0);
		G4VPhysicalVolume* pPlusLeaf = CreatePhysicalVolume(volName, lLeaf, 0, threeVecPlus, fEnvelopePhys);
		volName = "X-Leaf"+id_string;
		G4ThreeVector* threeVecMinus = new G4ThreeVector((-1)*LeafHalfLength + XMinusLeavesOpen[i] , y_cpos, 0.0);
		G4VPhysicalVolume* pMinusLeaf = CreatePhysicalVolume(volName, lLeaf, 0, threeVecMinus, fEnvelopePhys);
		XPlusLeaves.push_back(pPlusLeaf);
	XMinusLeaves.push_back(pMinusLeaf);
	}
	return fEnvelopePhys;
}
