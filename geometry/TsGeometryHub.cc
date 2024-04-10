//
// ********************************************************************
// *                                                                  *
// * Copyright 2024 The OpenTOPAS Collaboration                       *
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

#include "TsGeometryHub.hh"

#include "TsParameterManager.hh"
#include "TsExtensionManager.hh"
#include "TsGeometryManager.hh"

#include "TsVGeometryComponent.hh"

#include "TsGenericComponent.hh"
#include "TsAperture.hh"
#include "TsApertureArray.hh"
#include "TsBrachyApplicator.hh"
#include "TsBrachyShieldWedge.hh"
#include "TsBrachyShieldStar.hh"
#include "TsCompensator.hh"
#include "TsEye.hh"
#include "TsEyePlaque.hh"
#include "TsRangeModulator.hh"
#include "TsRidgeFilter.hh"
#include "TsMultiLeafCollimator.hh"
#include "TsMultiWireChamber.hh"
#include "TsPropeller.hh"
#include "TsImageCube.hh"
#include "TsXiOPatient.hh"
#include "TsDicomPatient.hh"
#include "TsBox.hh"
#include "TsCylinder.hh"
#include "TsSphere.hh"
#include "TsCADComponent.hh"
#include "TsJaws.hh"
#include "TsDivergingMLC.hh"
#include "TsPixelatedBox.hh"

#include "G4VPhysicalVolume.hh"

TsGeometryHub::TsGeometryHub(TsParameterManager* pM)
{
	// This Registry determins what components can be added from the GUI
	pM->RegisterComponentTypeName("TsBox");
	pM->RegisterComponentTypeName("TsCylinder");
	pM->RegisterComponentTypeName("TsSphere");
	pM->RegisterComponentTypeName("Group");
	pM->RegisterComponentTypeName("G4CutTubs");
	pM->RegisterComponentTypeName("G4Cons");
	pM->RegisterComponentTypeName("G4Para");
	pM->RegisterComponentTypeName("G4Trd");
	pM->RegisterComponentTypeName("G4RTrap");
	pM->RegisterComponentTypeName("G4GTrap");
	pM->RegisterComponentTypeName("G4Orb");
	pM->RegisterComponentTypeName("G4Torus");
	pM->RegisterComponentTypeName("G4HPolycone");
	pM->RegisterComponentTypeName("G4SPolycone");
	pM->RegisterComponentTypeName("G4HPolyhedra");
	pM->RegisterComponentTypeName("G4SPolyhedra");
	pM->RegisterComponentTypeName("G4EllipticalTube");
	pM->RegisterComponentTypeName("G4Ellipsoid");
	pM->RegisterComponentTypeName("G4EllipticalCone");
	pM->RegisterComponentTypeName("G4Paraboloid");
	pM->RegisterComponentTypeName("G4Hype");
	pM->RegisterComponentTypeName("G4Tet");
	pM->RegisterComponentTypeName("G4ExtrudedSolid");
	pM->RegisterComponentTypeName("G4TwistedBox");
	pM->RegisterComponentTypeName("G4RTwistedTrap");
	pM->RegisterComponentTypeName("G4GTwistedTrap");
	pM->RegisterComponentTypeName("G4TwistedTrd");
	pM->RegisterComponentTypeName("G4GenericTrap");
	pM->RegisterComponentTypeName("G4TwistedTubs");
}


TsGeometryHub::~TsGeometryHub()
{
}


TsVGeometryComponent* TsGeometryHub::InstantiateComponent(TsParameterManager* pM, TsExtensionManager* eM,
														  TsMaterialManager* mM, TsGeometryManager* gM,
														  TsVGeometryComponent* pgc, G4VPhysicalVolume* pv,
														  G4String childCompType, G4String childName)
{
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(childCompType);
#else
	childCompType.toLower();
#endif

	// First see if the user's extensions include this component
	TsVGeometryComponent* component = eM->InstantiateComponent(pM, mM, gM, pgc, pv, childCompType, childName);
	if (component) return component;

	if (childCompType=="tsaperture")
		return new TsAperture(pM, eM, mM, gM, pgc, pv, childName);
	else if (childCompType == "tsaperturearray")
	    return new TsApertureArray(pM, eM, mM, gM, pgc, pv, childName);
	else if (childCompType == "tsbrachyapplicator")
	    return new TsBrachyApplicator(pM, eM, mM, gM, pgc, pv, childName);
	else if (childCompType == "tsbrachyshieldwedge")
	    return new TsBrachyShieldWedge(pM, eM, mM, gM, pgc, pv, childName);
	else if (childCompType == "tsbrachyshieldstar")
	    return new TsBrachyShieldStar(pM, eM, mM, gM, pgc, pv, childName);
	else if (childCompType == "tscompensator")
		return new TsCompensator(pM, eM, mM, gM, pgc, pv, childName);
	else if (childCompType == "tseye")
		return new TsEye(pM, eM, mM, gM, pgc, pv, childName);
	else if (childCompType == "tseyeplaque")
		return new TsEyePlaque(pM, eM, mM, gM, pgc, pv, childName);
	else if (childCompType == "tsrangemodulator")
		return new TsRangeModulator(pM, eM, mM, gM, pgc, pv, childName);
	else if ( childCompType == "tsmultiwirechamber")
		return new TsMultiWireChamber(pM, eM, mM, gM, pgc, pv, childName);
	else if ( childCompType == "tsmultileafcollimator")
	    return new TsMultiLeafCollimator(pM, eM, mM, gM, pgc, pv, childName);
	else if ( childCompType == "tsridgefilter")
		return new TsRidgeFilter(pM, eM, mM, gM, pgc, pv, childName);
	else if ( childCompType == "tspropeller")
		return new TsPropeller(pM, eM, mM, gM, pgc, pv, childName);
	else if (childCompType == "tsimagecube" || childCompType == "tsxiopatient")
		return new TsImageCube(pM, eM, mM, gM, pgc, pv, childName);
	else if (childCompType == "tsxiopatient_old")
		return new TsXiOPatient(pM, eM, mM, gM, pgc, pv, childName);
	else if (childCompType == "tsdicompatient")
		return new TsDicomPatient(pM, eM, mM, gM, pgc, pv, childName);
	else if (childCompType == "tsbox")
		return new TsBox(pM, eM, mM, gM, pgc, pv, childName);
	else if (childCompType == "tscylinder")
		return new TsCylinder(pM, eM, mM, gM, pgc, pv, childName);
	else if (childCompType == "tssphere")
		return new TsSphere(pM, eM, mM, gM, pgc, pv, childName);
	else if (childCompType == "tscad")
	    return new TsCADComponent(pM, eM, mM, gM, pgc, pv, childName);
	else if (childCompType == "tsjaws")
		return new TsJaws(pM, eM, mM, gM, pgc, pv, childName);
	else if (childCompType == "tsdivergingmlc")
		return new TsDivergingMLC(pM, eM, mM, gM, pgc, pv, childName);
	else if (childCompType == "tspixelatedbox")
		return new TsPixelatedBox(pM, eM, mM, gM, pgc, pv, childName);
	else
		return new TsGenericComponent(pM, eM, mM, gM, pgc, pv, childName);
}


