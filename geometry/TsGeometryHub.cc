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
#include "TsDicomActivityMap.hh"
#include "TsBox.hh"
#include "TsCylinder.hh"
#include "TsSphere.hh"
#include "TsCADComponent.hh"
#include "TsJaws.hh"
#include "TsDivergingMLC.hh"
#include "TsPixelatedBox.hh"

#include "G4VPhysicalVolume.hh"

#include <string>
#include <utility>

namespace {

    G4String ToLowerKey(G4String value) {
        G4StrUtil::to_lower(value);
        return value;
    }

    void AddType(TsGeometryHub::GeometryRegistry& registry, const TsGeometryHub::GeometryTypeInfo& info) {
        registry[ToLowerKey(info.CanonicalName)] = info;
    }

    void AddParameterIfMissing(TsParameterManager* pM, const G4String& name, const G4String& value) {
        if (!pM->ParameterExists(name))
            pM->AddParameter(name, value);
    }

}


TsGeometryHub::GeometryRegistry& TsGeometryHub::GetRegistry() {
    static GeometryRegistry registry;
    static G4bool initialized = false;
    if (!initialized) {
        RegisterBuiltInTypes(registry);
        initialized = true;
    }
    return registry;
}


void TsGeometryHub::RegisterGeometryType(const GeometryTypeInfo& info) {
    GeometryRegistry& registry = GetRegistry();
    AddType(registry, info);
}


std::vector<TsGeometryHub::RequiredParameter> TsGeometryHub::GetExpandedRequiredParameters(const G4String& typeName, const G4String& childName, const G4String& parentName) {
    std::vector<RequiredParameter> expanded;
    G4String lookupType = typeName;
    G4StrUtil::to_lower(lookupType);
    
    GeometryRegistry& registry = GetRegistry();
    GeometryRegistry::iterator it = registry.find(lookupType);
    if (it == registry.end())
        return expanded;
    
    for (const auto& required : it->second.RequiredParameters) {
        RequiredParameter exp;
        exp.NameTemplate = FormatParameterName(required.NameTemplate, childName, parentName);
        exp.DefaultValue = required.DefaultValue;
        expanded.push_back(exp);
    }
    return expanded;
}


