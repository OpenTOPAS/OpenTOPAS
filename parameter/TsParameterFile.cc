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

#include "TsParameterFile.hh"

#include "TsParameterManager.hh"
#include "TsTempParameter.hh"
#include "TsVParameter.hh"
#include "TsTopasConfig.hh"
#include "TsDefaultParameters.hh"

#include "TsBooleanFromBoolean.hh"
#include "TsBooleanFromBooleanTimesBoolean.hh"
#include "TsBooleanFromValue.hh"
#include "TsBooleanFromTimeFeatureStep.hh"
#include "TsBooleanVectorFromBooleanVector.hh"
#include "TsBooleanVectorFromValue.hh"
#include "TsDoubleFromDouble.hh"
#include "TsDoubleFromDoublePlusDouble.hh"
#include "TsDoubleFromDoublePlusValue.hh"
#include "TsDoubleFromDoubleMinusDouble.hh"
#include "TsDoubleFromDoubleMinusValue.hh"
#include "TsDoubleFromDoubleTimesInteger.hh"
#include "TsDoubleFromDoubleTimesUnitless.hh"
#include "TsDoubleFromDoubleTimesValue.hh"
#include "TsDoubleFromIntegerTimesValue.hh"
#include "TsDoubleFromTimeFeature.hh"
#include "TsDoubleFromTimeFeatureStep.hh"
#include "TsDoubleFromUnitlessTimesValue.hh"
#include "TsDoubleFromValue.hh"
#include "TsDoubleFromValuePlusDouble.hh"
#include "TsDoubleFromValueMinusDouble.hh"
#include "TsDoubleFromValueTimesDouble.hh"
#include "TsDoubleFromValueTimesInteger.hh"
#include "TsDoubleFromValueTimesUnitless.hh"
#include "TsDoubleVectorFromDoubleVector.hh"
#include "TsDoubleVectorFromValue.hh"
#include "TsDoubleVectorFromScaleTimesDoubleVector.hh"
#include "TsDoubleVectorFromIntegerTimesDoubleVector.hh"
#include "TsDoubleVectorFromUnitlessTimesDoubleVector.hh"
#include "TsDoubleVectorFromValuePlusDouble.hh"
#include "TsDoubleVectorFromValuePlusDoubleVector.hh"
#include "TsDoubleVectorFromValueMinusDouble.hh"
#include "TsDoubleVectorFromValueMinusDoubleVector.hh"
#include "TsDoubleVectorFromValueTimesDouble.hh"
#include "TsDoubleVectorFromValueTimesDoubleVector.hh"
#include "TsDoubleVectorFromValueTimesInteger.hh"
#include "TsDoubleVectorFromValueTimesIntegerVector.hh"
#include "TsDoubleVectorFromValueTimesUnitless.hh"
#include "TsDoubleVectorFromValueTimesUnitlessVector.hh"
#include "TsIntegerFromInteger.hh"
#include "TsIntegerFromIntegerPlusInteger.hh"
#include "TsIntegerFromIntegerPlusValue.hh"
#include "TsIntegerFromIntegerMinusInteger.hh"
#include "TsIntegerFromIntegerMinusValue.hh"
#include "TsIntegerFromIntegerTimesInteger.hh"
#include "TsIntegerFromIntegerTimesUnitless.hh"
#include "TsIntegerFromIntegerTimesValue.hh"
#include "TsIntegerFromTimeFeatureStep.hh"
#include "TsIntegerFromUnitless.hh"
#include "TsIntegerFromValue.hh"
#include "TsIntegerFromValuePlusInteger.hh"
#include "TsIntegerFromValueMinusInteger.hh"
#include "TsIntegerFromValueTimesInteger.hh"
#include "TsIntegerFromValueTimesUnitless.hh"
#include "TsIntegerVectorFromIntegerVector.hh"
#include "TsIntegerVectorFromValue.hh"
#include "TsIntegerVectorFromScaleTimesIntegerVector.hh"
#include "TsIntegerVectorFromIntegerTimesIntegerVector.hh"
#include "TsIntegerVectorFromValuePlusInteger.hh"
#include "TsIntegerVectorFromValuePlusIntegerVector.hh"
#include "TsIntegerVectorFromValueMinusInteger.hh"
#include "TsIntegerVectorFromValueMinusIntegerVector.hh"
#include "TsIntegerVectorFromValueTimesInteger.hh"
#include "TsIntegerVectorFromValueTimesIntegerVector.hh"
#include "TsStringFromInteger.hh"
#include "TsStringFromIntegerPlusInteger.hh"
#include "TsStringFromIntegerPlusString.hh"
#include "TsStringFromIntegerPlusValue.hh"
#include "TsStringFromString.hh"
#include "TsStringFromStringPlusInteger.hh"
#include "TsStringFromStringPlusString.hh"
#include "TsStringFromStringPlusValue.hh"
#include "TsStringFromTimeFeatureStep.hh"
#include "TsStringFromValue.hh"
#include "TsStringFromValuePlusInteger.hh"
#include "TsStringFromValuePlusString.hh"
#include "TsStringVectorFromStringVector.hh"
#include "TsStringVectorFromValue.hh"
#include "TsStringVectorFromValuePlusInteger.hh"
#include "TsStringVectorFromValuePlusIntegerVector.hh"
#include "TsStringVectorFromValuePlusString.hh"
#include "TsStringVectorFromValuePlusStringVector.hh"
#include "TsUnitlessFromInteger.hh"
#include "TsUnitlessFromIntegerPlusInteger.hh"
#include "TsUnitlessFromIntegerPlusUnitless.hh"
#include "TsUnitlessFromIntegerPlusValue.hh"
#include "TsUnitlessFromIntegerMinusInteger.hh"
#include "TsUnitlessFromIntegerMinusUnitless.hh"
#include "TsUnitlessFromIntegerMinusValue.hh"
#include "TsUnitlessFromIntegerTimesInteger.hh"
#include "TsUnitlessFromIntegerTimesUnitless.hh"
#include "TsUnitlessFromIntegerTimesValue.hh"
#include "TsUnitlessFromTimeFeature.hh"
#include "TsUnitlessFromTimeFeatureStep.hh"
#include "TsUnitlessFromUnitless.hh"
#include "TsUnitlessFromUnitlessPlusInteger.hh"
#include "TsUnitlessFromUnitlessPlusUnitless.hh"
#include "TsUnitlessFromUnitlessPlusValue.hh"
#include "TsUnitlessFromUnitlessMinusInteger.hh"
#include "TsUnitlessFromUnitlessMinusUnitless.hh"
#include "TsUnitlessFromUnitlessMinusValue.hh"
#include "TsUnitlessFromUnitlessTimesUnitless.hh"
#include "TsUnitlessFromUnitlessTimesInteger.hh"
#include "TsUnitlessFromUnitlessTimesValue.hh"
#include "TsUnitlessFromValue.hh"
#include "TsUnitlessFromValuePlusInteger.hh"
#include "TsUnitlessFromValuePlusUnitless.hh"
#include "TsUnitlessFromValueMinusInteger.hh"
#include "TsUnitlessFromValueMinusUnitless.hh"
#include "TsUnitlessFromValueTimesInteger.hh"
#include "TsUnitlessFromValueTimesUnitless.hh"
#include "TsUnitlessVectorFromUnitlessVector.hh"
#include "TsUnitlessVectorFromValue.hh"
#include "TsUnitlessVectorFromScaleTimesUnitlessVector.hh"
#include "TsUnitlessVectorFromIntegerTimesUnitlessVector.hh"
#include "TsUnitlessVectorFromUnitlessTimesUnitlessVector.hh"
#include "TsUnitlessVectorFromValuePlusInteger.hh"
#include "TsUnitlessVectorFromValuePlusIntegerVector.hh"
#include "TsUnitlessVectorFromValuePlusUnitless.hh"
#include "TsUnitlessVectorFromValuePlusUnitlessVector.hh"
#include "TsUnitlessVectorFromValueMinusInteger.hh"
#include "TsUnitlessVectorFromValueMinusIntegerVector.hh"
#include "TsUnitlessVectorFromValueMinusUnitless.hh"
#include "TsUnitlessVectorFromValueMinusUnitlessVector.hh"
#include "TsUnitlessVectorFromValueTimesInteger.hh"
#include "TsUnitlessVectorFromValueTimesIntegerVector.hh"
#include "TsUnitlessVectorFromValueTimesUnitless.hh"
#include "TsUnitlessVectorFromValueTimesUnitlessVector.hh"

#include <fstream>

#include "G4Tokenizer.hh"

TsParameterFile::TsParameterFile(TsParameterManager* pM, const G4String& fileSpec,
								 const G4String& topFileSpec, TsParameterFile* transientFile)
:fPm(pM), fFileSpec(fileSpec), fParentFile(0), fTransientFile(transientFile)
{
	//G4cout << "instantiating TsParameterFile for fileSpec: " << fileSpec << G4endl;
	fDefaultFileSpec = "TOPAS_Built_In_Defaults";
	fIncludeFileSpecs = "";
	fIncludeFiles = new std::vector<TsParameterFile*>;
	fTempParameters = new std::map<G4String,TsTempParameter*>;
	fRealParameters = new std::map<G4String,TsVParameter*>;
	fTimeFeatureParameters = new std::vector<TsVParameter*>;
	// The transient parameter "file" has no actual file to read in, and its parent is the user's specified top file
	if (fFileSpec == "TransientParameters") {
		fTransientFile = this;
		fIncludeFileSpecs = topFileSpec;
	} else if (fFileSpec == fDefaultFileSpec) {
		TsDefaultParameters::SetDefaults(this);
	} else {
		std::ifstream infile(fileSpec);
		if (!infile) {
			if (fileSpec == "TsUserParameters.txt") {
				G4cerr << "Topas quitting. Unable to find top level parameter file." << G4endl;
				G4cerr << "Specify the top level parameter file on the command line," << G4endl;
				G4cerr << "or provide a file named TsUserParameters.txt." << G4endl;
			} else {
				G4cerr << "Topas quitting, unable to open parameter file:" << fileSpec << G4endl;
			}
			fPm->AbortSession(1);
		} else {
			std::vector<G4String>* names = new std::vector<G4String>;
			std::vector<G4String>* values = new std::vector<G4String>;
			fPm->ReadFile(fileSpec, infile, names, values);

			G4int length = names->size();
			for (G4int iToken=0; iToken<length; iToken++) {
				G4String name = (*names)[iToken];
				G4String value = (*values)[iToken];
				AddTempParameter(name, value);
			}
			delete names;
			delete values;
		}
	}

	// If there are no include files and this is not the default file, include the default file
	if (fIncludeFileSpecs=="" && fFileSpec!=fDefaultFileSpec)
		fIncludeFileSpecs = fDefaultFileSpec;

	// Instantiate all of the include files.
	TsParameterFile* includeFile;
	G4Tokenizer next(fIncludeFileSpecs);
	for (G4String includeFileSpec = next(); !includeFileSpec.empty(); includeFileSpec=next()) {
		// If include file has already been instantiated, get its pointer.
		// Otherwise, instantiate it (a recursive call to this current method).
		includeFile = fPm->GetParameterFile(includeFileSpec);
		if (!includeFile) {
			includeFile = new TsParameterFile(fPm, includeFileSpec, topFileSpec, fTransientFile);
			fPm->RegisterParameterFile(includeFileSpec, includeFile);
		}
		fIncludeFiles->push_back(includeFile);
	}

	// Check that no two of the included parameter file chains conflict
	CheckChainsForConflicts();

	// Now that all of the include files have been instantiated, we can convert the temporary parameters into real parameters.
	ProcessTempParameters();

	// Now that all parameters have been instantiated, check consistency with relative parameters
	std::map<G4String, TsVParameter*>::const_iterator iter;
	for (iter = fRealParameters->begin(); iter != fRealParameters->end(); ++iter)
		iter->second->CheckRelativeParameterConsistency();
}


