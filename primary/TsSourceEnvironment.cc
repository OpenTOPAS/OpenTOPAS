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

#include "TsSourceEnvironment.hh"

#include "TsVGeometryComponent.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4UnitsTable.hh"

TsSourceEnvironment::TsSourceEnvironment(TsParameterManager* pM, TsSourceManager* psM, G4String sourceName)
: TsSource(pM, psM, sourceName)
, fAccumulatedFluence(0.)
{}

TsSourceEnvironment::~TsSourceEnvironment()
{}

void TsSourceEnvironment::UpdateForEndOfRun()
{
	G4int nHistories = GetNumberOfHistoriesInRun();
	G4double radius = fComponent->GetExtent().GetExtentRadius();
	G4double fluence = nHistories/(pi*radius*radius);  // See documentation
	fAccumulatedFluence += fluence;

	G4cout << "\nTsSourceEnvironment::UpdateForEndOfRun" << G4endl;
	G4cout << "An isotropic, homogenous particle flux was generated on the inner surface of a spherical cavity." << G4endl;
	G4cout << "Radius of cavity is " << G4BestUnit(radius,"Length") << G4endl;
	G4cout << nHistories << " histories in this run" << G4endl;
	G4cout << "Omnidirectional fluence ( /mm2 ): " << fluence*mm2 << G4endl;
	G4cout << "Accumulated fluence this session ( /mm2 ): " << fAccumulatedFluence*mm2 << G4endl;
}
