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

#ifndef TsGeneratorEmittance_hh
#define TsGeneratorEmittance_hh

#include "TsVGenerator.hh"

#include "G4PhysicalConstants.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"

#include <cmath>
#include <fstream>
//Expecting users to give alpha, beta, emittance.
//Alpha can be 0 but beta and emittance can't be zero
//Gamma is internally calculated

class TsTwissParameters
{
private:
	G4double gamma;
	G4double alpha;
	G4double beta ;
	G4double emittance;
	G4String name;

public:
	 TsTwissParameters(G4String n): gamma(0), alpha(0), beta(0), emittance(0), name(n) {;}
	~TsTwissParameters() {;}

	inline void SetParameters(G4double a, G4double b, G4double e){
		if( e <= 0.0 || b <= 0.0 ){
			G4cerr << "Topas is exiting due to a serious error in beam source setup.. " << G4endl;
				G4cerr << "Emittance and Beta can't be less than zero: " << name << G4endl;
			exit(1);
		}
		alpha = a; beta = b; emittance = e; gamma = (1.0 + alpha*alpha)/beta;
	}

	inline void Gaussian(G4double& prob, G4double& u, G4double& v){
		u = G4RandGauss::shoot(0.0,prob);
		v = G4RandGauss::shoot(0.0,prob);
	}

	inline void KV(G4double& prob, G4double& u, G4double& v){
		//For given prob, return u and v
		G4double th =CLHEP::twopi*G4UniformRand();
		u = std::sqrt(prob)*cos(th);
		v = std::sqrt(prob)*sin(th);
	}

	inline void Transform(G4double& u, G4double& v, G4float& x, G4double& xp){
		//For given u and v, update x and xp
		x  = std::sqrt(beta*emittance)*u;
		xp = std::sqrt(emittance/beta)*v - (alpha/beta)*x;
	}

};

class TsGeneratorEmittance : public TsVGenerator
{
public:
	TsGeneratorEmittance(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* psM, G4String sourceName);
	~TsGeneratorEmittance();

	void ResolveParameters();

	void GeneratePrimaries(G4Event*);

private:
	G4String fEmittanceDistName;
	enum  emittance_distribution {BiGaussian, TWISS_kv, TWISS_gaussian, TWISS_wb} fEmittanceDistType;
	//For BiGaussian
	G4double fSigX, fSigXprime;
	G4double fSigY, fSigYprime;
	G4double fRhoX, fRhoY ; //correlation

	//For Twiss
	TsTwissParameters fTwissX;
	TsTwissParameters fTwissY;
	//Twiss Gaussians ONLY
	G4double fFractionX;
	G4double fFractionY;

	enum Shape { ELLIPSE, RECTANGLE, NONE };
	Shape fBeamShape;
	G4double fPositionCutoffX;
	G4double fPositionCutoffY;
};

#endif