TsParameterFile::~TsParameterFile()
{
	delete fTempParameters;
	delete fRealParameters;
}


// Only do the most minimal parsing at this stage, since some parameters don't have type specified directly.
// We won't know all the types until we have all the parent parameters loaded.
TsTempParameter* TsParameterFile::AddTempParameter(const G4String& typeAndName, const G4String& value)
{
	TsTempParameter* tempParameter = 0;
	G4bool isChangeable = false;

	//G4cout << "AddTempParameter called, file:" << fFileSpec << ", typeAndName:" << typeAndName << ", value:" << value << G4endl;
	if (typeAndName!="") {
		// If name is parentFile, this is not a real parameter, but rather is the name of a parent parameter file.
		G4String typeAndNameInLower = typeAndName;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(typeAndNameInLower);
#else
		typeAndNameInLower.toLower();
#endif
		if (typeAndNameInLower=="includefile") {
			if (value != "TOPAS_Built_In_Defaults")
				fIncludeFileSpecs += " " + value;
		} else {
			G4String type;
			G4String name;

			// Locate colon if there is one
			const auto colonPos = typeAndName.find( ":" );
			if (colonPos==G4String::npos) {
				// No colon means this parameter does not explicitly state a type
				type = "";
				name = typeAndName;
			} else if (colonPos==typeAndName.size()-1) {
				Quit(typeAndName, "Parameter has no name after the colon");
			} else {
				G4String leftSide = typeAndNameInLower.substr(0,colonPos);

				if (leftSide=="")
					Quit(typeAndName, "Parameter is missing characters before the colon");

				if (leftSide.substr(colonPos-1,1) == "c") {
					isChangeable = true;
					leftSide = leftSide.substr(0,colonPos-1);
				}

				if (leftSide=="d" || leftSide=="u" || leftSide=="i" || leftSide=="b" || leftSide=="s" ||
					leftSide=="dv" || leftSide=="uv" || leftSide=="iv" || leftSide=="bv" || leftSide=="sv") {
					type = leftSide;
					name = typeAndName.substr(colonPos+1);
				} else {
					Quit(typeAndName, "Parameter has incorrect characters before the colon");
				}
			}

			// Do not allow name to start with slash as this is a common user typing error
			if (name.substr(0,1) == "/")
				Quit(typeAndName, "Parameter name should not start with a slash");

			// To ignore case of names, we use the lower case version of the name as the map index
			G4String nameInLower(name);
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(nameInLower);
#else
			nameInLower.toLower();
#endif

			// Some parameters are not allowed to be changeable.
			// This is not an exhaustive list, but are the ones we've seen commonly misused.
			if (isChangeable) {
				G4String prefix = nameInLower.substr(0,2);
				size_t lastSlashPos;
				lastSlashPos = nameInLower.find_last_of( "/" );
				G4String suffix = nameInLower.substr(lastSlashPos+1);
				if ((prefix == "ge" && suffix == "type") ||
					(prefix == "sc" && suffix == "quantity") ||
					(prefix == "sc" && suffix == "component") ||
					(prefix == "so" && suffix == "type") ||
					(prefix == "so" && suffix == "phasespacemultipleuse") ||
					(prefix == "gr" && suffix == "type") ||
					(prefix == "ph" && suffix == "modules") ||
					(prefix == "ph" && suffix == "listname") ||
					(prefix == "vr" && suffix == "usevariancereduction") ||
					(prefix == "vr" && suffix == "component") ||
					(nameInLower == "ge/world/hlx") ||
					(nameInLower == "ge/world/hly") ||
					(nameInLower == "ge/world/hlz"))
					Quit(typeAndName, "This parameter should never be set to changeable.");
			}

			// We don't allow a given parameter to be redefined within the same file.
			// However, for patient material parameters, we skip this check as it is time consuming
			// and we know that these parameters were automatically generated from a trusted class.
			if (nameInLower.substr(0,16)!="ma/patienttissue") {
				std::map<G4String, TsTempParameter*>::const_iterator iter = fTempParameters->find(nameInLower);
				if (iter != fTempParameters->end())
					Quit(typeAndName, "Parameter has already been defined elsewhere in this same file");
			}

			tempParameter = new TsTempParameter(name, type, value, isChangeable);
			(*fTempParameters)[nameInLower] = tempParameter;
		}
	}
	return tempParameter;
}


void TsParameterFile::CheckChainsForConflicts() {

	// Now need to check that no two chains modify the same parameter unless top file sets that parameter absolutely.
	if (fIncludeFiles->size() > 1) {
		std::vector<TsParameterFile*>::iterator includeFileIter1;
		std::vector<TsParameterFile*>::iterator includeFileIter2;
		std::map<G4String, TsVParameter*>::const_iterator iterParam;

		// Loop over include files:
		for (includeFileIter1 = fIncludeFiles->begin(); includeFileIter1 != fIncludeFiles->end(); ++includeFileIter1) {
			TsParameterFile* includeFile1 = *includeFileIter1;

			// Loop over all parameter names in this include file
			std::map<G4String, TsVParameter*>* paramMap = new std::map<G4String, TsVParameter*>;
			includeFile1->GetAllParametersBeforeLinearized(paramMap);
			for (iterParam = paramMap->begin(); iterParam != paramMap->end(); ++iterParam) {
				G4String paramName = iterParam->first;

				// Loop over all other include files, looking for cases where
				// the same parameter name refers to two different parameters in the two different include files
				if (includeFileIter1 != fIncludeFiles->end()-1) {
					for (includeFileIter2 = includeFileIter1+1; includeFileIter2 != fIncludeFiles->end(); ++includeFileIter2) {
						TsParameterFile* includeFile2 = *includeFileIter2;
						TsVParameter* includeFile2Param = includeFile2->GetParameterBeforeLinearized(paramName);
						if (includeFile2Param && (includeFile2Param != iterParam->second)) {
							// Same parameter name refers to two different parameters in the two different include files
							std::map<G4String, TsTempParameter*>::const_iterator iterTemp = fTempParameters->find(paramName);
							if (iterTemp == fTempParameters->end()) {
								G4String message = "Parameter is set in two different parameter chains included from " + GetFileName()
								+ " and not set absolutely in that top file.";
								message += "\n  Chain 1:";
								std::vector<TsParameterFile*>* parameterFiles = new std::vector<TsParameterFile*>;
								includeFile1->BuildVectorOfParameterFiles(parameterFiles);
								std::vector<TsParameterFile*>::iterator iter2;
								for (iter2 = parameterFiles->begin(); iter2 != parameterFiles->end(); ++iter2)
									message += " " + (*iter2)->GetFileName();
								message += "\n  Chain 2:";
								parameterFiles->clear();
								includeFile2->BuildVectorOfParameterFiles(parameterFiles);
								for (iter2 = parameterFiles->begin(); iter2 != parameterFiles->end(); ++iter2)
									message += " " + (*iter2)->GetFileName();
								Quit (paramName, message);
							} else
								iterTemp->second->SetMustBeAbsolute();
						}
					}
				}

				// Protect against an object being controlled by two different arms of the parameter graph
				ProtectAgainstControlByDifferentArms(paramName, includeFile1, "ma/", "/components");
				ProtectAgainstControlByDifferentArms(paramName, includeFile1, "ge/", "/type");
				ProtectAgainstControlByDifferentArms(paramName, includeFile1, "so/", "/type");
				ProtectAgainstControlByDifferentArms(paramName, includeFile1, "sc/", "/quantity");
				ProtectAgainstControlByDifferentArms(paramName, includeFile1, "tf/", "/function");
				ProtectAgainstControlByDifferentArms(paramName, includeFile1, "gr/", "/type");
				ProtectAgainstControlByDifferentArms(paramName, includeFile1, "ph/", "/type");
				ProtectAgainstControlByDifferentArms(paramName, includeFile1, "vr/", "/type");
			}
		}
	}
}


