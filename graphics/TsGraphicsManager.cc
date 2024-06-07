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

#include "TsGraphicsManager.hh"

#include "TsParameterManager.hh"

#include "TsGraphicsView.hh"

#include "G4VisExecutive.hh"
#include "G4VisManager.hh"

TsGraphicsManager::TsGraphicsManager(TsParameterManager* pM, TsGeometryManager* gM)
:fPm(pM), fGm(gM), fUsingOpenGL(false), fUsingRayTracer(false), fCurrentView(0)
{
	if (getenv("TOPAS_HEADLESS_MODE") || fPm->IsFindingSeed())
		fEnabled = false;
	else
		fEnabled = fPm->GetBooleanParameter("Gr/Enable");

	if (fEnabled) {
		fVerbosity = fPm->GetIntegerParameter("Ts/SequenceVerbosity");

		// Create list of known graphics parameters
		std::vector<G4String>* parameterNames = new std::vector<G4String>;
		parameterNames->push_back("OnlyIncludeParticlesNamed");
		parameterNames->push_back("OnlyIncludeParticlesCharged");
		parameterNames->push_back("OnlyIncludeParticlesFromVolume");
		parameterNames->push_back("OnlyIncludeParticlesFromComponent");
		parameterNames->push_back("OnlyIncludeParticlesFromComponentOrSubComponentsOf");
		parameterNames->push_back("OnlyIncludeParticlesWithInitialKEAbove");
		parameterNames->push_back("OnlyIncludeParticlesWithInitialKEBelow");
		parameterNames->push_back("OnlyIncludeParticlesWithInitialMomentumAbove");
		parameterNames->push_back("OnlyIncludeParticlesWithInitialMomentumBelow");
		parameterNames->push_back("OnlyIncludeParticlesFromProcess");

		// Check that all graphics parameters refer to known parameters
		fPm->CheckFilterParameterNamesStartingWith("Graphics", parameterNames);

		// Create the store for the TsGraphicsViews
		fViews = new std::map<G4String, TsGraphicsView*>;

		fVisManager = new G4VisExecutive("Quiet");
		fVisManager->Initialize();

		// Sequence Manager will need to know early on whether there will be any OpenGL or RayTracer views
		G4String prefix = "Gr";
		G4String suffix = "/Type";
		std::vector<G4String>* values = new std::vector<G4String>;
		fPm->GetParameterNamesBracketedBy(prefix, suffix, values);
		G4int length = values->size();

		for (G4int iToken=0; iToken<length; iToken++) {
			G4String viewerParmName = (*values)[iToken];
			G4String viewerType = fPm->GetStringParameter(viewerParmName);
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(viewerType);
#else
			viewerType.toLower();
#endif
			if (viewerType == "opengl" || viewerType.substr(0,3) == "ogl")
				fUsingOpenGL = true;
			if (viewerType == "raytracer")
				fUsingRayTracer = true;
		}
	}
}


TsGraphicsManager::~TsGraphicsManager()
{
	if (fEnabled) {
		delete fViews;
		delete fVisManager;
	}
}


void TsGraphicsManager::Initialize() {
	if (fEnabled) {
		// Initialize the views
		G4String prefix = "Gr";
		G4String suffix = "/Type";
		std::vector<G4String>* values = new std::vector<G4String>;
		fPm->GetParameterNamesBracketedBy(prefix, suffix, values);
		G4int length = values->size();

		// First pass, create only OpenGL views
		for (G4int iToken=0; iToken<length; iToken++) {
			G4String viewerParmName = (*values)[iToken];
			G4String viewerType = fPm->GetStringParameter(viewerParmName);
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(viewerType);
#else
			viewerType.toLower();
#endif
			if (viewerType == "opengl" || viewerType.substr(0,3) == "ogl") {
				size_t pos1 = viewerParmName.find("/Type");
				G4String viewerName = viewerParmName.substr(3, pos1-3);
				(*fViews)[viewerName] = new TsGraphicsView(fPm, this, fGm, viewerName);
			}
		}

		// Second pass, create anything except OpenGL or HepRep views
		for (G4int iToken=0; iToken<length; iToken++) {
			G4String viewerParmName = (*values)[iToken];
			G4String viewerType = fPm->GetStringParameter(viewerParmName);
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(viewerType);
#else
			viewerType.toLower();
#endif
			if (viewerType != "opengl" && viewerType.substr(0,3) != "ogl" && viewerType != "heprep") {
				size_t pos1 = viewerParmName.find("/Type");
				G4String viewerName = viewerParmName.substr(3, pos1-3);
				(*fViews)[viewerName] = new TsGraphicsView(fPm, this, fGm, viewerName);
			}
		}

		// Third pass, create HepRep view
		G4bool haveHepRep = false;
		for (G4int iToken=0; iToken<length; iToken++) {
			G4String viewerParmName = (*values)[iToken];
			G4String viewerType = fPm->GetStringParameter(viewerParmName);
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(viewerType);
#else
			viewerType.toLower();
#endif
			if (viewerType == "heprep") {
				if (haveHepRep) {
					G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
					G4cerr << "There can only be one HepRep view per session." << G4endl;
					fPm->AbortSession(1);
				} else {
					size_t pos1 = viewerParmName.find("/Type");
					G4String viewerName = viewerParmName.substr(3, pos1-3);
					(*fViews)[viewerName] = new TsGraphicsView(fPm, this, fGm, viewerName);
					haveHepRep = true;
				}
			}
		}

		// Reset current view to zero to indicate that we are no longer building views.
		SetCurrentView(0);
	}
}