void TsGeometryHub::AddComponentFromGUI(TsParameterManager* pM, TsGeometryManager* gM, G4String& childName,
										G4String& parentName, G4String& childCompType, G4String& fieldName) {
	if (childCompType == "TsBox")
		TsBox::CreateDefaults(pM, childName, parentName);
	else if (childCompType == "TsCylinder")
		TsCylinder::CreateDefaults(pM, childName, parentName);
	else if (childCompType == "TsSphere")
		TsSphere::CreateDefaults(pM, childName, parentName);
	else if (!TsGenericComponent::CreateDefaults(pM, childName, parentName, childCompType)) {
		G4cerr << "Sorry, don't have the logic yet to add a " << childCompType << G4endl;
		return;
	}

	G4String parameterName;
	G4String transValue;

	parameterName = "s:Ge/" + childName + "/Type";
	transValue = "\"" + childCompType + "\"";
	pM->AddParameter(parameterName, transValue);

	parameterName = "s:Ge/" + childName + "/Parent";
	transValue = "\"" + parentName + "\"";
	pM->AddParameter(parameterName, transValue);

	parameterName = "dc:Ge/" + childName + "/TransX";
	transValue = "0. mm";
	pM->AddParameter(parameterName, transValue);

	parameterName = "dc:Ge/" + childName + "/TransY";
	transValue = "0. mm";
	pM->AddParameter(parameterName, transValue);

	parameterName = "dc:Ge/" + childName + "/TransZ";
	transValue = "0. mm";
	pM->AddParameter(parameterName, transValue);

	parameterName = "dc:Ge/" + childName + "/RotX";
	transValue = "0. deg";
	pM->AddParameter(parameterName, transValue);

	parameterName = "dc:Ge/" + childName + "/RotY";
	transValue = "0. deg";
	pM->AddParameter(parameterName, transValue);

	parameterName = "dc:Ge/" + childName + "/RotZ";
	transValue = "0. deg";
	pM->AddParameter(parameterName, transValue);

	if (childCompType != "Group") {
		parameterName = "sc:Ge/" + childName + "/Material";
		transValue = "\"G4_WATER\"";
		pM->AddParameter(parameterName, transValue);

		parameterName = "sc:Ge/" + childName + "/Color";
		transValue = "\"white\"";
		pM->AddParameter(parameterName, transValue);

		parameterName = "sc:Ge/" + childName + "/DrawingStyle";
		transValue = "\"FullWireFrame\"";
		pM->AddParameter(parameterName, transValue);

		parameterName = "bc:Ge/" + childName + "/Invisible";
		transValue = "\"False\"";
		pM->AddParameter(parameterName, transValue);
	}

	if (fieldName == "DipoleMagnet") {
		parameterName = "s:Ge/" + childName + "/Field";
		transValue = "\"DipoleMagnet\"";
		pM->AddParameter(parameterName, transValue);

		parameterName = "uc:Ge/" + childName + "/MagneticFieldDirectionX";
		transValue = "0.0";
		pM->AddParameter(parameterName, transValue);

		parameterName = "uc:Ge/" + childName + "/MagneticFieldDirectionY";
		transValue = "1.0";
		pM->AddParameter(parameterName, transValue);

		parameterName = "uc:Ge/" + childName + "/MagneticFieldDirectionZ";
		transValue = "0.0";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/MagneticFieldStrength";
		transValue = "3.0 tesla";
		pM->AddParameter(parameterName, transValue);
	} else if (fieldName == "QuadrupoleMagnet") {
		parameterName = "s:Ge/" + childName + "/Field";
		transValue = "\"QuadrupoleMagnet\"";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/MagneticFieldGradientX";
		transValue = "1. tesla/cm";
		pM->AddParameter(parameterName, transValue);

		parameterName = "dc:Ge/" + childName + "/MagneticFieldGradientY";
		transValue = "1. tesla/cm";
		pM->AddParameter(parameterName, transValue);
	}

	TsVGeometryComponent* parentComponent = gM->GetComponent(parentName);
	if (parentComponent)
		parentComponent->InstantiateAndConstructChild(childName);
}