void TsParameterFile::ProcessTempParameters(G4bool test) {
	//G4cout << "In ProcessTempParameters for file: " << fFileSpec << G4endl;
	// Loop over temp parameters.
	std::map<G4String, TsTempParameter*>::const_iterator iter;

	if (!test) {
	for (iter = fTempParameters->begin(); iter != fTempParameters->end(); ++iter) {
		TsTempParameter* tempParameter = iter->second;
		G4String name  = iter->first;
		G4String type  = tempParameter->GetType();
		G4String value = tempParameter->GetValue();
		G4bool isChangeable = tempParameter->IsChangeable();
		TsParameterFile* includeFile;

		// If same parameter is in any include files, check type consistency or fill in missing type
		std::vector<TsParameterFile*>::iterator iter2;
		for (iter2 = fIncludeFiles->begin(); iter2 != fIncludeFiles->end(); ++iter2) {
			includeFile = *iter2;
			if (includeFile->ParameterExists(name)) {
				// Found same parameter name
				// If type was missing, fill it in. Otherwise, check that type is consistent.
				if (type == "") {
					type = includeFile->GetTypeOfParameter(name);
					isChangeable = includeFile->IsChangeable(name);
					tempParameter->SetType(type);
					tempParameter->SetChangeable(isChangeable);
				} else {
					if (type != includeFile->GetTypeOfParameter(name))
						Quit (tempParameter->GetName(), "Parameter has different type in different files.");
					if (isChangeable != includeFile->IsChangeable(name)) {
						if (this==fTransientFile)
							Quit (tempParameter->GetName(), "An attempt has been made to override a non-changeable parameter.");
						else
							Quit (tempParameter->GetName(), "Parameter has different settings for changeable in different files.");
					}
				}
			}
		}

		// If parameter is a time feature function, add Value parameter
		if ( name.length() > 12 && name.substr(0,3) =="tf/") {
			TsTempParameter* tmp_tf_parameter = 0;
			TsVParameter* tf_parameter = 0;
			G4String param_type;
			G4String param_value;
			G4String param_name;
			G4String param_name_base;
			G4String unit_name;
			G4int nValues = 0;
			G4String* values = 0;

			if ( name.substr(name.length()-7) == "/values" ) {
				fPm->SetLastDirectAction("Instantiate Time Feature");
				fPm->SetLastDirectParameterName(name);

				G4int lengthTobase = sizeof("/Values") -1;
				G4Tokenizer next(value);
				G4String value1 = next();
				if (!fPm->IsInteger(value1, 32)) Quit(tempParameter, "Double vector parameter value must start with an integer.");
				nValues=G4UIcommand::ConvertToInt(value1);
				if (nValues < 0) Quit(tempParameter, "Double vector parameter has a negative integer for number of values");
				values = new G4String[nValues];

				for (G4int iToken=0; iToken<nValues; iToken++) {
					values[iToken] = next();
					if (values[iToken].empty()) Quit(tempParameter, "Double vector parameter value has wrong number of values.");
				}

				// In some of the below, we use the unusual value 0.000000001 so that diagnostic output will make it more obvious when
				// values were never properly initialized.
				if (type == "bv") {
					param_type = "b";
					param_value = "\"true\"";
				} else if (type=="dv") {
					unit_name = next();
					if (fPm->IsDouble(unit_name))
						Quit(tempParameter->GetName(), "Double vector parameter has another double where the unit was expected to be.\nCheck that correct number of values was provided.");

					param_type = "d";
					param_value = "0.000000001 " + unit_name;
				} else if (type=="iv") {
					param_type = "i";
					param_value = "0";
				} else if (type=="sv") {
					param_type = "s";
					param_value = "\"null\"";
				} else if (type=="uv") {
					param_type  = "u";
					param_value = "0.000000001";
				} else {
					Quit (tempParameter->GetName(), "Time feature step does not support this type.");
				}

				param_name_base = name.substr(0,name.length()-lengthTobase);
				param_name = (name.substr(0,name.length()-lengthTobase) + "/value");
				tmp_tf_parameter = new TsTempParameter(param_name, param_type, param_value, true);

				if (param_type =="b")
					tf_parameter = new TsBooleanFromTimeFeatureStep(fPm, this, tmp_tf_parameter,
																	param_name_base, nValues, values);
				else if (param_type =="d")
					tf_parameter = new TsDoubleFromTimeFeatureStep(fPm, this, tmp_tf_parameter,
																   param_name_base, nValues, values ,unit_name);
				else if (param_type =="i")
					tf_parameter = new TsIntegerFromTimeFeatureStep(fPm, this, tmp_tf_parameter,
																	param_name_base, nValues, values);
				else if (param_type =="s")
					tf_parameter = new TsStringFromTimeFeatureStep(fPm, this, tmp_tf_parameter,
																   param_name_base, nValues, values);
				else if (param_type =="u")
					tf_parameter = new TsUnitlessFromTimeFeatureStep(fPm, this, tmp_tf_parameter,
																	 param_name_base, nValues, values);

			} else if (name.substr(name.length()-9) == "/function") {
				fPm->SetLastDirectAction("Instantiate Time Feature");
				fPm->SetLastDirectParameterName(name);

				G4int lengthToBase = sizeof("/Function") -1;
				if (value.substr(0,1)!="\"")
					Quit (name, "Time feature function parameter has non-string value");

				const auto secondQuotePos = value.find('"',1);
				if (secondQuotePos==G4String::npos)
					Quit (name, "Time feature function parameter has non-string value");

				value = value.substr(1,secondQuotePos-1);
				fPm->Trim(value);

				G4Tokenizer next(value);
				G4String function_name = next();
				unit_name = next();
				if (fPm->IsDouble(unit_name))
					Quit(tempParameter->GetName(), "Double vector parameter has another double where the unit was expected to be.\nCheck that correct number of values was provided.");

				G4String lower_unit_name = unit_name;
#if GEANT4_VERSION_MAJOR >= 11
				G4StrUtil::to_lower(function_name);
				G4StrUtil::to_lower(lower_unit_name);
#else
				function_name.toLower();
				lower_unit_name.toLower();
#endif

				if (function_name!="step" && unit_name!="" && lower_unit_name=="boolean")
					Quit(tempParameter->GetName(), "Only time feature function that supports Boolean values is Step.");

				if (function_name!="step" && unit_name!="" && lower_unit_name=="string")
					Quit(tempParameter->GetName(), "Only time feature function that supports String values is Step.");

				if ((function_name=="sine" || function_name=="cosine") && unit_name!="")
					Quit(tempParameter->GetName(), "Sine and Cosine time features must be unitless.");

				if ( function_name != "step") {
					if (unit_name=="") {
						param_type  = "u";
						param_value = "0.000000001";
					} else if (fPm->GetUnitCategory(unit_name)=="None") {
						Quit(tempParameter->GetName(), "Time Feature Function parameter has invalid unit");
					} else {
						param_type = "d";
						param_value = "0.000000001 " + unit_name;
					}

					param_name_base = name.substr(0,name.length()-lengthToBase);
					param_name = (name.substr(0,name.length()-lengthToBase) + "/value");
					tmp_tf_parameter = new TsTempParameter(param_name, param_type, param_value, true);

					if ( param_type == "d" )
						tf_parameter = new TsDoubleFromTimeFeature(fPm, this, tmp_tf_parameter,
																   param_name_base, unit_name);
					else if ( param_type == "u")
						tf_parameter = new TsUnitlessFromTimeFeature(fPm, this, tmp_tf_parameter, param_name_base);
				}
			}

			if ( tf_parameter != 0) {
				fTimeFeatureParameters->push_back(tf_parameter);
				(*fRealParameters)[param_name] = tf_parameter;
			}
		}
	}

	// Since have guaranteed that nothing in one chain affects anything in another chain, rearrange the files into a single chain.
	// Create a list of all files in first chain, plus all files in second chain, third chain, etc.
	std::vector<TsParameterFile*>* parameterFiles = new std::vector<TsParameterFile*>;
	parameterFiles->push_back(this);

	std::vector<TsParameterFile*>::iterator iter2;
	for (iter2 = fIncludeFiles->begin(); iter2 != fIncludeFiles->end(); ++iter2)
		(*iter2)->BuildVectorOfParameterFiles(parameterFiles);

	//G4cout << ">>>> Vector of parameter files before culling, for file: " << fFileSpec << G4endl;
	//for (iter2 = parameterFiles->begin(); iter2 != parameterFiles->end(); ++iter2)
	//	G4cout << (*iter2)->GetFileName() << G4endl;

	// Zero out all of the parent pointers.
	for (iter2 = parameterFiles->begin(); iter2 != parameterFiles->end(); ++iter2)
		(*iter2)->ResetParentFileTo(0);

	// Working through this list backwards, reset the parent pointers, skipping over any that already have a parent pointer set.
	TsParameterFile* previousFile = 0;
	TsParameterFile* firstFile =0;
	for (iter2 = parameterFiles->end()-1; iter2 != parameterFiles->begin()-1; --iter2) {
		TsParameterFile* currentFile = (*iter2);
		if (!firstFile) {
			firstFile = currentFile;
			previousFile = currentFile;
		}

		TsParameterFile* parentFile = currentFile->GetParentFile();
		if (currentFile!=firstFile && !parentFile) {
			currentFile->ResetParentFileTo(previousFile);
			previousFile = currentFile;
		}
	}

	delete parameterFiles;
	}

	// Loop over all TsTempParameters, creating final parameters based on type.
	// Within each parameter type's constructor, additional checks are done, such as to see that numbers have correct format
	// and that relative parameters themselves have appropriate types.
	for (iter = fTempParameters->begin(); iter != fTempParameters->end(); ++iter) {
		TsTempParameter* tempParameter = iter->second;
		G4String name = iter->first;
		G4String type = tempParameter->GetType();
		G4String value = tempParameter->GetValue();
		TsVParameter* parameter = 0;
		G4String value1;
		G4String value2;
		G4String unit;
		G4String token;
		G4String commentToken;
		G4String tokenAfterUnit;
		G4int nValues;
		G4String* values;
		G4String typeOfValue2;

		fPm->SetLastDirectAction("Instantiate");
		fPm->SetLastDirectParameterName(tempParameter->GetName());

		if (type=="d") {
			// Double parameters may have values of the following forms (or one of these plus # comment)
			// number * name_of_double_parameter unit
			// number unit + name_of_double_parameter
			// number unit - name_of_double_parameter
			// number unit * name_of_unitless_or_integer_parameter
			// number unit
			// name_of_double_parameter + number unit
			// name_of_double_parameter + name_of_double_parameter unit
			// name_of_double_parameter - number unit
			// name_of_double_parameter - name_of_double_parameter unit
			// name_of_unitless_or_integer_parameter * number unit
			// name_of_double_parameter unit * number
			// name_of_double_parameter unit * name_of_unitless_or_integer_parameter
			// name_of_double_parameter unit
			G4Tokenizer next(value);
			value1 = next();
			token = next();

			if (fPm->IsDouble(value1)) {
				// Case is one of these:
				// number * name_of_double_parameter unit
				// number unit + name_of_double_parameter
				// number unit - name_of_double_parameter
				// number unit * name_of_unitless_or_integer_parameter
				// number unit
				if (token=="*") {
					// number * name_of_double_parameter unit
					value2 = next();
					parameter = new TsDoubleFromValueTimesDouble(fPm, this, tempParameter, value1, value2, next());
					commentToken = next();
				} else {
					// Case is one of these:
					// number unit + name_of_double_parameter
					// number unit - name_of_double_parameter
					// number unit * name_of_unitless_or_integer_parameter
					// number unit
					unit = token;
					tokenAfterUnit = next();

					if (tokenAfterUnit=="+") {
						// number unit + name_of_double_parameter
						parameter = new TsDoubleFromValuePlusDouble(fPm, this, tempParameter, value1, next(), unit);
						commentToken = next();
					} else if (tokenAfterUnit=="-") {
						// number unit - name_of_double_parameter
						parameter = new TsDoubleFromValueMinusDouble(fPm, this, tempParameter, value1, next(), unit);
						commentToken = next();
					} else if (tokenAfterUnit=="*") {
						// number unit * name_of_unitless_or_integer_parameter
						value2 = next();
						if (GetTempTypeOfParameter(value2)=="u")
							parameter = new TsDoubleFromValueTimesUnitless(fPm, this, tempParameter, value1, value2, unit);
						else
							parameter = new TsDoubleFromValueTimesInteger(fPm, this, tempParameter, value1, value2, unit);
						commentToken = next();
					} else {
						// number unit
						parameter = new TsDoubleFromValue(fPm, this, tempParameter, value1, unit);
						commentToken = tokenAfterUnit;
					}
				}
			} else {
				// Case is one of these:
				// name_of_double_parameter + number unit
				// name_of_double_parameter + name_of_double_parameter unit
				// name_of_double_parameter - number unit
				// name_of_double_parameter - name_of_double_parameter unit
				// name_of_unitless_or_integer_parameter * number unit
				// name_of_double_parameter unit * number
				// name_of_double_parameter unit * name_of_unitless_or_integer_parameter
				// name_of_double_parameter unit
				if (token=="+") {
					// Case is one of these:
					// name_of_double_parameter + number unit
					// name_of_double_parameter + name_of_double_parameter unit
					value2 = next();
					if (fPm->IsDouble(value2))
						// name_of_double_parameter + number unit
						parameter = new TsDoubleFromDoublePlusValue(fPm, this, tempParameter, value1, value2, next());
					else
						// name_of_double_parameter + name_of_double_parameter unit
						parameter = new TsDoubleFromDoublePlusDouble(fPm, this, tempParameter, value1, value2, next());
					commentToken = next();
				} else if (token=="-") {
					// Case is one of these:
					// name_of_double_parameter - number unit
					// name_of_double_parameter - name_of_double_parameter unit
					value2 = next();
					if (fPm->IsDouble(value2))
						// name_of_double_parameter - number unit
						parameter = new TsDoubleFromDoubleMinusValue(fPm, this, tempParameter, value1, value2, next());
					else
						// name_of_double_parameter - name_of_double_parameter unit
						parameter = new TsDoubleFromDoubleMinusDouble(fPm, this, tempParameter, value1, value2, next());
					commentToken = next();
				} else if (token=="*") {
					// name_of_unitless_or_integer_parameter * number unit
					value2 = next();
					if (GetTempTypeOfParameter(value1)=="u")
						parameter = new TsDoubleFromUnitlessTimesValue(fPm, this, tempParameter, value1, value2, next());
					else
						parameter = new TsDoubleFromIntegerTimesValue(fPm, this, tempParameter, value1, value2, next());
					commentToken = next();
				} else {
					// Case is one of these:
					// name_of_double_parameter unit * number
					// name_of_double_parameter unit * name_of_unitless_or_integer_parameter
					// name_of_double_parameter unit
					unit = token;
					tokenAfterUnit = next();
					if (tokenAfterUnit=="*") {
						value2 = next();
						if (fPm->IsDouble(value2))
							// name_of_double_parameter unit * number
							parameter = new TsDoubleFromDoubleTimesValue(fPm, this, tempParameter, value1, value2, unit);
						else if (GetTempTypeOfParameter(value2)=="u")
							// name_of_double_parameter unit * name_of_unitless_parameter
							parameter = new TsDoubleFromDoubleTimesUnitless(fPm, this, tempParameter, value1, value2, unit);
						else
							// name_of_double_parameter unit * name_of_integer_parameter
							parameter = new TsDoubleFromDoubleTimesInteger(fPm, this, tempParameter, value1, value2, unit);
						commentToken = next();
					} else {
						// name_of_double_parameter unit
						parameter = new TsDoubleFromDouble(fPm, this, tempParameter, value1, unit);
						commentToken = tokenAfterUnit;
					}
				}
			}
		} else if (type=="u") {
			// Unitless parameters may have values of the following forms (or one of these plus # comment)
			// number + name_of_unitless_or_integer_parameter
			// number - name_of_unitless_or_integer_parameter
			// number * name_of_unitless_or_integer_parameter
			// number
			// name_of_unitless_or_integer_parameter + number
			// name_of_unitless_or_integer_parameter + name_of_unitless_or_integer_parameter
			// name_of_unitless_or_integer_parameter - number
			// name_of_unitless_or_integer_parameter - name_of_unitless_or_integer_parameter
			// name_of_unitless_or_integer_parameter * number
			// name_of_unitless_or_integer_parameter * name_of_unitless_or_integer_parameter
			// name_of_unitless_or_integer_parameter
			G4Tokenizer next(value);
			value1 = next();
			token = next();

			if (fPm->IsDouble(value1)) {
				// Case is one of these:
				// number + name_of_unitless_or_integer_parameter
				// number - name_of_unitless_or_integer_parameter
				// number * name_of_unitless_or_integer_parameter
				// number
				if (token=="+") {
					// number + name_of_unitless_or_integer_parameter
					value2 = next();
					if (GetTempTypeOfParameter(value2)=="u")
						parameter = new TsUnitlessFromValuePlusUnitless(fPm, this, tempParameter, value1, value2);
					else
						parameter = new TsUnitlessFromValuePlusInteger(fPm, this, tempParameter, value1, value2);
					commentToken = next();
				} else if (token=="-") {
					// number - name_of_unitless_or_integer_parameter
					value2 = next();
					if (GetTempTypeOfParameter(value2)=="u")
						parameter = new TsUnitlessFromValueMinusUnitless(fPm, this, tempParameter, value1, value2);
					else
						parameter = new TsUnitlessFromValueMinusInteger(fPm, this, tempParameter, value1, value2);
					commentToken = next();
				} else if (token=="*") {
					// number * name_of_unitless_or_integer_parameter
					value2 = next();
					if (GetTempTypeOfParameter(value2)=="u")
						parameter = new TsUnitlessFromValueTimesUnitless(fPm, this, tempParameter, value1, value2);
					else
						parameter = new TsUnitlessFromValueTimesInteger(fPm, this, tempParameter, value1, value2);
					commentToken = next();
				} else {
					// number
					parameter = new TsUnitlessFromValue(fPm, this, tempParameter, value1);
					commentToken = token;
				}
			} else {
				// Case is one of these:
				// name_of_unitless_or_integer_parameter + number
				// name_of_unitless_or_integer_parameter + name_of_unitless_or_integer_parameter
				// name_of_unitless_or_integer_parameter - number
				// name_of_unitless_or_integer_parameter - name_of_unitless_or_integer_parameter
				// name_of_unitless_or_integer_parameter * number
				// name_of_unitless_or_integer_parameter * name_of_unitless_or_integer_parameter
				// name_of_unitless_or_integer_parameter
				if (token=="+") {
					// Case is one of these:
					// name_of_unitless_or_integer_parameter + number
					// name_of_unitless_or_integer_parameter + name_of_unitless_or_integer_parameter
					value2 = next();
					if (fPm->IsDouble(value2)) {
						// name_of_unitless_or_integer_parameter + number
						if (GetTempTypeOfParameter(value1)=="u")
							parameter = new TsUnitlessFromUnitlessPlusValue(fPm, this, tempParameter, value1, value2);
						else
							parameter = new TsUnitlessFromIntegerPlusValue(fPm, this, tempParameter, value1, value2);
					} else {
						// name_of_unitless_or_integer_parameter + name_of_unitless_or_integer_parameter
						if (GetTempTypeOfParameter(value1)=="u") {
							if (GetTempTypeOfParameter(value2)=="u")
								parameter = new TsUnitlessFromUnitlessPlusUnitless(fPm, this, tempParameter, value1, value2);
							else
								parameter = new TsUnitlessFromUnitlessPlusInteger(fPm, this, tempParameter, value1, value2);
						} else {
							if (GetTempTypeOfParameter(value2)=="i")
								parameter = new TsUnitlessFromIntegerPlusInteger(fPm, this, tempParameter, value1, value2);
							else
								parameter = new TsUnitlessFromIntegerPlusUnitless(fPm, this, tempParameter, value1, value2);
						}
					}
					commentToken = next();
				} else if (token=="-") {
					// Case is one of these:
					// name_of_unitless_or_integer_parameter - number
					// name_of_unitless_or_integer_parameter - name_of_unitless_or_integer_parameter
					value2 = next();
					if (fPm->IsDouble(value2)) {
						// name_of_unitless_or_integer_parameter - number
						if (GetTempTypeOfParameter(value1)=="u")
							parameter = new TsUnitlessFromUnitlessMinusValue(fPm, this, tempParameter, value1, value2);
						else
							parameter = new TsUnitlessFromIntegerMinusValue(fPm, this, tempParameter, value1, value2);
					} else {
						// name_of_unitless_or_integer_parameter - name_of_unitless_or_integer_parameter
						if (GetTempTypeOfParameter(value1)=="u") {
							if (GetTempTypeOfParameter(value2)=="u")
								parameter = new TsUnitlessFromUnitlessMinusUnitless(fPm, this, tempParameter, value1, value2);
							else
								parameter = new TsUnitlessFromUnitlessMinusInteger(fPm, this, tempParameter, value1, value2);
						} else {
							if (GetTempTypeOfParameter(value2)=="i")
								parameter = new TsUnitlessFromIntegerMinusInteger(fPm, this, tempParameter, value1, value2);
							else
								parameter = new TsUnitlessFromIntegerMinusUnitless(fPm, this, tempParameter, value1, value2);
						}
					}
					commentToken = next();
				} else if (token=="*") {
					// Case is one of these:
					// name_of_unitless_or_integer_parameter * number
					// name_of_unitless_or_integer_parameter * name_of_unitless_or_integer_parameter
					value2 = next();
					if (fPm->IsDouble(value2)) {
						// name_of_unitless_or_integer_parameter * number
						if (GetTempTypeOfParameter(value1)=="u")
							parameter = new TsUnitlessFromUnitlessTimesValue(fPm, this, tempParameter, value1, value2);
						else
							parameter = new TsUnitlessFromIntegerTimesValue(fPm, this, tempParameter, value1, value2);
					} else {
						// name_of_unitless_or_integer_parameter * name_of_unitless_or_integer_parameter
						if (GetTempTypeOfParameter(value1)=="u") {
							if (GetTempTypeOfParameter(value2)=="u")
								parameter = new TsUnitlessFromUnitlessTimesUnitless(fPm, this, tempParameter, value1, value2);
							else
								parameter = new TsUnitlessFromUnitlessTimesInteger(fPm, this, tempParameter, value1, value2);
						} else {
							if (GetTempTypeOfParameter(value2)=="i")
								parameter = new TsUnitlessFromIntegerTimesInteger(fPm, this, tempParameter, value1, value2);
							else
								parameter = new TsUnitlessFromIntegerTimesUnitless(fPm, this, tempParameter, value1, value2);
						}
					}
				} else {
					// name_of_unitless_or_integer_parameter
					if (GetTempTypeOfParameter(value1)=="u")
						parameter = new TsUnitlessFromUnitless(fPm, this, tempParameter, value1);
					else
						parameter = new TsUnitlessFromInteger(fPm, this, tempParameter, value1);
					commentToken = token;
				}
			}
		} else if (type=="i") {
			// Integer parameters may have values of the following forms (or one of these plus # comment)
			// integer + name_of_integer_parameter
			// integer - name_of_integer_parameter
			// integer * name_of_integer_parameter
			// integer * name_of_unitless_parameter
			// integer
			// name_of_integer_parameter + integer
			// name_of_integer_parameter + name_of_integer_parameter
			// name_of_integer_parameter - integer
			// name_of_integer_parameter - name_of_integer_parameter
			// name_of_integer_parameter * integer
			// name_of_integer_parameter * name_of_integer_parameter
			// name_of_integer_parameter * name_of_unitless_parameter
			// name_of_integer_parameter
			// name_of_unitless_parameter
			G4Tokenizer next(value);
			value1 = next();
			token = next();

			if (fPm->IsDouble(value1)) {
				// Case is one of these:
				// integer + name_of_integer_parameter
				// integer - name_of_integer_parameter
				// integer * name_of_integer_parameter
				// integer * name_of_unitless_parameter
				// integer
				if (token=="+") {
					// integer + name_of_integer_parameter
					parameter = new TsIntegerFromValuePlusInteger(fPm, this, tempParameter, value1, next());
					commentToken = next();
				} else if (token=="-") {
					// integer - name_of_integer_parameter
					parameter = new TsIntegerFromValueMinusInteger(fPm, this, tempParameter, value1, next());
					commentToken = next();
				} else if (token=="*") {
					value2 = next();
					if (GetTempTypeOfParameter(value2)=="u") {
						// integer * name_of_unitless_parameter
						parameter = new TsIntegerFromValueTimesUnitless(fPm, this, tempParameter, value1, value2);
					} else {
						// integer * name_of_integer_parameter
						parameter = new TsIntegerFromValueTimesInteger(fPm, this, tempParameter, value1, value2);
					}
					commentToken = next();
				} else {
					// integer
					parameter = new TsIntegerFromValue(fPm, this, tempParameter, value1);
					commentToken = token;
				}
			} else {
				// Case is one of these:
				// name_of_integer_parameter + integer
				// name_of_integer_parameter + name_of_integer_parameter
				// name_of_integer_parameter - integer
				// name_of_integer_parameter - name_of_integer_parameter
				// name_of_integer_parameter * integer
				// name_of_integer_parameter * name_of_integer_parameter
				// name_of_integer_parameter
				// name_of_unitless_parameter
				if (token=="+") {
					// Case is one of these:
					// name_of_integer_parameter + integer
					// name_of_integer_parameter + name_of_integer_parameter
					value2 = next();
					if (fPm->IsDouble(value2)) {
						// name_of_integer_parameter + integer
						parameter = new TsIntegerFromIntegerPlusValue(fPm, this, tempParameter, value1, value2);
					} else {
						// name_of_integer_parameter + name_of_integer_parameter
						parameter = new TsIntegerFromIntegerPlusInteger(fPm, this, tempParameter, value1, value2);
					}
					commentToken = next();
				} else if (token=="-") {
					// Case is one of these:
					// name_of_integer_parameter - integer
					// name_of_integer_parameter - name_of_integer_parameter
					value2 = next();
					if (fPm->IsDouble(value2)) {
						// name_of_integer_parameter - integer
						parameter = new TsIntegerFromIntegerMinusValue(fPm, this, tempParameter, value1, value2);
					} else {
						// name_of_integer_parameter - name_of_integer_parameter
						parameter = new TsIntegerFromIntegerMinusInteger(fPm, this, tempParameter, value1, value2);
					}
					commentToken = next();
				} else if (token=="*") {
					// Case is one of these:
					// name_of_integer_parameter * integer
					// name_of_integer_parameter * name_of_integer_parameter
					// name_of_integer_parameter * name_of_unitless_parameter
					value2 = next();
					if (fPm->IsDouble(value2)) {
						// name_of_integer_parameter * integer
						parameter = new TsIntegerFromIntegerTimesValue(fPm, this, tempParameter, value1, value2);
					} else {
						if (GetTempTypeOfParameter(value2)=="u")
							// name_of_integer_parameter * name_of_unitless_parameter
							parameter = new TsIntegerFromIntegerTimesUnitless(fPm, this, tempParameter, value1, value2);
						else {
							// name_of_integer_parameter * name_of_integer_parameter
							parameter = new TsIntegerFromIntegerTimesInteger(fPm, this, tempParameter, value1, value2);
						}
					}
					commentToken = next();
				} else {
					// Case is one of these:
					// name_of_unitless_parameter
					// name_of_integer_parameter
					if (GetTempTypeOfParameter(value1)=="u") {
						// name_of_unitless_parameter
						parameter = new TsIntegerFromUnitless(fPm, this, tempParameter, value1);
					} else {
						// name_of_integer_parameter
						parameter = new TsIntegerFromInteger(fPm, this, tempParameter, value1);
					}
					commentToken = token;
				}
			}
		} else if (type=="b") {
			// Boolean parameters may have values of the following forms (or one of these plus # comment)
			// name_of_boolean_parameter
			// name_of_boolean_parameter * name_of_boolean_parameter
			// value
			G4Tokenizer next(value);
			value1 = next();

			if (GetTempTypeOfParameter(value1)=="b") {
				// Case is one of these:
				// name_of_boolean_parameter * name_of_boolean_parameter
				// name_of_boolean_parameter
				token = next();
				if (token=="*") {
					// name_of_boolean_parameter * name_of_boolean_parameter
					parameter = new TsBooleanFromBooleanTimesBoolean(fPm, this, tempParameter, value1, next());
					commentToken = next();
				} else {
					// name_of_boolean_parameter
					parameter = new TsBooleanFromBoolean(fPm, this, tempParameter, value1);
					commentToken = token;
				}
			} else {
				// value
				parameter = new TsBooleanFromValue(fPm, this, tempParameter, value1);
				commentToken = next();
			}
		} else if (type=="s") {
			// String parameters may have values of the following forms (or one of these plus # comment)
			// string + name_of_integer_or_string_parameter
			// string
			// name_of_integer_or_string_parameter + string
			// name_of_integer_or_string_parameter + name_of_integer_or_string_parameter
			// name_of_integer_or_string_parameter
			if (value.substr(0,1)=="\"") {
				// Case is one of these:
				// string + name_of_integer_or_string_parameter
				// string
				// Change first and last characters to spaces, then trim.
				const auto secondQuotePos = value.find('"',1);
				if (secondQuotePos==G4String::npos)
					value1 = "";
				else
					value1 = value.substr(1,secondQuotePos-1);

				G4String restOfValue = value.substr(secondQuotePos+1);
				fPm->Trim(restOfValue);

				G4Tokenizer next(restOfValue);
				token = next();
				if (token=="+") {
					value2 = next();
					// string + name_of_integer_or_string_parameter
					if (GetTempTypeOfParameter(value2)=="i")
						parameter = new TsStringFromValuePlusInteger(fPm, this, tempParameter, value1, value2);
					else
						parameter = new TsStringFromValuePlusString(fPm, this, tempParameter, value1, value2);
					commentToken = next();
				} else {
					// string
					parameter = new TsStringFromValue(fPm, this, tempParameter, value1);
					commentToken = token;
				}
			} else {
				// Case is one of these:
				// name_of_integer_or_string_parameter + name_of_integer_or_string_parameter
				// name_of_integer_or_string_parameter + string
				// name_of_integer_or_string_parameter
				G4Tokenizer next(value);
				value1 = next();
				token = next();
				if (token=="+") {
					// Case is one of these:
					// name_of_integer_or_string_parameter + name_of_integer_or_string_parameter
					// name_of_integer_or_string_parameter + string
					const auto firstQuotePos = value.find('"');
					if (firstQuotePos==G4String::npos) {
						// name_of_integer_or_string_parameter + name_of_integer_or_string_parameter
						value2 = next();
						if (GetTempTypeOfParameter(value1)=="i") {
							if (GetTempTypeOfParameter(value2)=="i")
								parameter = new TsStringFromIntegerPlusInteger(fPm, this, tempParameter, value1, value2);
							else
								parameter = new TsStringFromIntegerPlusString(fPm, this, tempParameter, value1, value2);
						} else {
							if (GetTempTypeOfParameter(value2)=="i")
								parameter = new TsStringFromStringPlusInteger(fPm, this, tempParameter, value1, value2);
							else
								parameter = new TsStringFromStringPlusString(fPm, this, tempParameter, value1, value2);
						}
						commentToken = next();
					} else {
						// name_of_integer_or_string_parameter + string
						const auto secondQuotePos = value.find('"',firstQuotePos+1);
						value2 = value.substr(firstQuotePos+1,secondQuotePos-firstQuotePos-1);
						if (GetTempTypeOfParameter(value1)=="i")
							parameter = new TsStringFromIntegerPlusValue(fPm, this, tempParameter, value1, value2);
						else
							parameter = new TsStringFromStringPlusValue(fPm, this, tempParameter, value1, value2);
						commentToken = value.substr(secondQuotePos+1);
						fPm->Trim(commentToken);
					}
				} else {
					// name_of_integer_or_string_parameter
					if (GetTempTypeOfParameter(value1)=="i")
						parameter = new TsStringFromInteger(fPm, this, tempParameter, value1);
					else
						parameter = new TsStringFromString(fPm, this, tempParameter, value1);
					commentToken = next();
				}
			}
		} else if (type=="dv") {
			// Double vector parameters may have values of the following forms (or one of these plus # comment)
			// number * name_of_double_vector_parameter unit
			// number_of_values value1 value2 ... valueN * name_of_double_or_double_vector_parameter unit
			// number_of_values value1 value2 ... valueN unit + name_of_double_or_double_vector_parameter
			// number_of_values value1 value2 ... valueN unit - name_of_double_or_double_vector_parameter
			// number_of_values value1 value2 ... valueN unit * name_of_unitless_or_integer_or_unitless_vector_or_integer_vector_parameter
			// number_of_values value1 value2 ... valueN unit
			// name_of_unitless_or_integer_parameter * name_of_double_vector_parameter unit
			// name_of_double_vector_parameter unit
			G4Tokenizer next(value);
			value1 = next();
			if (fPm->IsDouble(value1)) {
				token = next();
				if (token=="*") {
					// number * name_of_double_vector_parameter unit
					value2 = next();
					unit = next();
					parameter = new TsDoubleVectorFromScaleTimesDoubleVector(fPm, this, tempParameter, value1, value2, unit);
					commentToken = next();
				} else {
					if (!fPm->IsInteger(value1, 32)) Quit(tempParameter, "Double vector parameter has a non-integer for number of values");
					nValues=G4UIcommand::ConvertToInt(value1);
					if (nValues < 0) Quit(tempParameter, "Double vector parameter has a negative integer for number of values");
					values = new G4String[nValues];

					values[0] = token;
					for (G4int iToken=1; iToken<nValues; iToken++) {
						values[iToken] = next();
						if (values[iToken].empty()) Quit(tempParameter, "Double vector parameter value has wrong number of values.");
					}

					token = next();
					if (token=="*") {
						// number_of_values value1 value2 ... valueN * name_of_double_or_double_vector_parameter unit
						value2 = next();
						unit = next();
						if (fPm->IsDouble(unit))
							Quit(tempParameter, "Double vector parameter has another double where the unit was expected to be.\nCheck that correct number of values was provided.");

						if (GetTempTypeOfParameter(value2)=="d")
							parameter = new TsDoubleVectorFromValueTimesDouble(fPm, this, tempParameter, nValues, values, value2, unit);
						else
							parameter = new TsDoubleVectorFromValueTimesDoubleVector(fPm, this, tempParameter, nValues, values, value2, unit);
						commentToken = next();
					} else {
						unit = token;
						if (fPm->IsDouble(unit))
							Quit(tempParameter, "Double vector parameter has another double where the unit was expected to be.\nCheck that correct number of values was provided.");

						token = next();
						if (token=="+") {
							// number_of_values value1 value2 ... valueN unit + name_of_double_or_double_vector_parameter
							value2 = next();
							if (GetTempTypeOfParameter(value2)=="d")
								parameter = new TsDoubleVectorFromValuePlusDouble(fPm, this, tempParameter, nValues, values, value2, unit);
							else
								parameter = new TsDoubleVectorFromValuePlusDoubleVector(fPm, this, tempParameter, nValues, values, value2, unit);
							commentToken = next();
						} else if (token=="-") {
							// number_of_values value1 value2 ... valueN unit - name_of_double_or_double_vector_parameter
							value2 = next();
							if (GetTempTypeOfParameter(value2)=="d")
								parameter = new TsDoubleVectorFromValueMinusDouble(fPm, this, tempParameter, nValues, values, value2, unit);
							else
								parameter = new TsDoubleVectorFromValueMinusDoubleVector(fPm, this, tempParameter, nValues, values, value2, unit);
							commentToken = next();
						} else if (token=="*") {
							// number_of_values value1 value2 ... valueN unit * name_of_unitless_or_integer_or_unitless_vector_or_integer_vector_parameter
							value2 = next();
							typeOfValue2 = GetTempTypeOfParameter(value2);
							if (typeOfValue2=="u")
								parameter = new TsDoubleVectorFromValueTimesUnitless(fPm, this, tempParameter, nValues, values, value2, unit);
							else if (typeOfValue2=="i")
								parameter = new TsDoubleVectorFromValueTimesInteger(fPm, this, tempParameter, nValues, values, value2, unit);
							else if (typeOfValue2=="uv")
								parameter = new TsDoubleVectorFromValueTimesUnitlessVector(fPm, this, tempParameter, nValues, values, value2, unit);
							else
								parameter = new TsDoubleVectorFromValueTimesIntegerVector(fPm, this, tempParameter, nValues, values, value2, unit);
							commentToken = next();
						} else {
							// number_of_values value1 value2 ... valueN unit
							parameter = new TsDoubleVectorFromValue(fPm, this, tempParameter, nValues, values, unit);
							commentToken = token;
						}
					}
				}
			} else {
				token = next();
				if (token=="*") {
					// name_of_unitless_or_integer_parameter * name_of_double_vector_parameter unit
					value2 = next();
					unit = next();
					if (GetTempTypeOfParameter(value1)=="u")
						parameter = new TsDoubleVectorFromUnitlessTimesDoubleVector(fPm, this, tempParameter, value1, value2, unit);
					else
						parameter = new TsDoubleVectorFromIntegerTimesDoubleVector(fPm, this, tempParameter, value1, value2, unit);
					commentToken = next();
				} else {
					// name_of_double_vector_parameter unit
					unit = token;
					parameter = new TsDoubleVectorFromDoubleVector(fPm, this, tempParameter, value1, unit);
					commentToken = next();
				}
			}
		} else if (type=="uv") {
			// Unitless vector parameters may have values of the following forms (or one of these plus # comment)
			// number * name_of_unitless_vector_parameter
			// number_of_values value1 value2 ... valueN + name_of_unitless_or_integer_or_unitless_vector_or_integer_vector_parameter
			// number_of_values value1 value2 ... valueN - name_of_unitless_or_integer_or_unitless_vector_or_integer_vector_parameter
			// number_of_values value1 value2 ... valueN * name_of_unitless_or_integer_or_unitless_vector_or_integer_vector_parameter
			// number_of_values value1 value2 ... valueN
			// name_of_unitless_or_integer_parameter * name_of_unitless_vector_parameter
			// name_of_unitless_vector_parameter
			G4Tokenizer next(value);
			value1 = next();
			if (fPm->IsDouble(value1)) {
				token = next();
				if (token=="*") {
					// number * name_of_unitless_vector_parameter
					value2 = next();
					parameter = new TsUnitlessVectorFromScaleTimesUnitlessVector(fPm, this, tempParameter, value1, value2);
					commentToken = next();
				} else {
					if (!fPm->IsInteger(value1, 32)) Quit(tempParameter, "Unitless vector parameter has a non-integer for number of values");
					nValues=G4UIcommand::ConvertToInt(value1);
					if (nValues < 0) Quit(tempParameter, "Unitless vector parameter has a negative integer for number of values");
					values = new G4String[nValues];

					values[0] = token;
					for (G4int iToken=1; iToken<nValues; iToken++) {
						values[iToken] = next();
						if (values[iToken].empty()) Quit(tempParameter, "Unitless vector parameter value has wrong number of values.");
					}

					token = next();
					if (token=="+") {
						// number_of_values value1 value2 ... valueN + name_of_unitless_or_integer_or_unitless_vector_or_integer_vector_parameter
						value2 = next();
						typeOfValue2 = GetTempTypeOfParameter(value2);
						if (typeOfValue2=="u")
							parameter = new TsUnitlessVectorFromValuePlusUnitless(fPm, this, tempParameter, nValues, values, value2);
						else if (typeOfValue2=="i")
							parameter = new TsUnitlessVectorFromValuePlusInteger(fPm, this, tempParameter, nValues, values, value2);
						else if (typeOfValue2=="uv")
							parameter = new TsUnitlessVectorFromValuePlusUnitlessVector(fPm, this, tempParameter, nValues, values, value2);
						else
							parameter = new TsUnitlessVectorFromValuePlusIntegerVector(fPm, this, tempParameter, nValues, values, value2);
						commentToken = next();
					} else if (token=="-") {
						// number_of_values value1 value2 ... valueN - name_of_unitless_or_integer_or_unitless_vector_or_integer_vector_parameter
						value2 = next();
						typeOfValue2 = GetTempTypeOfParameter(value2);
						if (typeOfValue2=="u")
							parameter = new TsUnitlessVectorFromValueMinusUnitless(fPm, this, tempParameter, nValues, values, value2);
						else if (typeOfValue2=="i")
							parameter = new TsUnitlessVectorFromValueMinusInteger(fPm, this, tempParameter, nValues, values, value2);
						else if (typeOfValue2=="uv")
							parameter = new TsUnitlessVectorFromValueMinusUnitlessVector(fPm, this, tempParameter, nValues, values, value2);
						else
							parameter = new TsUnitlessVectorFromValueMinusIntegerVector(fPm, this, tempParameter, nValues, values, value2);
						commentToken = next();
					} else if (token=="*") {
						// number_of_values value1 value2 ... valueN * name_of_unitless_or_integer_or_unitless_vector_or_integer_vector_parameter
						value2 = next();
						typeOfValue2 = GetTempTypeOfParameter(value2);
						if (typeOfValue2=="u")
							parameter = new TsUnitlessVectorFromValueTimesUnitless(fPm, this, tempParameter, nValues, values, value2);
						else if (typeOfValue2=="i")
							parameter = new TsUnitlessVectorFromValueTimesInteger(fPm, this, tempParameter, nValues, values, value2);
						else if (typeOfValue2=="uv")
							parameter = new TsUnitlessVectorFromValueTimesUnitlessVector(fPm, this, tempParameter, nValues, values, value2);
						else
							parameter = new TsUnitlessVectorFromValueTimesIntegerVector(fPm, this, tempParameter, nValues, values, value2);
						commentToken = next();
					} else {
						// number_of_values value1 value2 ... valueN
						parameter = new TsUnitlessVectorFromValue(fPm, this, tempParameter, nValues, values);
						commentToken = token;
					}
					delete[] values;
				}
			} else {
				token = next();
				if (token=="*") {
					// name_of_unitless_or_integer_parameter * name_of_unitless_vector_parameter
					value2 = next();
					if (GetTempTypeOfParameter(value1)=="u")
						parameter = new TsUnitlessVectorFromUnitlessTimesUnitlessVector(fPm, this, tempParameter, value1, value2);
					else
						parameter = new TsUnitlessVectorFromIntegerTimesUnitlessVector(fPm, this, tempParameter, value1, value2);
					commentToken = next();
				} else {
					// name_of_unitless_vector_parameter
					parameter = new TsUnitlessVectorFromUnitlessVector(fPm, this, tempParameter, value1);
					commentToken = token;
				}
			}
		} else if (type=="iv") {
			// Integer vector parameters may have values of the following forms (or one of these plus # comment)
			// integer * name_of_integer_vector_parameter
			// number_of_values value1 value2 ... valueN + name_of_integer_or_integer_vector_parameter
			// number_of_values value1 value2 ... valueN - name_of_integer_or_integer_vector_parameter
			// number_of_values value1 value2 ... valueN * name_of_integer_or_integer_vector_parameter
			// number_of_values value1 value2 ... valueN
			// name_of_integer_parameter * name_of_integer_vector_parameter
			// name_of_integer_vector_parameter
			G4Tokenizer next(value);
			value1 = next();
			if (fPm->IsInteger(value1, 32)) {
				token = next();
				if (token=="*") {
					// name_of_integer_parameter * name_of_integer_vector_parameter
					value2 = next();
					parameter = new TsIntegerVectorFromScaleTimesIntegerVector(fPm, this, tempParameter, value1, value2);
					commentToken = next();
				} else {
					nValues=G4UIcommand::ConvertToInt(value1);
					if (nValues < 0) Quit(tempParameter, "Integer vector parameter has a negative integer for number of values");
					values = new G4String[nValues];

					values[0] = token;
					for (G4int iToken=1; iToken<nValues; iToken++) {
						values[iToken] = next();
						if (values[iToken].empty()) Quit(tempParameter, "Integer vector parameter value has wrong number of values.");
					}

					token = next();
					if (token=="+") {
						// number_of_values value1 value2 ... valueN + name_of_integer_or_integer_vector_parameter
						value2 = next();
						if (GetTempTypeOfParameter(value2)=="i")
							parameter = new TsIntegerVectorFromValuePlusInteger(fPm, this, tempParameter, nValues, values, value2);
						else
							parameter = new TsIntegerVectorFromValuePlusIntegerVector(fPm, this, tempParameter, nValues, values, value2);
						commentToken = next();
					} else if (token=="-") {
						// number_of_values value1 value2 ... valueN - name_of_integer_or_integer_vector_parameter
						value2 = next();
						if (GetTempTypeOfParameter(value2)=="i")
							parameter = new TsIntegerVectorFromValueMinusInteger(fPm, this, tempParameter, nValues, values, value2);
						else
							parameter = new TsIntegerVectorFromValueMinusIntegerVector(fPm, this, tempParameter, nValues, values, value2);
						commentToken = next();
					} else if (token=="*") {
						// number_of_values value1 value2 ... valueN * name_of_integer_or_integer_vector_parameter
						value2 = next();
						if (GetTempTypeOfParameter(value2)=="i")
							parameter = new TsIntegerVectorFromValueTimesInteger(fPm, this, tempParameter, nValues, values, value2);
						else
							parameter = new TsIntegerVectorFromValueTimesIntegerVector(fPm, this, tempParameter, nValues, values, value2);
						commentToken = next();
					} else {
						// number_of_values value1 value2 ... valueN
						parameter = new TsIntegerVectorFromValue(fPm, this, tempParameter, nValues, values);
						commentToken = token;
					}
					delete[] values;
				}
			} else {
				token = next();
				if (token=="*") {
					// name_of_integer_parameter * name_of_integer_vector_parameter
					value2 = next();
					parameter = new TsIntegerVectorFromIntegerTimesIntegerVector(fPm, this, tempParameter, value1, value2);
					commentToken = next();
				} else {
					// name_of_integer_vector_parameter
					parameter = new TsIntegerVectorFromIntegerVector(fPm, this, tempParameter, value1);
					commentToken = token;
				}
			}
		} else if (type=="bv") {
			// Boolean vector parameters may have values of the following forms (or one of these plus # comment)
			// number_of_values value1 value2 ... valueN
			// name_of_boolean_vector_parameter
			G4Tokenizer next(value);
			value1 = next();
			if (fPm->IsInteger(value1, 32)) {
				// number_of_values value1 value2 ... valueN
				nValues=G4UIcommand::ConvertToInt(value1);
				if (nValues < 0) Quit(tempParameter, "Boolean vector parameter has a negative integer for number of values");
				values = new G4String[nValues];

				for (G4int iToken=0; iToken<nValues; iToken++) {
					values[iToken] = next();
					if (values[iToken].empty()) Quit(tempParameter, "Boolean vector parameter value has wrong number of values.");
				}

				parameter = new TsBooleanVectorFromValue(fPm, this, tempParameter, nValues, values);
				delete[] values;
				commentToken = next();
			} else {
				if (value.substr(0,1)=="\"") {
					// Seems like user omitted the required integer
					G4String message = "Boolean vector parameter has a non-integer where the nValues integer is expected.";
					Quit (tempParameter, message);
				}

				// name_of_boolean_vector_parameter
				parameter = new TsBooleanVectorFromBooleanVector(fPm, this, tempParameter, value1);
				commentToken = next();
			}
		} else if (type=="sv") {
			// String vector parameters may have values of the following forms (or one of these plus # comment)
			// number_of_values value1 value2 ... valueN + name_of_integer_or_string_or_integer_vector_or_string_vector_parameter" << G4endl;
			// number_of_values value1 value2 ... valueN
			// name_of_string_vector_parameter
			G4Tokenizer next(value);
			value1 = next();
			if (fPm->IsInteger(value1, 32)) {
				nValues=G4UIcommand::ConvertToInt(value1);
				if (nValues < 0) Quit(tempParameter, "String vector parameter has a negative integer for number of values");

				values = new G4String[nValues];

				for (G4int iToken=0; iToken<nValues; iToken++) {
					values[iToken] = next();
					if (values[iToken].empty()) Quit(tempParameter, "String vector parameter value has wrong number of values.");
				}

				token = next();
				if (token=="+") {
					// number_of_values value1 value2 ... valueN + name_of_integer_or_string_or_integer_vector_or_string_vector_parameter" << G4endl;
					value2 = next();
					typeOfValue2 = GetTempTypeOfParameter(value2);
					if (typeOfValue2=="i")
						parameter = new TsStringVectorFromValuePlusInteger(fPm, this, tempParameter, nValues, values, value2);
					else if (typeOfValue2=="s")
						parameter = new TsStringVectorFromValuePlusString(fPm, this, tempParameter, nValues, values, value2);
					else if (typeOfValue2=="iv")
						parameter = new TsStringVectorFromValuePlusIntegerVector(fPm, this, tempParameter, nValues, values, value2);
					else
						parameter = new TsStringVectorFromValuePlusStringVector(fPm, this, tempParameter, nValues, values, value2);
					commentToken = next();
				} else {
					// number_of_values value1 value2 ... valueN
					parameter = new TsStringVectorFromValue(fPm, this, tempParameter, nValues, values);
					commentToken = token;
				}
				delete[] values;
			} else {
				if (value.substr(0,1)=="\"") {
					// Seems like user omitted the required integer
					G4String message = "String vector parameter has a non-integer where the nValues integer is expected.";
					Quit (tempParameter, message);
				}

				// name_of_string_vector_parameter
				parameter = new TsStringVectorFromStringVector(fPm, this, tempParameter, value1);
				commentToken = next();
			}
		} else {
			// Type is unknown.
			Quit (name, "Parameter has unknown type");
		}

		// Anything left over must start with the comment character
		if (!commentToken.empty() && commentToken.substr(0,1)!="#") {
			G4String message = "Parameter has extra tokens that do not begin with the comment character.";
			if (type=="dv" || type=="uv" || type=="iv" || type=="bv" || type=="sv")
				message+= "\nProblem may be that the vector's stated length does not match the number of values provided.";
			Quit (tempParameter, message);
		}

		// Parameter is ready to store
		if (!test)
			(*fRealParameters)[iter->first] = parameter;
	}

	fTempParameters->clear();
}


