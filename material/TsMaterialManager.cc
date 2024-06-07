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

#include "TsMaterialManager.hh"

#include "TsParameterManager.hh"

#include "G4Element.hh"
#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4NistManager.hh"
#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

TsMaterialManager::TsMaterialManager(TsParameterManager* pM)
:fPm(pM), fSetOpticalPropertiesForPatientMaterials(false)
{
	fVerbosity = fPm->GetIntegerParameter("Ma/Verbosity");

	fSetOpticalPropertiesForPatientMaterials = fPm->GetBooleanParameter("Ma/SetOpticalPropertiesForPatientMaterials");

	// Create the store for the Elements
	fElementMap = new std::map<G4String,G4Element*>;

	// Create the store for the Materials
	fMaterialMap = new std::map<G4String,G4Material*>;
	fNistManager = G4NistManager::Instance();
}


TsMaterialManager::~TsMaterialManager()
{
	delete fElementMap;
	delete fMaterialMap;
}


G4Element* TsMaterialManager::GetElement(G4String name)
{
	// If the element is already in the store, return it.
	// Otherwise, instantiate it based on values in the parameters system.
	G4Element* element = 0;
	G4String symbol = fPm->GetStringParameter(GetFullElementParmName(name, "Symbol"));

	std::map<G4String, G4Element*>::const_iterator iter = fElementMap->find(name);
	if (iter == fElementMap->end()) {
		if (fPm->ParameterExists(GetFullElementParmName(name, "IsotopeNames"))) {
			// Build element from isotopes
			if (!fPm->ParameterExists(GetFullElementParmName(name, "IsotopeAbundances")))
				Quit(name, "IsotopeAbundances have not been specified");

			G4int nIsotopes = fPm->GetVectorLength(GetFullElementParmName(name, "IsotopeNames"));
			if (fPm->GetVectorLength(GetFullElementParmName(name, "IsotopeAbundances")) != nIsotopes)
				Quit(name, "Number of IsotopeAbundances does not match number of IsotopeNames");

			element = new G4Element(name, symbol, nIsotopes);
			G4String* isotopeNames = fPm->GetStringVector(GetFullElementParmName(name, "IsotopeNames"));
			G4double* isotopeAbundances = fPm->GetUnitlessVector(GetFullElementParmName(name, "IsotopeAbundances"));

			for (G4int i=0; i<nIsotopes; i++) {
				G4int isotopeZ = fPm->GetIntegerParameter(GetFullIsotopeParmName(isotopeNames[i], "Z"));
				G4int isotopeN = fPm->GetIntegerParameter(GetFullIsotopeParmName(isotopeNames[i], "N"));
				G4double isotopeA = fPm->GetDoubleParameter(GetFullIsotopeParmName(isotopeNames[i], "A"), "mass");
				G4Isotope* isotope = new G4Isotope(isotopeNames[i], isotopeZ, isotopeN, isotopeA);
				element->AddIsotope(isotope, isotopeAbundances[i]);
			}
		} else {
			// Take element from NIST
			element = fNistManager->FindOrBuildElement(symbol);
			if (!element) {
				G4cerr << "Topas is exiting unable to define element: " << name << ", due to unknown symbol: " << symbol << G4endl;
				G4cerr << "Use one of the standard symbols from the NIST database or define the element yourself from Isotopes." << G4endl;
				fPm->AbortSession(1);
			}
		}
		(*fElementMap)[name] = element;
	} else {
		element = iter->second;
	}
	return element;
}