void TsGeometryHub::RegisterBuiltInTypes(GeometryRegistry& registry) {
    AddType(registry, {"TsAperture",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsAperture(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"s:Ge/{child}/InputFile", ""},
            {"s:Ge/{child}/FileFormat", ""},
            {"d:Ge/{child}/RMax", ""},
            {"d:Ge/{child}/HL", ""}}});
    
    AddType(registry, {"TsApertureArray",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsApertureArray(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"dc:Ge/{child}/WidthBeamletAtIso", ""},
            {"dc:Ge/{child}/SpacingBeamletAtIso", ""},
            {"i:Ge/{child}/NBeamletsWidth", ""},
            {"i:Ge/{child}/NBeamletsLength", ""},
            {"dc:Ge/{child}/DistCollimatorDownstreamFaceToIso", ""},
            {"dc:Ge/{child}/HLX", ""},
            {"dc:Ge/{child}/HLY", ""},
            {"dc:Ge/{child}/CollimatorThickness", ""},
            {"dc:Ge/{child}/DistBremTargetToIso", ""},
            {"dc:Ge/{child}/DistBeamletVirtualFocusToIso", ""},
            {"dc:Ge/{child}/AngleOffset", ""},
            {"b:Ge/{child}/UseFullLengthBeamlets", ""},
            {"s:Ge/{child}/GeometryMethod", ""}}});
    
    AddType(registry, {"TsBrachyApplicator",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsBrachyApplicator(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"dc:Ge/{child}/CylinderLength", ""},
            {"dc:Ge/{child}/Radius", ""},
            {"i:Ge/{child}/NumberOfRadialHoles", ""},
            {"dc:Ge/{child}/HoleRadius", ""},
            {"dc:Ge/{child}/HoleOffset", ""}}});
    
    AddType(registry, {"TsBrachyShieldWedge",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsBrachyShieldWedge(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"dc:Ge/{child}/RMin", ""},
            {"dc:Ge/{child}/RMax", ""},
            {"dc:Ge/{child}/HL", ""},
            {"dc:Ge/{child}/AngleStart", ""},
            {"dc:Ge/{child}/AngleDistance", ""},
            {"i:Ge/{child}/NumberOfRadialHoles", ""},
            {"dc:Ge/{child}/HoleRadius", ""},
            {"dc:Ge/{child}/HoleOffset", ""}}});
    
    AddType(registry, {"TsBrachyShieldStar",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsBrachyShieldStar(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"dc:Ge/{child}/holeRadius", ""},
            {"dc:Ge/{child}/RMin", ""},
            {"dc:Ge/{child}/RMax", ""},
            {"dc:Ge/{child}/HL", ""},
            {"i:Ge/{child}/NumberOfRadialHoles", ""},
            {"dc:Ge/{child}/SpokeWidth", ""}}});
    
    AddType(registry, {"TsCompensator",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsCompensator(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"s:Ge/{child}/InputFile", ""},
            {"s:Ge/{child}/FileFormat", ""},
            {"dc:Ge/{child}/RMax", ""},
            {"s:Ge/{child}/Method", ""}}});
    
    AddType(registry, {"TsEye",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsEye(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"dc:Ge/{child}/Tumor/Radius", ""},
            {"dc:Ge/{child}/Sclera/Radius", ""},
            {"dc:Ge/{child}/Vitreous/RMajor", ""},
            {"dc:Ge/{child}/Vitreous/RMinor", ""},
            {"dc:Ge/{child}/Aqueous/Radius", ""},
            {"dc:Ge/{child}/Cornea/FrontRadius", ""},
            {"dc:Ge/{child}/Cornea/BackRadius", ""},
            {"dc:Ge/{child}/Iris/InnerRadius", ""},
            {"dc:Ge/{child}/Iris/OuterRadius", ""},
            {"dc:Ge/{child}/Iris/Length", ""},
            {"dc:Ge/{child}/Lens/FrontRadius", ""},
            {"dc:Ge/{child}/Lens/BackRadius", ""},
            {"dc:Ge/{child}/Lens/FrontOffset", ""},
            {"dc:Ge/{child}/Lens/BackOffset", ""},
            {"dc:Ge/{child}/Aqueous/LensOffset", ""}}});
    
    AddType(registry, {"TsEyePlaque",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsEyePlaque(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"dc:Ge/{child}/Eye/Radius", ""},
            {"dc:Ge/{child}/Sphere/ROuter", ""},
            {"dc:Ge/{child}/Sphere/RInner", ""},
            {"dc:Ge/{child}/Cylinder/ROuter", ""},
            {"dc:Ge/{child}/Cylinder/RInner", ""},
            {"dc:Ge/{child}/Cylinder/Length", ""},
            {"dc:Ge/{child}/SphereCutCylinder/ROuter", ""},
            {"dc:Ge/{child}/SphereCutCylinder/RInner", ""},
            {"s:Ge/{child}/Back/Material",""},
            {"s:Ge/{child}/Lip/Material",""}}});
    
    AddType(registry, {"TsRangeModulator",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsRangeModulator(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"dc:Ge/{child}/WheelRadius", ""},
            {"dc:Ge/{child}/WheelThickness", ""},
            {"i:Ge/{child}/NumBumps", ""},
            {"dc:Ge/{child}/BumpWidth", ""},
            {"dc:Ge/{child}/BumpHeight", ""},
            {"dc:Ge/{child}/InitialAngle", ""}}});
    
    AddType(registry, {"TsMultiWireChamber",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsMultiWireChamber(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"dc:Ge/{child}/HLX", ""},
            {"dc:Ge/{child}/HLY", ""},
            {"dc:Ge/{child}/HLZ", ""},
            {"i:Ge/{child}/NumberOfLayers", ""},
            {"dc:Ge/{child}/Layer1/PosZ", ""}}}); // PosZ per layer; GUI should extend per layer count
    
    AddType(registry, {"TsMultiLeafCollimator",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsMultiLeafCollimator(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"dc:Ge/{child}/Length", ""},
            {"dc:Ge/{child}/Thickness", ""},
            {"dv:Ge/{child}/LeafWidths", ""},
            {"dv:Ge/{child}/XPlusLeavesOpen", ""},
            {"dv:Ge/{child}/XMinusLeavesOpen", ""},
            {"dc:Ge/{child}/SAD", ""},
            {"dc:Ge/{child}/SourceToUpstreamSurfaceDistance", ""}}});
    
    AddType(registry, {"TsRidgeFilter",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsRidgeFilter(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"dv:Ge/{child}/ZPoints", ""},
            {"dv:Ge/{child}/XPoints", ""},
            {"dc:Ge/{child}/Width", ""},
            {"dv:Ge/{child}/Displacement", ""},
            {"dc:Ge/{child}/Length", ""}}});
    
    AddType(registry, {"TsPropeller",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsPropeller(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"i:Ge/{child}/NbOfBlades", ""},
            {"dc:Ge/{child}/Rin", ""},
            {"dc:Ge/{child}/Rout", ""},
            {"dv:Ge/{child}/Thickness", ""},
            {"dv:Ge/{child}/Angles", ""},
            {"sv:Ge/{child}/Materials", ""}}});
    
    AddType(registry, {"TsImageCube",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsImageCube(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {}});

    AddType(registry, {"TsXiOPatient",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsImageCube(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"s:Ge/{child}/DicomDirectory", ""}}});

    AddType(registry, {"TsDicomPatient",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsDicomPatient(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"s:Ge/{child}/DicomDirectory", ""},
            {"s:Ge/{child}/ImagingToMaterialConverter", ""},
            {"s:Ge/{child}/DicomRTStructFile", ""}}});
    
    AddType(registry, {"TsBox",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsBox(pM, eM, mM, gM, pgc, pv, childName);
        },
        TsBox::CreateDefaults,
        {}});
    
    AddType(registry, {"TsCylinder",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsCylinder(pM, eM, mM, gM, pgc, pv, childName);
        },
        TsCylinder::CreateDefaults,
        {}});
    
    AddType(registry, {"TsSphere",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsSphere(pM, eM, mM, gM, pgc, pv, childName);
        },
        TsSphere::CreateDefaults,
        {}});
    
    AddType(registry, {"TsCAD",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsCADComponent(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"s:Ge/{child}/InputFile", ""},
            {"s:Ge/{child}/FileFormat", ""},
            {"d:Ge/{child}/Units", ""}}});
    
    AddType(registry, {"TsJaws",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsJaws(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"dc:Ge/{child}/LX", ""},
            {"dc:Ge/{child}/LY", ""},
            {"dc:Ge/{child}/LZ", ""},
            {"dc:Ge/{child}/PositiveFieldSetting", ""},
            {"dc:Ge/{child}/NegativeFieldSetting", ""},
            {"dc:Ge/{child}/SourceToUpstreamSurfaceDistance", ""},
            {"dc:Ge/{child}/SAD", ""}}});
    
    AddType(registry, {"TsDivergingMLC",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsDivergingMLC(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"dc:Ge/{child}/Length", ""},
            {"dc:Ge/{child}/Thickness", ""},
            {"dv:Ge/{child}/LeafWidths", ""},
            {"dv:Ge/{child}/PositiveFieldSetting", ""},
            {"dv:Ge/{child}/NegativeFieldSetting", ""},
            {"dc:Ge/{child}/SAD", ""},
            {"dc:Ge/{child}/SourceToUpstreamSurfaceDistance", ""},
            {"s:Ge/{child}/LeafTravelAxis", ""}}});
    
    AddType(registry, {"TsPixelatedBox",
        [](TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc, G4VPhysicalVolume* pv, G4String& childName) {
            return new TsPixelatedBox(pM, eM, mM, gM, pgc, pv, childName);
        },
        nullptr,
        {{"dc:Ge/{child}/PixelSizeX", ""},
            {"dc:Ge/{child}/PixelSizeY", ""},
            {"dc:Ge/{child}/PixelSizeZ", ""},
            {"ic:Ge/{child}/NumberOfPixelsX", ""},
            {"ic:Ge/{child}/NumberOfPixelsY", ""},
            {"s:Ge/{child}/Pixel/Material", ""},
            {"dc:Ge/{child}/PitchX", ""},
            {"dc:Ge/{child}/PitchY", ""}}});
    
    // Generic Geant4 solids and grouping. Keep default creation to TsGenericComponent.
    const G4String genericTypes[] = {"Group", "G4CutTubs", "G4Cons", "G4Para", "G4Trd", "G4RTrap", "G4GTrap", "G4Orb",
        "G4Torus", "G4HPolycone", "G4SPolycone", "G4HPolyhedra", "G4SPolyhedra", "G4EllipticalTube",
        "G4Ellipsoid", "G4EllipticalCone", "G4Paraboloid", "G4Hype", "G4Tet", "G4ExtrudedSolid",
        "G4TwistedBox", "G4RTwistedTrap", "G4GTwistedTrap", "G4TwistedTrd", "G4GenericTrap", "G4TwistedTubs"};
    for (const auto& type : genericTypes) {
        AddType(registry, {type,
            nullptr,
            [type](TsParameterManager* pM, G4String& childName, G4String& parentName) {
                G4String childType = type;
                TsGenericComponent::CreateDefaults(pM, childName, parentName, childType);
            },
            {}});
    }
}


