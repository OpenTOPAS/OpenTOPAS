//
// ********************************************************************
// *                                                                  *
// * Copyright 2026 The TOPAS Collaboration                           *
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

#include "TsVElectroMagneticField.hh"

#include "TsParameterManager.hh"
#include "TsGeometryManager.hh"

#include "TsVGeometryComponent.hh"

#include "G4LogicalVolume.hh"
#include "G4EqMagElectricField.hh"
#include "G4MagIntegratorDriver.hh"
#include "G4FieldManager.hh"
#include "G4ChordFinder.hh"
#include "G4ExplicitEuler.hh"
#include "G4ImplicitEuler.hh"
#include "G4SimpleRunge.hh"
#include "G4SimpleHeum.hh"
#include "G4ClassicalRK4.hh"
#include "G4BogackiShampine23.hh"
#include "G4BogackiShampine45.hh"
#include "G4DormandPrince745.hh"
#include "G4DormandPrinceRK56.hh"
#include "G4DormandPrinceRK78.hh"
#include "G4TsitourasRK45.hh"
#include "G4SystemOfUnits.hh"
#include "G4TransportationManager.hh"

#include <limits>

TsVElectroMagneticField::TsVElectroMagneticField(TsParameterManager* pM, TsGeometryManager* gM,
								   TsVGeometryComponent* component):
