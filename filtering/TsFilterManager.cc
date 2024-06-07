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

#include "TsFilterManager.hh"

#include "TsParameterManager.hh"
#include "TsGeometryManager.hh"
#include "TsFilterManager.hh"

#include "TsFilterHub.hh"
#include "TsVFilter.hh"

TsFilterManager::TsFilterManager(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM)
:fPm(pM), fEm(eM), fMm(mM), fGm(gM)
{
	fTfVerbosity = fPm->GetIntegerParameter("Tf/Verbosity");

#ifdef TOPAS_MT
	fCurrentFilter.Put(0);
#else
	fCurrentFilter = 0;
#endif

	// Instantiate the filterHub, used to link in the specfic filters
	fFilterHub = new TsFilterHub();
}


TsFilterManager::~TsFilterManager()
{
}


TsVFilter* TsFilterManager::InstantiateFilter(TsVGenerator* generator)
{
	return InstantiateFilter(generator, 0);
}


TsVFilter* TsFilterManager::InstantiateFilter(TsVScorer* scorer)
{
	return InstantiateFilter(0, scorer);
}


TsVFilter* TsFilterManager::InstantiateFilter(TsVGenerator* generator, TsVScorer* scorer)
{
	TsVFilter* filter = fFilterHub->InstantiateFilter(fPm, fEm, fMm, fGm, this, generator, scorer);

	// Reset current filter to zero to indicate that we are no longer building filters.
	SetCurrentFilter(0);

	return filter;
}


void TsFilterManager::SetCurrentFilter(TsVFilter* filter) {
#ifdef TOPAS_MT
	fCurrentFilter.Put(filter);
#else
	fCurrentFilter = filter;
#endif

	if (filter) fFilters.push_back(filter);
}


void TsFilterManager::NoteAnyUseOfChangeableParameters(const G4String& name)
{
#ifdef TOPAS_MT
	if (fCurrentFilter.Get())
#else
	if (fCurrentFilter)
#endif
	{
		// Register use of this parameter for current filter and LastDirectParameter.
		// LastDirectParameter is needed because we ultimately need to tell the filter not the name of the changed parameter
		// but the name of the parameter the filter directly accesses (that was affected by the changed parameter).
		// We store strings rather than pointers because we permit a parmeter to be overridden by another parameter
		// of the same name.
		G4String directParm = fPm->GetLastDirectParameterName();

		// See if this parameter has already been registered as used by this filter (otherwise if this parameter
		// is interrogated twice for the same filter, the filter would update twice for the same change).
		G4String nameToLower = name;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(nameToLower);
#else
		nameToLower.toLower();
#endif
		G4bool matched = false;
		std::multimap< G4String, std::pair<TsVFilter*,G4String> >::const_iterator iter;
		for (iter = fChangeableParameterMap.begin(); iter != fChangeableParameterMap.end() && !matched; iter++) {
			G4String gotParm = iter->first;
			TsVFilter* gotFilter = iter->second.first;
			G4String gotDirectParm = iter->second.second;
#ifdef TOPAS_MT
			if (gotParm==nameToLower && gotFilter==fCurrentFilter.Get() && gotDirectParm==directParm)
#else
			if (gotParm==nameToLower && gotFilter==fCurrentFilter && gotDirectParm==directParm)
#endif
				matched = true;
		}

		if (!matched) {
#ifdef TOPAS_MT
			fChangeableParameterMap.insert(std::make_pair(nameToLower, std::make_pair(fCurrentFilter.Get(), directParm)));
			if (fTfVerbosity > 0)
				G4cout << "Registered use of changeable parameter: " << name << "  by filter: " << fCurrentFilter.Get()->GetName() << G4endl;
		}

		if (matched && fTfVerbosity > 0)
			G4cout << "TsFilterManager::NoteAnyUse Again called with current filter: " << fCurrentFilter.Get()->GetName() << ", param: " << name <<
			", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
		else if (fTfVerbosity > 0)
			G4cout << "TsFilterManager::NoteAnyUse First called with current filter: " << fCurrentFilter.Get()->GetName() << ", param: " << name <<
			", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
#else
		fChangeableParameterMap.insert(std::make_pair(nameToLower, std::make_pair(fCurrentFilter, directParm)));
		if (fTfVerbosity > 0)
			G4cout << "Registered use of changeable parameter: " << name << "  by filter: " << fCurrentFilter->GetName() << G4endl;
	}

	if (matched && fTfVerbosity > 0)
		G4cout << "TsFilterManager::NoteAnyUse Again called with current filter: " << fCurrentFilter->GetName() << ", param: " << name <<
		", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
	else if (fTfVerbosity > 0)
		G4cout << "TsFilterManager::NoteAnyUse First called with current filter: " << fCurrentFilter->GetName() << ", param: " << name <<
		", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
#endif
	}
}


void TsFilterManager::UpdateForSpecificParameterChange(G4String parameter) {
	std::multimap< G4String, std::pair<TsVFilter*,G4String> >::const_iterator iter;
	for (iter = fChangeableParameterMap.begin(); iter != fChangeableParameterMap.end(); iter++) {
		if (iter->first==parameter) {
			G4String directParameterName = iter->second.second;
			if (fTfVerbosity > 0)
				G4cout << "TsFilterManager::UpdateForSpecificParameterChange called for parameter: " << parameter <<
				", matched for filter: " << iter->second.first->GetName() << ", direct parameter: " << directParameterName << G4endl;
			iter->second.first->UpdateForSpecificParameterChange(directParameterName);
		}
	}
}


void TsFilterManager::UpdateForNewRun(G4bool rebuiltSomeComponents) {
	std::vector<TsVFilter*>::iterator iter;
	for (iter=fFilters.begin(); iter!=fFilters.end(); iter++)
		(*iter)->UpdateForNewRun(rebuiltSomeComponents);
}
