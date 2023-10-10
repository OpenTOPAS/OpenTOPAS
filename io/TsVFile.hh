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

#ifndef TsVFile_hh
#define TsVFile_hh

#include "TsParameterManager.hh"
#include "TsTopasConfig.hh"

#include "globals.hh"

class TsVFile
{
public:
	TsVFile(TsParameterManager* pM, G4String fileName, G4String mode, TsVFile *masterFile);
	~TsVFile();

	virtual void SetFileName(G4String newBaseFileName);

	G4bool HasHeaderFile() const { return fHasHeader; }
	G4String GetHeaderFileName() const { return fPathHeader; }
	G4String GetDataFileName() const { return fPathData; }
	virtual void ConfirmCanOpen();

	G4String fHeaderText;

protected:
	void SetFileExtensions(G4String extData, G4String extHeader="");

	TsParameterManager* fPm;
	TsVFile *fMasterFile;

	enum FileMode { OVERWRITE, INCREMENT, EXIT } fMode;
	G4bool fIsPathUpdated;
	G4bool fHasHeader;

	G4String fBaseFileName;  // path without extension or increment
	G4String fExtensionData;
	G4String fExtensionHeader;
	G4String fPathData;
	G4String fPathHeader;

private:
	void SetFileName(G4String newBaseFileName, G4int increment);
	bool IsWriteable(G4String filePath) const;
	bool FileExists(G4String filePath) const;
};

#endif
