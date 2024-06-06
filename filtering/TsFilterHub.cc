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

#include "TsFilterHub.hh"

#include "TsParameterManager.hh"
#include "TsExtensionManager.hh"
#include "TsMaterialManager.hh"
#include "TsGeometryManager.hh"
#include "TsFilterManager.hh"

#include "TsVFilter.hh"
#include "TsPseudoFilter.hh"
#include "TsFilterByGeneration.hh"
#include "TsFilterByCharge.hh"
#include "TsFilterByAtomicMass.hh"
#include "TsFilterByAtomicNumber.hh"
#include "TsFilterByKE.hh"
#include "TsFilterByInitialKE.hh"
#include "TsFilterByPrimaryKE.hh"
#include "TsFilterByMomentum.hh"
#include "TsFilterByInitialMomentum.hh"
#include "TsFilterByIncidentParticleCreatorProcess.hh"
#include "TsFilterByIncidentParticleGeneration.hh"
#include "TsFilterByIncidentParticleKE.hh"
#include "TsFilterByIncidentParticleMomentum.hh"
#include "TsFilterByIncidentParticleType.hh"
#include "TsFilterByCreatorProcess.hh"
#include "TsFilterByType.hh"
#include "TsFilterByOrigin.hh"
#include "TsFilterByInteractedOrTraversed.hh"
#include "TsFilterByInteractionCount.hh"
#include "TsFilterByRTStructure.hh"
#include "TsFilterByMaterial.hh"
#include "TsFilterByWeight.hh"
#include "TsFilterByTimeOfFlight.hh"
#include "TsFilterByInverse.hh"

TsFilterHub::TsFilterHub()
{;}


TsFilterHub::~TsFilterHub()
{;}


