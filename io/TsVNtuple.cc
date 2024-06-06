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

#include "TsVNtuple.hh"

#include <fstream>

#ifdef TOPAS_MT
#include "G4MTRunManager.hh"
#include "G4AutoLock.hh"

namespace {
	G4Mutex absorbBufferMutex = G4MUTEX_INITIALIZER;
}
#endif


TsVNtuple::TsVNtuple(TsParameterManager* pM, G4String fileName, G4String mode, TsVFile *masterFile)
: TsVFile(pM, fileName, mode, masterFile),
fNumberOfColumns(0), fNumberOfBufferEntries(0), fNumberOfEntries(0), fBufferSize(10000),
fSuppressColumnDescription(false)
{
#ifdef TOPAS_MT
	if (G4Threading::IsWorkerThread())
		fBufferSize /= G4MTRunManager::GetMasterRunManager()->GetNumberOfThreads();
#endif
}


TsVNtuple::~TsVNtuple()
{;}


void TsVNtuple::SetBufferSize(G4int bufferSize)
{
	fBufferSize = bufferSize;
#ifdef TOPAS_MT
	if (G4Threading::IsWorkerThread())
		fBufferSize /= G4MTRunManager::GetMasterRunManager()->GetNumberOfThreads();
#endif
}


void TsVNtuple::RegisterColumnD(G4double *address, const G4String& name, const G4String& unit)
{
	if (unit.empty()) {
		fNamesD[fNumberOfColumns] = name;
		fUnitsD.push_back(1.0);
	} else {
		fNamesD[fNumberOfColumns] = name + " [" + unit + "]";
		fUnitsD.push_back(fPm->GetUnitValue(unit));
	}

	fAddressesD.push_back(address);
	fBufferD.push_back(std::vector<G4double>());
	fNumberOfColumns++;
}


void TsVNtuple::RegisterColumnF(G4float *address, const G4String& name, const G4String& unit)
{
	if (unit.empty()) {
		fNamesF[fNumberOfColumns] = name;
		fUnitsF.push_back(1.0);
	} else {
		fNamesF[fNumberOfColumns] = name + " [" + unit + "]";
		fUnitsF.push_back(fPm->GetUnitValue(unit));
	}

	fAddressesF.push_back(address);
	fBufferF.push_back(std::vector<G4float>());
	fNumberOfColumns++;
}


void TsVNtuple::RegisterColumnI(G4int *address, const G4String& name)
{
	fNamesI[fNumberOfColumns] = name;
	fAddressesI.push_back(address);
	fBufferI.push_back(std::vector<G4int>());
	fNumberOfColumns++;
}

void TsVNtuple::RegisterColumnI8(int8_t *address, const G4String& name)
{
	fNamesI8[fNumberOfColumns] = name;
	fAddressesI8.push_back(address);
	fBufferI8.push_back(std::vector<int8_t>());
	fNumberOfColumns++;
}


void TsVNtuple::RegisterColumnB(G4bool *address, const G4String& name)
{
	fNamesB[fNumberOfColumns] = name;
	fAddressesB.push_back(address);
	fBufferB.push_back(std::vector<G4bool>());
	fNumberOfColumns++;
}


void TsVNtuple::RegisterColumnS(G4String *address, const G4String& name)
{
	fNamesS[fNumberOfColumns] = name;
	fAddressesS.push_back(address);
	fBufferS.push_back(std::vector<G4String>());
	fNumberOfColumns++;
}


void TsVNtuple::Fill()
{
	fNumberOfBufferEntries++;
	fNumberOfEntries++;
	FillBuffer();

	// In order to correctly restore an event, should only write when a new history is encountered
	if (fNumberOfBufferEntries == fBufferSize) {
#ifdef TOPAS_MT
		// For MT, we absorb ntuple not just at end of run, but also whenever buffer is full
		if (G4Threading::IsWorkerThread()) {
			TsVNtuple* masterNtuple = dynamic_cast<TsVNtuple*>(fMasterFile);
			masterNtuple->AbsorbWorkerNtuple(this);
		}
#else
		ConfirmCanOpen();
		WriteBuffer();
		ClearBuffer();
		fNumberOfBufferEntries = 0;
#endif
	}
}


