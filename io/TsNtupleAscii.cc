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

#include "TsNtupleAscii.hh"

#include <fstream>
#include <iomanip>

TsNtupleAscii::TsNtupleAscii(TsParameterManager* pM, G4String fileName, G4String mode, TsVFile *masterFile)
: TsVNtuple(pM, fileName, mode, masterFile)
{
	SetFileExtensions(".phsp", ".header");
}


TsNtupleAscii::~TsNtupleAscii()
{;}


void TsNtupleAscii::WriteBuffer()
{
	std::ofstream outFile(fPathData, std::ios::app);

	if (!outFile.good()) {
		G4cerr << "Topas is exiting due to a serious error in file output." << G4endl;
		G4cerr << "Output file: " << fPathData << " cannot be opened" << G4endl;
		fPm->AbortSession(1);
	}

	for (int iRow = 0; iRow < fNumberOfBufferEntries; ++iRow) {

		G4int iColD = 0;
		G4int iColF = 0;
		G4int iColI = 0;
		G4int iColB = 0;
		G4int iColS = 0;
		G4int iColI8 = 0;

		for (int iCol = 0; iCol < fNumberOfColumns; ++iCol) {

			if (fNamesD.find(iCol) != fNamesD.end()) {
				outFile << std::setw(12) << fBufferD[iColD][iRow] << " ";
				iColD++;
			} else if (fNamesF.find(iCol) != fNamesF.end()) {
				outFile << std::setw(12) << fBufferF[iColF][iRow] << " ";
				iColF++;
			} else if (fNamesI.find(iCol) != fNamesI.end()) {
				outFile << std::setw(12) << fBufferI[iColI][iRow] << " ";
				iColI++;
			} else if (fNamesB.find(iCol) != fNamesB.end()) {
				outFile << std::setw(2)  << fBufferB[iColB][iRow] << " ";
				iColB++;
			} else if (fNamesS.find(iCol) != fNamesS.end()) {
				outFile << std::setw(22) << fBufferS[iColS][iRow].substr(0,22) << " ";
				iColS++;
			} else if (fNamesI8.find(iCol) != fNamesI8.end()) {
				outFile << std::setw(12) << fBufferI8[iColI8][iRow] << " ";
				iColI8++;
			} else {
				outFile.close();
				G4cerr << "Topas is exiting due to a serious error in file output." << G4endl;
				G4cerr << "A column could not be found in buffer" << G4endl;
				fPm->AbortSession(1);
			}
		}
		outFile << G4endl;
	}

	outFile.close();
}


void TsNtupleAscii::GenerateColumnDescription()
{
	G4int nDigits = 0;
	G4int tmp = fNumberOfColumns;
	while (tmp) {
	    tmp /= 10;
	    nDigits++;
	}

	std::ostringstream desc;
	desc << G4endl << "Columns of data are as follows:" << G4endl;

	for (int iCol = 0; iCol < fNumberOfColumns; ++iCol) {

		if (fNamesD.find(iCol) != fNamesD.end()) {
			desc << std::setw(nDigits) << iCol+1 << ": " << fNamesD.find(iCol)->second << G4endl;
		} else if (fNamesF.find(iCol) != fNamesF.end()) {
			desc << std::setw(nDigits) << iCol+1 << ": " << fNamesF.find(iCol)->second << G4endl;
		} else if (fNamesI.find(iCol) != fNamesI.end()) {
			desc << std::setw(nDigits) << iCol+1 << ": " << fNamesI.find(iCol)->second << G4endl;
		} else if (fNamesB.find(iCol) != fNamesB.end()) {
			desc << std::setw(nDigits) << iCol+1 << ": " << fNamesB.find(iCol)->second << G4endl;
		} else if (fNamesS.find(iCol) != fNamesS.end()) {
			desc << std::setw(nDigits) << iCol+1 << ": " << fNamesS.find(iCol)->second << G4endl;
		} else if (fNamesI8.find(iCol) != fNamesI8.end()) {
			desc << std::setw(nDigits) << iCol+1 << ": " << fNamesI8.find(iCol)->second << G4endl;
		} else {
			G4cerr << "Topas is exiting due to a serious error in file output." << G4endl;
			G4cerr << "A column could not be found" << G4endl;
			fPm->AbortSession(1);
		}
	}
	desc << G4endl;

	fColumnDescription = desc.str();
}