TsGeometryHub::TsGeometryHub(TsParameterManager* pM)
{
    GeometryRegistry& registry = GetRegistry();
    for (auto const& entry : registry) {
        pM->RegisterComponentTypeName(entry.second.CanonicalName);
    }
}


TsGeometryHub::~TsGeometryHub()
{
}


TsVGeometryComponent* TsGeometryHub::InstantiateComponent(TsParameterManager* pM, TsExtensionManager* eM,
                                                          TsMaterialManager* mM, TsGeometryManager* gM,
                                                          TsVGeometryComponent* pgc, G4VPhysicalVolume* pv,
                                                          G4String childCompType, G4String childName)
{
    G4StrUtil::to_lower(childCompType);
    
    // First see if the user's extensions include this component
    TsVGeometryComponent* component = eM->InstantiateComponent(pM, mM, gM, pgc, pv, childCompType, childName);
    if (component) return component;
    
    GeometryRegistry& registry = GetRegistry();
    GeometryRegistry::iterator it = registry.find(childCompType);
    if (it != registry.end() && it->second.Factory)
        return it->second.Factory(pM, eM, mM, gM, pgc, pv, childName);
    
    return new TsGenericComponent(pM, eM, mM, gM, pgc, pv, childName);
}


void TsGeometryHub::AddComponentFromGUI(TsParameterManager* pM, TsGeometryManager* gM, G4String& childName,
										G4String& parentName, G4String& childCompType, G4String& fieldName) {
	G4String lookupType = childCompType;
	G4StrUtil::to_lower(lookupType);

    GeometryRegistry& registry = GetRegistry();
    GeometryRegistry::iterator it = registry.find(lookupType);
    
    G4bool defaultsCreated = false;
    if (it != registry.end() && it->second.DefaultsCreator) {
        it->second.DefaultsCreator(pM, childName, parentName);
        defaultsCreated = true;
    } else if (it != registry.end()) {
        // Known type but no explicit defaults creator; continue with generic placement defaults.
        defaultsCreated = true;
    }
    
    if (!defaultsCreated) {
        G4String typeForDefaults = (it != registry.end()) ? it->second.CanonicalName : childCompType;
        defaultsCreated = TsGenericComponent::CreateDefaults(pM, childName, parentName, typeForDefaults);
    }
    
    if (!defaultsCreated) {
        G4cerr << "Sorry, don't have the logic yet to add a " << childCompType << G4endl;
        return;
    }
    
    G4String parameterName;
    G4String transValue;
    
    parameterName = "s:Ge/" + childName + "/Type";
    transValue = "\"" + childCompType + "\"";
    AddParameterIfMissing(pM, parameterName, transValue);
    
    parameterName = "s:Ge/" + childName + "/Parent";
    transValue = "\"" + parentName + "\"";
    AddParameterIfMissing(pM, parameterName, transValue);
    
    parameterName = "dc:Ge/" + childName + "/TransX";
    transValue = "0. mm";
    AddParameterIfMissing(pM, parameterName, transValue);
    
    parameterName = "dc:Ge/" + childName + "/TransY";
    transValue = "0. mm";
    AddParameterIfMissing(pM, parameterName, transValue);
    
    parameterName = "dc:Ge/" + childName + "/TransZ";
    transValue = "0. mm";
    AddParameterIfMissing(pM, parameterName, transValue);
    
    parameterName = "dc:Ge/" + childName + "/RotX";
    transValue = "0. deg";
    AddParameterIfMissing(pM, parameterName, transValue);
    
    parameterName = "dc:Ge/" + childName + "/RotY";
    transValue = "0. deg";
    AddParameterIfMissing(pM, parameterName, transValue);
    
    parameterName = "dc:Ge/" + childName + "/RotZ";
    transValue = "0. deg";
    AddParameterIfMissing(pM, parameterName, transValue);
    
    if (childCompType != "Group") {
        parameterName = "sc:Ge/" + childName + "/Material";
        transValue = "\"G4_WATER\"";
        AddParameterIfMissing(pM, parameterName, transValue);
        
        parameterName = "sc:Ge/" + childName + "/Color";
        transValue = "\"white\"";
        AddParameterIfMissing(pM, parameterName, transValue);
        
        parameterName = "sc:Ge/" + childName + "/DrawingStyle";
        transValue = "\"FullWireFrame\"";
        AddParameterIfMissing(pM, parameterName, transValue);
        
        parameterName = "bc:Ge/" + childName + "/Invisible";
        transValue = "\"False\"";
        AddParameterIfMissing(pM, parameterName, transValue);
    }
    
    if (fieldName == "DipoleMagnet") {
        parameterName = "s:Ge/" + childName + "/Field";
        transValue = "\"DipoleMagnet\"";
        AddParameterIfMissing(pM, parameterName, transValue);
        
        parameterName = "uc:Ge/" + childName + "/MagneticFieldDirectionX";
        transValue = "0.0";
        AddParameterIfMissing(pM, parameterName, transValue);
        
        parameterName = "uc:Ge/" + childName + "/MagneticFieldDirectionY";
        transValue = "1.0";
        AddParameterIfMissing(pM, parameterName, transValue);
        
        parameterName = "uc:Ge/" + childName + "/MagneticFieldDirectionZ";
        transValue = "0.0";
        AddParameterIfMissing(pM, parameterName, transValue);
        
        parameterName = "dc:Ge/" + childName + "/MagneticFieldStrength";
        transValue = "3.0 tesla";
        AddParameterIfMissing(pM, parameterName, transValue);
    } else if (fieldName == "QuadrupoleMagnet") {
        parameterName = "s:Ge/" + childName + "/Field";
        transValue = "\"QuadrupoleMagnet\"";
        AddParameterIfMissing(pM, parameterName, transValue);
        
        parameterName = "dc:Ge/" + childName + "/MagneticFieldGradientX";
        transValue = "1. tesla/cm";
        AddParameterIfMissing(pM, parameterName, transValue);
        
        parameterName = "dc:Ge/" + childName + "/MagneticFieldGradientY";
        transValue = "1. tesla/cm";
        AddParameterIfMissing(pM, parameterName, transValue);
    }

	if (it != registry.end())
		ApplyRequiredParameters(pM, it->second, childName, parentName);

	// If adding a DICOM patient and a template exists (commonly Ge/Patient/*), copy mapping parameters.
	if (lookupType == "tsdicompatient")
		CopyDicomTemplateParametersIfAvailable(pM, "Patient", childName);

	TsVGeometryComponent* parentComponent = gM->GetComponent(parentName);
	if (parentComponent)
		parentComponent->InstantiateAndConstructChild(childName);
}


