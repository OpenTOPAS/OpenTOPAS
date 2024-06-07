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

#include "TsNtupleRoot.hh"
#if GEANT4_VERSION_MAJOR >= 11
#include "g4hntools_defs.hh"
#include "G4ToolsAnalysisManager.hh"
#else
#include "g4analysis_defs.hh"
#endif

TsNtupleRoot::TsNtupleRoot(TsParameterManager* pM, G4String fileName, G4String mode, TsVFile *masterFile, G4VAnalysisManager* analysisManager)
: TsVNtuple(pM, fileName, mode, masterFile),
fNtupleID(0), fAnalysisManager(analysisManager)
{
	// We can only write to a single file via G4AnalysisManager, so filename becomes the ntuple name
	fNtupleName = fBaseFileName;
	fBaseFileName = fAnalysisManager->GetFileName();
	SetFileExtensions(".root");
}


TsNtupleRoot::~TsNtupleRoot()
{;}


void TsNtupleRoot::SetFileName(G4String newBaseFileName)
{
	// We can only write to a single file via G4AnalysisManager, so we rename the ntuple instead
	fNtupleName = newBaseFileName;
	fIsPathUpdated = true;
}


void TsNtupleRoot::ConfirmCanOpen()
{
	if (fMasterFile != this)
		return;

	// If nothing has changed, no action required
	if (!fIsPathUpdated)
		return;

	if (fExtensionData.empty()) {
		G4cerr << "Topas is exiting due to a serious error in file IO." << G4endl;
		G4cerr << "Encountered file type that does not call SetFileExtensions() in constructor." << G4endl;
		fPm->AbortSession(1);
	}

	fNtupleID = fAnalysisManager->CreateNtuple(fNtupleName, fNtupleName);

	G4int iColD = 0;
	G4int iColF = 0;
	G4int iColI = 0;
	G4int iColB = 0;
	G4int iColS = 0;
	G4int iColI8 = 0;

	for (int iCol = 0; iCol < fNumberOfColumns; ++iCol) {

		if (fNamesD.find(iCol) != fNamesD.end()) {
			fAnalysisManager->CreateNtupleDColumn(fNtupleID, CreateValidBranchName(fNamesD[iCol]));
			iColD++;
		} else if (fNamesF.find(iCol) != fNamesF.end()) {
			fAnalysisManager->CreateNtupleFColumn(fNtupleID, CreateValidBranchName(fNamesF[iCol]));
			iColF++;
		} else if (fNamesI.find(iCol) != fNamesI.end()) {
			fAnalysisManager->CreateNtupleIColumn(fNtupleID, CreateValidBranchName(fNamesI[iCol]));
			iColI++;
		} else if (fNamesB.find(iCol) != fNamesB.end()) {
			fAnalysisManager->CreateNtupleIColumn(fNtupleID, CreateValidBranchName(fNamesB[iCol]));
			iColB++;
		} else if (fNamesS.find(iCol) != fNamesS.end()) {
			fAnalysisManager->CreateNtupleSColumn(fNtupleID, CreateValidBranchName(fNamesS[iCol]));
			iColS++;
		} else if (fNamesI8.find(iCol) != fNamesI8.end()) {
			fAnalysisManager->CreateNtupleIColumn(fNtupleID, CreateValidBranchName(fNamesI8[iCol]));
			iColI8++;
		} else {
			G4cerr << "Topas is exiting due to a serious error in file output." << G4endl;
			G4cerr << "A column could not be found" << G4endl;
			fPm->AbortSession(1);
		}
	}

	fAnalysisManager->FinishNtuple(fNtupleID);
	fIsPathUpdated = false;
}


void TsNtupleRoot::WriteBuffer()
{
	for (int iRow = 0; iRow < fNumberOfBufferEntries; ++iRow) {

		G4int iColD = 0;
		G4int iColF = 0;
		G4int iColI = 0;
		G4int iColB = 0;
		G4int iColS = 0;
		G4int iColI8 = 0;

		for (int iCol = 0; iCol < fNumberOfColumns; ++iCol) {

			if (fNamesD.find(iCol) != fNamesD.end()) {
				fAnalysisManager->FillNtupleDColumn(fNtupleID, iCol, fBufferD[iColD][iRow]);
				iColD++;
			} else if (fNamesF.find(iCol) != fNamesF.end()) {
				fAnalysisManager->FillNtupleFColumn(fNtupleID, iCol, fBufferF[iColF][iRow]);
				iColF++;
			} else if (fNamesI.find(iCol) != fNamesI.end()) {
				fAnalysisManager->FillNtupleIColumn(fNtupleID, iCol, fBufferI[iColI][iRow]);
				iColI++;
			} else if (fNamesB.find(iCol) != fNamesB.end()) {
				fAnalysisManager->FillNtupleIColumn(fNtupleID, iCol, fBufferB[iColB][iRow]);
				iColB++;
			} else if (fNamesS.find(iCol) != fNamesS.end()) {
				fAnalysisManager->FillNtupleSColumn(fNtupleID, iCol, fBufferS[iColS][iRow]);
				iColS++;
			} else if (fNamesI8.find(iCol) != fNamesI8.end()) {
				fAnalysisManager->FillNtupleIColumn(fNtupleID, iCol, fBufferI8[iColI8][iRow]);
				iColI8++;
			} else {
				G4cerr << "Topas is exiting due to a serious error in file output." << G4endl;
				G4cerr << "A column could not be found in buffer" << G4endl;
				fPm->AbortSession(1);
			}
		}

		fAnalysisManager->AddNtupleRow(fNtupleID);
	}
}


void TsNtupleRoot::Write()
{
	TsVNtuple::Write();
}


G4String TsNtupleRoot::CreateValidBranchName(G4String branchName)
{
	// replace non-alphanumeric characters with underscores
	for (G4String::iterator itr = branchName.begin(); itr != branchName.end(); ++itr) {
		if (!isalnum(*itr))
			*itr = '_';
	}

	// strip leading underscores
#if GEANT4_VERSION_MAJOR >= 11
   	G4StrUtil::lstrip(branchName,'_');
#else
    branchName.strip(G4String::leading, '_');
#endif

	return branchName;
}
