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

#ifndef TsMaterialManager_hh
#define TsMaterialManager_hh

#include "globals.hh"
#include <map>
#include <vector>

class TsParameterManager;

class G4NistManager;
class G4Element;
class G4Material;
class G4MaterialPropertiesTable;

class TsMaterialManager
{
public:
	TsMaterialManager(TsParameterManager* pm);
	~TsMaterialManager();

	G4Material* GetMaterial(G4String name);
	G4Material* GetMaterial(const char* name);

	G4String GetDefaultMaterialColor(G4String name);
	G4String GetDefaultMaterialColor(const char* name);

	// User classes should not access any methods or data beyond this point.
	void FillMaterialPropertiesTable(G4MaterialPropertiesTable* propertiesTable, G4String prefix);

	std::vector<G4String> GetMaterialNames();

private:
	G4Element* GetElement(G4String name);

	void SetConstantProperty(G4MaterialPropertiesTable* propertiesTable, G4String materialName, G4String propertyName);
	void SetVectorProperty(G4MaterialPropertiesTable* propertiesTable, G4String materialName, G4String propertyName, G4bool applySpline);

	G4String GetFullIsotopeParmName(G4String name, const char* suffix);
	G4String GetFullElementParmName(G4String name, const char* suffix);
	G4String GetFullMaterialParmName(G4String name, const char* suffix);

	void Quit(const G4String& name, const char* message);
	void Quit(const G4String& name, const G4String& message);

	TsParameterManager* fPm;
	G4int fVerbosity;
	G4bool fSetOpticalPropertiesForPatientMaterials;
	G4NistManager* fNistManager;
	std::map<G4String,G4Element*>* fElementMap;
	std::map<G4String,G4Material*>* fMaterialMap;
};

#endif
