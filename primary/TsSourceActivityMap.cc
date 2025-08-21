//
// ********************************************************************
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * TOPAS collaboration.                                             *
// * Use or redistribution of this code is not permitted without the  *
// * explicit approval of the TOPAS collaboration.                    *
// * Contact: Joseph Perl, perl@slac.stanford.edu                     *
// *                                                                  *
// ********************************************************************
//

#include "TsSourceActivityMap.hh"
#include "TsVGeometryComponent.hh"

#include "TsParameterManager.hh"

TsSourceActivityMap::TsSourceActivityMap(TsParameterManager* pM, TsSourceManager* psM, G4String sourceName) : TsSource(pM, psM, sourceName)
{
	ResolveParameters();
	actMap = (TsDicomActivityMap*)fComponent;
}

TsSourceActivityMap::~TsSourceActivityMap() { }

void TsSourceActivityMap::ResolveParameters()
{
	TsSource::ResolveParameters();

	if (fPm->ParameterExists(GetFullParmName("Radionuclide")))
		fRadionuclideName = fPm->GetStringParameter(GetFullParmName("Radionuclide"));
	else
	{
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << GetFullParmName("Radionuclide") << " needs to be specified." << G4endl;
		fPm->AbortSession(1);
	}
}
