//
// ********************************************************************
// *                                                                  *
// * This file was obtained from Topas MC Inc                         *
// * under the license agreement set forth at                         *
// * http://www.topasmc.org/registration                              *
// * Any use of this file constitutes full acceptance                 *
// * of this TOPAS MC license agreement.                              *
// *                                                                  *
// ********************************************************************
//

#include "TsSource.hh"
#include "TsDicomActivityMap.hh"


#ifndef TsSourceActivityMap_hh
#define TsSourceActivityMap_hh

class TsSourceActivityMap : public TsSource
{
public:
	TsSourceActivityMap(TsParameterManager* pM, TsSourceManager* psM, G4String sourceName);
	virtual ~TsSourceActivityMap();

	void ResolveParameters();

	inline TsDicomActivityMap* GetActivityMap()		{ return actMap; }
	inline G4String GetRadionuclideName()			{ return fRadionuclideName; }

private:
	TsDicomActivityMap* actMap;
	G4String fRadionuclideName;
};



#endif
