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

#include "TsParameterManager.hh"

#include "TsAutomaticImportanceSamplingOperator.hh"
#include "TsAutomaticImportanceSamplingProcess.hh"

#include "G4BiasingProcessInterface.hh"

TsAutomaticImportanceSamplingOperator::TsAutomaticImportanceSamplingOperator(TsParameterManager* pM, G4String name)
: G4VBiasingOperator(name), fPm(pM)
{
	fImportanceOperation = new TsAutomaticImportanceSamplingProcess(fPm, name);
	fAcceptedLogicalVolumeNames = fPm->GetStringVector("Vr/" + name + "/ApplyBiasingInVolumesNamed");
	fNbOfAcceptedLogicalVolumeNames = fPm->GetVectorLength("Vr/" + name + "/ApplyBiasingInVolumesNamed");
	if ( fNbOfAcceptedLogicalVolumeNames > 1 ) {
		G4cerr << "TOPAS is exiting due to a serius problem in variance reduction setup" << G4endl;
		G4cerr << "For automatic importance sampling, only one volume (binned) is allowed" << G4endl;
		fPm->AbortSession(1);
	}
					
	G4String inputFile = fPm->GetStringParameter("Vr/" + name + "/InputFile" );
	std::ifstream in(inputFile);
	if (!in.good() ) {
		G4cerr << "TOPAS is exiting due to a serius problem in variance reduction setup" << G4endl;
		G4cerr << "For automatic importance sampling parameter:" << G4endl;
		G4cerr << "Vr/" + name + "/InputFile. Input file not found" << G4endl;
		fPm->AbortSession(1);
	}
					
	G4int xbins = fPm->GetIntegerParameter("Ge/" + fAcceptedLogicalVolumeNames[0] + "/XBins");
	G4int ybins = fPm->GetIntegerParameter("Ge/" + fAcceptedLogicalVolumeNames[0] + "/YBins");
	G4int zbins = fPm->GetIntegerParameter("Ge/" + fAcceptedLogicalVolumeNames[0] + "/ZBins");
	G4double importance;
	G4cout << "[VRT] Assigning importance values " << G4endl;
	for ( int ii = 0; ii < xbins; ii++ ) {
		for ( int jj = 0; jj < ybins; jj++ ) {
			for ( int kk = 0; kk < zbins; kk++ ) {
				G4int idx = ii*ybins*zbins + jj*zbins + kk;
				in.read(reinterpret_cast<char*>(&importance), sizeof(importance));
				fImportanceValues[idx] = importance;
			}
		}
	}
	G4cout << "[VRT] Assigned importance values " << G4endl;
	fDivisionCounts.clear();
	fDivisionCounts.push_back(xbins);
	fDivisionCounts.push_back(ybins);
	fDivisionCounts.push_back(zbins);
	in.close();
}


TsAutomaticImportanceSamplingOperator::~TsAutomaticImportanceSamplingOperator()
{}


void TsAutomaticImportanceSamplingOperator::StartRun() {
	G4cout << GetName() << " : starting run " << G4endl;
	((TsAutomaticImportanceSamplingProcess*)fImportanceOperation)->SetImportanceValues(fImportanceValues);
	((TsAutomaticImportanceSamplingProcess*)fImportanceOperation)->SetDivisionCounts(fDivisionCounts);
}


G4VBiasingOperation*
TsAutomaticImportanceSamplingOperator::
ProposeNonPhysicsBiasingOperation(const G4Track*,
								  const G4BiasingProcessInterface*) {
	return fImportanceOperation;
}


G4bool TsAutomaticImportanceSamplingOperator::IsApplicable(G4LogicalVolume* logicalVolume) {
	G4bool applicable = false;
	for ( int i = 0; i < fNbOfAcceptedLogicalVolumeNames; i++ )
		if ( logicalVolume->GetName() == fAcceptedLogicalVolumeNames[i] )
			applicable = true;
	return applicable;
}
