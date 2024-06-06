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

#ifndef TsDivergingMLC_hh
#define TsDivergingMLC_hh

#include "TsVGeometryComponent.hh"

class G4GenericTrap;

class TsDivergingMLC : public TsVGeometryComponent
{
public:
	TsDivergingMLC(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM,
				   TsVGeometryComponent* parentComponent, G4VPhysicalVolume* parentVolume, G4String& name);
	~TsDivergingMLC();

	G4VPhysicalVolume* Construct();
	void UpdateForSpecificParameterChange(G4String parameter);

private:
	std::pair<G4GenericTrap*,G4GenericTrap*> ConstructLeafPair(G4int,G4double,G4double,G4double,G4double,G4double,G4bool);

private:
	// Leaf Position Variables
	G4double  fSUSD;
	G4double  fSAD;
	G4double  fMagnification;
	G4double  fHalfTotalMLCWidth;
	G4double  fLeafThickness;
	G4double  fLeafHalfLength;
	G4double  fMaximumLeafOpen;
	G4double* fXPlusLeavesOpen;
	G4double* fXMinusLeavesOpen;
	G4int fNbOfLeavesPerSide;

	G4bool fIsXMLC;

	// Generic Volume storage
	std::vector<G4GenericTrap*> fGenericTrapXPlusLeaves;
	std::vector<G4GenericTrap*> fGenericTrapXMinusLeaves;

	// Logic Volume storage
	std::vector<G4LogicalVolume*> fLogicXPlusLeaves;
	std::vector<G4LogicalVolume*> fLogicXMinusLeaves;

	// Physical Volume storage
	std::vector<G4VPhysicalVolume*> fPhysicalXPlusLeaves;
	std::vector<G4VPhysicalVolume*> fPhysicalXMinusLeaves;
};

#endif