void TsParameterFile::ProtectAgainstControlByDifferentArms(G4String paramName, TsParameterFile* includeFile1, G4String prefix, G4String suffix) {
	if (paramName.substr(0,prefix.length()) == prefix && paramName.substr(paramName.length()-suffix.length()) == suffix) {
		std::vector<TsParameterFile*>::iterator includeFileIter2;
		for (includeFileIter2 = fIncludeFiles->begin(); includeFileIter2 != fIncludeFiles->end(); ++includeFileIter2) {
			TsParameterFile* includeFile2 = *includeFileIter2;
			// Make sure not comparing the same include file to itself
			// But can't use above logic for loops because in this case either file may have the Ge/*/Type.
			if (includeFile2 != includeFile1) {
				// Find all parameters operating on the same component, scorer, etc.
				G4String firstPart = paramName.substr(0,paramName.length()-suffix.length()+1);
				std::vector<TsVParameter*>* matchedParameters = new std::vector<TsVParameter*>;
				includeFile2->GetParametersOperatingOn(firstPart, matchedParameters);
				const auto length = matchedParameters->size();

				// Loop over all those found parameters
				for (size_t iToken = 0; iToken < length; iToken++)
				{
					// See if the matched parameter is the exact same one as in the first file
					if ((*matchedParameters)[iToken] != includeFile1->GetParameter((*matchedParameters)[iToken]->GetName())) {
						G4String message = "  has been defined in chain:\n    ";
						std::vector<TsParameterFile*>* parameterFiles = new std::vector<TsParameterFile*>;
						includeFile1->BuildVectorOfParameterFiles(parameterFiles);
						std::vector<TsParameterFile*>::iterator iter2;
						for (iter2 = parameterFiles->begin(); iter2 != parameterFiles->end(); ++iter2)
							message += " " + (*iter2)->GetFileName();

						message += "\n  but has been modified by parameter ";
						message += (*matchedParameters)[iToken]->GetName();
						message += " in chain:\n    ";
						parameterFiles->clear();
						includeFile2->BuildVectorOfParameterFiles(parameterFiles);
						for (iter2 = parameterFiles->begin(); iter2 != parameterFiles->end(); ++iter2)
							message += " " + (*iter2)->GetFileName();
						Quit (paramName, message);
					}
				}
			}
		}
	}
}


