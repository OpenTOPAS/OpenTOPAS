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

#include "TsSourcePhaseSpace.hh"

#include "TsParameterManager.hh"

#include "TsTopasConfig.hh"

#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"

#include <fstream>
#include <sys/stat.h>

#ifdef TOPAS_MT
#include "G4MTRunManager.hh"
#include "G4AutoLock.hh"

namespace {
	G4Mutex readSomeDataMutex = G4MUTEX_INITIALIZER;
}
#endif

TsSourcePhaseSpace::TsSourcePhaseSpace(TsParameterManager* pM, TsSourceManager* psM, G4String sourceName) :
TsSource(pM, psM, sourceName),
fRecordLength(0), fFileSize(0), fFilePosition(0), fAsciiLine(""), fIgnoreUnsupportedParticles(false),
fIncludeEmptyHistories(false), fNumberOfEmptyHistoriesToAppend(0), fNumberOfEmptyHistoriesAppended(0),
fPreviousHistoryWasEmpty(false), fMultipleUse(1),
fIsBinary(false), fIsLimited(false), fLimitedHasZ(true), fLimitedHasWeight(true),
fLimitedAssumePhotonIsNewHistory(false), fLimitedAssumeEveryParticleIsNewHistory(false),
fLimitedAssumeFirstParticleIsNewHistory(false),
fPreCheck(true), fPreCheckNumberOfHistories(0), fPreCheckNumberOfNonEmptyHistories(0), fPreCheckNumberOfParticles(0),
fPreCheckShowParticleCountAtInterval(1000000),
fHeaderNumberOfHistories(0), fHeaderNumberOfNonEmptyHistories(0), fHeaderNumberOfParticles(0),
fPhaseSpaceScaleXPosBy(1.0), fPhaseSpaceScaleYPosBy(1.0), fPhaseSpaceScaleZPosBy(1.0),
fPhaseSpaceInvertXAxis(false), fPhaseSpaceInvertYAxis(false), fPhaseSpaceInvertZAxis(false)
{
	fFileName = fPm->GetStringParameter(GetFullParmName("PhaseSpaceFileName"));

    G4String headerFileSpec = fFileName+".header";
	std::ifstream headerFile(headerFileSpec);
	if (!headerFile) {
		G4cerr << "Error opening phase space header file:" << headerFileSpec << G4endl;
		fPm->AbortSession(1);
	}

	if (fPm->ParameterExists(GetFullParmName("PreCheckShowParticleCountAtInterval")))
		fPreCheckShowParticleCountAtInterval = fPm->GetIntegerParameter(GetFullParmName("PreCheckShowParticleCountAtInterval"));

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
			input >> fHeaderNumberOfHistories;
			hasTag2 = true;
		}

		// Read total number of particles
		if (aLine.find("$PARTICLES:")!=std::string::npos)
		{
			getline(headerFile,aLine);
			std::istringstream input(aLine);
			input >> fHeaderNumberOfParticles;
			hasTag3 = true;
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
		G4cout << "\nPhase Space file header indicates phase space is in Limited form." << G4endl;

		if (fPm->ParameterExists(GetFullParmName("LimitedAssumePhotonIsNewHistory")) &&
			fPm->GetBooleanParameter(GetFullParmName("LimitedAssumePhotonIsNewHistory")))
				fLimitedAssumePhotonIsNewHistory = true;

		if (fPm->ParameterExists(GetFullParmName("LimitedAssumeEveryParticleIsNewHistory")) &&
			fPm->GetBooleanParameter(GetFullParmName("LimitedAssumeEveryParticleIsNewHistory")))
				fLimitedAssumeEveryParticleIsNewHistory = true;

		if (fPm->ParameterExists(GetFullParmName("LimitedAssumeFirstParticleIsNewHistory")) &&
			fPm->GetBooleanParameter(GetFullParmName("LimitedAssumeFirstParticleIsNewHistory")))
				fLimitedAssumeFirstParticleIsNewHistory = true;
	} else {
		headerFile.open(headerFileSpec);

		getline(headerFile,aLine);
		if (aLine == "TOPAS Binary Phase Space") {
			fIsBinary = true;
			G4cout << "\nPhase Space file header indicates phase space is in TOPAS Binary form." << G4endl;
		} else if (aLine == "TOPAS ASCII Phase Space") {
			G4cout << "\nPhase Space file header indicates phase space is in TOPAS ASCII form." << G4endl;
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
		input >> fHeaderNumberOfHistories;

		getline(headerFile,aLine);
		if (aLine.substr(0,54) != "Number of Original Histories that Reached Phase Space:") {
			G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
			G4cerr << "Fourth line of header should start with \"Number of Original Histories that Reached Phase Space:\"" << G4endl;
			fPm->AbortSession(1);
		}
		std::istringstream input2(aLine.substr(54));
		input2 >> fHeaderNumberOfNonEmptyHistories;

		getline(headerFile,aLine);
		if (aLine.substr(0,27) != "Number of Scored Particles:") {
			G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
			G4cerr << "Fifth line of header should start with \"Number of Scored Particles:\"" << G4endl;
			fPm->AbortSession(1);
		}
		std::istringstream input3(aLine.substr(27));
		input3 >> fHeaderNumberOfParticles;

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

    G4String dataFileSpec = fFileName+".phsp";
    fFileSize = GetFileSize(dataFileSpec);

	if (fPm->ParameterExists(GetFullParmName("PhaseSpaceBufferSize"))) {
        G4cout << "Note: the parameter PhaseSpaceBufferSize is no longer used." << G4endl;
        G4cout << "Buffer size will always be set sufficient to hold number of histories assigned" << G4endl;
        G4cout << "to the worker thread (the Geant4 eventModulo)." << G4endl;
	}

	if (fPm->ParameterExists(GetFullParmName("IgnoreUnsupportedParticles")))
		fIgnoreUnsupportedParticles = fPm->GetBooleanParameter(GetFullParmName("IgnoreUnsupportedParticles"));

	if (fPm->ParameterExists(GetFullParmName("PhaseSpacePreCheck")))
		fPreCheck = fPm->GetBooleanParameter(GetFullParmName("PhaseSpacePreCheck"));

	if (fPm->ParameterExists(GetFullParmName("PhaseSpaceIncludeEmptyHistories")))
		fIncludeEmptyHistories = fPm->GetBooleanParameter(GetFullParmName("PhaseSpaceIncludeEmptyHistories"));

	if (fPreCheck) {
        G4cout << "Phase Space Reader performing PreCheck on file: " << fFileName << G4endl;

        // Null buffer is signal to method that we are doing PreCheck
		ReadSomeDataFromFileToBuffer(NULL);

        // Limited format header does not provide number of non-empty histories. So get from PreCheck.
        if (fIsLimited)
            fHeaderNumberOfNonEmptyHistories = fPreCheckNumberOfNonEmptyHistories;

		// Some phase space files do not contain the pseudo-particles that represent empty histories.
		// In such cases, we will have to add these empty histories at the end of the input from the file.
		G4long headerNumberOfEmptyHistories = fHeaderNumberOfHistories - fHeaderNumberOfNonEmptyHistories;
		G4long preCheckNumberOfEmptyHistories = fPreCheckNumberOfHistories - fPreCheckNumberOfNonEmptyHistories;
		if (fIncludeEmptyHistories && (preCheckNumberOfEmptyHistories == 0) && (headerNumberOfEmptyHistories !=0)) {
			G4cout << "Will append " << headerNumberOfEmptyHistories << " empty histories at end of the input from the file."  << G4endl;
			fNumberOfEmptyHistoriesToAppend = headerNumberOfEmptyHistories;
		}

        if (fIncludeEmptyHistories && (fHeaderNumberOfHistories != (fPreCheckNumberOfHistories + fNumberOfEmptyHistoriesToAppend))) {
			G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
            G4cerr << "Number of Original Histories listed in the header does not match the number of histories" << G4endl;
            G4cerr << "counted in the actual file (plus any empty histories to append)." << G4endl;
			G4cerr << "HeaderNumberOfHistories: " << fHeaderNumberOfHistories
				<< ", PreCheckNumberOfHistories: " << fPreCheckNumberOfHistories
				<< ", NumberOfEmptyHistoriesToAppend: " << fNumberOfEmptyHistoriesToAppend << G4endl;
          fPm->AbortSession(1);
        }

       if (fHeaderNumberOfNonEmptyHistories != fPreCheckNumberOfNonEmptyHistories) {
            G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
            G4cerr << "Number of Orginal Histories that Reached Phase Space listed in the header does not match" << G4endl;
            G4cerr << "the number of non-empty histories counted in the actual file." << G4endl;
		    G4cerr << "HeaderNumberOfNonEmptyHistories: " << fHeaderNumberOfNonEmptyHistories
				<< ", PreCheckNumberOfNonEmptyHistories: " << fPreCheckNumberOfNonEmptyHistories << G4endl;
            fPm->AbortSession(1);
        }

        if (fHeaderNumberOfParticles != fPreCheckNumberOfParticles) {
            G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
            G4cerr << "Number of Scored Particles listed in the header does not match the" << G4endl;
            G4cerr << "number of particles counted in the actual file." << G4endl;
			G4cerr << "HeaderNumberOfParticles: " << fHeaderNumberOfParticles
				<< ", PreCheckNumberOfParticles: " << fPreCheckNumberOfParticles << G4endl;
           fPm->AbortSession(1);
        }

		G4cout << "Phase Space Reader PreCheck showed no problems." << G4endl;
	} else {
		if (fIncludeEmptyHistories) {
			G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
			G4cerr << "PhaseSpacePreCheck can not be turned off if you have IncludeEmptyHistories." << G4endl;
			G4cerr << "TOPAS needs the PreCheck to tell whether the empty histories are included in the phsp" << G4endl;
			G4cerr << "or are to be appended at the end of the file read." << G4endl;
			fPm->AbortSession(1);
		}

		if (fIsLimited)
			fHeaderNumberOfNonEmptyHistories = fHeaderNumberOfHistories;
	}

	ResolveParameters();
}


TsSourcePhaseSpace::~TsSourcePhaseSpace()
{
}


void TsSourcePhaseSpace::ResolveParameters() {
	TsSource::ResolveParameters();

	if (fPm->ParameterExists(GetFullParmName("PhaseSpaceScaleXPosBy"))) {
		fPhaseSpaceScaleXPosBy = fPm->GetUnitlessParameter(GetFullParmName("PhaseSpaceScaleXPosBy"));

		if (fPhaseSpaceScaleXPosBy < 0) {
			G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
			G4cerr << "PhaseSpaceScaleXPosBy cannot be negative." << G4endl;
			fPm->AbortSession(1);
		}
	} else {
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
	} else {
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
	} else {
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
	if (fMultipleUse > 0) {
		if (fIncludeEmptyHistories)
			fNumberOfHistoriesInRun = fHeaderNumberOfHistories * fMultipleUse;
		else
			fNumberOfHistoriesInRun = fHeaderNumberOfNonEmptyHistories * fMultipleUse;
	} else if (fMultipleUse == 0) {
		if (fNumberOfEmptyHistoriesToAppend > 0) {
			G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
			G4cerr << "You have asked to include empty histories, and your header file tells us that there are some empty histories." << G4endl;
			G4cerr << "But since the empty histories are not actually present in your phsp file, we can't tell where in the" << G4endl;
			G4cerr << "history sequence they would belong." << G4endl;
			G4cerr << "We can only proceed if you either:" << G4endl;
			G4cerr << "a) set PhaseSpaceMultipleUse to a non-zero value, so we know to use the full set of histories." << G4endl;
			G4cerr << "or b) turn off use of empty histories." << G4endl;
			G4cerr << "or c) use a phsp file that actually includes the emtpy histories." << G4endl;
			fPm->AbortSession(1);
		}
	} else {
		G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
		G4cerr << "Negative values are not allowed for the parameter: " << GetFullParmName("PhaseSpaceMultipleUse") << G4endl;
		fPm->AbortSession(1);
	}

	if (fNumberOfHistoriesInRun > 1E9) {
		G4cerr << "TOPAS is quitting due to error in specification of particle source: " << GetName() << G4endl;
		G4cerr << "The source is attempting to do a run of: " << fNumberOfHistoriesInRun << " histories." << G4endl;
		G4cerr << "This exceeds the limit of 1E9." << G4endl;
		fPm->AbortSession(1);
	}
}


void TsSourcePhaseSpace::ReadSomeDataFromFileToBuffer(std::queue<TsPrimaryParticle>* particleBuffer)
{
#ifdef TOPAS_MT
    G4AutoLock l(&readSomeDataMutex);
#endif

    G4String dataFileSpec = fFileName+".phsp";
    fDataFile.open(dataFileSpec);
    if (!fDataFile) {
        G4cerr << "Error opening phase space data file:" << dataFileSpec << G4endl;
        fPm->AbortSession(1);
    }

    // Read one particle if this is the first read from the file.
    // Otherwise, we will already have one left over from last read operation.
    if (fFilePosition == 0) {
        ReadOneParticle(particleBuffer);
		if (fLimitedAssumeFirstParticleIsNewHistory)
			fPrimaryParticle.isNewHistory = true;
    } else {
        // Advance the seek pointer to next unread position in the file
        fDataFile.seekg(fFilePosition);
    }

    if (!fDataFile.good()) {
        G4cerr << "Problem after attempt to seek file position: " << fFilePosition << G4endl;
        fPm->AbortSession(1);
    }

    if (!fPrimaryParticle.isNewHistory) {
        G4cerr << "Error in phase space file: " << dataFileSpec << "." << G4endl;
        G4cerr << "First particle does not have the New History flag set." << G4endl;
		G4cerr << "We believe this should be forbidden in the Limited format," << G4endl;
		G4cerr << "but we have seen some files that do not have any New History flags." << G4endl;
		G4cerr << "We recommend against using this file." << G4endl;
		G4cerr << "But, depending what is really wrong with the file," << G4endl;
		G4cerr << "you may be able to get it to work by setting one or more of the following:" << G4endl;
		G4cerr << "b:So/" << GetName() << "/LimitedAssumeFirstParticleIsNewHistory = \"True\"" << G4endl;
		G4cerr << "b:So/" << GetName() << "/LimitedAssumeEveryParticleIsNewHistory = \"True\"" << G4endl;
		G4cerr << "b:So/" << GetName() << "/LimitedAssumePhotonIsNewHistory = \"True\"" << G4endl;
        fPm->AbortSession(1);
    }

    // Read entire file if just doing precheck. Otherwise read eventModulo histories.
    G4int bufferSize;
    if (particleBuffer) {
#ifdef TOPAS_MT
		if (G4MTRunManager::GetMasterRunManager()->GetNumberOfThreads() == 1)
			bufferSize = 10000;
		else
			bufferSize = G4MTRunManager::GetMasterRunManager()->GetEventModulo();
#else
		bufferSize = 10000;
#endif
    } else {
        bufferSize = INT_MAX;
    }

    G4int nHistoriesRead = 1;

    while ((nHistoriesRead <= bufferSize) &&
		   ((fDataFile.tellg() != -1) && ((fDataFile.tellg() <= (fFileSize-fRecordLength)) || (fNumberOfEmptyHistoriesAppended < fNumberOfEmptyHistoriesToAppend)))) {
        // Store particle to the buffer
        if (particleBuffer && (fIncludeEmptyHistories || fPrimaryParticle.particleDefinition != 0))
            particleBuffer->push(fPrimaryParticle);

		G4bool FileIsEmpty = false;
		if ((fDataFile.tellg() != -1) && (fDataFile.tellg() <= (fFileSize-fRecordLength))) {
			// If weight is negative, this means we have an empty history.
			// If weight is less than -1, it represents more than one empty history.
			// Then instead of reading another particle, just increment the weight to account for having already
			// represented one of these empty histories.
			if (fPrimaryParticle.weight < -1.)
				fPrimaryParticle.weight+= 1.;
			else
				FileIsEmpty = ReadOneParticle(particleBuffer);
		} else {
			fPrimaryParticle.particleDefinition = 0;
			fNumberOfEmptyHistoriesAppended++;
		}

        // If we're read the last particle in the file, close out last history
		if ((fDataFile.tellg() == -1) || FileIsEmpty || ((fDataFile.tellg() > (fFileSize-fRecordLength)) && (fNumberOfEmptyHistoriesAppended == fNumberOfEmptyHistoriesToAppend))) {
            if (particleBuffer && (fIncludeEmptyHistories || fPrimaryParticle.particleDefinition != 0))
                particleBuffer->push(fPrimaryParticle);
            nHistoriesRead++;
        } else {
            if (fPrimaryParticle.isNewHistory && (fIncludeEmptyHistories || fPrimaryParticle.particleDefinition != 0))
                nHistoriesRead++;
        }
    }

    // Store file position to use at next read
	if ((fDataFile.tellg() == -1) || ((fDataFile.tellg() > (fFileSize-fRecordLength)) && (fNumberOfEmptyHistoriesAppended == fNumberOfEmptyHistoriesToAppend))) {
        fFilePosition = 0;
		fNumberOfEmptyHistoriesAppended = 0;
	} else
        fFilePosition = fDataFile.tellg();

    fDataFile.close();
}


G4bool TsSourcePhaseSpace::ReadOneParticle(std::queue<TsPrimaryParticle>* particleBuffer)
{
    G4int particleCode;
    G4bool cosZIsNegative;

    if (fIsLimited) {
        // Reading limited data
        char conflatedParticleCodeChar;
        fDataFile.read(reinterpret_cast<char*>(&conflatedParticleCodeChar), 1);
        G4int conflatedParticleCode = G4int(conflatedParticleCodeChar);
        cosZIsNegative = (conflatedParticleCode < 0);
        particleCode = abs(conflatedParticleCode);

        G4float conflatedEnergy;
        fDataFile.read(reinterpret_cast<char*>(&conflatedEnergy), sizeof conflatedEnergy);
        fPrimaryParticle.isNewHistory = (conflatedEnergy < 0.);

        if (fLimitedAssumeEveryParticleIsNewHistory ||
			(fLimitedAssumePhotonIsNewHistory && particleCode == 1))
            fPrimaryParticle.isNewHistory = true;

		fPrimaryParticle.kEnergy = fabs(conflatedEnergy);

		fDataFile.read(reinterpret_cast<char*>(&fPrimaryParticle.posX),    sizeof fPrimaryParticle.posX);
		fDataFile.read(reinterpret_cast<char*>(&fPrimaryParticle.posY),    sizeof fPrimaryParticle.posY);

		if (fLimitedHasZ)
			fDataFile.read(reinterpret_cast<char*>(&fPrimaryParticle.posZ),    sizeof fPrimaryParticle.posZ);
		else
			fPrimaryParticle.posZ = 0.;

		fDataFile.read(reinterpret_cast<char*>(&fPrimaryParticle.dCos1), sizeof fPrimaryParticle.dCos1);
		fDataFile.read(reinterpret_cast<char*>(&fPrimaryParticle.dCos2), sizeof fPrimaryParticle.dCos2);

		if (fLimitedHasWeight)
			fDataFile.read(reinterpret_cast<char*>(&fPrimaryParticle.weight),    sizeof fPrimaryParticle.weight);
		else
			fPrimaryParticle.weight = 1.;

        // Ignore any additional parts of record
        // Advance to next particle record in file
        fFilePosition+=fRecordLength;
        fDataFile.seekg(fFilePosition);
    } else if (fIsBinary) {
        // Reading binary data
        fDataFile.read(reinterpret_cast<char*>(&fPrimaryParticle.posX),         sizeof fPrimaryParticle.posX);
        fDataFile.read(reinterpret_cast<char*>(&fPrimaryParticle.posY),         sizeof fPrimaryParticle.posY);
        fDataFile.read(reinterpret_cast<char*>(&fPrimaryParticle.posZ),         sizeof fPrimaryParticle.posZ);
        fDataFile.read(reinterpret_cast<char*>(&fPrimaryParticle.dCos1),        sizeof fPrimaryParticle.dCos1);
        fDataFile.read(reinterpret_cast<char*>(&fPrimaryParticle.dCos2),        sizeof fPrimaryParticle.dCos2);
        fDataFile.read(reinterpret_cast<char*>(&fPrimaryParticle.kEnergy),      sizeof fPrimaryParticle.kEnergy);
        fDataFile.read(reinterpret_cast<char*>(&fPrimaryParticle.weight),       sizeof fPrimaryParticle.weight);
        fDataFile.read(reinterpret_cast<char*>(&particleCode),                  sizeof particleCode);
        fDataFile.read(reinterpret_cast<char*>(&cosZIsNegative),                sizeof cosZIsNegative);
        fDataFile.read(reinterpret_cast<char*>(&fPrimaryParticle.isNewHistory), sizeof fPrimaryParticle.isNewHistory);

        // Ignore any additional parts of record
        // Advance to next particle record in file
        fFilePosition+=fRecordLength;
        fDataFile.seekg(fFilePosition);
    } else {
        // Reading ASCII data
        getline(fDataFile,fAsciiLine);
        if (!fDataFile.good()) return true;
        std::istringstream input(fAsciiLine);
        input >> fPrimaryParticle.posX >> fPrimaryParticle.posY >> fPrimaryParticle.posZ >> fPrimaryParticle.dCos1 >> fPrimaryParticle.dCos2
        >> fPrimaryParticle.kEnergy >> fPrimaryParticle.weight >> particleCode >> cosZIsNegative >> fPrimaryParticle.isNewHistory;
    }

    // Lack of particle buffer means we are doing PreCheck
    if (!particleBuffer) {
		if (fPrimaryParticle.weight >= 0.) {
            fPreCheckNumberOfParticles++;
			if (fPreCheckShowParticleCountAtInterval!=0 && std::fmod(fPreCheckNumberOfParticles, fPreCheckShowParticleCountAtInterval)==0)
				G4cout << "PreCheck processing particle: " << fPreCheckNumberOfParticles << G4endl;
		}

        if (fPrimaryParticle.isNewHistory) {
            if (fPrimaryParticle.weight < 0.) {
                fPreviousHistoryWasEmpty = true;
				fPreCheckNumberOfHistories -= fPrimaryParticle.weight;
			} else {
				fPreCheckNumberOfHistories++;
                fPreCheckNumberOfNonEmptyHistories++;
                fPreviousHistoryWasEmpty = false;
			}
        } else {
            if (fPrimaryParticle.weight < 0.) {
                G4cerr << "Error reading phase space file." << G4endl;
                G4cerr << "A particle has been read with a negative weight but no IsNewHistory flag." << G4endl;
				G4cerr << "Negative weight is used to represent one or more empty histories," << G4endl;
				G4cerr << "so must always have the IsNewHistory flag." << G4endl;
                fPm->AbortSession(1);
            }
            if (fPreviousHistoryWasEmpty) {
                G4cerr << "Error reading phase space file." << G4endl;
                G4cerr << "Read a particle that does not have the IsNewHistory flag" << G4endl;
                G4cerr << "right after reading an empty history. This does not make sense." << G4endl;
                fPm->AbortSession(1);
            }
        }

		if (particleCode > 999999999 && (particleCode % 10) != 0 && !fPm->GetBooleanParameter("Ts/TreatExcitedIonsAsGroundState"))
		{
			G4cerr << "A phase space input file or filter parameter is using a PDG" << G4endl;
			G4cerr << "particle code that corresponds to an ion in an excited state." << G4endl;
			G4cerr << "This is any ten digit PDG code that does not end in a zero." << G4endl;
			G4cerr << "The PDG code seen here was: " << particleCode << G4endl;
			G4cerr << "TOPAS can only handle such ions by treating them as ground state." << G4endl;
			G4cerr << "To accept this compromise, set" << G4endl;
			G4cerr << "Ts/TreatExcitedIonsAsGroundState to True." << G4endl;
			fPm->AbortSession(1);
		}
	}

	if (fPrimaryParticle.weight < 0.) {
		fPrimaryParticle.particleDefinition = 0;
	} else {
		fPrimaryParticle.posX = fPrimaryParticle.posX * fPhaseSpaceScaleXPosBy * cm;
		fPrimaryParticle.posY = fPrimaryParticle.posY * fPhaseSpaceScaleYPosBy * cm;
		fPrimaryParticle.posZ = fPrimaryParticle.posZ * fPhaseSpaceScaleZPosBy * cm;

		// Calculate Z direction cosine.
		// Note need to protect against round-off error making dCos3 imaginary.
		G4double zCosSquared = 1. - fPrimaryParticle.dCos1*fPrimaryParticle.dCos1 - fPrimaryParticle.dCos2*fPrimaryParticle.dCos2;
		if (zCosSquared < 0.)
			fPrimaryParticle.dCos3 = 0.;
		else
			fPrimaryParticle.dCos3 = sqrt(zCosSquared);

		if (cosZIsNegative) fPrimaryParticle.dCos3 *= -1.;

		// Invert coordinates if requested
		if (fPhaseSpaceInvertXAxis) {
			fPrimaryParticle.posX *= -1.;
			fPrimaryParticle.dCos1 *= -1.;
		}
		if (fPhaseSpaceInvertYAxis) {
			fPrimaryParticle.posY *= -1.;
			fPrimaryParticle.dCos2 *= -1.;
		}
		if (fPhaseSpaceInvertZAxis) {
			fPrimaryParticle.posZ *= -1.;
			fPrimaryParticle.dCos3 *= -1.;
		}

		// Correct units of energy
		fPrimaryParticle.kEnergy *= MeV;

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
						return false;
					} else {
						G4cerr << "TOPAS is quitting due to a serious error in specification of particle source: " << GetName() << G4endl;
						G4cerr << "\"limited\" format phase space does not support particle ID: " << particleCode << G4endl;
						fPm->AbortSession(1);
					}
			}
		}

		TsParticleDefinition resolvedDef = fPm->GetParticleDefinition(G4UIcommand::ConvertToString(particleCode));

		if (!resolvedDef.particleDefinition) {
			G4cerr << "Unknown particle type read from phase space file, particle code = " << particleCode << G4endl;
			fPm->AbortSession(1);
		}

		fPrimaryParticle.particleDefinition = resolvedDef.particleDefinition;

		fPrimaryParticle.isOpticalPhoton = resolvedDef.isOpticalPhoton;
		fPrimaryParticle.isGenericIon = resolvedDef.isGenericIon;
		fPrimaryParticle.ionCharge = resolvedDef.ionCharge;
	}
	return false;
}


G4long TsSourcePhaseSpace::GetFileSize(std::string filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}
