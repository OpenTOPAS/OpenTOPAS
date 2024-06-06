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

#include "TsIStore.hh"

#include "G4VPhysicalVolume.hh"
#include "G4GeometryCell.hh"
#include "G4GeometryCellStepStream.hh"
#include "G4LogicalVolume.hh"
#include "G4TransportationManager.hh"

G4ThreadLocal TsIStore* TsIStore::fInstance = 0;

TsIStore::TsIStore(const G4String& ParallelWorldName)
:fWorldVolume(G4TransportationManager::GetTransportationManager()->GetParallelWorld(ParallelWorldName))
{
	G4cout << " TsIStore:: ParallelWorldName = " << fWorldVolume->GetName() << G4endl;
	G4cout << " TsIStore:: fParallelWorldName = " << ParallelWorldName << G4endl;
}


void TsIStore::AddPropertyValueToGeometryCell(G4int propertyValue, const G4GeometryCell &gCell, G4int propertyOption) {
	if ( propertyOption == -1 )
		fGeomCellNumberOfSplit[gCell] = propertyValue;
	else if ( propertyOption == 0 )
		fGeomCellSymmetry[gCell] = propertyValue;
	else if ( propertyOption == 1 )
		fGeomCellRussianRoulette[gCell] = propertyValue;
}


G4int TsIStore::GetPropertyValue(const G4GeometryCell &gCell, G4int propertyOption) {
	G4int propertyValue = 0;
	if ( propertyOption == -1 )
        propertyValue = fGeomCellNumberOfSplit[gCell];
	else if ( propertyOption == 0 )
        propertyValue = fGeomCellSymmetry[gCell];
	else if ( propertyOption == 1 )
        propertyValue = fGeomCellRussianRoulette[gCell];

	return propertyValue;
}


TsIStore* TsIStore::GetInstance(const G4String& ParallelWorldName) {
	if (!fInstance) {
		G4cout << "TsStore:: Creating new IStore " << ParallelWorldName <<  G4endl;
		fInstance = new TsIStore(ParallelWorldName);
	}
	return fInstance;
}


G4double TsIStore::GetImportance(const G4GeometryCell &) const {
	return -1;
}


G4bool TsIStore::IsKnown(const G4GeometryCell &) const {
	return true;
}


const G4VPhysicalVolume& TsIStore::GetWorldVolume() const {
	return *fWorldVolume;
}


G4bool TsIStore::IsInWorld(const G4VPhysicalVolume &) const {
	return true;
}


const G4VPhysicalVolume* TsIStore::GetParallelWorldVolumePointer() const {
   return fWorldVolume;
}