G4bool TsParameterFile::ParameterExists(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	std::map<G4String, TsVParameter*>::const_iterator iter = fRealParameters->find(sLower);
	if (iter == fRealParameters->end()) {

		if (fParentFile)
			return fParentFile->ParameterExists(s);
		else
			return false;
	}
	return true;
}


TsVParameter* TsParameterFile::GetParameter(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	std::map<G4String, TsVParameter*>::const_iterator iter = fRealParameters->find(sLower);
	if (iter == fRealParameters->end()) {

		if (fParentFile)
			return fParentFile->GetParameter(s);
		else
			return 0;
	}
	return iter->second;
}


TsVParameter* TsParameterFile::GetParameterBeforeLinearized(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	std::map<G4String, TsVParameter*>::const_iterator iter = fRealParameters->find(sLower);
	if (iter == fRealParameters->end()) {
		std::vector<TsParameterFile*>::iterator iter2;
		for (iter2 = fIncludeFiles->begin(); iter2 != fIncludeFiles->end(); ++iter2)
			if ((*iter2)->GetParameterBeforeLinearized(s))
				return (*iter2)->GetParameterBeforeLinearized(s);

		return 0;
	}
	return iter->second;
}


G4int TsParameterFile::GetVectorLength(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	G4int nValues = 0;

	std::map<G4String, TsVParameter*>::const_iterator iter = fRealParameters->find(sLower);
	if (iter != fRealParameters->end()) {
		TsVParameter* parameter = iter->second;
		nValues = parameter->GetVectorLength();
	} else if (fParentFile)
		nValues = fParentFile->GetVectorLength(s);
	else
		Undefined(s);

	return nValues;
}


