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

#include "TsVFile.hh"

#include "G4UIcommand.hh"

#include <fstream>

TsVFile::TsVFile(TsParameterManager* pM, G4String fileName, G4String mode, TsVFile *masterFile)
: fPm(pM), fIsPathUpdated(true), fHasHeader(false), fBaseFileName(fileName)
{
	fMasterFile = masterFile ? masterFile : this;

#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(mode);
#else
	mode.toLower();
#endif
	if (mode == "overwrite") fMode = OVERWRITE;
	else if (mode == "increment") fMode = INCREMENT;
	else fMode = EXIT;
}


TsVFile::~TsVFile()
{;}


// public method, which checks if action is needed
void TsVFile::SetFileName(G4String newBaseFileName)
{
	if (fBaseFileName == newBaseFileName)
		return;

	SetFileName(newBaseFileName, 0);
}


// private method, which forces action
void TsVFile::SetFileName(G4String newBaseFileName, G4int increment)
{
	// Worker scorers get filename from master scorer (increment/exit would not work)
	if (fMasterFile != this) {
		fBaseFileName = fMasterFile->fBaseFileName;
		fPathData = fMasterFile->fPathData;
		fPathHeader = fMasterFile->fPathHeader;
	    return;
	}

	G4String incrementStr = "";
	if (increment > 0)
		incrementStr = "_" + G4UIcommand::ConvertToString(increment);

	G4String newPathData   = newBaseFileName + incrementStr + fExtensionData;
	G4String newPathHeader = newBaseFileName + incrementStr + fExtensionHeader;

	// Check data and header files simultaneously, to ensure increment is consistent
	G4bool dataExists = FileExists(newPathData);
	G4bool headerExists = FileExists(newPathHeader);

	switch (fMode) {
	case OVERWRITE:
		break;

	case INCREMENT:
		if (dataExists || (fHasHeader && headerExists)) {
			SetFileName(newBaseFileName, ++increment);
			return;
		}
		break;

	case EXIT:
		if (dataExists || (fHasHeader && headerExists)) {
			G4String foundPath = dataExists ? newPathData : newPathHeader;
			G4cerr << "Topas is exiting due to a serious error in file IO." << G4endl;
			G4cerr << "Output file: " << foundPath << " already exists" << G4endl;
			G4cerr << "If you really want to allow this, specify: " << G4endl;
			G4cerr << "IfOutputFileAlreadyExists as Overwrite or Increment." << G4endl;
			fPm->AbortSession(1);
		}
		break;
	}

	fBaseFileName = newBaseFileName;
	fPathData = newPathData;
	fPathHeader = newPathHeader;
	fIsPathUpdated = true;
}


void TsVFile::ConfirmCanOpen()
{
	// If nothing has changed, no action required
	if (!fIsPathUpdated)
		return;

	// Only master scorers write to disk
	if (fMasterFile != this)
		return;

	if (fExtensionData.empty()) {
		G4cerr << "Topas is exiting due to a serious error in file IO." << G4endl;
		G4cerr << "Encountered file type that does not call SetFileExtensions() in constructor." << G4endl;
		fPm->AbortSession(1);
	}

	G4bool dataWriteable = IsWriteable(fPathData);
	G4bool headerWriteable = false;
	if (fHasHeader)
		headerWriteable = IsWriteable(fPathHeader);

	if (!dataWriteable || (fHasHeader && !headerWriteable)) {
		G4String unwriteablePath = !dataWriteable ? fPathData : fPathHeader;
		G4cerr << "Topas is exiting due to a serious error in file IO." << G4endl;
		G4cerr << "Cannot write to output file: " << unwriteablePath << G4endl;
		fPm->AbortSession(1);
	}

	fIsPathUpdated = false;
}


void TsVFile::SetFileExtensions(G4String extData, G4String extHeader)
{
	fHasHeader = !extHeader.empty();
	fExtensionData = extData;
	fExtensionHeader = extHeader;

	SetFileName(fBaseFileName, 0);
}


bool TsVFile::IsWriteable(G4String filePath) const
{
	std::ofstream outFile(filePath);
	return outFile.good();
}


bool TsVFile::FileExists(G4String filePath) const
{
	std::ifstream inFile(filePath);
	return inFile.good();
}
