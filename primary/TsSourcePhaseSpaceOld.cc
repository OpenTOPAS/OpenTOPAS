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

#include "TsSourcePhaseSpaceOld.hh"

#include "TsParameterManager.hh"
#include "TsSequenceManager.hh"

#include "TsTopasConfig.hh"

#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"

#include <fstream>

#ifdef TOPAS_MT
#include "G4MTRunManager.hh"
#include "G4AutoLock.hh"

namespace {
	G4Mutex readSomeDataMutex = G4MUTEX_INITIALIZER;
}
#endif

TsSourcePhaseSpaceOld::TsSourcePhaseSpaceOld(TsParameterManager* pM, TsSourceManager* psM, G4String sourceName) :
TsSource(pM, psM, sourceName),
fIsBinary(false), fIsLimited(false), fLimitedHasZ(true), fLimitedHasWeight(true), fLimitedAssumePhotonIsNewHistory(false),
fRecordLength(0), fNumberOfOriginalHistories(0), fNumberOfHistoriesInFile(0), fNumberOfParticlesInFile(0),
fPreCheckNumberOfParticlesInFile(0), fPreCheckNumberOfHistoriesInFile(0), fMultipleUse(1),
fNumberOfHistoriesToReadFromFile(0), fRemainingNumberOfParticlesInFile(0), fPhaseSpaceBufferSize(10000),
fPhaseSpaceScaleXPosBy(1.0), fPhaseSpaceScaleYPosBy(1.0), fPhaseSpaceScaleZPosBy(1.0),
fPhaseSpaceInvertXAxis(false), fPhaseSpaceInvertYAxis(false), fPhaseSpaceInvertZAxis(false),
fIgnoreUnsupportedParticles(false), fIncludeEmptyHistories(false), fPreCheck(true), fFilePosition(0)
{
	fFileName = fPm->GetStringParameter(GetFullParmName("PhaseSpaceFileName"));

	G4String headerFileSpec = fFileName+".header";
	std::ifstream headerFile(headerFileSpec);
	if (!headerFile) {
		G4cerr << "Error opening phase space header file:" << headerFileSpec << G4endl;
		fPm->AbortSession(1);
	}

	// If header contains $RECORD_LENGTH:, $ORIG_HISTORIES: and $PARTICLES:
	// will assume this is a phase space is Limited format.
	// Otherwise, will expect first line to tell whether it is TOPAS ASCII or TOPAS Binary.
	G4bool hasTag1 = false;
	G4bool hasTag2 = false;
	G4bool hasTag3 = false;
	G4String aLine;
	while (headerFile.good()) {
		getline(headerFile,aLine);

		// Read record length
		if (aLine.find("$RECORD_LENGTH:")!=std::string::npos)
		{
			getline(headerFile,aLine);
			std::istringstream input(aLine);
			input >> fRecordLength;
			hasTag1 = true;
		}

		// Read number of original histories
		if (aLine.find("$ORIG_HISTORIES:")!=std::string::npos)
		{
			getline(headerFile,aLine);
			std::istringstream input(aLine);
			input >> fNumberOfOriginalHistories;
			hasTag2 = true;
			G4cout << "fNumberOfOriginalHistories: " << fNumberOfOriginalHistories << G4endl;
		}

		// Read total number of particles
		if (aLine.find("$PARTICLES:")!=std::string::npos)
		{
			getline(headerFile,aLine);
			std::istringstream input(aLine);
			input >> fNumberOfParticlesInFile;
			hasTag3 = true;
			G4cout << "fNumberOfParticlesInFile: " << fNumberOfParticlesInFile << G4endl;
		}

		// See if Z position is included
		if (aLine.find("Z is stored")!=std::string::npos &&
			aLine.find("0")!=std::string::npos)
			fLimitedHasZ = false;

		// See if Weight is included
		if (aLine.find("Weight is stored")!=std::string::npos &&
			aLine.find("0")!=std::string::npos)
			fLimitedHasWeight = false;
	}

	headerFile.close();

	if (hasTag1 && hasTag2 && hasTag3) {
		fIsLimited = true;
		G4cout << "Phase Space file header indicates phase space is in Limited form." << G4endl;

		if (fPm->ParameterExists(GetFullParmName("LimitedAssumePhotonIsNewHistory")) &&
			fPm->GetBooleanParameter(GetFullParmName("LimitedAssumePhotonIsNewHistory")))
				fLimitedAssumePhotonIsNewHistory = true;
	} else {
		headerFile.open(headerFileSpec);

		getline(headerFile,aLine);
		if (aLine == "TOPAS Binary Phase Space") {
			fIsBinary = true;
			G4cout << "Phase Space file header indicates phase space is in TOPAS Binary form." << G4endl;
		} else if (aLine == "TOPAS ASCII Phase Space") {
			G4cout << "Phase Space file header indicates phase space is in TOPAS ASCII form." << G4endl;
		} else {
			G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
			G4cerr << "Phase Space header file is either empty or has unrecognized form." << G4endl;
			fPm->AbortSession(1);
		}

		getline(headerFile,aLine);

		getline(headerFile,aLine);
		if (aLine.substr(0,29) != "Number of Original Histories:") {
			G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
			G4cerr << "Third line of header should start with \"Number of Original Histories:\"" << G4endl;
			fPm->AbortSession(1);
		}
		std::istringstream input(aLine.substr(29));
		input >> fNumberOfOriginalHistories;

		getline(headerFile,aLine);
		if (aLine.substr(0,54) != "Number of Original Histories that Reached Phase Space:") {
			G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
			G4cerr << "Fourth line of header should start with \"Number of Original Histories that Reached Phase Space:\"" << G4endl;
			fPm->AbortSession(1);
		}
		std::istringstream input2(aLine.substr(54));
		input2 >> fNumberOfHistoriesInFile;

		getline(headerFile,aLine);
		if (aLine.substr(0,27) != "Number of Scored Particles:") {
			G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
			G4cerr << "Fifth line of header should start with \"Number of Scored Particles:\"" << G4endl;
			fPm->AbortSession(1);
		}
		std::istringstream input3(aLine.substr(27));
		input3 >> fNumberOfParticlesInFile;

		if (fIsBinary) {
			getline(headerFile,aLine);
			if (aLine.substr(0,29) != "Number of Bytes per Particle:") {
				G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
				G4cerr << "Sixth line of header should start with \"Number of Bytes per Particle:\"" << G4endl;
				fPm->AbortSession(1);
			}
			std::istringstream input4(aLine.substr(29));
			input4 >> fRecordLength;
		}

		headerFile.close();
	}

	if (fPm->ParameterExists(GetFullParmName("PhaseSpaceBufferSize"))) {
		fPhaseSpaceBufferSize = fPm->GetIntegerParameter(GetFullParmName("PhaseSpaceBufferSize"));

		if (fPhaseSpaceBufferSize < 1) {
			G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
			G4cerr << "Buffer size has been specifed as zero or negative." << G4endl;
			fPm->AbortSession(1);
		}
	}

	if (fPm->ParameterExists(GetFullParmName("IgnoreUnsupportedParticles")))
		fIgnoreUnsupportedParticles = fPm->GetBooleanParameter(GetFullParmName("IgnoreUnsupportedParticles"));

	if (fPm->ParameterExists(GetFullParmName("PhaseSpacePreCheck")))
		fPreCheck = fPm->GetBooleanParameter(GetFullParmName("PhaseSpacePreCheck"));

	if (fPm->ParameterExists(GetFullParmName("PhaseSpaceIncludeEmptyHistories")))
		fIncludeEmptyHistories = fPm->GetBooleanParameter(GetFullParmName("PhaseSpaceIncludeEmptyHistories"));

	if (fIsLimited && fIncludeEmptyHistories && !fPreCheck) {
		G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
		G4cerr << "PhaseSpaceIncludeEmptyHistories cannot be performed without PhaseSpacePreCheck" << G4endl;
		G4cerr << "since Limited format phase space header does not provide enough information." << G4endl;
		G4cerr << "Switch to one of the TOPAS ASCII or TOPAS Binary format or enable PhaseSpacePreCheck." << G4endl;
		fPm->AbortSession(1);
	}

	if (fPreCheck) {
		ReadSomeDataFromFileToBuffer(NULL);

		if (fPreCheckNumberOfParticlesInFile != fNumberOfParticlesInFile) {
			G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
			G4cerr << "Number of particles found in file during PreCheck does not match what the header listed as" << G4endl;
			G4cerr << "Number of Scored Particles" << G4endl;
			fPm->AbortSession(1);
		}

		if (fIsLimited) {
			// In Limited format, header does not provide fNumberOfHistoriesInFile, so we have to take it from precheck
			fNumberOfHistoriesInFile = fPreCheckNumberOfHistoriesInFile;
		} else if (fPreCheckNumberOfHistoriesInFile != fNumberOfHistoriesInFile) {
			G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
			G4cerr << "Number of histories found in file during PreCheck does not match what the header listed as" << G4endl;
			G4cerr << "Number of Original Histories that Reached Phase Space" << G4endl;
			fPm->AbortSession(1);
		}
	}

	fRemainingNumberOfParticlesInFile = fNumberOfParticlesInFile;

	ResolveParameters();
}


