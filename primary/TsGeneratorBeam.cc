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

#include "TsGeneratorBeam.hh"

#include "TsParameterManager.hh"
#include "TsGeometryManager.hh"

#include "Randomize.hh"
#include "G4SystemOfUnits.hh"

TsGeneratorBeam::TsGeneratorBeam(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName) :
TsVGenerator(pM, gM, pgM, sourceName)
{
	ResolveParameters();
}


TsGeneratorBeam::~TsGeneratorBeam()
{
}


void TsGeneratorBeam::ResolveParameters() {
	TsVGenerator::ResolveParameters();

	// Validate parameters for position distribution
	G4String dist = fPm->GetStringParameter(GetFullParmName("BeamPositionDistribution"));
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(dist);
#else
	dist.toLower();
#endif
	if (dist == "none")
		fPositionDistribution = NONE;
	else if (dist == "flat")
		fPositionDistribution = FLAT;
	else if (dist == "gaussian")
		fPositionDistribution = GAUSSIAN;
	else {
		G4cout << "Particle source \"" << fSourceName << "\" has unknown BeamPositionDistribution \""
		<< fPm->GetStringParameter(GetFullParmName("BeamPositionDistribution")) << "\"" << G4endl;
		G4cout << "Accepted values are None, Flat and Gaussian." << G4endl;
		fPm->AbortSession(1);
	}

	if (fPositionDistribution == FLAT || fPositionDistribution == GAUSSIAN) {
		G4String beamShape = fPm->GetStringParameter(GetFullParmName("BeamPositionCutoffShape"));
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(beamShape);
#else
		beamShape.toLower();
#endif
		if (beamShape == "rectangle")
			fBeamShape = RECTANGLE;
		else if (beamShape == "ellipse")
			fBeamShape = ELLIPSE;
		else {
			G4cout << "Particle source \"" << fSourceName << "\" has unknown BeamPositionCutoffShape \""
			<< fPm->GetStringParameter(GetFullParmName("BeamPositionCutoffShape")) << "\"" << G4endl;
			G4cout << "Accepted values are Rectangle and Ellipse." << G4endl;
			fPm->AbortSession(1);
		}

		fPositionCutoffX = fPm->GetDoubleParameter(GetFullParmName("BeamPositionCutoffX"), "Length");
		if (fPositionCutoffX <= 0.) {
			G4cout << GetFullParmName("BeamPositionCutoffX") << " must be greater than zero." << G4endl;
			fPm->AbortSession(1);
		}

		fPositionCutoffY = fPm->GetDoubleParameter(GetFullParmName("BeamPositionCutoffY"), "Length");
		if (fPositionCutoffY <= 0.) {
			G4cout << GetFullParmName("BeamPositionCutoffY") << " must be greater than zero." << G4endl;
			fPm->AbortSession(1);
		}
	}
	if (fPositionDistribution == GAUSSIAN) {
		fPositionSpreadX = fPm->GetDoubleParameter(GetFullParmName("BeamPositionSpreadX"), "Length");
		if (fPositionSpreadX <= 0.) {
			G4cout << GetFullParmName("BeamPositionSpreadX") << " must be greater than zero." << G4endl;
			fPm->AbortSession(1);
		}

		fPositionSpreadY = fPm->GetDoubleParameter(GetFullParmName("BeamPositionSpreadY"), "Length");
		if (fPositionSpreadY <= 0.) {
			G4cout << GetFullParmName("BeamPositionSpreadY") << " must be greater than zero." << G4endl;
			fPm->AbortSession(1);
		}
	}

	// Validate parameters for angular distribution
	dist = fPm->GetStringParameter(GetFullParmName("BeamAngularDistribution"));
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(dist);
#else
	dist.toLower();
#endif
	if (dist == "none")
		fAngularDistribution = NONE;
	else if (dist == "flat")
		fAngularDistribution = FLAT;
	else if (dist == "gaussian")
		fAngularDistribution = GAUSSIAN;
	else {
		G4cout << "Particle source \"" << fSourceName << "\" has unknown BeamAngularDistribution \""
		<< fPm->GetStringParameter(GetFullParmName("BeamAngularDistribution")) << "\"" << G4endl;
		G4cout << "Accepted values are None, Flat and Gaussian." << G4endl;
		fPm->AbortSession(1);
	}

	if (fAngularDistribution == FLAT || fAngularDistribution == GAUSSIAN) {
		G4double angularCutoffX = fPm->GetDoubleParameter(GetFullParmName("BeamAngularCutoffX"), "Angle");
		if (angularCutoffX <= 0. || angularCutoffX > 180.*deg) {
			G4cout << GetFullParmName("BeamAngularCutoffX") << " must be in the range 0 < x <= 180 degrees." << G4endl;
			fPm->AbortSession(1);
		}
		fMarsagliaCutoffX = AngleToMarsagliaCoordinate(angularCutoffX);

		G4double angularCutoffY = fPm->GetDoubleParameter(GetFullParmName("BeamAngularCutoffY"), "Angle");
		if (angularCutoffY <= 0. || angularCutoffY > 180.*deg) {
			G4cout << GetFullParmName("BeamAngularCutoffY") << " must be in the range 0 < x <= 180 degrees." << G4endl;
			fPm->AbortSession(1);
		}
		fMarsagliaCutoffY = AngleToMarsagliaCoordinate(angularCutoffY);
	}
	if (fAngularDistribution == GAUSSIAN) {
		G4double angularSpreadX = fPm->GetDoubleParameter(GetFullParmName("BeamAngularSpreadX"), "Angle");
		if (angularSpreadX <= 0. || angularSpreadX > 180.*deg) {
			G4cout << GetFullParmName("BeamAngularSpreadX") << " must be in the range 0 < x <= 180 degrees." << G4endl;
			fPm->AbortSession(1);
		}
		fMarsagliaSpreadX = AngleToMarsagliaCoordinate(angularSpreadX);

		G4double angularSpreadY = fPm->GetDoubleParameter(GetFullParmName("BeamAngularSpreadY"), "Angle");
		if (angularSpreadY <= 0. || angularSpreadY > 180.*deg) {
			G4cout << GetFullParmName("BeamAngularSpreadY") << " must be in the range 0 < x <= 180 degrees." << G4endl;
			fPm->AbortSession(1);
		}
		fMarsagliaSpreadY = AngleToMarsagliaCoordinate(angularSpreadY);
	}
}


