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

#include "TsParameterManager.hh"
#include "TsGraphicsManager.hh"
#include "TsGeometryManager.hh"

#include "TsGraphicsView.hh"
#include "TsVGeometryComponent.hh"
#include "TsTopasConfig.hh"

#include "G4UImanager.hh"
#include "G4UIcommand.hh"
#include "G4VPhysicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4IonTable.hh"

TsGraphicsView::TsGraphicsView(TsParameterManager* pM, TsGraphicsManager* grM, TsGeometryManager* gM, G4String viewerName)
:fPm(pM), fGrm(grM), fGm(gM), fViewerName(viewerName), fRefreshEvery("run"), fColorModel("charge"),
fIncludeGeometry(true), fIncludeTrajectories(true), fUseSmoothTrajectories(true), fIncludeStepPoints(false), fIncludeAxes(false),
fIsActive(true), fAlreadyCreated(false), fMagneticFieldArrowDensity(0), fHadParameterChangeSinceLastRun(false)
{
	fVerbosity = fPm->GetIntegerParameter("Ts/SequenceVerbosity");

	fGrm->SetCurrentView(this);

	if (fPm->ParameterExists(GetFullParmName("Active")) &&
		!fPm->GetBooleanParameter(GetFullParmName("Active")))
		fIsActive = false;

	if (fIsActive)
		CreateView();
}


TsGraphicsView::~TsGraphicsView()
{
}


