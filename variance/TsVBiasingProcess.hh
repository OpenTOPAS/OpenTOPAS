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

#ifndef TsVBiasingProcess_hh
#define TsVBiasingProcess_hh

#include "TsGeometryManager.hh"

class TsParameterManager;

class TsVBiasingProcess
{
public:
	TsVBiasingProcess(G4String name, TsParameterManager* pM, TsGeometryManager* gM);
	
	virtual ~TsVBiasingProcess();
	
protected:
	virtual void ResolveParameters();
	virtual void Quit(const G4String& name, G4String message);
	virtual G4String GetFullParmName(G4String parmName);

	TsParameterManager* fPm;
	
public:
	void UpdateForSpecificParameterChange(G4String parameter);
	void UpdateForNewRun(G4bool rebuiltSomeComponents);
	
	virtual void AddBiasingProcess();
	virtual void Clear();
	virtual void Initialize();
	
	virtual G4String GetName() { return fName;};
	virtual G4String GetTypeName() { return fType;};
	
private:
	G4String fName;
	G4String fType;
	G4bool fHadParameterChangeSinceLastRun;
	G4int fVerbosity;
};
#endif