G4double TsParameterFile::GetDoubleParameter(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	G4double value = -99999999.;

	std::map<G4String, TsVParameter*>::const_iterator iter = fRealParameters->find(sLower);
	if (iter != fRealParameters->end()) {
		TsVParameter* parameter = iter->second;
		value = parameter->GetDoubleValue();
	} else if (fParentFile)
		value = fParentFile->GetDoubleParameter(s);
	else
		Undefined(s);

	return value;
}


G4double TsParameterFile::GetUnitlessParameter(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	G4double value = 99999999.;

	std::map<G4String, TsVParameter*>::const_iterator iter = fRealParameters->find(sLower);
	if (iter != fRealParameters->end()) {
		TsVParameter* parameter = iter->second;
		value = parameter->GetUnitlessValue();
	} else if (fParentFile)
		value = fParentFile->GetUnitlessParameter(s);
	else
		Undefined(s);

	return value;
}


G4int TsParameterFile::GetIntegerParameter(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	G4int value = 99999999;

	std::map<G4String, TsVParameter*>::const_iterator iter = fRealParameters->find(sLower);
	if (iter != fRealParameters->end()) {
		TsVParameter* parameter = iter->second;
		value = parameter->GetIntegerValue();
	} else if (fParentFile)
		value = fParentFile->GetIntegerParameter(s);
	else
		Undefined(s);

	return value;
}


