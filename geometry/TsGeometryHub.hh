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

#ifndef TsGeometryHub_hh
#define TsGeometryHub_hh

#include "globals.hh"
#include <functional>
#include <map>
#include <vector>

class TsParameterManager;
class TsExtensionManager;
class TsMaterialManager;
class TsGeometryManager;

class TsVGeometryComponent;
class G4VPhysicalVolume;

class TsGeometryHub
{
public:
	typedef std::function<TsVGeometryComponent*(TsParameterManager*, TsExtensionManager*, TsMaterialManager*, TsGeometryManager*, TsVGeometryComponent*, G4VPhysicalVolume*, G4String&)> GeometryFactory;
	typedef std::function<void(TsParameterManager*, G4String&, G4String&)> GeometryDefaultsCreator;

	struct RequiredParameter {
		G4String NameTemplate;
		G4String DefaultValue;
	};

	struct GeometryTypeInfo {
		G4String CanonicalName;
		GeometryFactory Factory;
		GeometryDefaultsCreator DefaultsCreator;
		std::vector<RequiredParameter> RequiredParameters;
	};

	typedef std::map<G4String, GeometryTypeInfo> GeometryRegistry;

	TsGeometryHub(TsParameterManager* pM);
	~TsGeometryHub();

	TsVGeometryComponent* InstantiateComponent(TsParameterManager* pM, TsExtensionManager* Em,
											   TsMaterialManager* mM, TsGeometryManager* gM, TsVGeometryComponent* pgc,
											   G4VPhysicalVolume* pv, G4String childCompType, G4String childName);

	static void RegisterGeometryType(const GeometryTypeInfo& info);
	static std::vector<RequiredParameter> GetExpandedRequiredParameters(const G4String& typeName, const G4String& childName, const G4String& parentName);

	static void AddComponentFromGUI(TsParameterManager* pM, TsGeometryManager* gM,
							 G4String& childName, G4String& parentName, G4String& typeName, G4String& fieldName);

private:
	static GeometryRegistry& GetRegistry();
	static void RegisterBuiltInTypes(GeometryRegistry& registry);
	static G4bool ApplyRequiredParameters(TsParameterManager* pM, const GeometryTypeInfo& info, G4String& childName, G4String& parentName);
	static G4String FormatParameterName(const G4String& nameTemplate, const G4String& childName, const G4String& parentName);
	static void CopyDicomTemplateParametersIfAvailable(TsParameterManager* pM, const G4String& sourceComponent, const G4String& destComponent);
	static void CopyParameterIfExists(TsParameterManager* pM, const G4String& source, const G4String& dest);
};
#endif