void TsGraphicsView::CreateView() {
	fAlreadyCreated = true;

	if (fPm->ParameterExists(GetFullParmName("IncludeGeometry")) &&
		!fPm->GetBooleanParameter(GetFullParmName("IncludeGeometry")))
		fIncludeGeometry = false;

	if (fPm->ParameterExists(GetFullParmName("IncludeTrajectories")) &&
		!fPm->GetBooleanParameter(GetFullParmName("IncludeTrajectories")))
		fIncludeTrajectories = false;

	if (fPm->ParameterExists(GetFullParmName("UseSmoothTrajectories")) &&
		!fPm->GetBooleanParameter(GetFullParmName("UseSmoothTrajectories")))
		fUseSmoothTrajectories = false;

	if (fPm->ParameterExists(GetFullParmName("IncludeStepPoints")) &&
		fPm->GetBooleanParameter(GetFullParmName("IncludeStepPoints")))
		fIncludeStepPoints = true;

	if (fPm->ParameterExists(GetFullParmName("IncludeAxes")) &&
		fPm->GetBooleanParameter(GetFullParmName("IncludeAxes")))
		fIncludeAxes = true;

	if (fPm->ParameterExists(GetFullParmName("MagneticFieldArrowDensity")))
		fMagneticFieldArrowDensity = fPm->GetIntegerParameter(GetFullParmName("MagneticFieldArrowDensity"));

	//G4UImanager::GetUIpointer()->ApplyCommand("/vis/verbose 6");

	// Create the geometry part of the scene
	G4UImanager::GetUIpointer()->ApplyCommand("/vis/scene/create " + fViewerName);

	if (fIncludeGeometry) {
		G4bool mainWorldIsVisible;

		if (fPm->ParameterExists(GetFullParmName("VisibleWorlds"))) {
			mainWorldIsVisible = false;

			G4String* visibleWorldNames = fPm->GetStringVector(GetFullParmName("VisibleWorlds"));
			G4int length = fPm->GetVectorLength(GetFullParmName("VisibleWorlds"));

			for (G4int i = 0; i < length; i++) {
				G4String testName = visibleWorldNames[i];
#if GEANT4_VERSION_MAJOR >= 11
				G4StrUtil::to_lower(testName);
#else
				testName.toLower();
#endif
				if (testName == "world")
					visibleWorldNames[i] = "World";
				if (testName == "all")
					visibleWorldNames[i] = "worlds";

				G4UImanager::GetUIpointer()->ApplyCommand("/vis/scene/add/volume " + visibleWorldNames[i]);

				if (visibleWorldNames[i]=="World" || visibleWorldNames[i]=="worlds")
					mainWorldIsVisible = true;
			}
		} else {
			mainWorldIsVisible = true;
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/scene/add/volume worlds");
		}

		if (!mainWorldIsVisible) {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/scene/add/volume World 0 0");
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/geometry/set/visibility World 0 0");
		}
	} else {
		if (!fIncludeTrajectories && !fIncludeStepPoints) {
			G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
			G4cerr << "Graphics has all three of geometry, trajectories and step points turned off." << G4endl;
			G4cerr << "That doesn't leave anything to draw." << G4endl;
			fPm->AbortSession(1);
		}

		// If you have no run duration models, Vis assumes this is a mistake and adds in the world.
		// So to really have nothing, add something and then turn it invisible.
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/scene/add/volume World 0 0");
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/geometry/set/visibility World 0 0");
	}

	// Control axes
	if (fIncludeAxes) {
		G4String axesComponentName = "World";
		if (fPm->ParameterExists(GetFullParmName("AxesComponent")))
			axesComponentName = fPm->GetStringParameter(GetFullParmName("AxesComponent"));

		TsVGeometryComponent* axesComponent = fGm->GetComponent(axesComponentName);
		if (axesComponent) {
			G4Point3D* axesCenter = axesComponent->GetTransRelToWorld();
			G4double axesPosX = axesCenter->x();
			G4double axesPosY = axesCenter->y();
			G4double axesPosZ = axesCenter->z();

			G4double axesSize = 3000.;
			if (fPm->ParameterExists(GetFullParmName("AxesSize")))
				axesSize = fPm->GetDoubleParameter(GetFullParmName("AxesSize"), "Length");

			G4String axesCommand = "/vis/scene/add/axes " +
			G4UIcommand::ConvertToString(axesPosX) + " " + G4UIcommand::ConvertToString(axesPosY) + " " +
			G4UIcommand::ConvertToString(axesPosZ) + " " + G4UIcommand::ConvertToString(axesSize) + " mm";

			G4UImanager::GetUIpointer()->ApplyCommand(axesCommand);
		} else {
			G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
			G4cerr << "Gr/" << fViewerName << "/AxesComponent is set to unknown component: " << axesComponentName << G4endl;
			fPm->AbortSession(1);
		}
	}

	// Control magnetic field visualization
	if (fMagneticFieldArrowDensity > 0) {
		G4String magnetiFieldCommand = "/vis/scene/add/magneticField " + G4UIcommand::ConvertToString(fMagneticFieldArrowDensity);
		G4UImanager::GetUIpointer()->ApplyCommand(magnetiFieldCommand);
	}

	// Set viewer type
	fViewerType = fPm->GetStringParameter(GetFullParmName("Type"));
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(fViewerType);
#else
	fViewerType.toLower();
#endif
	if (fViewerType=="opengl") {
		if (fGm->IsTooComplexForOGLS()) fViewerType = "OGLI";
		else fViewerType = "OGL";
	}
	else if (fViewerType=="heprep") fViewerType = "HepRepFile";
	else if (fViewerType=="vrml") fViewerType = "VRML2FILE";
	else if (fViewerType=="raytracer") fViewerType = "RayTracer";
	else if (fViewerType=="raytracerx") fViewerType = "RayTracerX";
	else if (fViewerType=="dawn") fViewerType = "DAWNFILE";
	else if (fViewerType=="gmocren") fViewerType = "gMocrenFile";
	else {
	    G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
	    G4cerr << "Parameter " << GetFullParmName("Type") << " specifies unknown viewer type: " << fPm->GetStringParameter(GetFullParmName("Type")) << G4endl;
	    fPm->AbortSession(1);
	}

	// Create the sceneHandler
	G4UImanager::GetUIpointer()->ApplyCommand("/vis/sceneHandler/create " + fViewerType);

	// Set refresh mode
	if (fPm->ParameterExists("Gr/RefreshEvery")) {
		fRefreshEvery = fPm->GetStringParameter("Gr/RefreshEvery");
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(fRefreshEvery);
#else
		fRefreshEvery.toLower();
#endif
		if (fRefreshEvery=="session") {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/scene/endOfEventAction accumulate 10000");
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/scene/endOfRunAction accumulate 10000");
		} else if (fRefreshEvery=="run") {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/scene/endOfEventAction accumulate 10000");
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/scene/endOfRunAction refresh");
		} else if (fRefreshEvery=="history") {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/scene/endOfEventAction refresh");
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/scene/endOfRunAction refresh");
		} else {
			G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
			G4cerr << "Gr/RefreshEvery has invalid value: " << fPm->GetStringParameter("Gr/RefreshEvery") << G4endl;
			G4cerr << "Should be either History, Run or Session." << G4endl;
			fPm->AbortSession(1);
		}
	}

	// Special for OpenGL and RayTracerX
	G4String windowSize = "";
	if (fViewerType.substr(0,3) == "OGL" || fViewerType == "RayTracerX") {
		G4int windowSizeX = 600;
		G4int windowSizeY = 600;
		G4int windowPosX = 0;
		G4int windowPosY = 0;
		if (fPm->ParameterExists(GetFullParmName("WindowSizeX")))
			windowSizeX = fPm->GetIntegerParameter(GetFullParmName("WindowSizeX"));
		if (fPm->ParameterExists(GetFullParmName("WindowSizeY")))
			windowSizeY = fPm->GetIntegerParameter(GetFullParmName("WindowSizeY"));
		if (fPm->ParameterExists(GetFullParmName("WindowPosX")))
			windowPosX = fPm->GetIntegerParameter(GetFullParmName("WindowPosX"));
		if (fPm->ParameterExists(GetFullParmName("WindowPosY")))
			windowPosY = fPm->GetIntegerParameter(GetFullParmName("WindowPosY"));
		windowSize = G4UIcommand::ConvertToString(windowSizeX) + "x"+ G4UIcommand::ConvertToString(windowSizeY)
		+ "-" + G4UIcommand::ConvertToString(windowPosX) + "+" + G4UIcommand::ConvertToString(windowPosY);
	}

	// Prevent OGL windows from drawing before we have a chance to increase displayListLimit
	if (fViewerType.substr(0,3) == "OGL")
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/disable");

	G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/create ! " + fViewerName + " " + windowSize);

	G4String fileName = fViewerName;
	if (fPm->ParameterExists(GetFullParmName("FileName")))
		fileName = fPm->GetStringParameter(GetFullParmName("FileName"));

	// Special for OpenGL
	if (fViewerType.substr(0,3) == "OGL") {
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/ogl/set/displayListLimit 5000000");

		if ((fPm->ParameterExists(GetFullParmName("CopyOpenGLToPDF")) && fPm->GetBooleanParameter(GetFullParmName("CopyOpenGLToPDF"))) ||
			(fPm->ParameterExists(GetFullParmName("CopyOpenGLToSVG")) && fPm->GetBooleanParameter(GetFullParmName("CopyOpenGLToSVG"))) ||
			(fPm->ParameterExists(GetFullParmName("CopyOpenGLToEPS")) && fPm->GetBooleanParameter(GetFullParmName("CopyOpenGLToEPS"))) ||
			(fPm->ParameterExists(GetFullParmName("CopyOpenGLToPS")) && fPm->GetBooleanParameter(GetFullParmName("CopyOpenGLToPS"))))
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/ogl/set/printFilename " + fileName);

		// Special for HepRep
	} else if (fViewerType=="HepRepFile") {
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/heprep/setFileName " + fileName);

		if (fPm->ParameterExists(GetFullParmName("CullInvisibles")) &&
			fPm->GetBooleanParameter(GetFullParmName("CullInvisibles")))
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/heprep/setCullInvisibles true");

		// Special for gMocren
	} else if (fViewerType=="gMocrenFile" && fPm->ParameterExists(GetFullParmName("Component"))) {
		G4String componentName = fPm->GetStringParameter(GetFullParmName("Component"));
		TsVGeometryComponent* component = fGm->GetComponent(componentName);
		if (component)
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/gMocren/setVolumeName " + componentName);
		else {
			G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
			G4cerr << "Gr/" << fViewerName << "/Component is set to unknown component: " << componentName << G4endl;
			fPm->AbortSession(1);
		}
	}

	if (fViewerType.substr(0,3) == "OGL")
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/enable");

	// Set trajectory model
	if (fIncludeTrajectories || fIncludeStepPoints) {
	    if (fUseSmoothTrajectories)
	        G4UImanager::GetUIpointer()->ApplyCommand("/vis/scene/add/trajectories smooth rich");
	    else
	        G4UImanager::GetUIpointer()->ApplyCommand("/vis/scene/add/trajectories rich");

		// Handle trajecory coloring options
		if ((fIncludeTrajectories || fIncludeStepPoints) && fPm->ParameterExists(GetFullParmName("ColorBy"))) {
			fColorModel = fPm->GetStringParameter(GetFullParmName("ColorBy"));
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(fColorModel);
#else
			fColorModel.toLower();
#endif
		}

		if (fColorModel=="charge") {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/create/drawByCharge " + fViewerName + "_" + fColorModel);

			if (fPm->ParameterExists(GetFullParmName("ColorByChargeColors"))) {
				// Colors are optional, not required, since graphics will fall back to Geant4 default drawByCharge color scheme
				if (fPm->ParameterExists(GetFullParmName("ColorByChargeColors"))) {
					G4String* chargeColors = fPm->GetStringVector(GetFullParmName("ColorByChargeColors"));
					if (fPm->GetVectorLength(GetFullParmName("ColorByChargeColors")) != 3) {
						G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
						G4cerr << GetFullParmName("ColorByChargeColors") << " must have exactly three colors." << G4endl;
						fPm->AbortSession(1);
					}

					G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/setRGBA -1 " + + GetColorAsRGBA(chargeColors[0]));
					G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/setRGBA 0 " + GetColorAsRGBA(chargeColors[1]));
					G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/setRGBA 1 " + GetColorAsRGBA(chargeColors[2]));
				}
			}
		} else if (fColorModel=="particletype") {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/create/drawByParticleID " + fViewerName + "_" + fColorModel);

			// Colors are optional, not required, since graphics will fall back to Geant4 default drawByParticleID color scheme
			if (fPm->ParameterExists(GetFullParmName("ColorByParticleTypeNames")) &&
				fPm->ParameterExists(GetFullParmName("ColorByParticleTypeColors"))) {

				// By default, seven colors were already set. Reset those to grey.
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/set gamma grey");
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/set e- grey");
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/set e+ grey");
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/set pi- grey");
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/set pi+ grey");
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/set proton grey");
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/set neutron grey");

				G4String* typeNames = fPm->GetStringVector(GetFullParmName("ColorByParticleTypeNames"));
				G4String* typeColors = fPm->GetStringVector(GetFullParmName("ColorByParticleTypeColors"));
				G4int length = fPm->GetVectorLength(GetFullParmName("ColorByParticleTypeNames"));
				if (fPm->GetVectorLength(GetFullParmName("ColorByParticleTypeColors")) != length) {
					G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
					G4cerr << "Number of tokens in " << GetFullParmName("ColorByParticleTypeNames") <<
					" does not match number of tokens in: " << GetFullParmName("ColorByParticleTypeColors") << G4endl;
					fPm->AbortSession(1);
				}
				
				std::map<G4String, G4String> molecules;
				molecules["solvatedelectron"] = "e_aq";
				molecules["hydroxil"] = "OH";
				molecules["hydroxide"] = "OHm";
				molecules["hydrogenperoxide"] = "H2O2";
				molecules["dihydrogen"] = "H_2";
				molecules["hydrogen"] = "H";
				molecules["hydronium"] = "H3O";
				molecules["superoxideanion"] = "O2m";
				molecules["hydroperoxy"] = "HO2";
				molecules["dioxidanide"] = "HO2m";

				for (G4int i = 0; i < length; i++) {
					// Standardize particle types by passing them through our helper method
#if GEANT4_VERSION_MAJOR >= 11
					G4StrUtil::to_lower(typeNames[i]);
#else
					typeNames[i].toLower();
#endif
					if ( molecules.find(typeNames[i]) != molecules.end() ) {
						G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/setRGBA " +
																  molecules[typeNames[i]] + " " + GetColorAsRGBA(typeColors[i]));
					} else {
						TsParticleDefinition resolvedDef = fPm->GetParticleDefinition(typeNames[i]);
						if (!resolvedDef.particleDefinition) {
							G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
							G4cerr << "Parameter " << GetFullParmName("ColorByParticleTypeNames") << " specifies unknown particle type: " << typeNames[i] << G4endl;
							fPm->AbortSession(1);
						}
						G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel +
																  "/setRGBA " + resolvedDef.particleDefinition->GetParticleName() + " " + GetColorAsRGBA(typeColors[i]));
					}
				}
			}
		} else if (fColorModel=="originvolume") {
			if (fPm->ParameterExists(GetFullParmName("ColorByOriginVolumeNames"))
				&& fPm->ParameterExists(GetFullParmName("ColorByOriginVolumeColors"))) {
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/create/drawByOriginVolume " + fViewerName + "_" + fColorModel);

				G4String* volumeNames = fPm->GetStringVector(GetFullParmName("ColorByOriginVolumeNames"));
				G4String* volumeColors = fPm->GetStringVector(GetFullParmName("ColorByOriginVolumeColors"));
				G4int length = fPm->GetVectorLength(GetFullParmName("ColorByOriginVolumeNames"));
				if (fPm->GetVectorLength(GetFullParmName("ColorByOriginVolumeColors")) != length) {
					G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
					G4cerr << "Number of tokens in " << GetFullParmName("ColorByOriginVolumeNames") <<
					" does not match number of tokens in: " << GetFullParmName("ColorByOriginVolumeColors") << G4endl;
					fPm->AbortSession(1);
				}

				for (G4int i = 0; i < length; i++) {
					if (fGm->GetPhysicalVolume(volumeNames[i]))
						G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/setRGBA " + volumeNames[i] + " " + GetColorAsRGBA(volumeColors[i]));
					else {
						G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
						G4cerr << "Gr/" << fViewerName << "ColorByOriginVolumeNames refers to an unknown Volume: " << volumeNames[i] << G4endl;
						fPm->AbortSession(1);
					}
				}
			} else {
				G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
				G4cerr << "Gr/" << fViewerName << " /ColorBy has been set to OriginVolume but you are missing a required" << G4endl;
				G4cerr << "associated parameter ColorByOriginVolumeNames or ColorByOriginVolumeColors." << G4endl;
				fPm->AbortSession(1);
			}
		} else if (fColorModel=="origincomponent") {
			if (fPm->ParameterExists(GetFullParmName("ColorByOriginComponentNames"))
				&& fPm->ParameterExists(GetFullParmName("ColorByOriginComponentColors"))) {
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/create/drawByOriginVolume " + fViewerName + "_" + fColorModel);

				G4String* componentNames = fPm->GetStringVector(GetFullParmName("ColorByOriginComponentNames"));
				G4String* componentColors = fPm->GetStringVector(GetFullParmName("ColorByOriginComponentColors"));
				G4int length = fPm->GetVectorLength(GetFullParmName("ColorByOriginComponentNames"));
				if (fPm->GetVectorLength(GetFullParmName("ColorByOriginComponentColors")) != length) {
					G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
					G4cerr << "Number of tokens in " << GetFullParmName("ColorByOriginComponentNames") <<
					" does not match number of tokens in: " << GetFullParmName("ColorByOriginComponentColors") << G4endl;
					fPm->AbortSession(1);
				}

				for (G4int i = 0; i < length; i++) {
					TsVGeometryComponent* originComponent = fGm->GetComponent(componentNames[i]);
					if (originComponent) {
						std::vector<G4VPhysicalVolume*> physicalVolumes = originComponent->GetAllPhysicalVolumes();
						for ( size_t iVol = 0; iVol < physicalVolumes.size(); iVol++)
							G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/setRGBA " + physicalVolumes[iVol]->GetName() + " " + GetColorAsRGBA(componentColors[i]));
					} else {
						G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
						G4cerr << "Gr/" << fViewerName << "ColorByOriginComponentNames refers to an unknown Component: " << componentNames[i] << G4endl;
						fPm->AbortSession(1);
					}
				}
			} else {
				G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
				G4cerr << "Gr/" << fViewerName << "/ColorBy has been set to OriginComponent but you are missing a required" << G4endl;
				G4cerr << "associated parameter ColorByOriginComponentNames or ColorByOriginComponentColors." << G4endl;
				fPm->AbortSession(1);
			}
		} else if (fColorModel=="origincomponentorsubcomponentof") {
			if (fPm->ParameterExists(GetFullParmName("ColorByOriginComponentNames"))
				&& fPm->ParameterExists(GetFullParmName("ColorByOriginComponentColors"))) {
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/create/drawByOriginVolume " + fViewerName + "_" + fColorModel);

				G4String* componentNames = fPm->GetStringVector(GetFullParmName("ColorByOriginComponentNames"));
				G4String* componentColors = fPm->GetStringVector(GetFullParmName("ColorByOriginComponentColors"));
				G4int length = fPm->GetVectorLength(GetFullParmName("ColorByOriginComponentNames"));
				if (fPm->GetVectorLength(GetFullParmName("ColorByOriginComponentColors")) != length) {
					G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
					G4cerr << "Number of tokens in " << GetFullParmName("ColorByOriginComponentNames") <<
					" does not match number of tokens in: " << GetFullParmName("ColorByOriginComponentColors") << G4endl;
					fPm->AbortSession(1);
				}

				for (G4int i = 0; i < length; i++) {
					TsVGeometryComponent* originComponent = fGm->GetComponent(componentNames[i]);
					if (originComponent) {
						// Insert all volumes of Component
						std::vector<G4VPhysicalVolume*> physicalVolumes = originComponent->GetAllPhysicalVolumes();
						for ( size_t iVol = 0; iVol < physicalVolumes.size(); iVol++)
							G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/setRGBA " + physicalVolumes[iVol]->GetName() + " " + GetColorAsRGBA(componentColors[i]));

						// Insert all volumes of all child Components
						std::vector<G4String> childNames = fGm->GetChildComponentsOf(componentNames[i]);
						std::vector<G4String>::iterator iter;
						for (iter = childNames.begin(); iter!=childNames.end(); iter++) {
							G4String childName = *iter;
							TsVGeometryComponent* childComponent = fGm->GetComponent(childName);
							if (childComponent) {
								std::vector<G4VPhysicalVolume*> childVolumes = childComponent->GetAllPhysicalVolumes();
								for ( size_t iVol = 0; iVol < childVolumes.size(); iVol++)
									G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/setRGBA " + childVolumes[iVol]->GetName() + " " + GetColorAsRGBA(componentColors[i]));
							}
						}
					} else {
						G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
						G4cerr << "Gr/" << fViewerName << "ColorByOriginComponentNames refers to an unknown Component: " << componentNames[i] << G4endl;
						fPm->AbortSession(1);
					}
				}
			} else {
				G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
				G4cerr << "Gr/" << fViewerName << "/ColorBy has been set to OriginComponentOrSubComonentOf but you are missing a required" << G4endl;
				G4cerr << "associated parameter ColorByOriginComponentNames or ColorByOriginComponentColors." << G4endl;
				fPm->AbortSession(1);
			}
		} else if (fColorModel=="energy") {
			if (fPm->ParameterExists(GetFullParmName("ColorByEnergyRanges"))
				&& fPm->ParameterExists(GetFullParmName("ColorByEnergyColors"))) {
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/create/drawByAttribute " + fViewerName + "_" + fColorModel);
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/setAttribute IKE");

				G4double* energyValues = fPm->GetDoubleVector(GetFullParmName("ColorByEnergyRanges"), "Energy");
				G4String* energyColors = fPm->GetStringVector(GetFullParmName("ColorByEnergyColors"));
				G4int length = fPm->GetVectorLength(GetFullParmName("ColorByEnergyRanges"));
				if (fPm->GetVectorLength(GetFullParmName("ColorByEnergyColors")) != length+1) {
					G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
					G4cerr << "Number of tokens in " << GetFullParmName("ColorByEnergyRanges") <<
					" needs to be one less than number of tokens in: " << GetFullParmName("ColorByEnergyColors") << G4endl;
					fPm->AbortSession(1);
				}

				G4double minValue = 0.;
				G4double maxValue;
				for (G4int i = 0; i < length+1; i++) {
					if (i < length)
						maxValue = energyValues[i];
					else
						maxValue = 10000.;

					G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/addInterval interval_" + G4UIcommand::ConvertToString(i) +
															  " " + G4UIcommand::ConvertToString(minValue) + " MeV " + G4UIcommand::ConvertToString(maxValue) + " MeV");
					G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/interval_" + G4UIcommand::ConvertToString(i) + "/setLineColourRGBA " + GetColorAsRGBA(energyColors[i]));

					if (fIncludeStepPoints) {
						G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/interval_" + G4UIcommand::ConvertToString(i) + "/setDrawStepPts true " );
						G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/interval_" + G4UIcommand::ConvertToString(i) + "/setDrawAuxPts true " );
					}

					if (!fIncludeTrajectories)
						G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/interval_" + G4UIcommand::ConvertToString(i) + "/setDrawLine false " );

					minValue = maxValue;
				}
			} else {
				G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
				G4cerr << "Gr/" << fViewerName << "/ColorBy has been set to Energy but you are missing a required" << G4endl;
				G4cerr << "associated parameter ColorByEnergyRanges or ColorByEnergyColors." << G4endl;
				fPm->AbortSession(1);
			}
		} else if (fColorModel=="momentum") {
			if (fPm->ParameterExists(GetFullParmName("ColorByMomentumRanges"))
				&& fPm->ParameterExists(GetFullParmName("ColorByMomentumColors"))) {
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/create/drawByAttribute " + fViewerName + "_" + fColorModel);
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/setAttribute IMag");

				G4double* momentumValues = fPm->GetDoubleVector(GetFullParmName("ColorByMomentumRanges"), "Energy");
				G4String* momentumColors = fPm->GetStringVector(GetFullParmName("ColorByMomentumColors"));
				G4int length = fPm->GetVectorLength(GetFullParmName("ColorByMomentumRanges"));
				if (fPm->GetVectorLength(GetFullParmName("ColorByMomentumColors")) != length+1) {
					G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
					G4cerr << "Number of tokens in " << GetFullParmName("ColorByMomentumRanges") <<
					" needs to be one less than number of tokens in: " << GetFullParmName("ColorByMomentumColors") << G4endl;
					fPm->AbortSession(1);
				}

				G4double minValue = 0.;
				G4double maxValue;
				for (G4int i = 0; i < length+1; i++) {
					if (i < length)
						maxValue = momentumValues[i];
					else
						maxValue = 10000.;

					G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/addInterval interval_" + G4UIcommand::ConvertToString(i) +
															  " " + G4UIcommand::ConvertToString(minValue) + " MeV " + G4UIcommand::ConvertToString(maxValue) + " MeV");
					G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/interval_" + G4UIcommand::ConvertToString(i) + "/setLineColourRGBA " + GetColorAsRGBA(momentumColors[i]));

					if (fIncludeStepPoints) {
						G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/interval_" + G4UIcommand::ConvertToString(i) + "/setDrawStepPts true " );
						G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/interval_" + G4UIcommand::ConvertToString(i) + "/setDrawAuxPts true " );
					}

					if (!fIncludeTrajectories)
						G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/interval_" + G4UIcommand::ConvertToString(i) + "/setDrawLine false " );

					minValue = maxValue;
				}
			} else {
				G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
				G4cerr << "Gr/" << fViewerName << "/ColorBy has been set to Momentum but you are missing a required" << G4endl;
				G4cerr << "associated parameter ColorByMomentumRanges or ColorByMomentumColors." << G4endl;
				fPm->AbortSession(1);
			}
		} else if (fColorModel=="generation") {
			if (fPm->ParameterExists(GetFullParmName("ColorByGenerationColors"))) {
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/create/drawByAttribute " + fViewerName + "_" + fColorModel);
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/setAttribute PID");
				G4String* generationColors = fPm->GetStringVector(GetFullParmName("ColorByGenerationColors"));
				if (fPm->GetVectorLength(GetFullParmName("ColorByGenerationColors")) != 2) {
					G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
					G4cerr << GetFullParmName("ColorByGenerationColors") << " must have exactly two colors." << G4endl;
					fPm->AbortSession(1);
				}

				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/addValue Primary_key 0");
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/Primary_key/setLineColourRGBA " + GetColorAsRGBA(generationColors[0]));

				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/default/setLineColourRGBA " + GetColorAsRGBA(generationColors[1]));
			} else {
				G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
				G4cerr << "Gr/" << fViewerName << "/ColorBy has been set to Generation but you are missing the required" << G4endl;
				G4cerr << "associated parameter ColorByGenerationColors." << G4endl;
				fPm->AbortSession(1);
			}
		} else if (fColorModel=="creatorprocess") {
			if (fPm->ParameterExists(GetFullParmName("ColorByCreatorProcessNames"))
				&& fPm->ParameterExists(GetFullParmName("ColorByCreatorProcessColors"))) {
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/create/drawByAttribute " + fViewerName + "_" + fColorModel);
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/setAttribute CPN");

				G4String* processNames = fPm->GetStringVector(GetFullParmName("ColorByCreatorProcessNames"));
				G4String* processColors = fPm->GetStringVector(GetFullParmName("ColorByCreatorProcessColors"));
				G4int length = fPm->GetVectorLength(GetFullParmName("ColorByCreatorProcessNames"));
				if (fPm->GetVectorLength(GetFullParmName("ColorByCreatorProcessColors")) != length) {
					G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
					G4cerr << "Number of tokens in " << GetFullParmName("ColorByCreatorProcessNames") <<
					" does not match number of tokens in: " << GetFullParmName("ColorByCreatorProcessColors") << G4endl;
					fPm->AbortSession(1);
				}

				G4String processName;
				G4String processNameLower;
				for (G4int i = 0; i < length; i++) {
					processName = processNames[i];
					processNameLower = processName;
#if GEANT4_VERSION_MAJOR >= 11
					G4StrUtil::to_lower(processNameLower);
#else
					processNameLower.toLower();
#endif
					if (processNameLower=="primary") processName = "None";
					G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/addValue " + processName + "_key " + processName);
					G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/" + processName + "_key/setLineColourRGBA " + GetColorAsRGBA(processColors[i]));

					if (fIncludeStepPoints) {
						G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/" + processName + "_key/setDrawStepPts true " );
						G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/" + processName + "_key/setDrawAuxPts true " );
					}

					if (!fIncludeTrajectories)
						G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/" + processName + "_key/setDrawLine false " );

				}
			} else {
				G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
				G4cerr << "Gr/" << fViewerName << "/ColorBy has been set to CreatorProcess but you are missing a required" << G4endl;
				G4cerr << "associated parameter ColorByCreatorProcessNames or ColorByCreatorProcessColors." << G4endl;
				fPm->AbortSession(1);
			}
		} else {
			G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
			G4cerr << "Gr/" << fViewerName << "/ColorBy is set to unknown mode." << G4endl;
			fPm->AbortSession(1);
		}

		// Handle trajectory line width
		if (fPm->ParameterExists(GetFullParmName("TrajectoryLinewidth"))) {
			G4int trajectoryLineWidth = fPm->GetIntegerParameter(GetFullParmName("TrajectoryLinewidth"));
			if (trajectoryLineWidth < 1) {
				G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
				G4cerr << "Gr/" << fViewerName << "/TrajectoryLinewidth must be greater than 0." << G4endl;
				fPm->AbortSession(1);
			}
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/default/setLineWidth " + G4UIcommand::ConvertToString(trajectoryLineWidth));
		}

		// Handle trajecory filtering options
		if (fPm->ParameterExists("Gr/OnlyIncludeParticlesNamed")) {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/create/particleFilter " + fViewerName + "_typeFilter");

			G4String* tokens = fPm->GetStringVector("Gr/OnlyIncludeParticlesNamed");
			G4int length = fPm->GetVectorLength("Gr/OnlyIncludeParticlesNamed");

			G4IonTable* theIonTable = G4IonTable::GetIonTable();
			for (G4int i = 0; i < length; i++) {
				TsParticleDefinition resolvedDef = fPm->GetParticleDefinition(tokens[i]);

				if (!resolvedDef.particleDefinition) {
					G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
					G4cerr << "Gr/OnlyIncludeParticlesNamed has unknown particle name: " << tokens[i] << G4endl;
					fPm->AbortSession(1);
				}

				if (resolvedDef.isGenericIon)
					tokens[i] = theIonTable->GetIonName(resolvedDef.ionZ, resolvedDef.ionA);

				G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/" + fViewerName + "_typeFilter/add " + tokens[i]);
			}
		}

		if (fPm->ParameterExists("Gr/OnlyIncludeParticlesCharged")) {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/create/chargeFilter " + fViewerName + "_chargeFilter");

			G4String* values = fPm->GetStringVector("Gr/OnlyIncludeParticlesCharged");
			G4int length = fPm->GetVectorLength("Gr/OnlyIncludeParticlesCharged");

			for (G4int i = 0; i < length; i++) {
#if GEANT4_VERSION_MAJOR >= 11
				G4StrUtil::to_lower(values[i]);
#else
				values[i].toLower();
#endif
				if (values[i] == "negative")
					G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/" + fViewerName + "_chargeFilter/add -1");
				if (values[i] == "neutral")
					G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/" + fViewerName + "_chargeFilter/add 0");
				if (values[i] == "positive")
					G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/" + fViewerName + "_chargeFilter/add 1");
			}
		}

		if (fPm->ParameterExists("Gr/OnlyIncludeParticlesFromVolume")) {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/create/originVolumeFilter " + fViewerName + "_volumeFilter");

			G4String* tokens = fPm->GetStringVector("Gr/OnlyIncludeParticlesFromVolume");
			G4int length = fPm->GetVectorLength("Gr/OnlyIncludeParticlesFromVolume");

			for (G4int i = 0; i < length; i++) {
				if (fGm->GetPhysicalVolume(tokens[i]))
					G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/" + fViewerName + "_volumeFilter/add " + tokens[i]);
				else {
					G4cerr << "Topas is exiting due to a serious error in graphics setup." << G4endl;
					G4cerr << "OnlyIncludeParticlesFromVolume = " << tokens[i] << " refers to an unknown Volume." << G4endl;
					fPm->AbortSession(1);
				}
			}
		}

		if (fPm->ParameterExists("Gr/OnlyIncludeParticlesFromComponent")) {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/create/originVolumeFilter " + fViewerName + "_componentFilter");

			G4String* tokens = fPm->GetStringVector("Gr/OnlyIncludeParticlesFromComponent");
			G4int length = fPm->GetVectorLength("Gr/OnlyIncludeParticlesFromComponent");

			for (G4int i = 0; i < length; i++) {
				TsVGeometryComponent* originComponent = fGm->GetComponent(tokens[i]);
				if (originComponent) {
					std::vector<G4VPhysicalVolume*> physicalVolumes = originComponent->GetAllPhysicalVolumes();
					for ( size_t iVol = 0; iVol < physicalVolumes.size(); iVol++)
						G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/" + fViewerName + "_componentFilter/add " + physicalVolumes[iVol]->GetName());
				} else {
					G4cerr << "Topas is exiting due to a serious error in graphics setup." << G4endl;
					G4cerr << "OnlyIncludeParticlesFromComponent = " << tokens[i] << " refers to an unknown Component." << G4endl;
					fPm->AbortSession(1);
				}
			}
		}

		if (fPm->ParameterExists("Gr/OnlyIncludeParticlesFromComponentOrSubComponentsOf")) {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/create/originVolumeFilter " + fViewerName + "_componentsFilter");

			G4String* tokens = fPm->GetStringVector("Gr/OnlyIncludeParticlesFromComponentOrSubComponentsOf");
			G4int length = fPm->GetVectorLength("Gr/OnlyIncludeParticlesFromComponentOrSubComponentsOf");

			for (G4int i = 0; i < length; i++) {
				TsVGeometryComponent* originComponent = fGm->GetComponent(tokens[i]);
				if (originComponent) {
					// Insert all volumes of Component
					std::vector<G4VPhysicalVolume*> physicalVolumes = originComponent->GetAllPhysicalVolumes();
					for ( size_t iVol = 0; iVol < physicalVolumes.size(); iVol++)
						G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/" + fViewerName + "_componentsFilter/add " + physicalVolumes[iVol]->GetName());

					// Insert all volumes of all child Components
					std::vector<G4String> childNames = fGm->GetChildComponentsOf(tokens[i]);
					std::vector<G4String>::iterator iter;
					for (iter = childNames.begin(); iter!=childNames.end(); iter++) {
						G4String childName = *iter;
						TsVGeometryComponent* childComponent = fGm->GetComponent(childName);
						if (childComponent) {
							std::vector<G4VPhysicalVolume*> childVolumes = childComponent->GetAllPhysicalVolumes();
							for ( size_t iVol = 0; iVol < childVolumes.size(); iVol++)
								G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/" + fViewerName + "_componentsFilter/add " + childVolumes[iVol]->GetName());
						}
					}
				} else {
					G4cerr << "Topas is exiting due to a serious error in graphics setup." << G4endl;
					G4cerr << "OnlyIncludeParticlesFromComponentOrSubComponentsOf = " << tokens[i] << " refers to an unknown Component." << G4endl;
					fPm->AbortSession(1);
				}
			}
		}

		if (fPm->ParameterExists("Gr/OnlyIncludeParticlesWithInitialKEAbove") ||
			fPm->ParameterExists("Gr/OnlyIncludeParticlesWithInitialKEBelow")) {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/create/attributeFilter " + fViewerName + "_energyFilter");
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/" + fViewerName + "_energyFilter/setAttribute IKE");

			G4double minValue = 0.;
			if (fPm->ParameterExists("Gr/OnlyIncludeParticlesWithInitialKEAbove"))
				minValue = fPm->GetDoubleParameter("Gr/OnlyIncludeParticlesWithInitialKEAbove", "Energy");

			G4double maxValue = 10000.;
			if (fPm->ParameterExists("Gr/OnlyIncludeParticlesWithInitialKEBelow"))
				maxValue = fPm->GetDoubleParameter("Gr/OnlyIncludeParticlesWithInitialKEBelow", "Energy");

			G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/" + fViewerName + "_energyFilter/addInterval "
													  + G4UIcommand::ConvertToString(minValue) + " MeV " + G4UIcommand::ConvertToString(maxValue) + " MeV");
		}

		if (fPm->ParameterExists("Gr/OnlyIncludeParticlesWithInitialMomentumAbove") ||
			fPm->ParameterExists("Gr/OnlyIncludeParticlesWithInitialMomentumBelow")) {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/create/attributeFilter " + fViewerName + "_momFilter");
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/" + fViewerName + "_momFilter/setAttribute IMag");

			G4double minValue = 0.;
			if (fPm->ParameterExists("Gr/OnlyIncludeParticlesWithInitialMomentumAbove"))
				minValue = fPm->GetDoubleParameter("Gr/OnlyIncludeParticlesWithInitialMomentumAbove", "Energy");

			G4double maxValue = 10000.;
			if (fPm->ParameterExists("Gr/OnlyIncludeParticlesWithInitialMomentumBelow"))
				maxValue = fPm->GetDoubleParameter("Gr/OnlyIncludeParticlesWithInitialMomentumBelow", "Energy");

			G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/" + fViewerName + "_momFilter/addInterval "
													  + G4UIcommand::ConvertToString(minValue) + " MeV " + G4UIcommand::ConvertToString(maxValue) + " MeV");
		}

		if (fPm->ParameterExists("Gr/OnlyIncludeParticlesFromProcess")) {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/create/attributeFilter " + fViewerName + "_cprFilter");
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/" + fViewerName + "_cprFilter/setAttribute CPN");

			G4String* tokens = fPm->GetStringVector("Gr/OnlyIncludeParticlesFromProcess");
			G4int length = fPm->GetVectorLength("Gr/OnlyIncludeParticlesFromProcess");

			G4String processName;
			G4String processNameLower;
			for (G4int i = 0; i < length; i++) {
				processName = tokens[i];
				processNameLower = processName;
#if GEANT4_VERSION_MAJOR >= 11
				G4StrUtil::to_lower(processNameLower);
#else
				processNameLower.toLower();
#endif
				if (processNameLower=="primary") processName = "None";
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/filtering/trajectories/" + fViewerName + "_cprFilter/addValue " + processName);
			}
		}

		if (!fIncludeTrajectories) {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/default/setDrawLine false " );
		}

		if (fIncludeStepPoints) {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/default/setDrawStepPts true " );
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/default/setDrawAuxPts true " );
		}
	}

	SetView();

	if (fViewerType.substr(0,3) != "OGL")
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/flush");
}