TsSourcePhaseSpaceOld::~TsSourcePhaseSpaceOld()
{
}


void TsSourcePhaseSpaceOld::ResolveParameters() {
	TsSource::ResolveParameters();

	if (fPm->ParameterExists(GetFullParmName("PhaseSpaceScaleXPosBy"))) {
		fPhaseSpaceScaleXPosBy = fPm->GetUnitlessParameter(GetFullParmName("PhaseSpaceScaleXPosBy"));

		if (fPhaseSpaceScaleXPosBy < 0) {
			G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
			G4cerr << "PhaseSpaceScaleXPosBy cannot be negative." << G4endl;
			fPm->AbortSession(1);
		}
	}
	else {
		if (fPm->ParameterExists(GetFullParmName("PhaseSpaceIgnoreXPos"))) {
			if (fPm->GetBooleanParameter(GetFullParmName("PhaseSpaceIgnoreXPos")))
				fPhaseSpaceScaleXPosBy = 0;

			G4cout << "PhaseSpaceIgnoreXPos is deprecated and will be removed in the next major release." << G4endl;
			G4cout << "Instead, please set PhaseSpaceScaleXPosBy = 0." << G4endl;
		}
	}

	if (fPm->ParameterExists(GetFullParmName("PhaseSpaceScaleYPosBy"))) {
		fPhaseSpaceScaleYPosBy = fPm->GetUnitlessParameter(GetFullParmName("PhaseSpaceScaleYPosBy"));

		if (fPhaseSpaceScaleYPosBy < 0) {
			G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
			G4cerr << "PhaseSpaceScaleYPosBy cannot be negative." << G4endl;
			fPm->AbortSession(1);
		}
	}
	else {
		if (fPm->ParameterExists(GetFullParmName("PhaseSpaceIgnoreYPos"))) {
			if (fPm->GetBooleanParameter(GetFullParmName("PhaseSpaceIgnoreYPos")))
				fPhaseSpaceScaleYPosBy = 0;

			G4cout << "PhaseSpaceIgnoreYPos is deprecated and will be removed in the next major release." << G4endl;
			G4cout << "Instead, please set PhaseSpaceScaleYPosBy = 0." << G4endl;
		}
	}

	if (fPm->ParameterExists(GetFullParmName("PhaseSpaceScaleZPosBy"))) {
		fPhaseSpaceScaleZPosBy = fPm->GetUnitlessParameter(GetFullParmName("PhaseSpaceScaleZPosBy"));

		if (fPhaseSpaceScaleZPosBy < 0) {
			G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
			G4cerr << "PhaseSpaceScaleZPosBy cannot be negative." << G4endl;
			fPm->AbortSession(1);
		}
	}
	else {
		if (fPm->ParameterExists(GetFullParmName("PhaseSpaceIgnoreZPos"))) {
			if (fPm->GetBooleanParameter(GetFullParmName("PhaseSpaceIgnoreZPos")))
				fPhaseSpaceScaleZPosBy = 0;

			G4cout << "PhaseSpaceIgnoreZPos is deprecated and will be removed in the next major release." << G4endl;
			G4cout << "Instead, please set PhaseSpaceScaleZPosBy = 0." << G4endl;
		}
	}

	if (fPm->ParameterExists(GetFullParmName("PhaseSpaceInvertXAxis")))
		fPhaseSpaceInvertXAxis = fPm->GetBooleanParameter(GetFullParmName("PhaseSpaceInvertXAxis"));

	if (fPm->ParameterExists(GetFullParmName("PhaseSpaceInvertYAxis")))
		fPhaseSpaceInvertYAxis = fPm->GetBooleanParameter(GetFullParmName("PhaseSpaceInvertYAxis"));

	if (fPm->ParameterExists(GetFullParmName("PhaseSpaceInvertZAxis")))
		fPhaseSpaceInvertZAxis = fPm->GetBooleanParameter(GetFullParmName("PhaseSpaceInvertZAxis"));

	if (fPm->ParameterExists(GetFullParmName("PhaseSpaceMultipleUse")))
		fMultipleUse = fPm->GetIntegerParameter(GetFullParmName("PhaseSpaceMultipleUse"));

	// fNumberOfHistoriesInRun is the number of times we want this source to be called.
	// fNumberOfHistoriesToReadFromFile is how many of those should be read from the file.
	// Any additional histories are requested Empty Histories,
	// histories that make up for Original Histories that never made it to the phase space plane,
	// but that the user still wants counted for statistical purposes.
	if (fMultipleUse > 0) {
		if (fIncludeEmptyHistories)
			fNumberOfHistoriesInRun = fNumberOfOriginalHistories * fMultipleUse;
		else
			fNumberOfHistoriesInRun = fNumberOfHistoriesInFile * fMultipleUse;
		fNumberOfHistoriesToReadFromFile = fNumberOfHistoriesInFile * fMultipleUse;
	} else if (fMultipleUse == 0) {
		if (fIncludeEmptyHistories && (fNumberOfHistoriesInFile != fNumberOfOriginalHistories)) {
			G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
			G4cerr << "Some Empty Histories exist for this phase space file, and you have asked to use them, " << G4endl;
			G4cerr << "but we cannot do this since " << GetFullParmName("PhaseSpaceMultipleUse") << " is zero." << G4endl;
			G4cerr << "There is no statistically valid way to handle empty histories unless you are using the" << G4endl;
			G4cerr << "entire phase space file an integral number of times." << G4endl;
			fPm->AbortSession(1);
		}

		fNumberOfHistoriesToReadFromFile = fNumberOfHistoriesInRun;
	} else {
		G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
		G4cerr << "Negative values are not allowed for the parameter: " << GetFullParmName("PhaseSpaceMultipleUse") << G4endl;
		fPm->AbortSession(1);
	}
	
	// Refuse to run if there are time features and empty histories.
	// Do not test on Ts/NumberOfSequentualTimes, since this may be more than 1 without there actually
	// being any time change (this is a way to work around Geant4's maximum number of histories per run).
	if ((fNumberOfHistoriesInFile != fNumberOfOriginalHistories) &&
		(fPm->GetDoubleParameter("Tf/TimelineStart", "Time") != fPm->GetDoubleParameter("Tf/TimelineEnd", "Time"))) {
		G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
		G4cerr << "The phase space file contains empty histories, while the job includes time features." << G4endl;
		G4cerr << "TOPAS can not know what time to associate with these empty histories." << G4endl;
		fPm->AbortSession(1);
	}
}