TsVFilter* TsFilterHub::InstantiateFilter(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM,
	TsGeometryManager* gM, TsFilterManager* fM, TsVGenerator* generator, TsVScorer* scorer)
{
	G4String filterName = "PseudoFilter";
	TsVFilter* parentFilter = new TsPseudoFilter(filterName, pM, mM, gM, fM, generator, scorer);

	// Start with user filters
	parentFilter = eM->InstantiateFilter(pM, mM, gM, fM, generator, scorer, parentFilter);

	// Generation filter
	filterName = "OnlyIncludeParticlesOfGeneration";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByGeneration(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false);

	// Charge filter
	filterName = "OnlyIncludeParticlesCharged";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByCharge(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false);
	filterName = "OnlyIncludeParticlesNotCharged";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByCharge(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true);

	// Atomic Mass filter
	filterName = "OnlyIncludeParticlesOfAtomicMass";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByAtomicMass(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false);
	filterName = "OnlyIncludeParticlesNotOfAtomicMass";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByAtomicMass(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false);
	filterName = "OnlyIncludeIfParticleOrAncestorOfAtomicMass";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByAtomicMass(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, true);
	filterName = "OnlyIncludeIfParticleOrAncestorNotOfAtomicMass";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByAtomicMass(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, true);

	// Atomic Number filter
	filterName = "OnlyIncludeParticlesOfAtomicNumber";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByAtomicNumber(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false);
	filterName = "OnlyIncludeParticlesNotOfAtomicNumber";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByAtomicNumber(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false);
	filterName = "OnlyIncludeIfParticleOrAncestorOfAtomicNumber";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByAtomicNumber(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, true);
	filterName = "OnlyIncludeIfParticleOrAncestorNotOfAtomicNumber";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByAtomicNumber(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, true);

	// Kinetic Energy filter
	filterName = "OnlyIncludeParticlesWithKEBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 1);
	filterName = "OnlyIncludeParticlesWithKENotBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 1);
	filterName = "OnlyIncludeParticlesWithKE";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 2);
	filterName = "OnlyIncludeParticlesWithKENot";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 2);
	filterName = "OnlyIncludeParticlesWithKEAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 3);
	filterName = "OnlyIncludeParticlesWithKENotAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 3);

	// Primary Particle Kinetic Energy filter
	filterName = "OnlyIncludeParticlesWithPrimaryKEBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByPrimaryKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 1);
	filterName = "OnlyIncludeParticlesWithPrimaryKENotBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByPrimaryKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 1);
	filterName = "OnlyIncludeParticlesWithPrimaryKE";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByPrimaryKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 2);
	filterName = "OnlyIncludeParticlesWithPrimaryKENot";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByPrimaryKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 2);
	filterName = "OnlyIncludeParticlesWithPrimaryKEAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByPrimaryKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 3);
	filterName = "OnlyIncludeParticlesWithPrimaryKENotAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByPrimaryKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 3);

	// Initial Track Kinetic Energy filter
	filterName = "OnlyIncludeParticlesWithInitialKEBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInitialKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 1);
	filterName = "OnlyIncludeParticlesWithInitialKENotBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInitialKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 1);
	filterName = "OnlyIncludeParticlesWithInitialKE";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInitialKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 2);
	filterName = "OnlyIncludeParticlesWithInitialKENot";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInitialKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 2);
	filterName = "OnlyIncludeParticlesWithInitialKEAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInitialKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 3);
	filterName = "OnlyIncludeParticlesWithInitialKENotAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInitialKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 3);

	// Momentum filter
	filterName = "OnlyIncludeParticlesWithMomentumBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByMomentum(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 1);
	filterName = "OnlyIncludeParticlesWithMomentumNotBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByMomentum(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 1);
	filterName = "OnlyIncludeParticlesWithMomentum";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByMomentum(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 2);
	filterName = "OnlyIncludeParticlesWithMomentumNot";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByMomentum(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 2);
	filterName = "OnlyIncludeParticlesWithMomentumAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByMomentum(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 3);
	filterName = "OnlyIncludeParticlesWithMomentumNotAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByMomentum(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 3);

	// Initial Track Momentum filter
	filterName = "OnlyIncludeParticlesWithInitialMomentumBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInitialMomentum(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 1);
	filterName = "OnlyIncludeParticlesWithInitialMomentumNotBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInitialMomentum(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 1);
	filterName = "OnlyIncludeParticlesWithInitialMomentum";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInitialMomentum(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 2);
	filterName = "OnlyIncludeParticlesWithInitialMomentumNot";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInitialMomentum(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 2);
	filterName = "OnlyIncludeParticlesWithInitialMomentumAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInitialMomentum(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 3);
	filterName = "OnlyIncludeParticlesWithInitialMomentumNotAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInitialMomentum(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 3);

	// Incident Track Kinetic Energy filter
	filterName = "OnlyIncludeIfIncidentParticleKEBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByIncidentParticleKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 1);
	filterName = "OnlyIncludeIfIncidentParticleKENotBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByIncidentParticleKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 1);
	filterName = "OnlyIncludeIfIncidentParticleKE";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByIncidentParticleKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 2);
	filterName = "OnlyIncludeIfIncidentParticleKENot";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByIncidentParticleKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 2);
	filterName = "OnlyIncludeIfIncidentParticleKEAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByIncidentParticleKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 3);
	filterName = "OnlyIncludeIfIncidentParticleKENotAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByIncidentParticleKE(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 3);


	// Incident Track Momentum filter
	filterName = "OnlyIncludeIfIncidentParticleMomentumBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByIncidentParticleMomentum(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 1);
	filterName = "OnlyIncludeIfIncidentParticleMomentumNotBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByIncidentParticleMomentum(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 1);
	filterName = "OnlyIncludeIfIncidentParticleMomentum";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByIncidentParticleMomentum(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 2);
	filterName = "OnlyIncludeIfIncidentParticleMomentumNot";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByIncidentParticleMomentum(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 2);
	filterName = "OnlyIncludeIfIncidentParticleMomentumAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByIncidentParticleMomentum(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 3);
	filterName = "OnlyIncludeIfIncidentParticleMomentumNotAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByIncidentParticleMomentum(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 3);

	// Incident Particle Creator Process filter
	filterName = "OnlyIncludeIfIncidentParticlesFromProcess";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByIncidentParticleCreatorProcess(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false);
	filterName = "OnlyIncludeIfIncidentParticlesNotFromProcess";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByIncidentParticleCreatorProcess(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true);

	// Incident Particle Name filter
	filterName = "OnlyIncludeIfIncidentParticlesNamed";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByIncidentParticleType(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false);
	filterName = "OnlyIncludeIfIncidentParticlesNotNamed";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByIncidentParticleType(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true);

	// Incident Particle Generation filter
	filterName = "OnlyIncludeIfIncidentParticlesOfGeneration";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByIncidentParticleGeneration(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false);

	// Creator Process filter
	filterName = "OnlyIncludeParticlesFromProcess";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByCreatorProcess(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false);
	filterName = "OnlyIncludeParticlesNotFromProcess";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByCreatorProcess(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false);
	filterName = "OnlyIncludeIfParticleOrAncestorFromProcess";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByCreatorProcess(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, true);
	filterName = "OnlyIncludeIfParticleOrAncestorNotFromProcess";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByCreatorProcess(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, true);

	// Particle Name filter
	filterName = "OnlyIncludeParticlesNamed";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByType(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false);
	filterName = "OnlyIncludeParticlesNotNamed";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByType(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false);
	filterName = "OnlyIncludeIfParticleOrAncestorNamed";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByType(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, true);
	filterName = "OnlyIncludeIfParticleOrAncestorNotNamed";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByType(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, true);

	// Origin filter
	filterName = "OnlyIncludeParticlesFromVolume";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByOrigin(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false, true, false);
	filterName = "OnlyIncludeParticlesNotFromVolume";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByOrigin(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false, true, false);
	filterName = "OnlyIncludeParticlesFromComponent";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByOrigin(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false, false, false);
	filterName = "OnlyIncludeParticlesNotFromComponent";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByOrigin(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false, false, false);
	filterName = "OnlyIncludeParticlesFromComponentOrSubComponentsOf";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByOrigin(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false, false, true);
	filterName = "OnlyIncludeParticlesNotFromComponentOrSubComponentsOf";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByOrigin(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false, false, true);
	filterName = "OnlyIncludeIfParticleOrAncestorFromVolume";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByOrigin(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, true, true, false);
	filterName = "OnlyIncludeIfParticleOrAncestorNotFromVolume";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByOrigin(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, true, true, false);
	filterName = "OnlyIncludeIfParticleOrAncestorFromComponent";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByOrigin(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, true, false, false);
	filterName = "OnlyIncludeIfParticleOrAncestorNotFromComponent";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByOrigin(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, true, false, false);
	filterName = "OnlyIncludeIfParticleOrAncestorFromComponentOrSubComponentsOf";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByOrigin(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, true, false, true);
	filterName = "OnlyIncludeIfParticleOrAncestorNotFromComponentOrSubComponentsOf";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByOrigin(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, true, false, true);

	// Interacted filter
	filterName = "OnlyIncludeIfParticleInteractedInVolume";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false, true, false, false, false);
	filterName = "OnlyIncludeIfParticleNotInteractedInVolume";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false, true, false, false, false);
	filterName = "OnlyIncludeIfParticleInteractedInComponent";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false, false, false, false, false);
	filterName = "OnlyIncludeIfParticleNotInteractedInComponent";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false, false, false, false, false);
	filterName = "OnlyIncludeIfParticleInteractedInComponentOrSubComponentsOf";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false, false, true, false, false);
	filterName = "OnlyIncludeIfParticleNotInteractedInComponentOrSubComponentsOf";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false, false, true, false, false);
	filterName = "OnlyIncludeIfParticleOrAncestorInteractedInVolume";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, true, true, false, false, false);
	filterName = "OnlyIncludeIfParticleOrAncestorNotInteractedInVolume";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, true, true, false, false, false);
	filterName = "OnlyIncludeIfParticleOrAncestorInteractedInComponent";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, true, false, false, false, false);
	filterName = "OnlyIncludeIfParticleOrAncestorNotInteractedInComponent";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, true, false, false, false, false);
	filterName = "OnlyIncludeIfParticleOrAncestorInteractedInComponentOrSubComponentsOf";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, true, false, true, false, false);
	filterName = "OnlyIncludeIfParticleOrAncestorNotInteractedInComponentOrSubComponentsOf";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, true, false, true, false, false);

	// Last Interacted filter
	filterName = "OnlyIncludeIfParticleLastInteractedInVolume";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false, true, false, false, true);
	filterName = "OnlyIncludeIfParticleNotLastInteractedInVolume";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false, true, false, false, true);
	filterName = "OnlyIncludeIfParticleLastInteractedInComponent";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false, false, false, false, true);
	filterName = "OnlyIncludeIfParticleNotLastInteractedInComponent";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false, false, false, false, true);
	filterName = "OnlyIncludeIfParticleLastInteractedInComponentOrSubComponentsOf";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false, false, true, false, true);
	filterName = "OnlyIncludeIfParticleNotLastInteractedInComponentOrSubComponentsOf";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false, false, true, false, true);

	// Interaction Count Filter
	filterName = "OnlyIncludeParticlesWithInteractionCountBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractionCount(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 1);
	filterName = "OnlyIncludeParticlesWithInteractionCountNotBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractionCount(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 1);
	filterName = "OnlyIncludeParticlesWithInteractionCount";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractionCount(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 2);
	filterName = "OnlyIncludeParticlesWithInteractionCountNot";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractionCount(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 2);
	filterName = "OnlyIncludeParticlesWithInteractionCountAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractionCount(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 3);
	filterName = "OnlyIncludeParticlesWithInteractionCountNotAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByWeight(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 3);

	// Traversed filter
	filterName = "OnlyIncludeIfParticleTraversedVolume";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false, true, false, true, false);
	filterName = "OnlyIncludeIfParticleNotTraversedVolume";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false, true, false, true, false);
	filterName = "OnlyIncludeIfParticleTraversedComponent";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false, false, false, true, false);
	filterName = "OnlyIncludeIfParticleNotTraversedComponent";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false, false, false, true, false);
	filterName = "OnlyIncludeIfParticleTraversedComponentOrSubComponentsOf";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false, false, true, true, false);
	filterName = "OnlyIncludeIfParticleNotTraversedComponentOrSubComponentsOf";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false, false, true, true, false);
	filterName = "OnlyIncludeIfParticleOrAncestorTraversedVolume";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, true, true, false, true, false);
	filterName = "OnlyIncludeIfParticleOrAncestorNotTraversedVolume";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, true, true, false, true, false);
	filterName = "OnlyIncludeIfParticleOrAncestorTraversedComponent";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, true, false, false, true, false);
	filterName = "OnlyIncludeIfParticleOrAncestorNotTraversedComponent";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, true, false, false, true, false);
	filterName = "OnlyIncludeIfParticleOrAncestorTraversedComponentOrSubComponentsOf";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, true, false, true, true, false);
	filterName = "OnlyIncludeIfParticleOrAncestorNotTraversedComponentOrSubComponentsOf";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, true, false, true, true, false);

	// Last Traversed filter
	filterName = "OnlyIncludeIfParticleLastTraversedVolume";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false, true, false, true, true);
	filterName = "OnlyIncludeIfParticleNotLastTraversedVolume";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false, true, false, true, true);
	filterName = "OnlyIncludeIfParticleLastTraversedComponent";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false, false, false, true, true);
	filterName = "OnlyIncludeIfParticleNotLastTraversedComponent";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false, false, false, true, true);
	filterName = "OnlyIncludeIfParticleLastTraversedComponentOrSubComponentsOf";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, false, false, true, true, true);
	filterName = "OnlyIncludeIfParticleNotLastTraversedComponentOrSubComponentsOf";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInteractedOrTraversed(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, false, false, true, true, true);

	// DICOM RT Structure filter
	filterName = "OnlyIncludeIfInRTStructure";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByRTStructure(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false);
	filterName = "OnlyIncludeIfNotInRTStructure";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByRTStructure(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true);

	// Material filter
	filterName = "OnlyIncludeIfInMaterial";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByMaterial(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false);
	filterName = "OnlyIncludeIfNotInMaterial";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByMaterial(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true);

	// Particle weight filter
	filterName = "OnlyIncludeParticlesWithWeightBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByWeight(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 1);
	filterName = "OnlyIncludeParticlesWithWeightNotBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByWeight(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 1);
	filterName = "OnlyIncludeParticlesWithWeight";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByWeight(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 2);
	filterName = "OnlyIncludeParticlesWithWeightNot";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByWeight(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 2);
	filterName = "OnlyIncludeParticlesWithWeightAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByWeight(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false, 3);
	filterName = "OnlyIncludeParticlesWithWeightNotAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByWeight(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true, 3);
	
	// Time of Flight Filter
	filterName = "OnlyIncludeParticlesWithTimeOfFlightBelow";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByTimeOfFlight(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, true);
	filterName = "OnlyIncludeParticlesWithTimeOfFlightAbove";
	pM->RegisterFilterName(filterName);
	if (HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByTimeOfFlight(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false);

	// Invert filter
	filterName = "InvertFilter";
	pM->RegisterFilterName(filterName);
	if (parentFilter && HaveFilterNamed(pM, generator, scorer, filterName))
		parentFilter = new TsFilterByInverse(filterName, pM, mM, gM, fM, generator, scorer, parentFilter, false);

	pM->RegisterFilterName("OnlyIncludeParticlesGoing");

	return parentFilter;
}


G4bool TsFilterHub::HaveFilterNamed(TsParameterManager* pM, TsVGenerator* generator, TsVScorer* scorer, G4String filterName) {
	if (generator)
		return pM->ParameterExists(generator->GetFullParmName(filterName));
	else
		return pM->ParameterExists(scorer->GetFullParmName(filterName));
}