void TsGraphicsView::UpdateForSpecificParameterChange(G4String parameter) {
	if (fVerbosity>0)
		G4cout << "TsGraphicsView::UpdateForSpecificParameterChange called for parameter: " << parameter << G4endl;

#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(parameter);
#else
	parameter.toLower();
#endif

	if (parameter==GetFullParmNameLower("Active")) {
		fIsActive = fPm->GetBooleanParameter(GetFullParmName("Active"));
		if (fIsActive && !fAlreadyCreated)
			CreateView();
	} else if (parameter==GetFullParmNameLower("IncludeTrajectories")) {
		fIncludeTrajectories = fPm->GetBooleanParameter(GetFullParmName("IncludeTrajectories"));
		if (fIncludeTrajectories)
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/default/setDrawLine true " );
		else
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/default/setDrawLine false " );
	} else if (parameter==GetFullParmNameLower("IncludeStepPoints")) {
		fIncludeStepPoints = fPm->GetBooleanParameter(GetFullParmName("IncludeStepPoints"));
		if (fIncludeStepPoints) {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/default/setDrawStepPts true " );
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/default/setDrawAuxPts true " );
		} else {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/default/setDrawStepPts false " );
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/default/setDrawAuxPts false " );
		}
	} else if (parameter==GetFullParmNameLower("TrajectoryLinewidth")) {
		G4int trajectoryLineWidth = fPm->GetIntegerParameter(GetFullParmName("TrajectoryLinewidth"));
		if (trajectoryLineWidth < 1) {
			G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
			G4cerr << "Gr/" << fViewerName << "/TrajectoryLinewidth must be greater than 0." << G4endl;
			fPm->AbortSession(1);
		}
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/default/setLineWidth " + G4UIcommand::ConvertToString(trajectoryLineWidth));
	}

	// We will update all other parameters at new run if any graphics parameter has changed
	fHadParameterChangeSinceLastRun = true;
}


