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

#include "TsFileHub.hh"

#include "TsParameterManager.hh"
#include "TsScoringManager.hh"
#include "TsExtensionManager.hh"

#include "TsNtupleAscii.hh"
#include "TsNtupleBinary.hh"
#include "TsNtupleRoot.hh"

#if GEANT4_VERSION_MAJOR >= 11
#include "g4hntools_defs.hh"
#include "G4ToolsAnalysisManager.hh"
#else
#include "g4analysis_defs.hh"
#endif

TsFileHub::TsFileHub(TsScoringManager* scM)
: fScm(scM)
{;}


TsFileHub::~TsFileHub()
{;}


TsVFile* TsFileHub::InstantiateFile(TsParameterManager* pM, TsExtensionManager*,
									  G4String fileName, G4String fileMode, G4String fileType,
									  TsVFile *masterFile)
{
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(fileType);
#else
	fileType.toLower();
#endif

	if (fileType == "ascii") {
		return new TsNtupleAscii(pM, fileName, fileMode, masterFile);
	} else if (fileType == "binary" || fileType == "limited") {
		return new TsNtupleBinary(pM, fileName, fileMode, masterFile);
	} else if (fileType == "root") {
		return new TsNtupleRoot(pM, fileName, fileMode, masterFile, fScm->GetRootAnalysisManager());
	} else return NULL;
}