fPm(pM), fGm(gM), fComponent(component) {
	fGm->SetCurrentElectroMagneticField(this);

	fNavigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();

	fFieldEquation = new G4EqMagElectricField( this );

	G4String stepperParameter = fComponent->GetFullParmName("FieldStepper");
	if (!fPm->ParameterExists(stepperParameter)) {
		// Preserve the established TOPAS field construction when no override
		// was requested; do not synthesize or retrieve a parameter value.
		fStepper = new G4ClassicalRK4(fFieldEquation, 8);
	} else {
		G4String requestedStepper = fPm->GetStringParameter(stepperParameter);
		G4String stepperName = requestedStepper;
		G4StrUtil::to_lower(stepperName);

		if (stepperName == "expliciteuler")
			fStepper = new G4ExplicitEuler(fFieldEquation, 8);
		else if (stepperName == "impliciteuler")
			fStepper = new G4ImplicitEuler(fFieldEquation, 8);
		else if (stepperName == "simplerunge")
			fStepper = new G4SimpleRunge(fFieldEquation, 8);
		else if (stepperName == "simpleheum")
			fStepper = new G4SimpleHeum(fFieldEquation, 8);
		else if (stepperName == "bogackishampine23")
			fStepper = new G4BogackiShampine23(fFieldEquation, 8);
		else if (stepperName == "bogackishampine45")
			fStepper = new G4BogackiShampine45(fFieldEquation, 8);
		else if (stepperName == "dormandprince745")
			fStepper = new G4DormandPrince745(fFieldEquation, 8);
		else if (stepperName == "dormandprincerk56")
			fStepper = new G4DormandPrinceRK56(fFieldEquation, 8);
		else if (stepperName == "dormandprincerk78")
			fStepper = new G4DormandPrinceRK78(fFieldEquation, 8);
		else if (stepperName == "tsitourasrk45")
			fStepper = new G4TsitourasRK45(fFieldEquation, 8);
		else if (stepperName == "classicalrk4")
			fStepper = new G4ClassicalRK4(fFieldEquation, 8);
		else {
			G4cerr << "Topas is exiting due to a serious error in electromagnetic field setup." << G4endl;
			G4cerr << "Parameter name: " << stepperParameter << G4endl;
			G4cerr << "Value is not a valid electromagnetic FieldStepper: " << requestedStepper << G4endl;
			fPm->AbortSession(1);
		}
	}

	G4double StepMin = 1.0 * mm;
	G4String stepMinimumParameter = fComponent->GetFullParmName("FieldStepMinimum");
	if (fPm->ParameterExists(stepMinimumParameter)) {
		StepMin = fPm->GetDoubleParameter(stepMinimumParameter, "Length");
		if (StepMin <= 0.) {
			G4cerr << "Topas is exiting due to a serious error in electromagnetic field setup." << G4endl;
			G4cerr << "Parameter name: " << stepMinimumParameter << G4endl;
			G4cerr << "Value must be greater than zero." << G4endl;
			fPm->AbortSession(1);
		}
	}

	G4double dChord = 1.0e-1 * mm;;
	G4String deltaChordParameter = fComponent->GetFullParmName("FieldDeltaChord");
	if (fPm->ParameterExists(deltaChordParameter)) {
		dChord = fPm->GetDoubleParameter(deltaChordParameter, "Length");
		if (dChord <= 0.) {
			G4cerr << "Topas is exiting due to a serious error in electromagnetic field setup." << G4endl;
			G4cerr << "Parameter name: " << deltaChordParameter << G4endl;
			G4cerr << "Value must be greater than zero." << G4endl;
			fPm->AbortSession(1);
		}
	}

	fIntgrDriver = new G4MagInt_Driver(StepMin, fStepper, fStepper->GetNumberOfVariables());

	fChordFinder = new G4ChordFinder(fIntgrDriver);
	fChordFinder->SetDeltaChord(dChord);

	fFieldManager = new G4FieldManager(this, fChordFinder, false);

	G4String parameterName = fComponent->GetFullParmName("FieldDeltaOneStep");
	if (fPm->ParameterExists(parameterName)) {
		G4double value = fPm->GetDoubleParameter(parameterName, "Length");
		if (value <= 0.) {
			G4cerr << "Topas is exiting due to a serious error in electromagnetic field setup." << G4endl;
			G4cerr << "Parameter name: " << parameterName << G4endl;
			G4cerr << "Value must be greater than zero." << G4endl;
			fPm->AbortSession(1);
		}
		fFieldManager->SetDeltaOneStep(value);
	}

	parameterName = fComponent->GetFullParmName("FieldDeltaIntersection");
	if (fPm->ParameterExists(parameterName)) {
		G4double value = fPm->GetDoubleParameter(parameterName, "Length");
		if (value <= 0.) {
			G4cerr << "Topas is exiting due to a serious error in electromagnetic field setup." << G4endl;
			G4cerr << "Parameter name: " << parameterName << G4endl;
			G4cerr << "Value must be greater than zero." << G4endl;
			fPm->AbortSession(1);
		}
		fFieldManager->SetDeltaIntersection(value);
	}

	G4String minimumEpsilonParameter = fComponent->GetFullParmName("FieldMinimumEpsilonStep");
	G4String maximumEpsilonParameter = fComponent->GetFullParmName("FieldMaximumEpsilonStep");
	G4bool hasMinimumEpsilon = fPm->ParameterExists(minimumEpsilonParameter);
	G4bool hasMaximumEpsilon = fPm->ParameterExists(maximumEpsilonParameter);
	if (hasMinimumEpsilon || hasMaximumEpsilon) {
		G4double minimumEpsilon = hasMinimumEpsilon ?
			fPm->GetUnitlessParameter(minimumEpsilonParameter) : fFieldManager->GetMinimumEpsilonStep();
		G4double maximumEpsilon = hasMaximumEpsilon ?
			fPm->GetUnitlessParameter(maximumEpsilonParameter) : fFieldManager->GetMaximumEpsilonStep();
		G4double minimumAcceptedEpsilon = 1000. * std::numeric_limits<G4double>::epsilon();
		G4double maximumAcceptedEpsilon = G4FieldManager::GetMaxAcceptedEpsilon();
		if (minimumEpsilon < minimumAcceptedEpsilon || minimumEpsilon > maximumEpsilon ||
			maximumEpsilon > maximumAcceptedEpsilon) {
			G4cerr << "Topas is exiting due to a serious error in electromagnetic field setup." << G4endl;
			G4cerr << "Parameters " << minimumEpsilonParameter << " and " << maximumEpsilonParameter << G4endl;
			G4cerr << "must satisfy " << minimumAcceptedEpsilon
			       << " <= minimum epsilon <= maximum epsilon <= " << maximumAcceptedEpsilon << "." << G4endl;
			fPm->AbortSession(1);
		}
		// Expand the accepted interval before narrowing it, avoiding Geant4's
		// corrective warning and transient modification of the other bound.
		if (hasMinimumEpsilon && hasMaximumEpsilon &&
			minimumEpsilon > fFieldManager->GetMaximumEpsilonStep()) {
			fFieldManager->SetMaximumEpsilonStep(maximumEpsilon);
			fFieldManager->SetMinimumEpsilonStep(minimumEpsilon);
		} else {
			if (hasMinimumEpsilon)
				fFieldManager->SetMinimumEpsilonStep(minimumEpsilon);
			if (hasMaximumEpsilon)
				fFieldManager->SetMaximumEpsilonStep(maximumEpsilon);
		}
	}

	fComponent->GetEnvelopeLogicalVolume()->SetFieldManager(fFieldManager, true);
}


TsVElectroMagneticField::~TsVElectroMagneticField() {;}


void TsVElectroMagneticField::ResolveParameters() {
}


void TsVElectroMagneticField::UpdateForSpecificParameterChange(G4String parameter)
{
	if (fPm->GetIntegerParameter("Ts/SequenceVerbosity") > 0)
		G4cout << "TsVElectroMagneticField::UpdateForSpecificParameterChange called to do nothing for parameter: " << parameter << G4endl;
}


void TsVElectroMagneticField::UpdateForNewRun() {
	ResolveParameters();
}


TsVGeometryComponent* TsVElectroMagneticField::GetComponent() {
	return fComponent;
}