void TsVNtuple::Write()
{
	ConfirmCanOpen();
	WriteBuffer();
	ClearBuffer();
	fNumberOfBufferEntries = 0;

	if (fHasHeader)
		WriteHeader();
}


void TsVNtuple::AbsorbWorkerNtuple(TsVNtuple* workerNtuple)
{
#ifdef TOPAS_MT
	G4AutoLock l(&absorbBufferMutex);
#endif

	// Write master buffer to disk if it cannot accommodate worker buffer
	if (fNumberOfBufferEntries + workerNtuple->fNumberOfBufferEntries > fBufferSize) {
		ConfirmCanOpen();
		WriteBuffer();
		ClearBuffer();
		fNumberOfBufferEntries = 0;
	}

	AbsorbBufferFromWorkerNtuple(workerNtuple);
	fNumberOfBufferEntries += workerNtuple->fNumberOfBufferEntries;
	fNumberOfEntries += workerNtuple->fNumberOfBufferEntries;
	workerNtuple->ClearBuffer();
	workerNtuple->fNumberOfBufferEntries = 0;
}


void TsVNtuple::FillBuffer()
{
	std::vector<G4double*>::iterator itrAddressD = fAddressesD.begin();
	std::vector<std::vector<G4double> >::iterator itrBufferD = fBufferD.begin();
	std::vector<G4double>::const_iterator itrUnitD = fUnitsD.begin();
	while (itrAddressD != fAddressesD.end()) {
		itrBufferD->push_back(**itrAddressD / *itrUnitD);
		itrAddressD++;
		itrBufferD++;
		itrUnitD++;
	}

	std::vector<G4float*>::iterator itrAddressF = fAddressesF.begin();
	std::vector<std::vector<G4float> >::iterator itrBufferF = fBufferF.begin();
	std::vector<G4double>::const_iterator itrUnitF = fUnitsF.begin();
	while (itrAddressF != fAddressesF.end()) {
		itrBufferF->push_back(**itrAddressF / *itrUnitF);
		itrAddressF++;
		itrBufferF++;
		itrUnitF++;
	}

	std::vector<G4int*>::iterator itrAddressI = fAddressesI.begin();
	std::vector<std::vector<G4int> >::iterator itrBufferI = fBufferI.begin();
	while (itrAddressI != fAddressesI.end()) {
		itrBufferI->push_back(**itrAddressI);
		itrAddressI++;
		itrBufferI++;
	}

	std::vector<int8_t*>::iterator itrAddressI8 = fAddressesI8.begin();
	std::vector<std::vector<int8_t> >::iterator itrBufferI8 = fBufferI8.begin();
	while (itrAddressI8 != fAddressesI8.end()) {
		itrBufferI8->push_back(**itrAddressI8);
		itrAddressI8++;
		itrBufferI8++;
	}

	std::vector<G4bool*>::iterator itrAddressB = fAddressesB.begin();
	std::vector<std::vector<G4bool> >::iterator itrBufferB = fBufferB.begin();
	while (itrAddressB != fAddressesB.end()) {
		itrBufferB->push_back(**itrAddressB);
		itrAddressB++;
		itrBufferB++;
	}

	std::vector<G4String*>::iterator itrAddressS = fAddressesS.begin();
	std::vector<std::vector<G4String> >::iterator itrBufferS = fBufferS.begin();
	while (itrAddressS != fAddressesS.end()) {
		itrBufferS->push_back(**itrAddressS);
		itrAddressS++;
		itrBufferS++;
	}
}