G4Material* TsMaterialManager::GetMaterial(G4String fullMaterialName)
{
	// If material is one of the variants from variable density materials,
	// need to start by making sure the base material exists.
	G4String name;
	G4int pos = fullMaterialName.find( "_VariableDensityBin_" );
	if (pos == -1)
		name = fullMaterialName;
	else
		name = fullMaterialName.substr(0,pos);

	G4double temperature = NTP_Temperature;
	if (fPm->ParameterExists(GetFullMaterialParmName(name, "Temperature")))
		temperature = fPm->GetDoubleParameter(GetFullMaterialParmName(name, "Temperature"), "Temperature");

	G4double pressure = CLHEP::STP_Pressure;
	if (fPm->ParameterExists(GetFullMaterialParmName(name, "Pressure")))
		pressure = fPm->GetDoubleParameter(GetFullMaterialParmName(name, "Pressure"), "Pressure");

	// If the material is already in the store, return it. Otherwise, instantiate it.
	G4Material* material;

	std::map<G4String, G4Material*>::const_iterator iter = fMaterialMap->find(name);
	if (iter == fMaterialMap->end()) {
		if ( name.substr(0,3)=="G4_") {
			// User is asking for a material from the NIST database or a clone thereof
			if (fPm->ParameterExists(GetFullMaterialParmName(name, "CloneFromMaterial"))) {
				G4String originalMaterialName = fPm->GetStringParameter(GetFullMaterialParmName(name, "CloneFromMaterial"));
				GetMaterial(originalMaterialName);
				material = fNistManager->BuildMaterialWithNewDensity(name, originalMaterialName,
										fPm->GetDoubleParameter(GetFullMaterialParmName(name, "CloneWithDensity"), "Volumic Mass"),
										temperature, pressure);
			} else {
				material = fNistManager->FindOrBuildMaterial(name);
				if (!material) {
					G4String message = "Could not find any NIST definition of the material.";
					message += "\nNote that NIST materials must be specified with exact case.";
					message += "\nThe full list of materials can be found in Appendix 10 of the Geant4 Application Developer's Guide:";
					message += "\nhttps://geant4-userdoc.web.cern.ch/geant4-userdoc/UsersGuides/ForApplicationDeveloper/html/Appendix/materialNames.html#g4matrdb";
					Quit(name, message);
				}
			}
		} else {
			// User is defining a material not from the NIST database
			if (!fPm->ParameterExists(GetFullMaterialParmName(name, "Density")))
				Quit(name, "This material has no density defined");
			G4double density = fPm->GetDoubleParameter(GetFullMaterialParmName(name, "Density"), "Volumic Mass");

			G4State state = kStateUndefined;
			if (fPm->ParameterExists(GetFullMaterialParmName(name, "State"))) {
				G4String stateString = fPm->GetStringParameter(GetFullMaterialParmName(name, "State"));
#if GEANT4_VERSION_MAJOR >= 11
				G4StrUtil::to_lower(stateString);
#else
				stateString.toLower();
#endif
				if (stateString == "solid") state = kStateSolid;
				else if (stateString == "liquid") state = kStateLiquid;
				else if (stateString == "gas") state = kStateGas;
				else if (stateString != "undefined")
					Quit(name, "State has been set to an invalid value. Must be one of Solid, Liquid, Gas or Undefined.");
			}

			if (fPm->ParameterExists(GetFullMaterialParmName(name, "BaseMaterial"))) {
				// User is making a new material from a base material
				G4String baseMaterialName = fPm->GetStringParameter(GetFullMaterialParmName(name, "BaseMaterial"));
				G4Material* baseMaterial = GetMaterial(baseMaterialName);
				material = new G4Material(name, density, baseMaterial, state, temperature, pressure);
			} else {
				// User is making a material by hand
				if (fPm->ParameterExists(GetFullMaterialParmName(name, "AtomicNumber")) ||
					fPm->ParameterExists(GetFullMaterialParmName(name, "AtomicMass"))) {
					// Create a material from a single element.
					if (!fPm->ParameterExists(GetFullMaterialParmName(name, "AtomicNumber")) ||
						!fPm->ParameterExists(GetFullMaterialParmName(name, "AtomicMass"))) {
						Quit(name, "When specifying AtomicNumer or AtomicMass, both are required.");
					}

					G4int anumber = fPm->GetIntegerParameter(GetFullMaterialParmName(name, "AtomicNumber"));
					G4double amass = fPm->GetDoubleParameter(GetFullMaterialParmName(name, "AtomicMass"), "mass");
					material = new G4Material(name, anumber, amass, density, state, temperature, pressure);
				} else {
					// Create a material from several components (either elements or materials)
					if (!fPm->ParameterExists(GetFullMaterialParmName(name, "Components")))
						Quit(name, "Components not defined");
					if (!fPm->ParameterExists(GetFullMaterialParmName(name, "Fractions")))
						Quit(name, "Fractions not defined");

					G4int nComponents = fPm->GetVectorLength(GetFullMaterialParmName(name, "Components"));
					G4int nFractions = fPm->GetVectorLength(GetFullMaterialParmName(name, "Fractions"));

					if (nComponents!=nFractions)
						Quit(name, "Number of fractions does not match number of components");

					G4String* components = fPm->GetStringVector(GetFullMaterialParmName(name, "Components"));
					G4double* fractions = fPm->GetUnitlessVector(GetFullMaterialParmName(name, "Fractions"));

					if (fPm->ParameterExists(GetFullMaterialParmName(name, "NormalizeFractions")) &&
						fPm->GetBooleanParameter(GetFullMaterialParmName(name, "NormalizeFractions")))
					{
						G4double sumFractions = 0;
						for (auto i = 0; i < nFractions; ++i)
							sumFractions += fractions[i];
						for (auto i = 0; i < nFractions; ++i)
							fractions[i] /= sumFractions;
					}

					material = new G4Material(name, density, nFractions, state, temperature, pressure);

					G4bool buildFromMaterial = false;
					if (fPm->ParameterExists(GetFullMaterialParmName(name, "BuildFromMaterials")))
						buildFromMaterial = fPm->GetBooleanParameter(GetFullMaterialParmName(name, "BuildFromMaterials"));

					G4double totalFractions = 0.;
					for (G4int i=0; i<nFractions; i++) {
						if (buildFromMaterial)
							material->AddMaterial( GetMaterial(components[i]), fractions[i]);
						else
							material->AddElement( GetElement(components[i]), fractions[i]);
						totalFractions+=fractions[i];
					}

					if (totalFractions < .9999 || totalFractions > 1.0001) {
						G4String message = "Fractions do not add up to 1. (exact requirement is .9999 < total < 1.0001 ). \nYour total was: "
						+ G4UIcommand::ConvertToString(totalFractions);
						Quit(name, message);
					}

					delete[] components;
					delete[] fractions;
				}
			}

			if (fPm->ParameterExists(GetFullMaterialParmName(name, "MeanExcitationEnergy")))
				material->GetIonisation()->SetMeanExcitationEnergy(fPm->GetDoubleParameter(GetFullMaterialParmName(name, "MeanExcitationEnergy"), "Energy"));

			// Don't bother checking optical properties for patient materials
			if (fSetOpticalPropertiesForPatientMaterials || (name.length() < 19 || name.substr(0,19) != "PatientTissueFromHU")) {
				if (fPm->ParameterExists(GetFullMaterialParmName(name, "BirksConstant")))
					material->GetIonisation()->SetBirksConstant(fPm->GetUnitlessParameter(GetFullMaterialParmName(name, "BirksConstant"))*mm/MeV);

				G4MaterialPropertiesTable* propertiesTable = new G4MaterialPropertiesTable();
				FillMaterialPropertiesTable(propertiesTable, "Ma/" + name + "/");
				material->SetMaterialPropertiesTable(propertiesTable);
			}

			if (fVerbosity > 0)
				G4cout << "Defined material named: " << name << G4endl;

			if (fPm->ParameterExists(GetFullMaterialParmName(name, "VariableDensityBins"))) {
				G4int relativeDensityBins = fPm->GetIntegerParameter(GetFullMaterialParmName(name, "VariableDensityBins"));
				G4double relativeDensityMin = fPm->GetUnitlessParameter(GetFullMaterialParmName(name, "VariableDensityMin"));
				G4double relativeDensityMax = fPm->GetUnitlessParameter(GetFullMaterialParmName(name, "VariableDensityMax"));
				G4double relativeDensityIncrement = (relativeDensityMax - relativeDensityMin) / (relativeDensityBins - 1);

				for (G4int iBin = 0; iBin < relativeDensityBins; iBin++) {
					G4String newName = name + "_VariableDensityBin_" + G4UIcommand::ConvertToString(iBin);
					(*fMaterialMap)[newName] = new G4Material(newName, material->GetDensity() * (relativeDensityMin + iBin * relativeDensityIncrement), material);
					if (fVerbosity > 0)
						G4cout << "Defined material named: " << newName << G4endl;
				}
			}
		}
		(*fMaterialMap)[name] = material;
	}

	// Can't just use result of previous find because we may be looking for a variable material
	// whereas the previous find was looking only for base material.
	std::map<G4String, G4Material*>::const_iterator iter2 = fMaterialMap->find(fullMaterialName);
	if (iter2 == fMaterialMap->end())
		Quit(name, "The base material has not been defined as having the appropriate number of variable density bins.");
	material = iter2->second;

	return material;
}


