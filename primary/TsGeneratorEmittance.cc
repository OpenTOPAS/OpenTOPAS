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

#include "TsGeneratorEmittance.hh"

#include "TsParameterManager.hh"

#include "Randomize.hh"

TsGeneratorEmittance::TsGeneratorEmittance(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* psM, G4String sourceName) :
TsVGenerator(pM,gM,psM,sourceName), fTwissX(fSourceName), fTwissY(fSourceName)
{
	ResolveParameters();
}

TsGeneratorEmittance::~TsGeneratorEmittance(){;}

void TsGeneratorEmittance::ResolveParameters(){
	TsVGenerator::ResolveParameters();

	fEmittanceDistName = fPm->GetStringParameter(GetFullParmName("Distribution"));
#if GEANT4_VERSION_MAJOR >= 11
   	G4StrUtil::to_lower(fEmittanceDistName);
#else
    fEmittanceDistName.toLower();
#endif

	if ( fEmittanceDistName == "bigaussian" ){
		fEmittanceDistType = BiGaussian;
	} else if ( fEmittanceDistName == "twiss_kv" ){
		fEmittanceDistType = TWISS_kv;
	} else if ( fEmittanceDistName == "twiss_gaussian" ){
		fEmittanceDistType = TWISS_gaussian;
	} else if ( fEmittanceDistName == "twiss_waterbag" ){
		fEmittanceDistType = TWISS_wb;
	} else {
		G4cout << "Particle source \"" << fSourceName << "\" has unknown Distribution \""
		<< fPm->GetStringParameter(GetFullParmName("Distribution")) << "\"" << G4endl;
		G4cout << "Accepted values are BiGaussian, Twiss_KV, Twiss_Gaussian and Twiss_Waterbag." << G4endl;
		fPm->AbortSession(1);
	}

	switch(fEmittanceDistType){
	case BiGaussian:
		{
		fSigX      = fPm->GetDoubleParameter(GetFullParmName("SigmaX"), "Length");
		fSigXprime = fPm->GetUnitlessParameter(GetFullParmName("SigmaXprime")); //Radian
		fRhoX      = fPm->GetUnitlessParameter(GetFullParmName("CorrelationX"));

		fSigY      = fPm->GetDoubleParameter(GetFullParmName("SigmaY"), "Length");
		fSigYprime = fPm->GetUnitlessParameter(GetFullParmName("SigmaYprime"));
		fRhoY      = fPm->GetUnitlessParameter(GetFullParmName("CorrelationY"));
		}
		break;
	case TWISS_kv: //K-V distribution
		{
		G4double a,b,e;
		a = fPm->GetUnitlessParameter(GetFullParmName("AlphaX"));
		b = fPm->GetDoubleParameter(GetFullParmName("BetaX"), "Length");
		e = fPm->GetDoubleParameter(GetFullParmName("EmittanceX"),"Length");
		fTwissX.SetParameters(a,b,e);

		a = fPm->GetUnitlessParameter(GetFullParmName("AlphaY"));
		b = fPm->GetDoubleParameter(GetFullParmName("BetaY"), "Length");
		e = fPm->GetDoubleParameter(GetFullParmName("EmittanceY"),"Length");
		fTwissY.SetParameters(a,b,e);
		}
		break;
	case TWISS_gaussian: //Twiss Gaussian distribution
		{
		G4double a,b,e;
		a = fPm->GetUnitlessParameter(GetFullParmName("AlphaX"));
		b = fPm->GetDoubleParameter(GetFullParmName("BetaX"), "Length");
		e = fPm->GetDoubleParameter(GetFullParmName("EmittanceX"),"Length");
		fTwissX.SetParameters(a,b,e);

		a = fPm->GetUnitlessParameter(GetFullParmName("AlphaY"));
		b = fPm->GetDoubleParameter(GetFullParmName("BetaY"), "Length");
		e = fPm->GetDoubleParameter(GetFullParmName("EmittanceY"),"Length");
		fTwissY.SetParameters(a,b,e);
		fFractionX = fPm->GetUnitlessParameter(GetFullParmName("ParticleFractionX"));
		fFractionY = fPm->GetUnitlessParameter(GetFullParmName("ParticleFractionY"));
		if( (fFractionX >=1.0 || fFractionX <=0.0) || (fFractionY >=1.0 || fFractionY <=0.0) ){
			G4cerr << "Topas is exiting due to a serious error in beam source setup.. " << G4endl;
			G4cerr << "Please use ParticleFractionOfX/Y between (0, 1): " << fSourceName << G4endl;
			fPm->AbortSession(1);
			}
		}
		break;
	case TWISS_wb: //Waterbag distribution
		{
		G4double a,b,e;
		a = fPm->GetUnitlessParameter(GetFullParmName("AlphaX"));
		b = fPm->GetDoubleParameter(GetFullParmName("BetaX"), "Length");
		e = fPm->GetDoubleParameter(GetFullParmName("EmittanceX"),"Length");
		fTwissX.SetParameters(a,b,e);

		a = fPm->GetUnitlessParameter(GetFullParmName("AlphaY"));
		b = fPm->GetDoubleParameter(GetFullParmName("BetaY"), "Length");
		e = fPm->GetDoubleParameter(GetFullParmName("EmittanceY"),"Length");
		fTwissY.SetParameters(a,b,e);
		}
		break;
	default:
		G4cout << "Particle source \"" << fSourceName << "\" has unknown Distribution \""
		<< fPm->GetStringParameter(GetFullParmName("Distribution")) << "\"" << G4endl;
		G4cout << "Accepted values are BiGaussian, Twiss_KV, Twiss_Gaussian and Twiss_Waterbag." << G4endl;
		fPm->AbortSession(1);
	}

	fBeamShape = NONE;
	if (fPm->ParameterExists(GetFullParmName("BeamPositionCutoffShape"))) {
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
		else if (beamShape != "none") {
		   G4cout << GetFullParmName("BeamPositionCutoffShape") << " must be either Rectangle, Ellipse or None." << G4endl;
		   fPm->AbortSession(1);
	   }
	}

	if (fBeamShape != NONE) {
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
}


void TsGeneratorEmittance::GeneratePrimaries(G4Event *anEvent){
	if(CurrentSourceHasGeneratedEnough()) return;

	TsPrimaryParticle p;
	G4double xprime = 0., yprime = 0.;
	G4double Ux, Vx, Uy,Vy;

	// Loop over position generation until get a postion that falls within the desired beam shape.
	G4bool needToSample = true;

	G4int nSampled = 0;
	while (needToSample) {
		switch(fEmittanceDistType){
		case BiGaussian:
			{
			//X axis: sample ux and vx from normal distribution and transform to correlation
			Ux = G4RandGauss::shoot();
			Vx = G4RandGauss::shoot();
			p.posX = fSigX*Ux;
			xprime = fSigXprime*(fRhoX*Ux+Vx*sqrt(1.0-fRhoX*fRhoX));
			//Y axis
			Uy = G4RandGauss::shoot();
			Vy = G4RandGauss::shoot();
			p.posY = fSigY*Uy;
			yprime = fSigYprime*(fRhoY*Uy+Vy*sqrt(1.0-fRhoY*fRhoY));
			}
			break;
		case TWISS_kv:
			{
			//From the ellipse formula, (x**2 + xp**2)/sqrt(ex*bx) + (y**2 + yp**2)/sqrt(ey*by) - 1 = 0
			//Px = (x**2 + xp**2)/sqrt(ex*bx), and then Py = (y**2 + yp**2)/sqrt(ey*by) = 1 - Px
			G4double Px = G4UniformRand();
			G4double Py = 1.0 - Px;

			fTwissX.KV(Px,Ux,Vx);
			fTwissY.KV(Py,Uy,Vy);

			fTwissX.Transform(Ux,Vx, p.posX, xprime);
			fTwissY.Transform(Uy,Vy, p.posY, yprime);
			}
			break;
		case TWISS_gaussian:
			{
			//In order to enclose given fraction of particles in the unit circle, sigx/sigy is the sigma for each axis.
			//Then the number of particles satisfying that (u**2 + v**2) <= 1.0 devidied by total # = fFractionX.
			G4double sigx = 1.0/std::sqrt(-2.0*std::log(1.0 - fFractionX)); //radius
			G4double sigy = 1.0/std::sqrt(-2.0*std::log(1.0 - fFractionY));

			fTwissX.Gaussian(sigx, Ux,Vx);
			fTwissY.Gaussian(sigy, Uy,Vy);

			fTwissX.Transform(Ux,Vx, p.posX, xprime);
			fTwissY.Transform(Uy,Vy, p.posY, yprime);
			}
			break;

		case TWISS_wb:
			{
			//Waterbag distribution assumes that particles are uniformly distributed in 4D-ellipse/norm_circle.
			//sample a radius in [0,1] and take sqrt twice before pass it to KV distribution.
			G4double R_HyperEllipse =  std::sqrt( std::sqrt(G4UniformRand() ) );
			G4double Px = R_HyperEllipse*G4UniformRand();
			G4double Py = R_HyperEllipse - Px;

			//Same with KV from below...
			fTwissX.KV(Px,Ux,Vx);
			fTwissY.KV(Py,Uy,Vy);

			fTwissX.Transform(Ux,Vx, p.posX,xprime);
			fTwissY.Transform(Uy,Vy, p.posY,yprime);
			}
			break;
		default:
			G4cout << "Particle source \"" << fSourceName << "\" has unknown Distribution \""
			<< fPm->GetStringParameter(GetFullParmName("Distribution")) << "\"" << G4endl;
			G4cout << "Accepted values are BiGaussian, Twiss_KV, Twiss_Gaussian and Twiss_Waterbag." << G4endl;
			fPm->AbortSession(1);
		}

		// See if position is within the desired beam shape
		switch (fBeamShape) {
		case ELLIPSE:
			if ( ((p.posX*p.posX) / (fPositionCutoffX*fPositionCutoffX) + (p.posY*p.posY) / (fPositionCutoffY*fPositionCutoffY) ) < 1. )
				needToSample = false;
			break;
		case RECTANGLE:
			if ( fabs(p.posX) < fPositionCutoffX && fabs(p.posY) < fPositionCutoffY )
				needToSample = false;
			break;
		default:
			needToSample = false;
		}

		nSampled++;
		if (nSampled == 10000) {
			G4cout << "Particle source \"" << fSourceName << "\" has positional constraints set too tight." << G4endl;
			G4cout << "Was unable to find a good particle postion in 10,000 samples." << G4endl;
			fPm->AbortSession(1);
		}
	}

	G4double dir_x  = tan(xprime);
	G4double dir_y  = tan(yprime);

	//convert to directional cosines
	G4double norm_dir = sqrt(dir_x*dir_x+dir_y*dir_y+1.0); //+1 for z
	p.dCos1 = dir_x/norm_dir;
	p.dCos2 = dir_y/norm_dir;
	p.dCos3 = 1.0/norm_dir;

	p.posZ  = 0.0;

	SetEnergy(p);
	SetParticleType(p);

	p.weight = 1.0;
	p.isNewHistory = true;

	TransformPrimaryForComponent(&p);
	GenerateOnePrimary(anEvent, p);
	AddPrimariesToEvent(anEvent);
}
