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

#include "TsVImagingToMaterial.hh"

#include "TsParameterManager.hh"

#include "TsVGeometryComponent.hh"

TsVImagingToMaterial::TsVImagingToMaterial(TsParameterManager* pM,
										   TsVGeometryComponent* component, std::vector<G4Material*>* materialList)
:fPm(pM), fComponent(component), fMaterialList(materialList)
{
}


TsVImagingToMaterial::~TsVImagingToMaterial()
{;}


G4String TsVImagingToMaterial::GetDataType() {
	G4String dataTypeString = "s";
	
	if (fPm->ParameterExists(GetFullParmName("DataType"))) {
		dataTypeString = fPm->GetStringParameter(GetFullParmName("DataType"));
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(dataTypeString);
#else
		dataTypeString.toLower();
#endif
		if (dataTypeString!="short" && dataTypeString!="int"  && dataTypeString!="float") {
			G4cerr << "Error: " << GetFullParmName("DataType") << "Has invalid value: " << fPm->GetStringParameter(GetFullParmName("DataType")) << G4endl;
			G4cerr << "Value must be either SHORT, INT or FLOAT." << G4endl;
			fPm->AbortSession(1);
		}
		dataTypeString = dataTypeString.substr(0,1);
	} else {
		dataTypeString = "s";
	}
	
	return dataTypeString;
}


G4int TsVImagingToMaterial::GetNumberOfZSections() {
	G4String paramTypeNumberOfVoxelsZ = fPm->GetTypeOfParameter(GetFullParmName("NumberOfVoxelsZ"));
	G4String paramTypeVoxelSizeZ = fPm->GetTypeOfParameter(GetFullParmName("VoxelSizeZ"));
	
	if (paramTypeNumberOfVoxelsZ == "i" && paramTypeVoxelSizeZ == "d") {
		return 1;
	} else if (paramTypeNumberOfVoxelsZ == "iv" && paramTypeVoxelSizeZ == "dv") {
	
		G4int lengthOfNumberOfVoxelsZ = fPm->GetVectorLength(GetFullParmName("NumberOfVoxelsZ"));
		G4int lengthOfVoxelSizeZ = fPm->GetVectorLength(GetFullParmName("VoxelSizeZ"));
		if (lengthOfNumberOfVoxelsZ != lengthOfVoxelSizeZ) {
			G4cerr << "ERROR: " << GetFullParmName("NumberOfVoxelsZ") << " needs to have same number of entries as "
				<< GetFullParmName("VoxelSizeZ") << G4endl;
			fPm->AbortSession(1);
		}
		return lengthOfNumberOfVoxelsZ;
	}

	G4cerr << "ERROR: " << GetFullParmName("NumberOfVoxelsZ") << " and " << GetFullParmName("VoxelSizeZ") <<
		" have incompatible parameter types." << G4endl;
	G4cerr << GetFullParmName("NumberOfVoxelsZ") << " should be type i and " << GetFullParmName("VoxelSizeZ") <<
		" should be type d or" << G4endl;
	G4cerr << GetFullParmName("NumberOfVoxelsZ") << " should be type iv and " << GetFullParmName("VoxelSizeZ") <<
		" should be type dv" << G4endl;
	fPm->AbortSession(1);
	return 0;
}

	
G4int TsVImagingToMaterial::GetNumberOfVoxelsX() {
	return fPm->GetIntegerParameter(GetFullParmName("NumberOfVoxelsX"));
}

	
G4int TsVImagingToMaterial::GetNumberOfVoxelsY() {
	return fPm->GetIntegerParameter(GetFullParmName("NumberOfVoxelsY"));
}

	
G4int* TsVImagingToMaterial::GetNumberOfVoxelsZ() {
	G4String paramType = fPm->GetTypeOfParameter(GetFullParmName("NumberOfVoxelsZ"));
	if (paramType == "i") {
		G4int* numberOfVoxelsZ = new G4int[1];
		numberOfVoxelsZ[0] = fPm->GetIntegerParameter(GetFullParmName("NumberOfVoxelsZ"));
		return numberOfVoxelsZ;
	}
	return fPm->GetIntegerVector(GetFullParmName("NumberOfVoxelsZ"));
}


G4double TsVImagingToMaterial::GetVoxelSizeX() {
	return fPm->GetDoubleParameter(GetFullParmName("VoxelSizeX"), "Length");
}


G4double TsVImagingToMaterial::GetVoxelSizeY() {
	return fPm->GetDoubleParameter(GetFullParmName("VoxelSizeY"), "Length");
}


G4double* TsVImagingToMaterial::GetVoxelSizeZ() {
	G4String paramType = fPm->GetTypeOfParameter(GetFullParmName("VoxelSizeZ"));
	if (paramType == "d") {
		G4double* voxelSizeZ = new G4double[1];
		voxelSizeZ[0] = fPm->GetDoubleParameter(GetFullParmName("VoxelSizeZ"), "Length");
		return voxelSizeZ;
	}
	return fPm->GetDoubleVector(GetFullParmName("VoxelSizeZ"), "Length");
}


unsigned short TsVImagingToMaterial::AssignMaterial(std::vector< signed short >*, G4int) {
	return 0;
}


