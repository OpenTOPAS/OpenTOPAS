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

#include "TsVBiasingProcess.hh"
#include "TsGeometryManager.hh"
#include "TsParameterManager.hh"

TsVBiasingProcess::TsVBiasingProcess(G4String name, TsParameterManager* pM, TsGeometryManager* )
:fPm(pM), fName(name), fHadParameterChangeSinceLastRun(false)
{
	fVerbosity = fPm->GetIntegerParameter("Ts/SequenceVerbosity");
	if (fVerbosity>0)
		G4cout << "Registering biasing technique: " << fName << G4endl;
	
	ResolveParameters();
}


TsVBiasingProcess::~TsVBiasingProcess()
{;}


void TsVBiasingProcess::UpdateForSpecificParameterChange(G4String parameter) {
	if (fVerbosity>0)
		G4cout << "TsVBiasingProcess::UpdateForSpecificParameterChange for biasing: "
		<< fName << " called for parameter: " << parameter << G4endl;
	
	fHadParameterChangeSinceLastRun = true;
}


void TsVBiasingProcess::UpdateForNewRun(G4bool rebuiltSomeComponents) {
	if (fVerbosity>0)
		G4cout << "TsVBiasingProcess::UpdateForNewRun for biasing: "
		<< GetName() << " called with fHadParameterChangeSinceLastRun: "
		<< fHadParameterChangeSinceLastRun << ", rebuiltSomeComponents: "
		<< rebuiltSomeComponents << G4endl;
	
	if (fHadParameterChangeSinceLastRun) {
		ResolveParameters();
		fHadParameterChangeSinceLastRun = false;
	}
}


void TsVBiasingProcess::ResolveParameters() {
	if (fVerbosity>0)
		G4cout << "TsVBiasingProcess::ResolveParameters" << G4endl;
	
	fType = fPm->GetStringParameter(GetFullParmName("Type"));
}


void TsVBiasingProcess::AddBiasingProcess() {
	if (fVerbosity>0)
		G4cout << "TsVBiasingProcess:AddBiasingProcess" << G4endl;
}


void TsVBiasingProcess::Clear() {
	if (fVerbosity>0)
		G4cout << "TsVBiasingProcess:Clear" << G4endl;
}


void TsVBiasingProcess::Initialize() {
	if (fVerbosity>0)
		G4cout << "TsVBiasingProcess::Initialize" << G4endl;
}


G4String TsVBiasingProcess::GetFullParmName(G4String parmName) {
	G4String fullName = "Vr/" + fName + "/" + parmName;
	return fullName;
}


void TsVBiasingProcess::Quit(const G4String& name, G4String message) {
	G4cerr << "Topas is exiting due to a serious error in geometry setup." << G4endl;
	G4cerr << "Parameter name: " << name << G4endl;
	G4cerr << message << G4endl;
	fPm->AbortSession(1);
}