G4String TsGeometryHub::FormatParameterName(const G4String& nameTemplate, const G4String& childName, const G4String& parentName) {
	G4String parameterName = nameTemplate;
	size_t pos = parameterName.find("{child}");
	if (pos != std::string::npos)
		parameterName.replace(pos, 7, childName);
    
    pos = parameterName.find("{parent}");
    if (pos != std::string::npos)
        parameterName.replace(pos, 8, parentName);
    
    return parameterName;
}


G4bool TsGeometryHub::ApplyRequiredParameters(TsParameterManager* pM, const GeometryTypeInfo& info, G4String& childName, G4String& parentName) {
	for (const auto& required : info.RequiredParameters) {
		G4String parameterName = FormatParameterName(required.NameTemplate, childName, parentName);
		if (pM->ParameterExists(parameterName))
			continue;
        
        // If a default is provided, add it; otherwise allow GUI/user-supplied value to stand.
        if (!required.DefaultValue.empty()) {
            pM->AddParameter(parameterName, required.DefaultValue);
        }
	}
	return true;
}


void TsGeometryHub::CopyDicomTemplateParametersIfAvailable(TsParameterManager* pM, const G4String& sourceComponent, const G4String& destComponent) {
	const G4String baseSrc = "Ge/" + sourceComponent + "/";
	const G4String baseDst = "Ge/" + destComponent + "/";

	// Imaging converter selection
	CopyParameterIfExists(pM, "s:" + baseSrc + "ImagingToMaterialConverter", "s:" + baseDst + "ImagingToMaterialConverter");

	// Schneider mapping parameters commonly provided by HUtoMaterialSchneider.txt
	CopyParameterIfExists(pM, "dv:" + baseSrc + "DensityCorrection", "dv:" + baseDst + "DensityCorrection");
	CopyParameterIfExists(pM, "iv:" + baseSrc + "SchneiderHounsfieldUnitSections", "iv:" + baseDst + "SchneiderHounsfieldUnitSections");
	CopyParameterIfExists(pM, "uv:" + baseSrc + "SchneiderDensityOffset", "uv:" + baseDst + "SchneiderDensityOffset");
	CopyParameterIfExists(pM, "uv:" + baseSrc + "SchneiderDensityFactor", "uv:" + baseDst + "SchneiderDensityFactor");
	CopyParameterIfExists(pM, "uv:" + baseSrc + "SchneiderDensityFactorOffset", "uv:" + baseDst + "SchneiderDensityFactorOffset");
	CopyParameterIfExists(pM, "sv:" + baseSrc + "SchneiderElements", "sv:" + baseDst + "SchneiderElements");
	CopyParameterIfExists(pM, "iv:" + baseSrc + "SchneiderHUToMaterialSections", "iv:" + baseDst + "SchneiderHUToMaterialSections");

	for (G4int i = 1; i <= 25; ++i) {
		std::stringstream ss;
		ss << i;
		CopyParameterIfExists(pM,
							  "uv:" + baseSrc + "SchneiderMaterialsWeight" + ss.str(),
							  "uv:" + baseDst + "SchneiderMaterialsWeight" + ss.str());
	}
}


void TsGeometryHub::CopyParameterIfExists(TsParameterManager* pM, const G4String& source, const G4String& dest) {
	if (pM->ParameterExists(dest)) return;
	if (!pM->ParameterExists(source)) return;

	size_t colon = source.find(':');
	if (colon == std::string::npos) return;
	G4String typePrefix = source.substr(0, colon);
	G4String name = source.substr(colon + 1);

	G4String value = pM->GetParameterValueAsString(typePrefix, name);
	pM->AddParameter(dest, value, false, true);
}
