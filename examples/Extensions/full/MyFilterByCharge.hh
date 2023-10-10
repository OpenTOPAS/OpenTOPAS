//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * This file was obtained from Topas MC Inc under the license       *
// * agreement set forth at http://www.topasmc.org/registration       *
// * Any use of this file constitutes full acceptance of              *
// * this TOPAS MC license agreement.                                 *
// *                                                                  *
// ********************************************************************
//

#ifndef MyFilterByCharge_hh
#define MyFilterByCharge_hh

#include "TsVFilter.hh"

class MyFilterByCharge : public TsVFilter
{
public:
	MyFilterByCharge(G4String name, TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM,
					 TsFilterManager* fM, TsVGenerator* generator, TsVScorer* scorer, TsVFilter* parentFilter);

	virtual ~MyFilterByCharge();
	
	void ResolveParameters();
	
	virtual G4bool Accept(const G4Step*) const;
	virtual G4bool AcceptTrack(const G4Track*) const;

private:
	G4bool fAcceptNegative;
	G4bool fAcceptNeutral;
	G4bool fAcceptPositive;
};
#endif

