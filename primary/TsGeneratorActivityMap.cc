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

#include "TsGeneratorActivityMap.hh"
#include "TsDicomActivityMap.hh"

#include "TsParameterManager.hh"

#include "G4RandomDirection.hh"
#include "G4IonTable.hh"
#include "G4ParticleTable.hh"

TsGeneratorActivityMap::TsGeneratorActivityMap(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName) :
TsVGenerator(pM, gM, pgM, sourceName)
{
	fSource = (TsSourceActivityMap*)fPs;
}


TsGeneratorActivityMap::~TsGeneratorActivityMap() { }

void TsGeneratorActivityMap::GeneratePrimaries(G4Event* anEvent)
{
	if (CurrentSourceHasGeneratedEnough())
		return;

	// Get radionuclide name
	G4String radionuclideName = fSource->GetRadionuclideName();
	radionuclideName.toLower();
	G4int A = 0;
	G4int Z = 0;
	G4double excitEnergy = 0;
	G4ParticleDefinition* ion;
	// Check most common names, like Y90
	if (radionuclideName == "y90" || invertRadionuclideName(radionuclideName) == "y90")
	{
		A = 90;
		Z = 39;
		ion = G4IonTable::GetIonTable()->GetIon(Z, A, excitEnergy);
	}
	else if (radionuclideName == "tc99" || radionuclideName == "tc99m" || radionuclideName == "99tc" || radionuclideName == "99tcm" || radionuclideName == "99mtc")
	{
		A = 99;
		Z = 43;
		excitEnergy = 142.0 * keV;
		ion = G4IonTable::GetIonTable()->GetIon(Z, A, excitEnergy);
	}
	else // Assuming 'GenericIon(Z,A,E)' or 'GenericIon(Z,A)'
		ion = G4ParticleDefinition::GetParticleTable()->FindParticle(radionuclideName);

	G4double xVoxelSize = fSource->GetActivityMap()->GetVoxelSizeX();
	G4double yVoxelSize = fSource->GetActivityMap()->GetVoxelSizeY();
	G4double zVoxelSize = fSource->GetActivityMap()->GetVoxelSizeZ();

	TsPrimaryParticle p;
	p.particleDefinition = ion;

	// Mandatory in generator class
	fParticleDefinition = p.particleDefinition;
	fBeamEnergyParameterExists = "true";
	fEnergy = excitEnergy;

	G4Point3D selectedPoint = SampleVoxel();

	G4double costheta = G4RandFlat::shoot( -1., 1);
	G4double sintheta = sqrt(1. - costheta*costheta);
	G4double phi = 2.* CLHEP::pi * G4UniformRand();
	G4double sinphi = sin(phi);
	G4double cosphi = cos(phi);
	G4double px = sintheta * cosphi;
	G4double py = sintheta * sinphi;
	G4double pz = costheta;
	G4double mag = std::sqrt((px*px) + (py*py) + (pz*pz));

	p.dCos1 = px / mag;
	p.dCos2 = py / mag;
	p.dCos3 = pz / mag;
		
	G4double xShift = G4RandFlat::shoot( -1., 1) * xVoxelSize/2.0;
	G4double yShift = G4RandFlat::shoot( -1., 1) * yVoxelSize/2.0;
	G4double zShift = G4RandFlat::shoot( -1., 1) * zVoxelSize/2.0;

	p.posX = selectedPoint.x() + xShift;
	p.posY = selectedPoint.y() + yShift;
	p.posZ = selectedPoint.z() + zShift;

	SetEnergy(p);
	SetParticleType(p);

	p.weight = 1.;
	p.isNewHistory = true;

	GenerateOnePrimary(anEvent, p);

	AddPrimariesToEvent(anEvent);
}

G4Point3D TsGeneratorActivityMap::SampleVoxel()
{
	std::vector<G4Point3D> positions = fSource->GetActivityMap()->GetSourcePositions();
	std::vector<G4double> counts = fSource->GetActivityMap()->GetSourceCounts();

	G4double accum = 0;
	std::vector<G4double> accumulatedCounts;
	for (G4int i = 0; i < (G4int)counts.size(); i++)
	{
		accum += counts[i];
		accumulatedCounts.push_back(accum);
	}
	for (G4int i = 0; i < (G4int)accumulatedCounts.size(); i++)
		accumulatedCounts[i] /= accum;

	G4double rand = G4UniformRand();
	G4int iSelected = 0;
	for (G4int i = 0; i < (G4int)accumulatedCounts.size(); i++)
	{
		if (rand < accumulatedCounts[i])
		{
			iSelected = i;
			break;
		}
	}
	return positions[iSelected];
}

G4String TsGeneratorActivityMap::invertRadionuclideName(G4String radionuclideName)
{
	const size_t N = radionuclideName.length();
	G4String digits, alpha;
	bool firstDigit = false;
	for (G4int i = 0; i < (G4int)N; i++)
	{
		if (isDigitOrDot(radionuclideName[i]))
		{
			if (i == 0) firstDigit = true;
			digits += radionuclideName[i];
		}
		else
		{
			alpha += radionuclideName[i];
		}
	}
	if (firstDigit)
		return alpha + digits;
	else
		return digits + alpha;
}

bool TsGeneratorActivityMap::isDigitOrDot(const char c)
{
	bool res = true;
	switch (c)
	{
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	case '.':
		break;
	default:
		res = false;
	}
	return res;
}