G4bool TsParameterFile::GetBooleanParameter(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	G4bool value = false;

	std::map<G4String, TsVParameter*>::const_iterator iter = fRealParameters->find(sLower);
	if (iter != fRealParameters->end()) {
		TsVParameter* parameter = iter->second;
		value = parameter->GetBooleanValue();
	} else if (fParentFile)
		value = fParentFile->GetBooleanParameter(s);
	else
		Undefined(s);

	return value;
}


G4String TsParameterFile::GetStringParameter(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	G4String value = "invalid";

	std::map<G4String, TsVParameter*>::const_iterator iter = fRealParameters->find(sLower);
	if (iter != fRealParameters->end()) {
		TsVParameter* parameter = iter->second;
		value = parameter->GetStringValue();
	} else if (fParentFile)
		value = fParentFile->GetStringParameter(s);
	else
		Undefined(s);

	return value;
}


G4double* TsParameterFile::GetDoubleVector(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	G4double* value = new G4double[1];
	value[0] = -9999999.;

	std::map<G4String, TsVParameter*>::const_iterator iter = fRealParameters->find(sLower);
	if (iter != fRealParameters->end()) {
		TsVParameter* parameter = iter->second;
		value = parameter->GetDoubleVector();
	} else if (fParentFile)
		value = fParentFile->GetDoubleVector(s);
	else
		Undefined(s);

	return value;
}


G4double* TsParameterFile::GetUnitlessVector(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	G4double* value = new G4double[1];
	value[0] = -9999999.;

	std::map<G4String, TsVParameter*>::const_iterator iter = fRealParameters->find(sLower);
	if (iter != fRealParameters->end()) {
		TsVParameter* parameter = iter->second;
		value = parameter->GetUnitlessVector();
	} else if (fParentFile)
		value = fParentFile->GetUnitlessVector(s);
	else
		Undefined(s);

	return value;
}


G4int* TsParameterFile::GetIntegerVector(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	G4int* value = new G4int[1];
	value[0] = -9999999;

	std::map<G4String, TsVParameter*>::const_iterator iter = fRealParameters->find(sLower);
	if (iter != fRealParameters->end()) {
		TsVParameter* parameter = iter->second;
		value = parameter->GetIntegerVector();
	} else if (fParentFile)
		value = fParentFile->GetIntegerVector(s);
	else
		Undefined(s);

	return value;
}


G4bool* TsParameterFile::GetBooleanVector(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	G4bool* value = new G4bool[1];
	value[0] = false;

	std::map<G4String, TsVParameter*>::const_iterator iter = fRealParameters->find(sLower);
	if (iter != fRealParameters->end()) {
		TsVParameter* parameter = iter->second;
		value = parameter->GetBooleanVector();
	} else if (fParentFile)
		value = fParentFile->GetBooleanVector(s);
	else
		Undefined(s);

	return value;
}


G4String* TsParameterFile::GetStringVector(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	G4String* value = new G4String[1];
	value[0] = "";

	std::map<G4String, TsVParameter*>::const_iterator iter = fRealParameters->find(sLower);
	if (iter != fRealParameters->end()) {
		TsVParameter* parameter = iter->second;
		value = parameter->GetStringVector();
	} else if (fParentFile)
		value = fParentFile->GetStringVector(s);
	else
		Undefined(s);

	return value;
}


G4String TsParameterFile::GetHTMLValueOfParameter(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	G4String output = "";
	std::map<G4String, TsVParameter*>::const_iterator iter = fRealParameters->find(sLower);
	if (iter != fRealParameters->end()) {
		TsVParameter* aParam = iter->second;
		output = aParam->GetHTMLValue();
	}

	return output;
}


