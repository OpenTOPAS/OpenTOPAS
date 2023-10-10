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

#ifndef TsVNtuple_hh
#define TsVNtuple_hh

#include "TsVFile.hh"

#include <vector>
#include <stdint.h>

class TsVNtuple : public TsVFile
{
public:
	TsVNtuple(TsParameterManager* pM, G4String fileName, G4String mode, TsVFile *masterFile);
	~TsVNtuple();

	void SetBufferSize(G4int bufferSize);

	virtual void RegisterColumnD(G4double *address, const G4String& name, const G4String& unit);
	virtual void RegisterColumnF(G4float  *address, const G4String& name, const G4String& unit);
	virtual void RegisterColumnI(G4int    *address, const G4String& name);
	virtual void RegisterColumnI8(int8_t  *address, const G4String& name);
	virtual void RegisterColumnB(G4bool   *address, const G4String& name);
	virtual void RegisterColumnS(G4String *address, const G4String& name);

	void Fill();
	virtual void Write();
	void AbsorbWorkerNtuple(TsVNtuple* workerNtuple);

	void SuppressColumnDescription(G4bool suppress) { fSuppressColumnDescription = suppress; }
	G4String fHeaderPrefix;
	G4String fHeaderSuffix;

	G4long GetNumberOfEntries() { return fNumberOfEntries; }

protected:
	void FillBuffer();
	void AbsorbBufferFromWorkerNtuple(TsVNtuple* workerNtuple);
	void ClearBuffer();
	void WriteHeader();

	// should be implemented by derived classes
	virtual void WriteBuffer() = 0;
	virtual void GenerateColumnDescription() {;}

	G4int fNumberOfColumns;
	G4int fNumberOfBufferEntries;
	G4long fNumberOfEntries;
	G4int fBufferSize;

	std::vector<G4double*> fAddressesD;
	std::vector<G4float*>  fAddressesF;
	std::vector<G4int*>    fAddressesI;
	std::vector<int8_t*>  fAddressesI8;
	std::vector<G4bool*>   fAddressesB;
	std::vector<G4String*> fAddressesS;

	std::map<G4int, G4String> fNamesD;
	std::map<G4int, G4String> fNamesF;
	std::map<G4int, G4String> fNamesI;
	std::map<G4int, G4String> fNamesI8;
	std::map<G4int, G4String> fNamesB;
	std::map<G4int, G4String> fNamesS;

	std::vector<std::vector<G4double> > fBufferD;
	std::vector<std::vector<G4float> >  fBufferF;
	std::vector<std::vector<G4int> >    fBufferI;
	std::vector<std::vector<int8_t> >  fBufferI8;
	std::vector<std::vector<G4bool> >   fBufferB;
	std::vector<std::vector<G4String> > fBufferS;

	std::vector<G4double> fUnitsD;
	std::vector<G4double> fUnitsF;

	G4bool fSuppressColumnDescription;
	G4String fColumnDescription;
};

#endif
