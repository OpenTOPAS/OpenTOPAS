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

#include "TsNtupleBinary.hh"

#include <fstream>
#include <iomanip>

TsNtupleBinary::TsNtupleBinary(TsParameterManager* pM, G4String fileName, G4String mode, TsVFile *masterFile)
: TsVNtuple(pM, fileName, mode, masterFile)
{
	SetFileExtensions(".phsp", ".header");
}


TsNtupleBinary::~TsNtupleBinary()
{;}


void TsNtupleBinary::RegisterColumnS(G4String*, const G4String&)
{
	G4cerr << "Topas is exiting due to a serious error in file output." << G4endl;
	G4cerr << "Binary output is unable to support strings." << G4endl;
	fPm->AbortSession(1);
}


void TsNtupleBinary::WriteBuffer()
{
	std::ofstream outFile(fPathData, std::ios::app|std::ios::binary);

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
		G4int iColI8 = 0;

		for (int iCol = 0; iCol < fNumberOfColumns; ++iCol) {

			if (fNamesD.find(iCol) != fNamesD.end()) {
				outFile.write( reinterpret_cast<char*>(&fBufferD[iColD][iRow]), sizeof fBufferD[iColD][iRow]);
				iColD++;
			} else if (fNamesF.find(iCol) != fNamesF.end()) {
				outFile.write( reinterpret_cast<char*>(&fBufferF[iColF][iRow]), sizeof fBufferF[iColF][iRow]);
				iColF++;
			} else if (fNamesI.find(iCol) != fNamesI.end()) {
				outFile.write( reinterpret_cast<char*>(&fBufferI[iColI][iRow]), sizeof fBufferI[iColI][iRow]);
				iColI++;
			} else if (fNamesB.find(iCol) != fNamesB.end()) {
				G4bool tmp = static_cast<G4bool>(fBufferB[iColB][iRow]);
				outFile.write( reinterpret_cast<char*>(&tmp), sizeof tmp);
				iColB++;
			} else if (fNamesI8.find(iCol) != fNamesI8.end()) {
				outFile.write( reinterpret_cast<char*>(&fBufferI8[iColI8][iRow]), sizeof fBufferI8[iColI8][iRow]);
				iColI8++;
			} else {
				outFile.close();
				G4cerr << "Topas is exiting due to a serious error in file output." << G4endl;
				G4cerr << "A column could not be found in buffer" << G4endl;
				fPm->AbortSession(1);
			}
		}
	}

	outFile.close();
}


void TsNtupleBinary::GenerateColumnDescription()
{
	std::ostringstream desc;
	desc << G4endl << "Byte order of each record is as follows:" << G4endl;

	G4int nBytes = 0;
	for (int iCol = 0; iCol < fNumberOfColumns; ++iCol) {

		if (fNamesD.find(iCol) != fNamesD.end()) {
			G4int size = sizeof(G4double);
			desc << "f" << size << ": " << fNamesD.find(iCol)->second << G4endl;
			nBytes += size;
		} else if (fNamesF.find(iCol) != fNamesF.end()) {
			G4int size = sizeof(G4float);
			desc << "f" << size << ": " << fNamesF.find(iCol)->second << G4endl;
			nBytes += size;
		} else if (fNamesI.find(iCol) != fNamesI.end()) {
			G4int size = sizeof(G4int);
			desc << "i" << size << ": " << fNamesI.find(iCol)->second << G4endl;
			nBytes += size;
		} else if (fNamesB.find(iCol) != fNamesB.end()) {
			G4int size = sizeof(G4bool);
			desc << "b" << size << ": " << fNamesB.find(iCol)->second << G4endl;
			nBytes += size;
		} else if (fNamesI8.find(iCol) != fNamesI8.end()) {
			G4int size = sizeof(int8_t);
			desc << "i" << size << ": " << fNamesI8.find(iCol)->second << G4endl;
			nBytes += size;
		} else {
			G4cerr << "Topas is exiting due to a serious error in file output." << G4endl;
			G4cerr << "A column could not be found" << G4endl;
			fPm->AbortSession(1);
		}
	}
	desc << G4endl;

	std::ostringstream desc2;
	desc2 << "Number of Bytes per Particle: " << nBytes << G4endl;

	fColumnDescription = desc2.str() + desc.str();
}
