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

#include "TsBorderSurface.hh"

#include "TsParameterManager.hh"
#include "TsGeometryManager.hh"

#include "TsVGeometryComponent.hh"

#include "G4LogicalBorderSurface.hh"
#include "G4OpticalSurface.hh"

TsBorderSurface::TsBorderSurface(TsParameterManager* pm, TsGeometryManager* gm,
								 G4String borderSurfaceParameterName)
:fPm(pm), fGm(gm), fBorderSurfaceParameterName(borderSurfaceParameterName),
fBorderSurface(0), fMarkedForRebuild("true")
{
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(fBorderSurfaceParameterName);
#else
	fBorderSurfaceParameterName.toLower();
#endif

	size_t pos = fBorderSurfaceParameterName.find("opticalbehaviorto");
	fFromComponentName = fBorderSurfaceParameterName.substr(3,pos-4);
	fToComponentName   = fBorderSurfaceParameterName.substr(pos+18);
}


TsBorderSurface::~TsBorderSurface()
{;}


void TsBorderSurface::CreateBorderSurface()
{
	if (fMarkedForRebuild) {
		if (fBorderSurface)
			delete fBorderSurface;

		fBorderSurface = new G4LogicalBorderSurface(fFromComponentName+"To"+fToComponentName,
						fGm->GetComponent(fFromComponentName)->GetEnvelopePhysicalVolume(),
						fGm->GetComponent(fToComponentName)->GetEnvelopePhysicalVolume(),
						fGm->GetOpticalSurface(fPm->GetStringParameter(fBorderSurfaceParameterName)));

		fMarkedForRebuild = false;
	}
}


void TsBorderSurface::MarkForRebuild()
{
	fMarkedForRebuild = true;
}


void TsBorderSurface::MarkForRebuildIfMatchesDestination(G4String toComponentName) {
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(toComponentName);
#else
	toComponentName.toLower();
#endif

	if (toComponentName == fToComponentName)
		fMarkedForRebuild = true;
}
