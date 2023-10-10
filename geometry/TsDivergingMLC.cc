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

#include "TsDivergingMLC.hh"

#include "TsParameterManager.hh"

#include "G4Box.hh"
#include "G4Trap.hh"
#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"
#include "G4LogicalVolume.hh"

TsDivergingMLC::TsDivergingMLC(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
							   TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name)
: TsVGeometryComponent(pM, eM, mM, gM, parentComponent, parentVolume, name)
{;}


TsDivergingMLC::~TsDivergingMLC()
{;}


void TsDivergingMLC::UpdateForSpecificParameterChange(G4String parameter)
{
	if (parameter == GetFullParmNameLower("NegativeFieldSetting") || parameter == GetFullParmNameLower("PositiveFieldSetting")) {
		fNegFieldSettings  = fPm->GetDoubleVector(GetFullParmName("NegativeFieldSetting"), "Length"); // Var should be fNegFieldSettings
		fPosFieldSettings = fPm->GetDoubleVector(GetFullParmName("PositiveFieldSetting"), "Length");
		G4ThreeVector xPosLeafPositions;
		G4ThreeVector XNegLeafPositions;
		for (int i= 0; i < fNLeavesPerSide; ++i) {
			if ( (fNegFieldSettings[i] - fPosFieldSettings[i]) < 0.0 ) {
				G4cerr << "Topas is exiting due to a serious error in definition of component: " << fName << G4endl;
				G4cerr << "Detected Leaf collision at leaf number " << i << G4endl;
				G4cerr << "NegativeFieldSetting: " << fNegFieldSettings[i]/cm << " (cm)" << G4endl;
				G4cerr << "PositiveFieldSetting: " << fPosFieldSettings[i]/cm << " (cm) "<< G4endl;
				fPm->AbortSession(1);
			}

			xPosLeafPositions = (fXPosLeaves[i])->GetTranslation();
			XNegLeafPositions = (fXNegLeaves[i])->GetTranslation();
			xPosLeafPositions.setX(fLeafHalfLength + fNegFieldSettings[i]);
			XNegLeafPositions.setX((-1)*fLeafHalfLength + fPosFieldSettings[i]);
			(fXPosLeaves[i])->SetTranslation(xPosLeafPositions);
			(fXNegLeaves[i])->SetTranslation(XNegLeafPositions);
		}
		AddToReoptimizeList(fEnvelopeLog);
	} else {
		// For any other parameters, fall back to the base class Update method
		TsVGeometryComponent::UpdateForSpecificParameterChange(parameter);
	}
}