void TsGraphicsManager::SetCurrentView(TsGraphicsView* view) {
	fCurrentView = view;
}


void TsGraphicsManager::NoteAnyUseOfChangeableParameters(const G4String& name)
{
	if (fCurrentView)
	{
		// Register use of this parameter for current view and LastDirectParameter.
		// LastDirectParameter is needed because we ultimately need to tell the view not the name of the changed parameter
		// but the name of the parameter the view directly accesses (that was affected by the changed parameter).
		// We store strings rather than pointers because we permit a parmeter to be overridden by another parameter
		// of the same name.
		G4String directParm = fPm->GetLastDirectParameterName();

		// See if this parameter has already been registered as used by this view (otherwise if this parameter
		// is interrogated twice for the same view, the view would update twice for the same change).
		G4bool matched = false;
		G4String nameToLower = name;
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(nameToLower);
#else
		nameToLower.toLower();
#endif
		std::multimap< G4String, std::pair<TsGraphicsView*, G4String> >::const_iterator iter;
		for (iter = fChangeableParameterMap.begin(); iter != fChangeableParameterMap.end() && !matched; iter++) {
			G4String gotParm = iter->first;
			TsGraphicsView* gotView = iter->second.first;
			G4String gotDirectParm = iter->second.second;
			if (gotParm==nameToLower && gotView==fCurrentView && gotDirectParm==directParm)
				matched = true;
		}

		if (!matched) {
			fChangeableParameterMap.insert(std::make_pair(nameToLower, std::make_pair(fCurrentView, directParm)));
			if (fPm->IGetIntegerParameter("Tf/Verbosity") > 0)
				G4cout << "Registered use of changeable parameter: " << name << "  by view: " << fCurrentView->GetName() << G4endl;
		}

		if (matched && fVerbosity>0)
			G4cout << "TsGraphicsManager::NoteAnyUse Again called with current view: " << fCurrentView->GetName() << ", param: " << name <<
			", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
		else if (fVerbosity>0)
			G4cout << "TsGraphicsManager::NoteAnyUse First called with current view: " << fCurrentView->GetName() << ", param: " << name <<
			", lastDirectParam: " << fPm->GetLastDirectParameterName() << ", lastDirectAction: " << fPm->GetLastDirectAction() << G4endl;
	}
}


void TsGraphicsManager::UpdateForSpecificParameterChange(G4String parameter) {
	std::multimap< G4String, std::pair<TsGraphicsView*,G4String> >::const_iterator iter;
	for (iter = fChangeableParameterMap.begin(); iter != fChangeableParameterMap.end(); iter++) {
		if (iter->first==parameter) {
			G4String directParameterName = iter->second.second;
			if (fVerbosity>0)
				G4cout << "TsGraphicsManager::UpdateForSpecificParameterChange called for parameter: " << parameter <<
				", matched for view: " << iter->second.first->GetName() << ", direct parameter: " << directParameterName << G4endl;
			iter->second.first->UpdateForSpecificParameterChange(directParameterName);
		}
	}
}


void TsGraphicsManager::UpdateForNewRun(G4bool rebuiltSomeComponents) {
	if (fEnabled) {
		if (fVerbosity>0)
			G4cout << "TsGraphicsManager::UpdateForNewRun called" << G4endl;
		std::map<G4String, TsGraphicsView*>::const_iterator iter;
		for (iter = fViews->begin(); iter != fViews->end(); ++iter)
			iter->second->UpdateForNewRun(rebuiltSomeComponents);
	}
}


void TsGraphicsManager::UpdateForEndOfRun() {
	if (fEnabled) {
		std::map<G4String, TsGraphicsView*>::const_iterator iter;
		for (iter = fViews->begin(); iter != fViews->end(); ++iter)
			iter->second->UpdateForEndOfRun();
	}
}


void TsGraphicsManager::UpdateForEndOfSession() {
	if (fEnabled) {
		std::map<G4String, TsGraphicsView*>::const_iterator iter;
		for (iter = fViews->begin(); iter != fViews->end(); ++iter)
			iter->second->UpdateForEndOfSession();
	}
}


G4bool TsGraphicsManager::UsingOpenGL() {
	return fUsingOpenGL;
}


G4bool TsGraphicsManager::UsingRayTracer() {
	return fUsingRayTracer;
}


G4VViewer* TsGraphicsManager::GetCurrentViewer() {
	return fVisManager->GetCurrentViewer();
}
