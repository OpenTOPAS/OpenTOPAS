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
#include "G4SystemOfUnits.hh"
#include "G4TransportationManager.hh"

TsVElectroMagneticField::TsVElectroMagneticField(TsParameterManager* pM, TsGeometryManager* gM,
								   TsVGeometryComponent* component):
fPm(pM), fGm(gM), fComponent(component) {
	fGm->SetCurrentElectroMagneticField(this);

	fNavigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();

	fFieldEquation = new G4EqMagElectricField( this );

	G4String stepper_name = "ClassicalRK4";
	if (fPm->ParameterExists(fComponent->GetFullParmName("FieldStepper")))
		stepper_name = fPm->GetStringParameter(fComponent->GetFullParmName("FieldStepper"));

#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(stepper_name);
#else
	stepper_name.toLower();
#endif
	if (stepper_name == "expliciteuler")
		fStepper = new G4ExplicitEuler(fFieldEquation, 8);
	else if (stepper_name == "impliciteuler")
		fStepper = new G4ImplicitEuler(fFieldEquation, 8);
	else if (stepper_name == "simplerunge")
		fStepper = new G4SimpleRunge(fFieldEquation, 8);
	else if (stepper_name == "simpleheum")
		fStepper = new G4SimpleHeum(fFieldEquation, 8);
	else
		fStepper = new G4ClassicalRK4(fFieldEquation, 8);

	G4double StepMin = 1.0 * mm;
	if (fPm->ParameterExists(fComponent->GetFullParmName("FieldStepMinimum")))
		StepMin = fPm->GetDoubleParameter(fComponent->GetFullParmName("FieldStepMinimum"), "Length");

	G4double dChord = 1.0e-1 * mm;;
	if (fPm->ParameterExists(fComponent->GetFullParmName("FieldDeltaChord")))
		dChord = fPm->GetDoubleParameter(fComponent->GetFullParmName("FieldDeltaChord"), "Length");

	fIntgrDriver = new G4MagInt_Driver(StepMin, fStepper, fStepper->GetNumberOfVariables());

	fChordFinder = new G4ChordFinder(fIntgrDriver);
	fChordFinder->SetDeltaChord(dChord);

	fFieldManager = new G4FieldManager(this, fChordFinder, false);

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