void TsMaterialManager::FillMaterialPropertiesTable(G4MaterialPropertiesTable* propertiesTable, G4String prefix) {
    const std::vector<G4String>& constPropertyNames = propertiesTable->GetMaterialConstPropertyNames();
    for (size_t t = 0; t < constPropertyNames.size(); t++ )
        SetConstantProperty(propertiesTable, prefix, constPropertyNames[t]);
    
    const std::vector<G4String>& vectPropertyNames = propertiesTable->GetMaterialPropertyNames();
    for ( size_t t = 0; t < vectPropertyNames.size(); t++ )
        SetVectorProperty(propertiesTable, prefix, vectPropertyNames[t], true);
}


void TsMaterialManager::SetConstantProperty(G4MaterialPropertiesTable* propertiesTable, G4String prefix, G4String propertyName) {
	G4String parameterName = prefix + propertyName;

    if (fPm->ParameterExists(parameterName)){
        G4double property;
        if (fPm->GetUnitOfParameter(parameterName) == "" ) {
            property = fPm->GetUnitlessParameter(parameterName);
        } else {
            property = fPm->GetDoubleParameter(parameterName, fPm->GetUnitCategoryOfParameter(parameterName));
        }
        
#if GEANT4_VERSION_MAJOR >= 11
        G4StrUtil::to_upper(propertyName);
#else
        propertyName.toUpper();
#endif
        propertiesTable->AddConstProperty(propertyName, property, true);

		if (fVerbosity > 0)
			G4cout << "Added constant for optical property from parameter: " << parameterName << G4endl;
	}
}


