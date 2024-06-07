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

#include "TsSource.hh"

#include "TsParameterManager.hh"
#include "TsGeometryManager.hh"
#include "TsSourceManager.hh"
#include "TsVGeometryComponent.hh"

#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4IonTable.hh"
#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"

#include "Randomize.hh"

TsSource::TsSource(TsParameterManager* pM, TsSourceManager* psM, G4String sourceName)
:fPm(pM), fSourceName(sourceName),
fPsm(psM), fHadParameterChangeSinceLastRun(false), fProbabilityOfUsingAGivenRandomTime(1.),
fTotalHistoriesGenerated(0), fTotalParticlesGenerated(0), fTotalParticlesSkipped(0)
{
	fVerbosity = fPm->GetIntegerParameter("So/Verbosity");

	if (fPm->IsRandomMode())
		fNumberOfHistoriesInRandomJob = fPm->GetIntegerParameter(GetFullParmName("NumberOfHistoriesInRandomJob"));
	else
		fNumberOfHistoriesInRandomJob = 0;

	fPsm->SetCurrentSource(this);

	ResolveParameters();
}


TsSource::~TsSource()
{
}


void TsSource::UpdateForSpecificParameterChange(G4String parameter) {
	if (fVerbosity>0)
		G4cout << "TsSource::UpdateForSpecificParameterChange called for parameter: " << parameter << G4endl;
	fHadParameterChangeSinceLastRun = true;
}


void TsSource::UpdateForNewRun(G4bool rebuiltSomeComponents) {
	if (fVerbosity>0)
		G4cout << "TsSource::UpdateForNewRun for source: " << GetName() << " called with fHadParameterChangeSinceLastRun: " <<
		fHadParameterChangeSinceLastRun << ", rebuiltSomeComponents: " << rebuiltSomeComponents << G4endl;

	if (fHadParameterChangeSinceLastRun) {
		ResolveParameters();
		fHadParameterChangeSinceLastRun = false;
	}
}


void TsSource::UpdateForEndOfRun() {
}


void TsSource::ResolveParameters() {
	if (fVerbosity>0)
		G4cout << "TsSource::ResolveParameters" << G4endl;

	G4String componentName = fPm->GetStringParameter(GetFullParmName("Component"));
	fComponent = fPsm->GetGeometryManager()->GetComponent(componentName);
	if (!fComponent) {
		G4cerr << "Topas is exiting. Particle source: " << fSourceName << G4endl;
		G4cerr <<" has been placed relative to unknown Geometry Component: " << componentName << G4endl;
		fPm->AbortSession(1);
	}

	if (fPm->ParameterExists(GetFullParmName("NumberOfHistoriesInRun"))) {
		fNumberOfHistoriesInRun = fPm->GetIntegerParameter(GetFullParmName("NumberOfHistoriesInRun"));
		if (fNumberOfHistoriesInRun > 1E9) {
			G4cerr << "Topas is exiting. Particle source: " << fSourceName << G4endl;
			G4cerr << "has NumberOfHistoriesInRun set greater than 10^9." << G4endl;
			fPm->AbortSession(1);
		}
	} else
		fNumberOfHistoriesInRun = 0;

	if (fPm->ParameterExists(GetFullParmName("ProbabilityOfUsingAGivenRandomTime")))
		fProbabilityOfUsingAGivenRandomTime = fPm->GetUnitlessParameter(GetFullParmName("ProbabilityOfUsingAGivenRandomTime"));
	else
		fProbabilityOfUsingAGivenRandomTime = -1.;
}


G4bool TsSource::RandomModeNeedsMoreRuns() {
	return fTotalHistoriesGenerated < fNumberOfHistoriesInRandomJob;
}


G4int TsSource::GetNumberOfHistoriesInRun() {
	return fNumberOfHistoriesInRun;
}


G4long TsSource::GetNumberOfHistoriesInRandomJob() {
	return fNumberOfHistoriesInRandomJob;
}


G4double TsSource::GetProbabilityOfUsingAGivenRandomTime() {
	return fProbabilityOfUsingAGivenRandomTime;
}


G4String TsSource::GetFullParmName(const char* parm) {
	G4String fullName = "So/"+fSourceName+"/"+parm;
	return fullName;
}


G4String TsSource::GetName() {
	return fSourceName;
}


void TsSource::NoteNumberOfHistoriesGenerated(G4int number) {
	fTotalHistoriesGenerated += (G4long)number;
}

void TsSource::NoteNumberOfParticlesGenerated(G4long number) {
	fTotalParticlesGenerated += number;
}


void TsSource::NoteNumberOfParticlesSkipped(G4long number) {
	fTotalParticlesSkipped += number;
}


void TsSource::Finalize() {
	G4cout << "Particle source " << fSourceName <<": Total number of histories: "<< fTotalHistoriesGenerated << G4endl;

	if (fTotalParticlesSkipped > 0) {
		G4cout << G4endl;

		if (fTotalParticlesSkipped==1)
			G4cout << "Particle source " << fSourceName << " skipped generating 1 particle due to unknown particle type." << G4endl;
		else
			G4cout << "Particle source " << fSourceName << " skipped generating " << fTotalParticlesSkipped << " particles to unknown particle type." << G4endl;
		G4cout << "It may be that your current physics list doesn't define all of the particle types you're trying to use in this session." << G4endl;
		G4cout << G4endl;
	}
}
