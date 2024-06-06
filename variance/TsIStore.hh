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

#ifndef TsIStore_hh
#define TsIStore_hh TsIStore_hh

#include "G4GeometryCellImportance.hh"
#include "G4GeometryCellComp.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VIStore.hh"

#include <map>

class TsIStore : public G4VIStore
{
public:
	static TsIStore* GetInstance(const G4String& ParallelWorldName);

protected:
	explicit TsIStore(const G4String& ParallelWorldName);
	~TsIStore() override = default;

public:
	void AddPropertyValueToGeometryCell(G4int propertyValue, const G4GeometryCell &gCell, G4int propertyOption);
	G4int GetPropertyValue(const G4GeometryCell &gCell, G4int propertyOption);

private:
	std::map<G4GeometryCell, G4int, G4GeometryCellComp> fGeomCellNumberOfSplit;
	std::map<G4GeometryCell, G4int, G4GeometryCellComp> fGeomCellSymmetry;
	std::map<G4GeometryCell, G4int, G4GeometryCellComp> fGeomCellRussianRoulette;

	const G4VPhysicalVolume* fWorldVolume;

	static G4ThreadLocal TsIStore* fInstance;

	G4GeometryCellImportance fGeometryCelli;

public:
	G4double GetImportance(const G4GeometryCell &gCell) const override;
	G4bool IsKnown(const G4GeometryCell &gCell) const override;
	const G4VPhysicalVolume& GetWorldVolume() const override;
	virtual G4bool IsInWorld(const G4VPhysicalVolume &) const;
	virtual const G4VPhysicalVolume* GetParallelWorldVolumePointer() const;

};

#endif