G4long TsSourcePhaseSpaceOld::GetNumberOfHistoriesToReadFromFile() {
	return fNumberOfHistoriesToReadFromFile;
}


G4bool TsSourcePhaseSpaceOld::ReadSomeDataFromFileToBuffer(std::queue<TsPrimaryParticle>* particleBuffer)
{
#ifdef TOPAS_MT
	G4AutoLock l(&readSomeDataMutex);
#endif

	G4String dataFileSpec = fFileName+".phsp";
	std::ifstream dataFile(dataFileSpec);
	if (!dataFile) {
		G4cout << "Error opening phase space data file:" << dataFileSpec << G4endl;
		fPm->AbortSession(1);
	}

	// Move to next unread position in the file
	dataFile.seekg(fFilePosition);

	G4long bufferSize = fPhaseSpaceBufferSize;

	if (fRemainingNumberOfParticlesInFile < bufferSize )
		bufferSize = fRemainingNumberOfParticlesInFile;

#ifdef TOPAS_MT
	if (G4MTRunManager::GetMasterRunManager()->GetNumberOfThreads() > 1)
		bufferSize = std::min(bufferSize, (G4long)(G4MTRunManager::GetMasterRunManager()->GetEventModulo()));
#endif

	if (!particleBuffer)
		bufferSize = INT_MAX;

	G4String aLine;

	if (dataFile.good()) {
		// Struct to hold one particle at a time
		TsPrimaryParticle p;

		G4int particleCode;
		G4bool cosZIsNegative;

		for (int i=0;i<bufferSize && dataFile.good();++i) {
			if (fIsLimited) {
				// Reading limited data
				char conflatedParticleCodeChar;
				dataFile.read(reinterpret_cast<char*>(&conflatedParticleCodeChar), 1);
				G4int conflatedParticleCode = G4int(conflatedParticleCodeChar);
				cosZIsNegative = (conflatedParticleCode < 0);
				particleCode = abs(conflatedParticleCode);

				G4float conflatedEnergy;
				dataFile.read(reinterpret_cast<char*>(&conflatedEnergy), sizeof conflatedEnergy);
				p.isNewHistory = (conflatedEnergy < 0.);

				if (fLimitedAssumePhotonIsNewHistory && conflatedParticleCode == 1)
					p.isNewHistory = true;

				p.kEnergy = fabs(conflatedEnergy);

				dataFile.read(reinterpret_cast<char*>(&p.posX),	sizeof p.posX);
				dataFile.read(reinterpret_cast<char*>(&p.posY),	sizeof p.posY);

				if (fLimitedHasZ)
					dataFile.read(reinterpret_cast<char*>(&p.posZ),	sizeof p.posZ);
				else
					p.posZ = 0.;

				dataFile.read(reinterpret_cast<char*>(&p.dCos1), sizeof p.dCos1);
				dataFile.read(reinterpret_cast<char*>(&p.dCos2), sizeof p.dCos2);

				if (fLimitedHasWeight)
					dataFile.read(reinterpret_cast<char*>(&p.weight),	sizeof p.weight);
				else
					p.weight = 1.;

				// Ignore any additional parts of record
				// Advance to next particle record in file
				fFilePosition+=fRecordLength;
				dataFile.seekg(fFilePosition);
			} else if (fIsBinary) {
				// Reading binary data
				dataFile.read(reinterpret_cast<char*>(&p.posX),			sizeof p.posX);
				dataFile.read(reinterpret_cast<char*>(&p.posY),			sizeof p.posY);
				dataFile.read(reinterpret_cast<char*>(&p.posZ),			sizeof p.posZ);
				dataFile.read(reinterpret_cast<char*>(&p.dCos1),		sizeof p.dCos1);
				dataFile.read(reinterpret_cast<char*>(&p.dCos2),		sizeof p.dCos2);
				dataFile.read(reinterpret_cast<char*>(&p.kEnergy),		sizeof p.kEnergy);
				dataFile.read(reinterpret_cast<char*>(&p.weight),		sizeof p.weight);
				dataFile.read(reinterpret_cast<char*>(&particleCode),	sizeof particleCode);
				dataFile.read(reinterpret_cast<char*>(&cosZIsNegative),	sizeof cosZIsNegative);
				dataFile.read(reinterpret_cast<char*>(&p.isNewHistory),	sizeof p.isNewHistory);

				// Temorarily ignoring rest of the record
				// Advance to next particle record in file
				fFilePosition+=fRecordLength;
				dataFile.seekg(fFilePosition);
			} else {
				// Reading ASCII data
				getline(dataFile,aLine);
				std::istringstream input(aLine);
				input >> p.posX >> p.posY >> p.posZ >> p.dCos1 >> p.dCos2
				>> p.kEnergy >> p.weight >> particleCode >> cosZIsNegative >> p.isNewHistory;
			}

			if (!particleBuffer) {
				if (dataFile.good()) {
					fPreCheckNumberOfParticlesInFile ++;
					fPreCheckNumberOfHistoriesInFile += p.isNewHistory;
				}
			} else {
				p.posX = p.posX * fPhaseSpaceScaleXPosBy * cm;
				p.posY = p.posY * fPhaseSpaceScaleYPosBy * cm;
				p.posZ = p.posZ * fPhaseSpaceScaleZPosBy * cm;

				// Calculate Z direction cosine.
				// Note need to protect against round-off error making dCos3 imaginary.
				G4double zCosSquared = 1. - p.dCos1*p.dCos1 - p.dCos2*p.dCos2;
				if (zCosSquared < 0.)
					p.dCos3 = 0.;
				else
					p.dCos3 = sqrt(zCosSquared);

				if (cosZIsNegative) p.dCos3 *= -1.;

				// Invert coordinates if requested
				if (fPhaseSpaceInvertXAxis) {
					p.posX *= -1.;
					p.dCos1 *= -1.;
				}
				if (fPhaseSpaceInvertYAxis) {
					p.posY *= -1.;
					p.dCos2 *= -1.;
				}
				if (fPhaseSpaceInvertZAxis) {
					p.posZ *= -1.;
					p.dCos3 *= -1.;
				}

				// Correct units of energy
				p.kEnergy *= MeV;

				// Particle definition
				if (fIsLimited) {
					switch(particleCode)
					{
						case 1:
							particleCode = 22;  // gamma
							break;
						case 2:
							particleCode = 11;  // electron
							break;
						case 3:
							particleCode = -11;  // positron
							break;
						case 4:
							particleCode = 2112;  // neutron
							break;
						case 5:
							particleCode = 2212;  // proton
							break;
						default:
							if (fIgnoreUnsupportedParticles) {
								continue;
							} else {
								G4cout << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
								G4cout << "\"limited\" format phase space does not support particle ID: " << particleCode << G4endl;
								fPm->AbortSession(1);
							}
					}
				}

				TsParticleDefinition resolvedDef = fPm->GetParticleDefinition(G4UIcommand::ConvertToString(particleCode));

				if (!resolvedDef.particleDefinition)
					G4cout << "Unknown particle type read from phase space file, particle code = " << particleCode << G4endl;

				p.particleDefinition = resolvedDef.particleDefinition;

				p.isOpticalPhoton = resolvedDef.isOpticalPhoton;
				p.isGenericIon = resolvedDef.isGenericIon;
				p.ionCharge = resolvedDef.ionCharge;

				// Store particle to the buffer
				particleBuffer->push(p);
				fRemainingNumberOfParticlesInFile--;
			}
		}
	}

	// Store current file position
	fFilePosition = dataFile.tellg();

	// If there are no more lines, reset counter and position to be ready for multiple use
	getline(dataFile,aLine);
	if (!dataFile.good()) {
		fRemainingNumberOfParticlesInFile = fNumberOfParticlesInFile;
		fFilePosition = 0;
	}

	dataFile.close();

	// Returns True if there is still more data in file
	return (int)fFilePosition != 0;
}