void TsMaterialManager::SetVectorProperty(G4MaterialPropertiesTable* propertiesTable, G4String prefix, G4String propertyName, G4bool applySpline) {
	G4String valuesParameterName   = prefix + propertyName + "/Values";
	G4String energiesParameterName = prefix + propertyName + "/Energies";

	if (fPm->ParameterExists(valuesParameterName)) {
        G4double* property;
        if (fPm->GetUnitOfParameter(valuesParameterName) == "" ) {
            property = fPm->GetUnitlessVector(valuesParameterName);
        } else {
            property = fPm->GetDoubleVector(valuesParameterName, fPm->GetUnitCategoryOfParameter(valuesParameterName));
        }

		G4int nValues = fPm->GetVectorLength(valuesParameterName);

		if (!fPm->ParameterExists(energiesParameterName))
			Quit(valuesParameterName, "Requires additional parameter: " + energiesParameterName);

		if (fPm->GetVectorLength(energiesParameterName) != nValues)
			Quit(valuesParameterName, "must be same length as: " + energiesParameterName);

		G4double* photonEnergies = fPm->GetDoubleVector(energiesParameterName, "Energy");
		
#if GEANT4_VERSION_MAJOR >= 11
        G4StrUtil::to_upper(propertyName);
		propertiesTable->AddProperty(propertyName, photonEnergies, property, nValues, true, applySpline);
#else
        propertyName.toUpper();
		propertiesTable->AddProperty(propertyName, photonEnergies, property, nValues)->SetSpline(applySpline);
#endif

		if (fVerbosity > 0)
			G4cout << "Added vector for optical property from parameter: " << valuesParameterName << G4endl;
	}
}