void TsGraphicsView::UpdateForNewRun(G4bool rebuiltSomeComponents) {
	if (fVerbosity>0)
		G4cout << "TsGraphicsView::UpdateForNewRun for view: " << GetName() << " called with fHadParameterChangeSinceLastRun: " <<
		fHadParameterChangeSinceLastRun << ", rebuiltSomeComponents: " << rebuiltSomeComponents << G4endl;

	if (fIsActive) {
		if (fHadParameterChangeSinceLastRun)
			ResolveParameters();
		else if (rebuiltSomeComponents)
			CacheGeometryPointers();

			// Since changed parameter might affect the view, update the view now
			if (fHadParameterChangeSinceLastRun) {
				G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/select " + fViewerName);
				SetView();
			}

			// Cause any geometry updates to appear on the screen
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/scene/notifyHandlers " + fViewerName);

		fHadParameterChangeSinceLastRun = false;
	}
}


void TsGraphicsView::UpdateForEndOfRun() {
	if (fVerbosity>0)
		G4cout << "TsGraphicsView::UpdateForEndOfRun for view: " << GetName() << G4endl;

	if (fIsActive) {
		if (fRefreshEvery=="run")
			SaveToHardCopyIfRequested();

		if (fPm->ParameterExists(GetFullParmName("ParticleFlightNumberOfFrames"))) {
			G4int nFrames = fPm->GetIntegerParameter(GetFullParmName("ParticleFlightNumberOfFrames"));
			if (nFrames != 0) {
				if (nFrames < 0) {
					G4cerr << GetFullParmName("ParticleFlightNumberOfFrames") << " may not be less than 0" << G4endl;
					fPm->AbortSession(1);
				}

				G4double timeStart = 0.;
				if (fPm->ParameterExists(GetFullParmName("ParticleFlightTimeStart"))) {
					timeStart = fPm->GetDoubleParameter(GetFullParmName("ParticleFlightTimeStart"), "Time");
					if (timeStart < 0.) {
						G4cerr << GetFullParmName("ParticleFlightTimeStart") << " may not be less than 0." << G4endl;
						fPm->AbortSession(1);
					}
				}

				G4double timeEnd = 60. * ns;
				if (fPm->ParameterExists(GetFullParmName("ParticleFlightTimeEnd"))) {
					timeEnd = fPm->GetDoubleParameter(GetFullParmName("ParticleFlightTimeEnd"), "Time");
				} else {
					G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
					G4cerr << GetFullParmName("ParticleFlightNumberOfFrames") << " has been set without setting" << G4endl;
					G4cerr << GetFullParmName("ParticleFlightTimeEnd") << G4endl;
					fPm->AbortSession(1);
				}

				G4int framesPerSecond = 24;
				if (fPm->ParameterExists(GetFullParmName("ParticleFlightFramesPerSecond"))) {
					framesPerSecond = fPm->GetIntegerParameter(GetFullParmName("ParticleFlightFramesPerSecond"));
					if (framesPerSecond <= 0) {
						G4cerr << GetFullParmName("ParticleFlightFramesPerSecond") << " must be greater than 0" << G4endl;
						fPm->AbortSession(1);
					}
				}

				G4double segmentFactor = 1.;
				if (fPm->ParameterExists(GetFullParmName("ParticleFlightSegmentFactor"))) {
					segmentFactor = fPm->GetUnitlessParameter(GetFullParmName("ParticleFlightSegmentFactor"));
					if (segmentFactor < 0.) {
						G4cerr << GetFullParmName("ParticleFlightSegmentFactor") << " must be greater than 0." << G4endl;
						fPm->AbortSession(1);
					}
				}

				G4double fadeFactor = 1.;
				if (fPm->ParameterExists(GetFullParmName("ParticleFlightFadeFactor"))) {
					fadeFactor = fPm->GetUnitlessParameter(GetFullParmName("ParticleFlightFadeFactor"));
					if (fadeFactor < 0. || fadeFactor > 1.) {
						G4cerr << GetFullParmName("ParticleFlightFadeFactor") << " must be between 0. and 1." << G4endl;
						fPm->AbortSession(1);
					}
				}

				G4double sliceInterval = 0.01 * ns;
				if (fPm->ParameterExists(GetFullParmName("ParticleFlightSliceInterval"))) {
					sliceInterval = fPm->GetDoubleParameter(GetFullParmName("ParticleFlightSliceInterval"), "Time");
					if (sliceInterval < 0.) {
						G4cerr << GetFullParmName("ParticleFlightSliceInterval") << " must be greater than 0." << G4endl;
						fPm->AbortSession(1);
					}
				}

				G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/" + fViewerName + "_" + fColorModel + "/default/setTimeSliceInterval " + G4UIcommand::ConvertToString(sliceInterval) + " ns");

				G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/timeWindow/fadeFactor "
					+ G4UIcommand::ConvertToString(fadeFactor));

				G4double timeInterval = (timeEnd - timeStart) / nFrames;

				for (G4int iFrame=0; iFrame < nFrames; iFrame++) {
					G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/timeWindow/startTime "
						+ G4UIcommand::ConvertToString(timeStart + (iFrame * timeInterval)) + " ns "
															  + G4UIcommand::ConvertToString(segmentFactor * timeInterval) + " ns");

					usleep(1000000. / framesPerSecond);
				}

				G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/timeWindow/startTime 0. ns 36000. s");
			}
		}
	}
}