G4String TsParameterFile::GetTypeOfParameter(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	G4String type = "";
	std::map<G4String, TsVParameter*>::const_iterator iter = fRealParameters->find(sLower);
	if (iter != fRealParameters->end())
		type = iter->second->GetType();
	else if (fParentFile)
		type = fParentFile->GetTypeOfParameter(s);

	return type;
}


G4String TsParameterFile::GetTempTypeOfParameter(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	G4String type = "";
	std::map<G4String, TsTempParameter*>::const_iterator iter = fTempParameters->find(sLower);
	if (iter != fTempParameters->end())
		type = iter->second->GetType();
	else
		type = GetTypeOfParameter(s);

	return type;
}


G4String TsParameterFile::GetUnitOfParameter(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	G4String unit = "";
	std::map<G4String, TsVParameter*>::const_iterator iter = fRealParameters->find(sLower);

	// If the parameter exists in this file, and has a defined type in this file, get its units.
	// Otherwise, look for the parameter in a parent file.
	if (iter != fRealParameters->end() && iter->second->GetType()!="")
		unit = iter->second->GetUnit();
	else if (fParentFile)
		unit = fParentFile->GetUnitOfParameter(s);
	else
		unit="";

	return unit;
}


G4bool TsParameterFile::IsChangeable(const G4String& s)
{
	G4String sLower = s;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(sLower);
#else
	sLower.toLower();
#endif
	std::map<G4String, TsVParameter*>::const_iterator iter = fRealParameters->find(sLower);
	if (iter == fRealParameters->end()) {
		if (fParentFile)
			return fParentFile->IsChangeable(s);
		else
			Undefined(s);
	}

	return iter->second->IsChangeable();
}


// This is only called after the parameter tree has been linearized,
// hence at this point there is a unique parentFile rather than multiple includeFiles.
TsParameterFile* TsParameterFile::GetParentFile()
{
	return fParentFile;
}


void TsParameterFile::GetAllParameters(std::map<G4String, TsVParameter*>* parameterMap)
{
	parameterMap->insert(fRealParameters->begin(), fRealParameters->end());

	if (fParentFile)
		fParentFile->GetAllParameters(parameterMap);
}


void TsParameterFile::GetAllParametersBeforeLinearized(std::map<G4String, TsVParameter*>* parameterMap)
{
	parameterMap->insert(fRealParameters->begin(), fRealParameters->end());

	std::vector<TsParameterFile*>::iterator iter;
	for (iter = fIncludeFiles->begin(); iter != fIncludeFiles->end(); ++iter)
		(*iter)->GetAllParametersBeforeLinearized(parameterMap);
}


// Return all parameter names that begin with the given value
void TsParameterFile::GetParameterNamesStartingWith(const G4String& nameToMatchMixed, std::vector<G4String>* parameterNames)
{
	G4String nameToMatch = nameToMatchMixed;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(nameToMatch);
#else
	nameToMatch.toLower();
#endif
	G4String name;
	G4bool found;
	size_t length;
	size_t iToken;

	std::map<G4String, TsVParameter*>::const_iterator iter;
	for (iter = fRealParameters->begin(); iter != fRealParameters->end(); ++iter) {
		name = iter->first;

		// Reduce name to everything before last slash
		if ( name.length() > nameToMatch.length() && name.substr(0,nameToMatch.length()) == nameToMatch ) {
			// Only add if not already in the vector
			found = false;
			length = parameterNames->size();
			for (iToken=0; !found && iToken<length; iToken++)
				for (iToken=0; !found && iToken<length; iToken++) {
					G4String parameterLower = (*parameterNames)[iToken];
#if GEANT4_VERSION_MAJOR >= 11
					G4StrUtil::to_lower(parameterLower);
#else
					parameterLower.toLower();
#endif
					if (parameterLower==name)
						found = true;
				}

			if (!found)
				parameterNames->push_back(iter->second->GetName());
		}
	}

	if (fParentFile)
		fParentFile->GetParameterNamesStartingWith(nameToMatch, parameterNames);
}


// Return all parameters operating on the given component, scorer, etc.
void TsParameterFile::GetParametersOperatingOn(const G4String& nameToMatch, std::vector<TsVParameter*>* parameters)
{
	G4String name;
	G4String nameToTest;
	size_t lastSlashPos;
	G4bool found;
	size_t length;
	size_t iToken;

	std::map<G4String, TsVParameter*>::const_iterator iter;
	for (iter = fRealParameters->begin(); iter != fRealParameters->end(); ++iter) {
		name = iter->first;

		// Reduce name to everything before last slash
		lastSlashPos = name.find_last_of( "/" );
		if (lastSlashPos != G4String::npos)
			nameToTest = name.substr(0,lastSlashPos+1);
		else
			nameToTest = name;

		if (nameToTest == nameToMatch) {
			// Only add if not already in the vector
			found = false;
			length = parameters->size();
			for (iToken=0; !found && iToken<length; iToken++) {
				G4String parameterLower = (*parameters)[iToken]->GetName();
#if GEANT4_VERSION_MAJOR >= 11
				G4StrUtil::to_lower(parameterLower);
#else
				parameterLower.toLower();
#endif
				if (parameterLower==name)
					found = true;
			}

			if (!found)
				parameters->push_back(iter->second);
		}
	}

	if (fParentFile)
		fParentFile->GetParametersOperatingOn(nameToMatch, parameters);
}


// Return all parameter names that begin and end with the given values
void TsParameterFile::GetParameterNamesBracketedBy(const G4String& prefix, const G4String& suffix, std::vector<G4String>* parameterNames)
{
	G4String prefixLower = prefix;
	G4String suffixLower = suffix;
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(prefixLower);
	G4StrUtil::to_lower(suffixLower);
#else
	prefixLower.toLower();
	suffixLower.toLower();
#endif
	G4bool found;
	size_t length;
	size_t iToken;

	std::map<G4String, TsVParameter*>::const_iterator iter;
	for (iter = fRealParameters->begin(); iter != fRealParameters->end(); ++iter) {
		G4String name = iter->first;
		if ( name.length() > prefixLower.length() + suffixLower.length() &&
			name.substr(0,prefixLower.length()) == prefixLower && name.substr(name.length()-suffixLower.length()) == suffixLower ) {
			// Only add if not already in the vector
			found = false;
			length = parameterNames->size();
			for (iToken=0; !found && iToken<length; iToken++) {
				G4String parameterLower = (*parameterNames)[iToken];
#if GEANT4_VERSION_MAJOR >= 11
				G4StrUtil::to_lower(parameterLower);
#else
				parameterLower.toLower();
#endif
				if (parameterLower==name)
					found = true;
			}

			if (!found)
				parameterNames->push_back(iter->second->GetName());
		}
	}

	if (fParentFile)
		fParentFile->GetParameterNamesBracketedBy(prefixLower, suffixLower, parameterNames);
}


// Return all filter parameter names that begin with the given value
void TsParameterFile::CheckFilterParameterNamesStartingWith(const G4String& prefix, std::vector<G4String>* filterNames)
{
	G4String prefixLower = prefix.substr(0,2);
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(prefixLower);
#else
	prefixLower.toLower();
#endif
	G4String suffix;
	G4String token;
	G4bool found;
	const auto length = filterNames->size();

	size_t iToken;

	std::map<G4String, TsVParameter*>::const_iterator iter;
	for (iter = fRealParameters->begin(); iter != fRealParameters->end(); ++iter) {
		G4String name = iter->first;
		if ( name.length() > prefixLower.length() + 14 &&
			name.substr(0,prefixLower.length()) == prefixLower) {
			// Have a parameter that starts correctly and is long enough to possibly have suffix staring with /onlyinclude.
			// Now need to extract the suffix by finding the last occurence of /
			G4Tokenizer next(name);
			suffix = "";
			token = next("/");

			while (token != "") {
				suffix = token;
				token = next("/");
			}

			if (suffix.length() > 11 && suffix.substr(0,11) == "onlyinclude") {
				found = false;
				for (iToken=0; !found && iToken<length; iToken++) {
					G4String filterNameLower = (*filterNames)[iToken];
#if GEANT4_VERSION_MAJOR >= 11
					G4StrUtil::to_lower(filterNameLower);
#else
					filterNameLower.toLower();
#endif
					if (filterNameLower==suffix)
						found = true;
				}

				if (!found) {
					G4cerr << "Topas is exiting due to a serious error in parameter file: " << fFileSpec << G4endl;
					G4cerr << "Parameter name: " << iter->second->GetName() << " is not a known " << prefix << " filter." << G4endl;
					G4cerr << "Complete list of known " << prefix << " filters is as follows:" << G4endl;
					for (iToken=0; !found && iToken<length; iToken++)
						G4cout << "    " << (*filterNames)[iToken] << G4endl;
					fPm->AbortSession(1);
				}
			}
		}
	}

	if (fParentFile)
		fParentFile->CheckFilterParameterNamesStartingWith(prefix, filterNames);
}


void TsParameterFile::GetTimeFeatureParameters(std::vector<TsVParameter*>* timefeatureParameters) {

	size_t tf_size = fTimeFeatureParameters->size();
	for (size_t i=0; i < tf_size; ++i) {
		timefeatureParameters->push_back( (*fTimeFeatureParameters)[i]);
	}

	if (fParentFile)
		fParentFile->GetTimeFeatureParameters(timefeatureParameters);
}


void TsParameterFile::BuildVectorOfParameterFiles(std::vector<TsParameterFile*>* parameterFiles) {
	parameterFiles->push_back(this);

	std::vector<TsParameterFile*>::iterator iter;
	for (iter = fIncludeFiles->begin(); iter != fIncludeFiles->end(); ++iter)
		(*iter)->BuildVectorOfParameterFiles(parameterFiles);
}


G4String TsParameterFile::GetFileName()
{
	return fFileSpec;
}


void TsParameterFile::ResetParentFileTo(TsParameterFile* parent) {
	fParentFile = parent;
}


void TsParameterFile::Quit(TsTempParameter* tempParameter, const char* message) {
	// If value was being evaluated just to write out to a parameter dump, no need to quit."
	if (fPm->NowDoingParameterDump()) {
		fPm->UnableToCalculateForDump();
		return;
	}

	G4cerr << "Topas is exiting due to a serious error in parameter file: " << fFileSpec << G4endl;
	G4cerr << "Parameter name: " << tempParameter->GetType() << ":" << tempParameter->GetName() << G4endl;
	G4cerr << "has unsupported value: " << tempParameter->GetValue() << G4endl;
	G4cerr << message << G4endl;
	fPm->AbortSession(1);
}


void TsParameterFile::Quit(const G4String& name, const char* message) {
	// If value was being evaluated just to write out to a parameter dump, no need to quit."
	if (fPm->NowDoingParameterDump()) {
		fPm->UnableToCalculateForDump();
		return;
	}

	G4cerr << "Topas is exiting due to a serious error in parameter file: " << fFileSpec << G4endl;
	G4cerr << "Parameter name: " << name << G4endl;
	G4cerr << message << G4endl;
	fPm->AbortSession(1);
}


void TsParameterFile::Undefined(const G4String& name) {
	// If value was being evaluated just to write out to a parameter dump, no need to quit."
	if (fPm->NowDoingParameterDump()) {
		fPm->UnableToCalculateForDump();
		return;
	}

	G4cerr << "Topas is exiting due to a serious error." << G4endl;
	G4cerr << "Parameter name: " << name << " has not been defined." << G4endl;
	fPm->AbortSession(1);
}