G4Material* TsMaterialManager::GetMaterial(const char* c)
{
	G4String sVal = c;
	return GetMaterial(sVal);
}


std::vector<G4String> TsMaterialManager::GetMaterialNames()
{
	std::vector<G4String>* names = new std::vector<G4String>;

	std::vector<G4String>* values = new std::vector<G4String>;
	fPm->GetParameterNamesBracketedBy("Ma", "/Components", values);
	for (G4int iToken=0; iToken<(G4int)values->size(); iToken++) {
		G4String parameterName = (*values)[iToken];
		names->push_back(parameterName.substr(3,parameterName.length()-14));
	}

	G4int nExtraMaterials = fPm->GetVectorLength("Ma/ExtraMaterialNamesForQtMenu");
	G4String* extraMaterials = fPm->GetStringVector("Ma/ExtraMaterialNamesForQtMenu");
	for (G4int i=0; i<nExtraMaterials; i++)
		names->push_back(extraMaterials[i]);

	return *names;
}


G4String TsMaterialManager::GetDefaultMaterialColor(G4String name)
{
	// Find the Default Material Color
	G4String materialDefaultColor = "";
	G4int pos = name.find( "_VariableDensityBin_" );
	if (pos != -1)
		name = name.substr(0,pos);

	if (fPm->ParameterExists(GetFullMaterialParmName(name,"DefaultColor")))
		materialDefaultColor = fPm->GetStringParameter(GetFullMaterialParmName(name,"DefaultColor"));
	else {
		materialDefaultColor = fPm->GetStringParameter("Ma/DefaultColor");
	}

	return materialDefaultColor;
}


G4String TsMaterialManager::GetDefaultMaterialColor(const char* c)
{
	G4String sVal = c;
	return GetDefaultMaterialColor(sVal);
}


// Given an element name and a partial parameter name, such as "Carbon" and "Density",
// return the full parameter name, such as "El/Carbon/Density"
G4String TsMaterialManager::GetFullIsotopeParmName(G4String name, const char* suffix)
{
	G4String parmName = "Is/"+name+"/"+suffix;
	return parmName;
}


// Given an element name and a partial parameter name, such as "Carbon" and "Density",
// return the full parameter name, such as "El/Carbon/Density"
G4String TsMaterialManager::GetFullElementParmName(G4String name, const char* suffix)
{
	G4String parmName = "El/"+name+"/"+suffix;
	return parmName;
}


// Given a material name and a partial parameter name, such as "Air" and "Density",
// return the full parameter name, such as "Ma/Air/Density"
G4String TsMaterialManager::GetFullMaterialParmName(G4String name, const char* suffix)
{
	G4String parmName = "Ma/"+name+"/"+suffix;
	return parmName;
}


void TsMaterialManager::Quit(const G4String& name, const char* message) {
	G4cerr << "Topas is exiting due to a serious error for material name: " << name << G4endl;
	G4cerr << message << G4endl;
	fPm->AbortSession(1);
}


void TsMaterialManager::Quit(const G4String& name, const G4String& message) {
	G4cerr << "Topas is exiting due to a serious error for material name: " << name << G4endl;
	G4cerr << message << G4endl;
	fPm->AbortSession(1);
}