void TsGraphicsView::UpdateForEndOfSession() {
	if (fVerbosity>0)
		G4cout << "TsGraphicsView::UpdateForEndOfSession for view: " << GetName() << G4endl;

	if (fIsActive && fRefreshEvery=="session")
		SaveToHardCopyIfRequested();
}


void TsGraphicsView::SaveToHardCopyIfRequested() {
	// Need this to make sure each view gets the modeling that was specified for that view above
	if (fIncludeTrajectories || fIncludeStepPoints )
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/modeling/trajectories/select " + fViewerName + "_" + fColorModel);

	if (fViewerType.substr(0,3) == "OGL") {
		// Without this, only the last view will get updated
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/scene/notifyHandlers " + fViewerName);

		if (fPm->ParameterExists(GetFullParmName("CopyOpenGLToPDF")) && fPm->GetBooleanParameter(GetFullParmName("CopyOpenGLToPDF"))) {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/select " + fViewerName);
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/ogl/set/exportFormat pdf");
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/ogl/export");
		}

		if (fPm->ParameterExists(GetFullParmName("CopyOpenGLToSVG")) && fPm->GetBooleanParameter(GetFullParmName("CopyOpenGLToSVG"))) {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/select " + fViewerName);
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/ogl/set/exportFormat svg");
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/ogl/export");
		}

		if (fPm->ParameterExists(GetFullParmName("CopyOpenGLToEPS")) && fPm->GetBooleanParameter(GetFullParmName("CopyOpenGLToEPS"))) {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/select " + fViewerName);
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/ogl/set/exportFormat eps");
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/ogl/export");
		}

		if (fPm->ParameterExists(GetFullParmName("CopyOpenGLToPS")) && fPm->GetBooleanParameter(GetFullParmName("CopyOpenGLToPS"))) {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/select " + fViewerName);
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/ogl/set/exportFormat ps");
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/ogl/export");
		}
	} else {
		// HepRepFile already does the flush itself
		if (fViewerType != "HepRepFile")
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/flush " + fViewerName);
	}
}


