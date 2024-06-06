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

#ifndef TsFilterByInteractedOrTraversed_hh
#define TsFilterByInteractedOrTraversed_hh

#include "TsVFilter.hh"

class G4VPhysicalVolume;

class TsFilterByInteractedOrTraversed : public TsVFilter
{
public:
	TsFilterByInteractedOrTraversed(G4String name, TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM,
									TsFilterManager* fM, TsVGenerator* generator, TsVScorer* scorer, TsVFilter* parentFilter, G4bool invert,
					 G4bool includeAncestors, G4bool namesAreVolumes,
					 G4bool includeChildren, G4bool testTraversed,
					 G4bool onlyConsiderLastVolume);
	virtual ~TsFilterByInteractedOrTraversed();

	void ResolveParameters();
	void CacheGeometryPointers();

	virtual G4bool Accept(const G4Step*) const;
	virtual G4bool AcceptTrack(const G4Track*) const;

private:
	G4bool fIncludeAncestors;
	G4bool fNamesAreVolumes;
	G4bool fIncludeChildren;
	G4bool fTestTraversed;
	G4bool fOnlyConsiderLastVolume;
	G4String* fNames;
	G4int fNamesLength;
	std::vector<G4VPhysicalVolume*> fVolumes;
};
#endif