G4VPhysicalVolume* TsDivergingMLC::Construct()
{
	BeginConstruction();
	fXPosLeaves.clear();
	fXNegLeaves.clear();

	fNLeavesPerSide = fPm->GetVectorLength(GetFullParmName("LeafWidths"));

	G4int nPosFieldSettings = fPm->GetVectorLength(GetFullParmName("PositiveFieldSetting"));
	if (fNLeavesPerSide != nPosFieldSettings) {
		G4cerr << "Topas is exiting due to a serious error in definition of component: " << fName << G4endl;
		G4cerr << GetFullParmName("LeafWidths") << " specifies number of leaves: " << fNLeavesPerSide << G4endl;
		G4cerr << "while " << G4endl;
		G4cerr << GetFullParmName("PositiveFieldSetting") << " specifies number of leaves: " << nPosFieldSettings << G4endl;
		fPm->AbortSession(1);
	}

	G4int nNegFieldSettings = fPm->GetVectorLength(GetFullParmName("NegativeFieldSetting"));
	if (fNLeavesPerSide != nNegFieldSettings) {
		G4cerr << "Topas is exiting due to a serious error in definition of component: " << fName << G4endl;
		G4cerr << GetFullParmName("LeafWidths") << " specifies number of leaves: " << fNLeavesPerSide << G4endl;
		G4cerr << "while " << G4endl;
		G4cerr << GetFullParmName("NegativeFieldSetting") << " specifies number of leaves: " << nNegFieldSettings << G4endl;
		fPm->AbortSession(1);
	}

	fNegFieldSettings  = fPm->GetDoubleVector(GetFullParmName("PositiveFieldSetting"), "Length");
	fPosFieldSettings = fPm->GetDoubleVector(GetFullParmName("NegativeFieldSetting"), "Length");

	fLeafHalfLength = 0.5 * fPm->GetDoubleParameter(GetFullParmName("Length"), "Length"); //x
	G4double leafThickness = fPm->GetDoubleParameter(GetFullParmName("Thickness"), "Length"); //z
	G4double* leafWidths = fPm->GetDoubleVector(GetFullParmName("LeafWidths"), "Length");  //y

	if ( fPm->ParameterExists(GetFullParmName("MaximumLeafOpen")) ) {
		fMaximumLeafOpen = fPm->GetDoubleParameter(GetFullParmName("MaximumLeafOpen"), "Length");
	} else {
		fMaximumLeafOpen = 6.0 * fLeafHalfLength;
	}

	G4double SAD = fPm->GetDoubleParameter(GetFullParmName("SAD"), "Length");
	if (SAD < 0) {
		G4cerr << "Topas is exiting due to a serious error in definition of component: " << fName << G4endl;
		G4cerr << GetFullParmName("SAD") << " must be positive, but had value: " << SAD << G4endl;
		fPm->AbortSession(1);
	}

	G4double SourceToUpstreamSurfaceDistance = fPm->GetDoubleParameter(GetFullParmName("SourceToUpstreamSurfaceDistance"), "Length");

	G4String transZName = "dc:Ge/" + fName + "/TransZ";
	G4double distanceSource2MLCcenter = SourceToUpstreamSurfaceDistance + leafThickness * 0.5;

	G4double trapezoidSizeFactor = leafThickness / SAD;
	G4double upstreamFactor = (distanceSource2MLCcenter - leafThickness * 0.5) / SAD;
	G4double downstreamFactor = (distanceSource2MLCcenter + leafThickness * 0.5) / SAD;
	G4double ySum = 0.0;
	for (G4int i = 0; i < fNLeavesPerSide; ++i) {
		ySum += leafWidths[i];

		if ( fabs(fNegFieldSettings[i]) > fMaximumLeafOpen || fabs(fPosFieldSettings[i]) > fMaximumLeafOpen) {
			G4cerr << "Topas is exiting due to a serious error in definition of component: " << fName << G4endl;
			G4cerr << "Leaf pair " << i << " is open wider than the limit: " << fMaximumLeafOpen/cm << " (cm)" << G4endl;
			G4cerr << "If you want to adjust this limit, use the parameter: " << GetFullParmName("MaximumLeafOpen") << G4endl;
			fPm->AbortSession(1);
		}

		if ( (fNegFieldSettings[i] - fPosFieldSettings[i]) < 0.0 ) {
			G4cerr << "Topas is exiting due to a serious error in definition of component: " << fName << G4endl;
			G4cerr << "Detected Leaf collision at leaf number " << i << G4endl;
			G4cerr << "NegativeFieldSetting: " << fNegFieldSettings[i]/cm << " (cm)" << G4endl;
			G4cerr << "PositiveFieldSetting: " << fPosFieldSettings[i]/cm << " (cm) "<< G4endl;
			fPm->AbortSession(1);
		}
	}

	G4double yCentralPos = -1 * (ySum);
	G4double* xUpstreamHigh = new G4double[fNLeavesPerSide];
	G4double* xUpstreamCenter = new G4double[fNLeavesPerSide];
	G4double* xUpstreamLow = new G4double[fNLeavesPerSide];
	G4double* xDownstreamHigh = new G4double[fNLeavesPerSide];
	G4double* xDownstreamCenter = new G4double[fNLeavesPerSide];
	G4double* xDownstreamLow = new G4double[fNLeavesPerSide];
	for (G4int i = 0; i < fNLeavesPerSide; ++i)
	{
		if (i == 0) {
			yCentralPos = yCentralPos + leafWidths[i];
		} else {
			yCentralPos = yCentralPos + (leafWidths[i-1] + leafWidths[i]);
		}

		xUpstreamHigh[i] = upstreamFactor * (yCentralPos + leafWidths[i]);
		xUpstreamCenter[i] = upstreamFactor * yCentralPos;
		xUpstreamLow[i] = upstreamFactor * (yCentralPos - leafWidths[i]);

		xDownstreamHigh[i] = downstreamFactor * (yCentralPos + leafWidths[i]);
		xDownstreamCenter[i] = downstreamFactor * yCentralPos;
		xDownstreamLow[i] = downstreamFactor * (yCentralPos - leafWidths[i]);
	}

	G4RotationMatrix* positiveJawROT = new G4RotationMatrix;
	G4RotationMatrix* negativeJawROT = new G4RotationMatrix;

	positiveJawROT->rotateX(0 * deg);
	positiveJawROT->rotateY(0 * deg);
	positiveJawROT->rotateZ(90 * deg);

	negativeJawROT->rotateX(0 * deg);
	negativeJawROT->rotateY(0 * deg);
	negativeJawROT->rotateZ(90 * deg);

	G4String envelopeMaterialName = fParentComponent->GetResolvedMaterialName();
	G4Box* wholeBoxSolid = new G4Box(fName, fLeafHalfLength * 2 + fMaximumLeafOpen * 2, ySum, 0.5 * leafThickness);
	fEnvelopeLog = CreateLogicalVolume(fName, envelopeMaterialName, wholeBoxSolid);
	fEnvelopePhys = CreatePhysicalVolume(fEnvelopeLog);

	for (G4int i = 0; i < fNLeavesPerSide; ++i)
	{
		G4String leafNumberString = G4UIcommand::ConvertToString(i);
		G4Trap* posLeafSolid = new G4Trap(fName + "_gLeaf", leafThickness / 2.,
											atan((xDownstreamCenter[i] - xUpstreamCenter[i]) / leafThickness) * 0.25,
											0 * deg, (fLeafHalfLength - trapezoidSizeFactor * fNegFieldSettings[i]),
											(xUpstreamHigh[i] - xUpstreamLow[i]), (xUpstreamHigh[i] - xUpstreamLow[i]),
											0, fLeafHalfLength, (xDownstreamCenter[i] - xDownstreamLow[i]),
											(xDownstreamCenter[i] - xDownstreamLow[i]), 0);

		G4Trap* negLeafSolid = new G4Trap(fName + "_gLeaf", leafThickness / 2.,
											atan((xDownstreamCenter[i] - xUpstreamCenter[i]) / leafThickness) * 0.25, 0 * deg,
											fLeafHalfLength + trapezoidSizeFactor * fPosFieldSettings[i], xUpstreamHigh[i] - xUpstreamLow[i],
											xUpstreamHigh[i] - xUpstreamLow[i], 0, fLeafHalfLength,
											xDownstreamCenter[i] - xDownstreamLow[i],
											xDownstreamCenter[i] - xDownstreamLow[i], 0);

		G4LogicalVolume* negativelLeaf  = CreateLogicalVolume(negLeafSolid);
		G4LogicalVolume* positivelLeaf  = CreateLogicalVolume(posLeafSolid);
		G4String volName = fName + "_X+Leaf"+leafNumberString;
		G4ThreeVector* threeVecPlus = new G4ThreeVector(fLeafHalfLength + (fNegFieldSettings[i] * upstreamFactor), (xUpstreamCenter[i] + xDownstreamCenter[i]) * 0.5, 0.0);
		G4VPhysicalVolume* pPlusLeaf = CreatePhysicalVolume(volName, positivelLeaf, positiveJawROT, threeVecPlus, fEnvelopePhys);

		volName = fName + "_X-Leaf"+leafNumberString;
		G4ThreeVector* threeVecMinus = new G4ThreeVector(- fLeafHalfLength + (fPosFieldSettings[i] * upstreamFactor), (xUpstreamCenter[i] + xDownstreamCenter[i]) * 0.5, 0.0);
		G4VPhysicalVolume* pMinusLeaf = CreatePhysicalVolume(volName, negativelLeaf, negativeJawROT, threeVecMinus, fEnvelopePhys);
		fXPosLeaves.push_back(pPlusLeaf);
		fXNegLeaves.push_back(pMinusLeaf);
	}
	return fEnvelopePhys;
}