unsigned short TsVImagingToMaterial::AssignMaterialInt(std::vector< int >*, G4int) {
	return 0;
}


unsigned short TsVImagingToMaterial::AssignMaterialFloat(std::vector< float >*, G4int) {
	return 0;
}


G4String TsVImagingToMaterial::GetFullParmName(G4String name) {
	return fComponent->GetFullParmName(name);
}


G4Material* TsVImagingToMaterial::GetMaterial(G4String name) {
	return fComponent->GetMaterial(name);
}

G4Material* TsVImagingToMaterial::GetMaterial(const char* c) {
	return fComponent->GetMaterial(c);
}


G4int TsVImagingToMaterial::DefineNewMaterial(G4String materialName, G4double density, G4double meanExcitationEnergy,
											  G4String colorName, G4int numberOfElements,
											  G4String* elementNames, G4double* elementFractions,
											  G4int relativeDensityBins, G4double relativeDensityMin, G4double relativeDensityMax) {
	G4String parameterName = "d:Ma/" + materialName + "/Density";
	G4String parametervalue = G4UIcommand::ConvertToString( density /(g/cm3) ) + " g/cm3";
	fPm->AddParameter(parameterName, parametervalue);

	if (meanExcitationEnergy!=0.) {
		parameterName = "d:Ma/" + materialName + "/MeanExcitationEnergy";
		parametervalue = G4UIcommand::ConvertToString( meanExcitationEnergy /(eV) ) + " eV";
		fPm->AddParameter(parameterName, parametervalue);
	}

	parameterName = "s:Ma/" + materialName + "/DefaultColor";
	parametervalue = G4String("\"" + colorName + "\"");
	fPm->AddParameter(parameterName, parametervalue);

	G4String elementsString = G4UIcommand::ConvertToString(numberOfElements);
	G4String fractionsString = G4UIcommand::ConvertToString(numberOfElements);
	for (G4int i = 0; i < numberOfElements; i++) {
		elementsString += " \""  + elementNames[i] + "\"";
		fractionsString += " " + G4UIcommand::ConvertToString(elementFractions[i]);
	}

	parameterName = "sv:Ma/" + materialName + "/Components";
	fPm->AddParameter(parameterName, elementsString);

	parameterName = "uv:Ma/" + materialName + "/Fractions";
	fPm->AddParameter(parameterName, fractionsString);

	if (relativeDensityBins != 0) {
		parameterName = "i:Ma/" + materialName + "/VariableDensityBins";
		parametervalue = G4UIcommand::ConvertToString(relativeDensityBins);
		fPm->AddParameter(parameterName, parametervalue);

		parameterName = "u:Ma/" + materialName + "/VariableDensityMin";
		parametervalue = G4UIcommand::ConvertToString(relativeDensityMin);
		fPm->AddParameter(parameterName, parametervalue);

		parameterName = "u:Ma/" + materialName + "/VariableDensityMax";
		parametervalue = G4UIcommand::ConvertToString(relativeDensityMax);
		fPm->AddParameter(parameterName, parametervalue);
	}

	G4Material* baseMaterial = GetMaterial(materialName);
	fMaterialList->push_back(baseMaterial);

	for (G4int iBin = 0; iBin < relativeDensityBins; iBin++) {
		G4String newName = materialName + "_VariableDensityBin_" + G4UIcommand::ConvertToString(iBin);
		fMaterialList->push_back(GetMaterial(newName));
	}

	return fMaterialList->size() - 1 - relativeDensityBins;
}


G4int TsVImagingToMaterial::DefineNewMaterial(G4String materialName, G4double density, G4String baseMaterialName, G4String colorName) {
	G4String parameterName = "d:Ma/" + materialName + "/Density";
	G4String parametervalue = G4UIcommand::ConvertToString( density /(g/cm3) ) + " g/cm3";
	fPm->AddParameter(parameterName, parametervalue);

	parameterName = "s:Ma/" + materialName + "/BaseMaterial";
	parametervalue = G4String("\"" + baseMaterialName + "\"");
	fPm->AddParameter(parameterName, parametervalue);

	parameterName = "s:Ma/" + materialName + "/DefaultColor";
	parametervalue = G4String("\"" + colorName + "\"");
	fPm->AddParameter(parameterName, parametervalue);

	fMaterialList->push_back(GetMaterial(materialName));

	return fMaterialList->size() - 1;
}


void TsVImagingToMaterial::Trim(std::string& instring)
{
	// Remove leading and trailing whitespace
	static const char whitespace[] = " \n\t\v\r\f";
	instring.erase( 0, instring.find_first_not_of(whitespace) );
	instring.erase( instring.find_last_not_of(whitespace) + 1U );
}


// This method is taken from:
// http://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf
std::istream& TsVImagingToMaterial::safeGetline(std::istream& is, std::string& t)
{
	t.clear();

	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.

	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

	for(;;) {
	    int c = sb->sbumpc();
	    switch (c) {
			case '\n':
				return is;
			case '\r':
				if(sb->sgetc() == '\n')
					sb->sbumpc();
				return is;
			case EOF:
				// Also handle the case when the last line has no line ending
				if(t.empty())
					is.setstate(std::ios::eofbit);
				return is;
			default:
				t += (char)c;
	    }
	}
}