void TsVNtuple::AbsorbBufferFromWorkerNtuple(TsVNtuple* workerNtuple)
{
	if (fBufferD.size() != workerNtuple->fBufferD.size() ||
		fBufferF.size() != workerNtuple->fBufferF.size() ||
		fBufferI.size() != workerNtuple->fBufferI.size() ||
		fBufferI8.size() != workerNtuple->fBufferI8.size() ||
		fBufferB.size() != workerNtuple->fBufferB.size() ||
		fBufferS.size() != workerNtuple->fBufferS.size()) {

		G4cerr << "Topas is exiting due to a serious error in file output." << G4endl;
		G4cerr << "Inconsistent buffers found when absorbing ntuples across threads." << G4endl;
		fPm->AbortSession(1);
	}

	std::vector<std::vector<G4double> >::iterator itrMasterD = fBufferD.begin();
	std::vector<std::vector<G4double> >::iterator itrWorkerD = workerNtuple->fBufferD.begin();
	while (itrMasterD != fBufferD.end()) {
		itrMasterD->insert(itrMasterD->end(), itrWorkerD->begin(), itrWorkerD->end());
		itrMasterD++;
		itrWorkerD++;
	}

	std::vector<std::vector<G4float> >::iterator itrMasterF = fBufferF.begin();
	std::vector<std::vector<G4float> >::iterator itrWorkerF = workerNtuple->fBufferF.begin();
	while (itrMasterF != fBufferF.end()) {
		itrMasterF->insert(itrMasterF->end(), itrWorkerF->begin(), itrWorkerF->end());
		itrMasterF++;
		itrWorkerF++;
	}

	std::vector<std::vector<G4int> >::iterator itrMasterI = fBufferI.begin();
	std::vector<std::vector<G4int> >::iterator itrWorkerI = workerNtuple->fBufferI.begin();
	while (itrMasterI != fBufferI.end()) {
		itrMasterI->insert(itrMasterI->end(), itrWorkerI->begin(), itrWorkerI->end());
		itrMasterI++;
		itrWorkerI++;
	}

	std::vector<std::vector<int8_t> >::iterator itrMasterI8 = fBufferI8.begin();
	std::vector<std::vector<int8_t> >::iterator itrWorkerI8 = workerNtuple->fBufferI8.begin();
	while (itrMasterI8 != fBufferI8.end()) {
		itrMasterI8->insert(itrMasterI8->end(), itrWorkerI8->begin(), itrWorkerI8->end());
		itrMasterI8++;
		itrWorkerI8++;
	}

	std::vector<std::vector<G4bool> >::iterator itrMasterB = fBufferB.begin();
	std::vector<std::vector<G4bool> >::iterator itrWorkerB = workerNtuple->fBufferB.begin();
	while (itrMasterB != fBufferB.end()) {
		itrMasterB->insert(itrMasterB->end(), itrWorkerB->begin(), itrWorkerB->end());
		itrMasterB++;
		itrWorkerB++;
	}

	std::vector<std::vector<G4String> >::iterator itrMasterS = fBufferS.begin();
	std::vector<std::vector<G4String> >::iterator itrWorkerS = workerNtuple->fBufferS.begin();
	while (itrMasterS != fBufferS.end()) {
		itrMasterS->insert(itrMasterS->end(), itrWorkerS->begin(), itrWorkerS->end());
		itrMasterS++;
		itrWorkerS++;
	}
}


void TsVNtuple::ClearBuffer()
{
	for (std::vector<std::vector<G4double> >::iterator itr=fBufferD.begin(); itr!=fBufferD.end(); ++itr)
		itr->clear();

	for (std::vector<std::vector<G4float> >::iterator itr=fBufferF.begin(); itr!=fBufferF.end(); ++itr)
		itr->clear();

	for (std::vector<std::vector<G4int> >::iterator itr=fBufferI.begin(); itr!=fBufferI.end(); ++itr)
		itr->clear();

	for (std::vector<std::vector<int8_t> >::iterator itr=fBufferI8.begin(); itr!=fBufferI8.end(); ++itr)
		itr->clear();

	for (std::vector<std::vector<G4bool> >::iterator itr=fBufferB.begin(); itr!=fBufferB.end(); ++itr)
		itr->clear();

	for (std::vector<std::vector<G4String> >::iterator itr=fBufferS.begin(); itr!=fBufferS.end(); ++itr)
		itr->clear();
}


void TsVNtuple::WriteHeader()
{
	std::ofstream outFile(fPathHeader, std::ios::out);

	if (!outFile.good()) {
		G4cerr << "Topas is exiting due to a serious error in file output." << G4endl;
		G4cerr << "Output file: " << fPathHeader << " cannot be opened" << G4endl;
		fPm->AbortSession(1);
	}

	if (!fHeaderPrefix.empty())
		outFile << fHeaderPrefix;

	if (!fSuppressColumnDescription) {
		if (fColumnDescription.empty())
			GenerateColumnDescription();
		outFile << fColumnDescription;
	}

	if (!fHeaderSuffix.empty())
		outFile << fHeaderSuffix;

	outFile.close();
}