void TsGeneratorBeam::GeneratePrimaries(G4Event* anEvent)
{
	if (CurrentSourceHasGeneratedEnough())
		return;

	TsPrimaryParticle p;

	SetPosition(p);
	SetDirection(p);
	SetEnergy(p);
	SetParticleType(p);

	// Weight is always 1 for Beam source
	p.weight = 1.;

	// Always a fresh history for Beam source
	p.isNewHistory = true;

	TransformPrimaryForComponent(&p);
	GenerateOnePrimary(anEvent, p);
	AddPrimariesToEvent(anEvent);
}

void TsGeneratorBeam::SetPosition(TsPrimaryParticle &p) const
{
	// Loop over position generation until get a postion that falls within the desired beam shape
	G4bool needToSample = true;
	G4int nSampled = 0;
	while (needToSample) {

		switch (fPositionDistribution) {
		case FLAT:
			p.posX = G4RandFlat::shoot(-fPositionCutoffX, fPositionCutoffX);
			p.posY = G4RandFlat::shoot(-fPositionCutoffY, fPositionCutoffY);
			break;
		case GAUSSIAN:
			p.posX = G4RandGauss::shoot(0., fPositionSpreadX);
			p.posY = G4RandGauss::shoot(0., fPositionSpreadY);
			break;
		case NONE:
			p.posX = 0.;
			p.posY = 0.;
			needToSample = false;
			break;
		}

		// See if position is within the desired beam shape
		if (fPositionDistribution == FLAT || fPositionDistribution == GAUSSIAN) {
			switch (fBeamShape) {
			case ELLIPSE:
				if ( ((p.posX*p.posX) / (fPositionCutoffX*fPositionCutoffX) + (p.posY*p.posY) / (fPositionCutoffY*fPositionCutoffY) ) < 1. )
					needToSample = false;
				break;
			case RECTANGLE:
				if ( fabs(p.posX) < fPositionCutoffX && fabs(p.posY) < fPositionCutoffY )
					needToSample = false;
				break;
			}
		}

		nSampled++;
		if (nSampled == 10000) {
			G4cout << "Particle source \"" << fSourceName << "\" has positional constraints set too tight." << G4endl;
			G4cout << "Was unable to find a good particle postion in 10,000 samples." << G4endl;
			fPm->AbortSession(1);
		}
	}

	// Z position is always exactly at the generator component for Beam source
	p.posZ = 0.;
}


void TsGeneratorBeam::SetDirection(TsPrimaryParticle &p) const
{
	// Generating directions requires a complex sequence of coordinate transforms.
	// I extend the Marsaglia (1972) method of generating vectors on the unit sphere,
	// in order to support generating vectors within elliptical cones.

	if (fAngularDistribution == NONE) {
		p.dCos1 = 0.;
		p.dCos2 = 0.;
		p.dCos3 = 1.;
		return;
	}

	// Loop over direction generation until get an angle that falls within the cutoff range
	G4float marsagliaX = 0;
	G4float marsagliaY = 0;
	G4bool needToSample = true;
	G4int nSampled = 0;
	while (needToSample) {

		switch (fAngularDistribution) {
		case FLAT:
			marsagliaX = G4RandFlat::shoot(-fMarsagliaCutoffX, fMarsagliaCutoffX);
			marsagliaY = G4RandFlat::shoot(-fMarsagliaCutoffY, fMarsagliaCutoffY);
			break;
		case GAUSSIAN:
			marsagliaX = G4RandGauss::shoot(0., fMarsagliaSpreadX);
			marsagliaY = G4RandGauss::shoot(0., fMarsagliaSpreadY);
			break;
		default:
			break;
		}

		if ( ((marsagliaX*marsagliaX) / (fMarsagliaCutoffX*fMarsagliaCutoffX) + (marsagliaY*marsagliaY) / (fMarsagliaCutoffY*fMarsagliaCutoffY) ) <= 1. )
			needToSample = false;

		nSampled++;
		if (nSampled == 10000) {
			G4cout << "Particle source \"" << fSourceName << "\" has angular constraints set too tight." << G4endl;
			G4cout << "Was unable to find a good particle direction in 10,000 samples." << G4endl;
			fPm->AbortSession(1);
		}
	}

	G4float marsagliaRadius2 = marsagliaX*marsagliaX + marsagliaY*marsagliaY;
	G4float tmp = sqrt(std::max(0., 1.-marsagliaRadius2));

	p.dCos1 = 2. * marsagliaX * tmp;
	p.dCos2 = 2. * marsagliaY * tmp;
	p.dCos3 = 1. - (2. * marsagliaRadius2);
}


G4float TsGeneratorBeam::AngleToMarsagliaCoordinate(G4float angle) const
{
	G4float y = tan(angle);
	y = y*y;
	if (angle <= 90.*deg)
		return sqrt((y/(y+1) - 1./sqrt(y+1) + 1./(y+1)) / 2.);
	else
		return sqrt((y/(y+1) + 1./sqrt(y+1) + 1./(y+1)) / 2.);
}