void TsGraphicsView::ResolveParameters() {
	if (fVerbosity>0)
		G4cout << "TsGraphicsView::ResolveParameters" << G4endl;
	CacheGeometryPointers();
}


void TsGraphicsView::CacheGeometryPointers() {
	if (fVerbosity>0)
		G4cout << "TsGraphicsView::CacheGeometryPointers" << G4endl;
}


// Note order of operations is important throughout SetView
void TsGraphicsView::SetView() {
	G4UImanager::GetUIpointer()->ApplyCommand("/vis/disable");

	if (fPm->ParameterExists(GetFullParmName("HiddenLineRemovalForGeometry")) &&
		fPm->GetBooleanParameter(GetFullParmName("HiddenLineRemovalForGeometry")))
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/hiddenEdge 1");

	if (fPm->ParameterExists(GetFullParmName("HiddenLineRemovalForTrajectories"))) {
		if (fPm->GetBooleanParameter(GetFullParmName("HiddenLineRemovalForTrajectories")))
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/hiddenMarker 1");
		else
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/hiddenMarker 0");
	}

	if (fPm->ParameterExists(GetFullParmName("BackgroundColor")))
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/background " +
			GetColorAsRGBA(fPm->GetStringParameter(GetFullParmName("BackgroundColor"))));

	if (fPm->ParameterExists(GetFullParmName("LineWidth")))
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/globalLineWidthScale " +
			G4UIcommand::ConvertToString(fPm->GetIntegerParameter(GetFullParmName("LineWidth"))));
	else
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/globalLineWidthScale 2");

	if (fPm->ParameterExists(GetFullParmName("StepPointSize")))
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/globalMarkerScale " +
			G4UIcommand::ConvertToString(fPm->GetIntegerParameter(GetFullParmName("StepPointSize"))/2));
	else
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/globalMarkerScale 2");

	if (fPm->ParameterExists(GetFullParmName("Projection"))) {
		G4String projection = fPm->GetStringParameter(GetFullParmName("Projection"));
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(projection);
#else
		projection.toLower();
#endif
		if (projection == "perspective") {
			G4double perspectiveAngle = 30.;
			if (fPm->ParameterExists(GetFullParmName("PerspectiveAngle")))
				perspectiveAngle = fPm->GetDoubleParameter(GetFullParmName("PerspectiveAngle"), "Angle") / deg;
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/projection " + projection + " " + G4UIcommand::ConvertToString(perspectiveAngle));
		} else if (projection == "orthogonal") {
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/projection " + projection);
		} else {
			G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
			G4cerr << "Gr/" << fViewerName << "/Projection has been set to an unknown value." << G4endl;
			G4cerr << "Should be either Perspective or Orthogonal." << G4endl;
			fPm->AbortSession(1);
		}
	}

	if (fPm->ParameterExists(GetFullParmName("TargetPointX"))) {
		G4double targetPosX = fPm->GetDoubleParameter(GetFullParmName("TargetPointX"), "Length");
		G4double targetPosY = fPm->GetDoubleParameter(GetFullParmName("TargetPointY"), "Length");
		G4double targetPosZ = fPm->GetDoubleParameter(GetFullParmName("TargetPointZ"), "Length");

		G4String targetCommand = "/vis/viewer/set/targetPoint " +
		G4UIcommand::ConvertToString(targetPosX) + " " + G4UIcommand::ConvertToString(targetPosY) + " " +
		G4UIcommand::ConvertToString(targetPosZ) + " mm";

		G4UImanager::GetUIpointer()->ApplyCommand(targetCommand);
	} else if (fPm->ParameterExists(GetFullParmName("CenterOn"))) {
		G4String targetComponentName = fPm->GetStringParameter(GetFullParmName("CenterOn"));
		TsVGeometryComponent* targetComponent = fGm->GetComponent(targetComponentName);
		if (targetComponent) {
			G4Point3D* targetCenter = targetComponent->GetTransRelToWorld();
			G4double targetPosX = targetCenter->x();
			G4double targetPosY = targetCenter->y();
			G4double targetPosZ = targetCenter->z();

			G4String targetCommand = "/vis/viewer/set/targetPoint " +
			G4UIcommand::ConvertToString(targetPosX) + " " + G4UIcommand::ConvertToString(targetPosY) + " " +
			G4UIcommand::ConvertToString(targetPosZ) + " mm";

			G4UImanager::GetUIpointer()->ApplyCommand(targetCommand);
		} else {
			G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
			G4cerr << "Gr/" << fViewerName << "/CenterOn is set to unknown component: " << targetComponentName << G4endl;
			fPm->AbortSession(1);
		}
	} else {
		G4double transX = 0.;
		G4double transY = 0.;
		if (fPm->ParameterExists(GetFullParmName("TransX")))
			transX = fPm->GetUnitlessParameter(GetFullParmName("TransX"));
		if (fPm->ParameterExists(GetFullParmName("TransY")))
			transY = fPm->GetUnitlessParameter(GetFullParmName("TransY"));
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/panTo " + G4UIcommand::ConvertToString(transX) + " " + G4UIcommand::ConvertToString(transY));
	}

	G4double theta = 90.;
	G4double phi = 0.;
	if (fPm->ParameterExists(GetFullParmName("Theta")))
		theta = fPm->GetDoubleParameter(GetFullParmName("Theta"), "Angle") / deg;
	if (fPm->ParameterExists(GetFullParmName("Phi")))
		phi = fPm->GetDoubleParameter(GetFullParmName("Phi"), "Angle") / deg;
	G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/viewpointThetaPhi " + G4UIcommand::ConvertToString(theta) + " " + G4UIcommand::ConvertToString(phi));

	if (fPm->ParameterExists(GetFullParmName("Scale"))) {
		if (fPm->GetVectorLength(GetFullParmName("Scale")) != 3) {
			G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
			G4cerr << "Gr/" << fViewerName << "/Scale has wrong number of value. Must have 3 values." << G4endl;
			fPm->AbortSession(1);
		}

		G4double* scaleVector = fPm->GetUnitlessVector(GetFullParmName("Scale"));
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/scaleTo " +
			G4UIcommand::ConvertToString(scaleVector[0]) + " " +
			G4UIcommand::ConvertToString(scaleVector[1]) + " " +
			G4UIcommand::ConvertToString(scaleVector[2]));
	}

	G4double zoom = 1.;

	G4String zoomToFitComponentName = "";
	if (fPm->ParameterExists(GetFullParmName("ZoomToFit")))
		zoomToFitComponentName = fPm->GetStringParameter(GetFullParmName("ZoomToFit"));

	if (zoomToFitComponentName != "") {
		TsVGeometryComponent* zoomToFitComponent = fGm->GetComponent(zoomToFitComponentName);
		if (zoomToFitComponent) {
			zoom = zoomToFitComponent->GetExtent().GetExtentRadius() * .7;
		} else {
			G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
			G4cerr << "Gr/" << fViewerName << "/ZoomToFit has been set to a non-existant component name: " << zoomToFitComponentName << G4endl;
			fPm->AbortSession(1);
		}
	} else if (fPm->ParameterExists(GetFullParmName("Zoom"))) {
		zoom = fPm->GetUnitlessParameter(GetFullParmName("Zoom"));
	}

	G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/zoomTo " + G4UIcommand::ConvertToString(zoom));

	if (fPm->ParameterExists(GetFullParmName("UpVector"))) {
		if (fPm->GetVectorLength(GetFullParmName("UpVector")) != 3) {
			G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
			G4cerr << "Gr/" << fViewerName << "/UpVector has wrong length. Must be 3." << G4endl;
			fPm->AbortSession(1);
		}

		G4double* upVector = fPm->GetUnitlessVector(GetFullParmName("UpVector"));
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/upVector " +
												  G4UIcommand::ConvertToString(upVector[0]) + " " +
												  G4UIcommand::ConvertToString(upVector[1]) + " " +
												  G4UIcommand::ConvertToString(upVector[2]));
	}

	if (fPm->ParameterExists(GetFullParmName("RotationStyle"))) {
		G4String rotationStyle = fPm->GetStringParameter(GetFullParmName("RotationStyle"));
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(rotationStyle);
#else
		rotationStyle.toLower();
#endif
		if (rotationStyle == "constrained")
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/rotationStyle constrainUpDirection");
		else if (rotationStyle == "free")
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/rotationStyle freeRotation");
		else {
			G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
			G4cerr << "Gr/" << fViewerName << "/RotationStyle has invalid value: " <<  fPm->GetStringParameter(GetFullParmName("RotationStyle")) << G4endl;
			G4cerr << "Must be either Constrained or Free." << G4endl;
			fPm->AbortSession(1);
		}
	}

	if (fPm->ParameterExists(GetFullParmName("nCutawayPlanes"))) {
		G4int nCutawayPlanes = fPm->GetIntegerParameter(GetFullParmName("nCutawayPlanes"));
		if (nCutawayPlanes < 0) {
			G4cerr << "Topas is exiting due to error in graphics setup." << G4endl;
			G4cerr << "Gr/" << fViewerName << "/nCutawayPlanes has been set to a negative number." << G4endl;
			fPm->AbortSession(1);
		}

		G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/clearCutawayPlanes");

		for (G4int iPlane = 1; iPlane <= nCutawayPlanes; iPlane++) {
			G4String posParamString = GetFullParmName("CutawayPlane") + "/" + G4UIcommand::ConvertToString(iPlane);
			G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/addCutawayPlane " +
				G4UIcommand::ConvertToString(fPm->GetDoubleParameter(posParamString + "/XPos", "Length")) + " " +
				G4UIcommand::ConvertToString(fPm->GetDoubleParameter(posParamString + "/YPos", "Length")) + " " +
				G4UIcommand::ConvertToString(fPm->GetDoubleParameter(posParamString + "/ZPos", "Length")) + " mm " +
				G4UIcommand::ConvertToString(fPm->GetUnitlessParameter(posParamString + "/XDir")) + " " +
				G4UIcommand::ConvertToString(fPm->GetUnitlessParameter(posParamString + "/YDir")) + " " +
				G4UIcommand::ConvertToString(fPm->GetUnitlessParameter(posParamString + "/ZDir")));
		}
	}

	G4UImanager::GetUIpointer()->ApplyCommand("/vis/enable");
}


G4String TsGraphicsView::GetName() {
	return fViewerName;
}


G4String TsGraphicsView::GetColorAsRGBA(G4String color) {
	G4int* colorValues = fPm->GetIntegerVector("Gr/Color/" + color);

	G4int aValue;
	if (fPm->GetVectorLength("Gr/Color/" + color) == 4)
		aValue = colorValues[3];
	else
		aValue = 255;

	return
	G4UIcommand::ConvertToString(colorValues[0]/255.) + " " +
	G4UIcommand::ConvertToString(colorValues[1]/255.) + " " +
	G4UIcommand::ConvertToString(colorValues[2]/255.) + " " +
	G4UIcommand::ConvertToString(aValue/255.);
}


G4String TsGraphicsView::GetFullParmName(const char* parmName) {
	G4String fullName = "Gr/"+fViewerName+"/"+parmName;
	return fullName;
}


G4String TsGraphicsView::GetFullParmNameLower(const char* parmName) {
	G4String fullName = GetFullParmName(parmName);
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(fullName);
#else
	fullName.toLower();
#endif
	return fullName;
}
