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

#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include "TsVBinnedScorer.hh"

#include "TsParameterManager.hh"
#include "TsMaterialManager.hh"
#include "TsGeometryManager.hh"
#include "TsVGeometryComponent.hh"
#include "TsScoringManager.hh"

#include "TsDicomPatient.hh"
#include "TsOutcomeModelList.hh"
#include "TsTrackInformation.hh"

#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4Step.hh"
#include "G4VAnalysisManager.hh"
#include "G4VisAttributes.hh"
#if GEANT4_VERSION_MAJOR >= 11
#include "g4hntools_defs.hh"
#include "G4ToolsAnalysisManager.hh"
#else
#include "g4analysis_defs.hh"
#endif
#include "G4Tokenizer.hh"
#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"
#include "G4Transform3D.hh"

#include "gdcmImageWriter.h"
#include "gdcmImageReader.h"
#include "gdcmAnonymizer.h"
#include "gdcmMediaStorage.h"
#include "gdcmUIDGenerator.h"

#include <sstream>
#include <iomanip>

TsVBinnedScorer::TsVBinnedScorer(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
									   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer),
fNeedToUpdateFileSpecs(true), fOutFileSpec1(""), fOutFileSpec2(""),
fOutputToBinary(false), fOutputToCsv(false), fOutputToRoot(false), fOutputToXml(false), fOutputToDicom(false),
fVHOutFileSpec1(""), fVHOutFileSpec2(""),
fReferencedDicomPatient(NULL),
fUnitName("unspecified"), fUnitWasSet(false),
fNormFactor(1.), fNBins(1),
fHistogramID(0), fVHistogramID(0),
fBinByIncidentEnergy(false), fBinByPreStepEnergy(false), fBinByStepDepositEnergy(false), fBinByPrimaryEnergy(false),
fBinByTime(false), fBinLog(false), fNEorTBins(0), fBinMin(0.), fBinMax(0.), fBinWidth(0.),
fReportSum(false), fReportMean(false), fReportHistories(false), fReportCountInBin(false),
fReportSecondMoment(false), fReportVariance(false), fReportStandardDeviation(false), fReportMin(false), fReportMax(false),
fReportCVolHist(false), fReportDVolHist(false), fReportOutcome(0),
fAccumulateSecondMoment(false), fAccumulateMean(false), fAccumulateCount(false),
fNumberOfOutputColumns(0), fOutputPosition(0), fRestoreResultsFromFile(false), fNReadBackValues(0),
fReadBackHasSum(false), fReadBackHasMean(false), fReadBackHasHistories(false), fReadBackHasCountInBin(false),
fReadBackHasSecondMoment(false), fReadBackHasVariance(false), fReadBackHasStandardDeviation(false), fReadBackHasMin(false), fReadBackHasMax(false),
fColorBy(""), fColorByTotal(0), fSparsify(false), fSparsifyThreshold(0.), fSingleIndex(false),
fSumLimit(0.), fStandardDeviationLimit(0.), fRelativeSDLimit(0.), fCountLimit(0), fRepeatSequenceTestBin(0)
{
	if (fOutFileType == "binary") fOutputToBinary = true;
	else if (fOutFileType == "csv") fOutputToCsv = true;
	else if (fOutFileType == "root") fOutputToRoot = true;
	else if (fOutFileType == "xml") fOutputToXml = true;
	else if (fOutFileType == "dicom") fOutputToDicom = true;

	G4String* reportValues;

	if (fPm->ParameterExists(GetFullParmName("Report"))) {
		reportValues = fPm->GetStringVector(GetFullParmName("Report"));
		fNReportValues = fPm->GetVectorLength(GetFullParmName("Report"));
	} else {
		reportValues = new G4String[1];
		reportValues[0] = "Sum";
		fNReportValues = 1;
	}

	// Represent reportValues as a vector of interger codes for faster handling during i/o
	fReportValues = new G4int[fNReportValues];

	G4String reportValue;
	for (G4int i = 0; i < fNReportValues; i++) {
		reportValue = reportValues[i];
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(reportValue);
#else
		reportValue.toLower();
#endif
		if (reportValue == "sum") {
			fReportSum = true;
			fReportValues[i] = 0;
		} else if (reportValue == "mean") {
			fReportMean = true;
			fReportValues[i] = 1;
		} else if (reportValue == "histories") {
			fReportHistories = true;
			fReportValues[i] = 2;
		} else if (reportValue == "count_in_bin") {
			fReportCountInBin = true;
			fReportValues[i] = 3;
		} else if (reportValue == "second_moment") {
			fReportSecondMoment = true;
			fReportValues[i] = 4;
		} else if (reportValue == "variance") {
			fReportVariance = true;
			fReportValues[i] = 5;
		} else if (reportValue == "standard_deviation") {
			fReportStandardDeviation = true;
			fReportValues[i] = 6;
		} else if (reportValue == "min") {
			fReportMin = true;
			fReportValues[i] = 7;
		} else if (reportValue == "max") {
			fReportMax = true;
			fReportValues[i] = 8;
		} else if (reportValue == "cumulativevolumehistogram") {
			fReportCVolHist = true;
			fReportValues[i] = 9;
		} else if (reportValue == "differentialvolumehistogram") {
			fReportDVolHist = true;
			fReportValues[i] = 10;
		}else {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "The parameter Sc/" << GetName() << "/Report has an unknown option: " << reportValues[i] << G4endl;
			G4cerr << "Value should be one or more of the following:" << G4endl;
			G4cerr << "Sum, Mean, Histories, Count_In_Bin, Second_Moment," << G4endl;
			G4cerr << "Variance, Standard_Deviation, Min, Max," << G4endl;
			G4cerr << "CumulativeVolumeHistogram, DifferentialVolumeHistogram" << G4endl;
			fPm->AbortSession(1);
		}
		fNumberOfOutputColumns++;
	}

	if (fNumberOfOutputColumns==0) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "The scorer named: " << GetName() << " is not set to report anything." << G4endl;
		G4cerr << "Check the valueof the parameter Sc/" << GetName() << "/Report" << G4endl;
		fPm->AbortSession(1);
	}

	// Volume Histograms should not actually count as output columns
	if (fReportCVolHist || fReportDVolHist)
		fNumberOfOutputColumns--;

	// Cannot do both kinds of volume histogram in same scorer
	if (fReportCVolHist && fReportDVolHist) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "The scorer named: " << GetName() << " is set to report both" << G4endl;
		G4cerr << "CumulativeVolumeHistogram and DifferentialVolumeHistogram." << G4endl;
		G4cerr << "You cannot do both in the same scorer." << G4endl;
		fPm->AbortSession(1);
	}

	G4bool HaveSetALimit = false;

	G4String parmName = GetFullParmName("RepeatSequenceUntilSumGreaterThan");
	if (fPm->ParameterExists(parmName)) {
		if (fReportSum || fReportMean || fReportSecondMoment || fReportVariance || fReportStandardDeviation ||
			fReportCVolHist || fReportDVolHist) {
			G4String limitType = fPm->GetTypeOfParameter(parmName);
			if (limitType == "i") {
				fSumLimit = fPm->GetIntegerParameter(parmName);
			} else if (limitType == "u") {
				fSumLimit = fPm->GetUnitlessParameter(parmName);
			} else if (limitType == "d") {
				fSumLimit = fPm->GetDoubleParameter(parmName, fPm->GetUnitCategory(fPm->GetUnitOfParameter(parmName)));
			} else {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << parmName << " must be have type of either d, u or i." << G4endl;
				fPm->AbortSession(1);
			}
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "To use the parameter: " << parmName << G4endl;
			G4cerr << "this scorer's Report parameter must include at least one of:" << G4endl;
			G4cerr << "Sum, Mean, Second_Moment, Variance, Standard_Deviation," << G4endl;
			G4cerr << "CumulativeVolumeHistogram or DifferentialVolumeHistogram." << G4endl;
			fPm->AbortSession(1);
		}
		HaveSetALimit = true;
	}

	parmName = GetFullParmName("RepeatSequenceUntilStandardDeviationLessThan");
	if (fPm->ParameterExists(parmName)) {
		if (fReportSecondMoment || fReportVariance || fReportStandardDeviation) {
			G4String limitType = fPm->GetTypeOfParameter(parmName);
			if (limitType == "i") {
				fStandardDeviationLimit = fPm->GetIntegerParameter(parmName);
			} else if (limitType == "u") {
				fStandardDeviationLimit = fPm->GetUnitlessParameter(parmName);
			} else if (limitType == "d") {
				fStandardDeviationLimit = fPm->GetDoubleParameter(parmName, fPm->GetUnitCategory(fPm->GetUnitOfParameter(parmName)));
			} else {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << parmName << " must be have type of either d, u or i." << G4endl;
				fPm->AbortSession(1);
			}
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "To use the parameter: " << parmName << G4endl;
			G4cerr << "this scorer's Report parameter must include at least one of:" << G4endl;
			G4cerr << "Second_Moment, Variance or Standard_Deviation." << G4endl;
			fPm->AbortSession(1);
		}
		HaveSetALimit = true;
	}

	parmName = GetFullParmName("RepeatSequenceUntilRelativeStandardDeviationLessThan");
	if (fPm->ParameterExists(parmName)) {
		if (fReportSecondMoment || fReportVariance || fReportStandardDeviation) {
			fRelativeSDLimit = fPm->GetUnitlessParameter(parmName);

			if (fRelativeSDLimit < 0. || fRelativeSDLimit > 1.) {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << "Value of: " << parmName << " must be between 0 and 1." << G4endl;
				fPm->AbortSession(1);
			}
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "To use the parameter: " << parmName << G4endl;
			G4cerr << "this scorer's Report parameter must include at least one of:" << G4endl;
			G4cerr << "Second_Moment, Variance or Standard_Deviation." << G4endl;
			fPm->AbortSession(1);
		}
		HaveSetALimit = true;
	}

	parmName = GetFullParmName("RepeatSequenceUntilCountGreaterThan");
	if (fPm->ParameterExists(parmName)) {
		if (fReportCountInBin) {
			fCountLimit = fPm->GetIntegerParameter(parmName);
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "To use the parameter: " << parmName << G4endl;
			G4cerr << "this scorer's Report parameter must include Count_In_Bin." << G4endl;
			fPm->AbortSession(1);
		}
		HaveSetALimit = true;
	}

	if (HaveSetALimit && fOutputAfterRun & !fOutputAfterRunShouldAccumulate) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "The scorer: " << GetName() << G4endl;
		G4cerr << "has one of the RepeatSequence parameters set," << G4endl;
		G4cerr << "while it also has OutputAfterRun set to True." << G4endl;
		G4cerr << "This combination is only allowed if you also set this scorer" << G4endl;
		G4cerr << "to OutputAfterRunShouldAccumulate." << G4endl;
		G4cerr << "Otherwise, after each run, the scorer would be cleared" << G4endl;
		G4cerr << "so the limit would never be reached." << G4endl;
		fPm->AbortSession(1);
	}

	// Need histogram ID consistent with master thread for G4AnalysisManager to work
#ifdef TOPAS_MT
	if (G4Threading::IsWorkerThread()) {
		if (fOutputToRoot || fOutputToXml) {
			TsVBinnedScorer* masterScorer = dynamic_cast<TsVBinnedScorer*>(fScm->GetMasterScorerByID(fUID));
			fVHistogramID = masterScorer->fVHistogramID;
			fHistogramID  = masterScorer->fHistogramID;
		}
	}
#endif

	if (fPm->ParameterExists(GetFullParmName("Sparsify")))
		fSparsify = fPm->GetBooleanParameter(GetFullParmName("Sparsify"));

	if (fSparsify) {
		if (!fReportSum && !fReportMean && !fReportSecondMoment && !fReportVariance && !fReportStandardDeviation) {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "Sparsify can only be used if Report options include at least one of" << G4endl;
			G4cerr << "Sum, Mean, Second_Moment, Variance or Standard_Deviation" << G4endl;
			fPm->AbortSession(1);
		}
	}

	if (fPm->ParameterExists(GetFullParmName("SingleIndex")))
		fSingleIndex = fPm->GetBooleanParameter(GetFullParmName("SingleIndex"));

	fEvtMap = new std::map<G4int, G4double>;
}


TsVBinnedScorer::~TsVBinnedScorer()
{;}


void TsVBinnedScorer::GetAppropriatelyBinnedCopyOfComponent(G4String componentName)
{
	TsVScorer::GetAppropriatelyBinnedCopyOfComponent(componentName);

	// Counter to include FNEorTBins plus, for energy case, 2 or 3 extra columns
	G4int nEorTBinsTotal = 0;

	// See if energy bins are requested
	if (fPm->ParameterExists(GetFullParmName("EBins"))) {
		if (fPm->ParameterExists(GetFullParmName("EBinEnergy"))) {
			G4String EBinEnergy = fPm->GetStringParameter(GetFullParmName("EBinEnergy"));
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(EBinEnergy);
#else
			EBinEnergy.toLower();
#endif
			if (EBinEnergy == "incidenttrack")
				fBinByIncidentEnergy = true;
			else if (EBinEnergy == "prestep")
				fBinByPreStepEnergy = true;
			else if (EBinEnergy == "depositedinstep")
				fBinByStepDepositEnergy = true;
			else if (EBinEnergy == "primarytrack") {
				fBinByPrimaryEnergy = true;
				fPm->SetNeedsTrackingAction();
			} else {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << "The scorer named: " << GetName() << " has EBinEnergy set to an unknown value:" <<
					fPm->GetStringParameter(GetFullParmName("EBinEnergy"))<< G4endl;
				G4cerr << "Acceptable values are: IncidentTrack, PreStep, DepositedInStep or PrimaryTrack" << G4endl;
				fPm->AbortSession(1);
			}

		} else {
			fBinByIncidentEnergy = true;
		}

		fNEorTBins = fPm->GetIntegerParameter(GetFullParmName("EBins"));

		if (fNEorTBins < 0) {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "The scorer named: " << GetName() << " has EBins less than 0" << G4endl;
			fPm->AbortSession(1);
		} else if (fNEorTBins > 0) {
			if (fBinByIncidentEnergy)
				nEorTBinsTotal = fNEorTBins + 3;
			else
				nEorTBinsTotal = fNEorTBins + 2;

			// If energy bins are requested, max must be specified, while min is optional and defaults to zero
			if (fPm->ParameterExists(GetFullParmName("EBinMax")))
				fBinMax = fPm->GetDoubleParameter(GetFullParmName("EBinMax"), "Energy");
			else {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << "The scorer named: " << GetName() << " has energy binning defined with no EBinMax defined" << G4endl;
				fPm->AbortSession(1);
			}

			if (fPm->ParameterExists(GetFullParmName("EBinLog")))
				fBinLog = fPm->GetBooleanParameter(GetFullParmName("EBinLog"));

			if (fPm->ParameterExists(GetFullParmName("EBinMin")) || fBinLog)
				fBinMin = fPm->GetDoubleParameter(GetFullParmName("EBinMin"), "Energy");

			if (fBinMin > fBinMax) {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << "The scorer named: " << GetName() << " has EBinMin greater than EBinMax" << G4endl;
				fPm->AbortSession(1);
			} else {
				if ( !fBinLog ) {
					fBinWidth = (fBinMax - fBinMin) / fNEorTBins;
				} else {
					if ( fBinMin == 0 ) {
						G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
						G4cerr << "The scorer named: " << GetName() << " has EBinMin equal to zero and Log spacing is activated" << G4endl;
						fPm->AbortSession(1);
					}
					G4double logXMin = std::log10(fBinMin);
					G4double logXMax = std::log10(fBinMax);
					fBinWidth = (logXMax-logXMin) / fNEorTBins;
					for ( int i = 0; i < fNEorTBins; i++ )
						fTempEBins.push_back( std::pow(10, logXMin + i * fBinWidth) );
				}
			}
		}
	}

	// See if time bins are requested
	if (fPm->ParameterExists(GetFullParmName("TimeBins"))) {
		fNEorTBins = fPm->GetIntegerParameter(GetFullParmName("TimeBins"));

		if (fNEorTBins < 0) {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "The scorer named: " << GetName() << " has TimeBins less than 0" << G4endl;
			fPm->AbortSession(1);
		} else if (fNEorTBins > 0) {
			nEorTBinsTotal = fNEorTBins + 2;
			fBinByTime = true;

			if (fBinByIncidentEnergy || fBinByPreStepEnergy || fBinByStepDepositEnergy || fBinByPrimaryEnergy) {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << "The scorer named: " << GetName() << " has both EBins and TimeBins" << G4endl;
				G4cerr << "A given scorer can only have one or the other of these two options." << G4endl;
				fPm->AbortSession(1);
			}

			// If energy bins are requested, max must be specified, while min is optional and defaults to zero
			if (fPm->ParameterExists(GetFullParmName("TimeBinMax")))
				fBinMax = fPm->GetDoubleParameter(GetFullParmName("TimeBinMax"), "Time");
			else {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << "The scorer named: " << GetName() << " has time binning defined with no TimeBinMax defined" << G4endl;
				fPm->AbortSession(1);
			}

			if (fPm->ParameterExists(GetFullParmName("TimeBinMin")))
				fBinMin = fPm->GetDoubleParameter(GetFullParmName("TimeBinMin"), "Time");

			if (fBinMin > fBinMax) {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << "The scorer named: " << GetName() << " has TimeBinMin greater than TimeBinMax" << G4endl;
				fPm->AbortSession(1);
			} else {
				fBinWidth = (fBinMax - fBinMin) / fNEorTBins;
			}
		}
	}

	if (fSparsify && fNEorTBins) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Sparsify can not be used with Energy or Time binning." << G4endl;
		fPm->AbortSession(1);
	}

	G4long testInLong;
	if (nEorTBinsTotal == 0) {
		testInLong = fNDivisions;
	} else {
		testInLong = (G4long)nEorTBinsTotal * (G4long)fNDivisions;
	}

	if (testInLong > INT_MAX) {
		G4cerr << "Component " << GetName() << " has too many bins." << G4endl;
		G4cerr << "Maximum allowed total number is " << INT_MAX << G4endl;
		G4cerr << "Number of Divisions was found to be: " << fNDivisions << G4endl;
		G4cerr << "Number of Energy or Time Bins was found to be: " << nEorTBinsTotal << G4endl;
		G4cerr << "(includes some bins for overflow, underflow and uncounted)." << G4endl;
		G4cerr << "Total number of bins was therefore: " << testInLong << G4endl;
		fPm->AbortSession(1);
	}

	fNBins = testInLong;

	// Check units and binning for limit tests
	if (fSumLimit > 0.) {
		G4String parmName = GetFullParmName("RepeatSequenceUntilSumGreaterThan");
		G4String limitType = fPm->GetTypeOfParameter(parmName);
		if (limitType == "d") {
			if (fUnitName == "") {
				G4cerr << parmName << " has parameter type d but this scorer does not expect any units." << G4endl;
				G4cerr << "This scorer expects parameter type i or u." << G4endl;
				fPm->AbortSession(1);
			} else {
				if (fPm->GetUnitCategory(fPm->GetUnitOfParameter(parmName)) != fPm->GetUnitCategory(fUnitName)) {
					G4cerr << parmName << " unit category does not match this scorer's unit category." << G4endl;
					G4cerr << "This scorer expects units category: " << fPm->GetUnitCategory(fUnitName) << G4endl;
					fPm->AbortSession(1);
				}
			}
		} else {
			if (fUnitName != "") {
				G4cerr << parmName << " has wrong parameter type for this scorer." << G4endl;
				G4cerr << "This scorer expects a dimensioned parameter with units of category: " << fPm->GetUnitCategory(fUnitName) << G4endl;
				fPm->AbortSession(1);
			}
		}
	}

	if (fStandardDeviationLimit > 0.) {
		G4String parmName = GetFullParmName("RepeatSequenceUntilStandardDeviationLessThan");
		G4String limitType = fPm->GetTypeOfParameter(parmName);
		if (limitType == "d") {
			if (fUnitName == "") {
				G4cerr << parmName << " has parameter type d but this scorer does not expect any units." << G4endl;
				G4cerr << "This scorer expects parameter type i or u." << G4endl;
				fPm->AbortSession(1);
			} else {
				if (fPm->GetUnitCategory(fPm->GetUnitOfParameter(parmName)) != fPm->GetUnitCategory(fUnitName)) {
					G4cerr << parmName << " unit category does not match this scorer's unit category." << G4endl;
					G4cerr << "This scorer expects units category: " << fPm->GetUnitCategory(fUnitName) << G4endl;
					fPm->AbortSession(1);
				}
			}
		} else {
			if (fUnitName != "") {
				G4cerr << parmName << " has wrong parameter type for this scorer." << G4endl;
				G4cerr << "This scorer expects a dimensioned parameter with units of category: " << fPm->GetUnitCategory(fUnitName) << G4endl;
				fPm->AbortSession(1);
			}
		}
	}

	G4String binParmName;
	G4int repeatSequenceTestI = 0;
	G4int repeatSequenceTestJ = 0;
	G4int repeatSequenceTestK = 0;
	G4int repeatSequenceTestEorT = 0;

	binParmName = "RepeatSequenceTest" + fComponent->GetDivisionName(0) + "Bin";
	if (fPm->ParameterExists(GetFullParmName(binParmName))) {
		repeatSequenceTestI = fPm->GetIntegerParameter(GetFullParmName(binParmName));
		if (repeatSequenceTestI < 0) OutOfRange( GetFullParmName(binParmName), "can not be less than zero");
		if (repeatSequenceTestI >= fNi) OutOfRange( GetFullParmName(binParmName), "must be less than number of bins (remember bin numbering starts from zero)");
	}

	binParmName = "RepeatSequenceTest" + fComponent->GetDivisionName(1) + "Bin";
	if (fPm->ParameterExists(GetFullParmName(binParmName))) {
		repeatSequenceTestJ = fPm->GetIntegerParameter(GetFullParmName(binParmName));
		if (repeatSequenceTestJ < 0) OutOfRange( GetFullParmName(binParmName), "can not be less than zero");
		if (repeatSequenceTestJ >= fNj) OutOfRange( GetFullParmName(binParmName), "must be less than number of bins (remember bin numbering starts from zero)");
	}

	binParmName = "RepeatSequenceTest" + fComponent->GetDivisionName(2) + "Bin";
	if (fPm->ParameterExists(GetFullParmName(binParmName))) {
		repeatSequenceTestK = fPm->GetIntegerParameter(GetFullParmName(binParmName));
		if (repeatSequenceTestK < 0) OutOfRange( GetFullParmName(binParmName), "can not be less than zero");
		if (repeatSequenceTestK >= fNk) OutOfRange( GetFullParmName(binParmName), "must be less than number of bins (remember bin numbering starts from zero)");
	}

	if (fPm->ParameterExists(GetFullParmName("EBins"))) {
		binParmName = "RepeatSequenceTestEBin";
		if (fPm->ParameterExists(GetFullParmName(binParmName))) {
			repeatSequenceTestEorT = fPm->GetIntegerParameter(GetFullParmName(binParmName));
			if (repeatSequenceTestEorT < 0) OutOfRange( GetFullParmName(binParmName), "can not be less than zero");
			if (repeatSequenceTestEorT >= fNEorTBins) OutOfRange( GetFullParmName(binParmName), "must be less than number of bins (remember bin numbering starts from zero)");
		}
	}

	if (fPm->ParameterExists(GetFullParmName("TimeBins"))) {
		binParmName = "RepeatSequenceTestTimeBin";
		if (fPm->ParameterExists(GetFullParmName(binParmName))) {
			repeatSequenceTestEorT = fPm->GetIntegerParameter(GetFullParmName(binParmName));
			if (repeatSequenceTestEorT < 0) OutOfRange( GetFullParmName(binParmName), "can not be less than zero");
			if (repeatSequenceTestEorT >= fNEorTBins) OutOfRange( GetFullParmName(binParmName), "must be less than number of bins (remember bin numbering starts from zero)");
		}
	}

	fRepeatSequenceTestBin = repeatSequenceTestI*fNj*fNk + repeatSequenceTestJ*fNk + repeatSequenceTestK;
	if (fNEorTBins > 0) {
		if (fBinByIncidentEnergy)
			fRepeatSequenceTestBin = fRepeatSequenceTestBin * (fNEorTBins + 3) + repeatSequenceTestEorT + 1;
		else
			fRepeatSequenceTestBin = fRepeatSequenceTestBin * (fNEorTBins + 2) + repeatSequenceTestEorT + 1;
	}
}


void TsVBinnedScorer::UpdateForNewRun(G4bool rebuiltSomeComponents) {
	TsVScorer::UpdateForNewRun(rebuiltSomeComponents);

#ifdef TOPAS_MT
	TsVBinnedScorer* masterScorer = dynamic_cast<TsVBinnedScorer*>(fScm->GetMasterScorerByID(fUID));
	if (masterScorer == this) {
#endif
		if (fOutputToXml || fOutputToRoot) {

			G4int runID = GetRunID();
			G4String histogramName = fOutFileName;
			G4bool needNewHistogram = false;

			if (fPm->GetIntegerParameter("Tf/NumberOfSequentialTimes") > 1 && fOutputAfterRun && !fPm->IsRandomMode()) {
				G4int nPadding = fPm->GetIntegerParameter("Ts/RunIDPadding");

				std::ostringstream runString;
				runString << "_Run_" << std::setw(nPadding) << std::setfill('0') << runID;

				histogramName = histogramName + runString.str();
				needNewHistogram = true;
			}
			else if (runID == 0) {
				needNewHistogram = true;
			}

			if (needNewHistogram) {
				if (fNumberOfOutputColumns > 0)
					CreateHistogram(histogramName, false);
				if (fReportCVolHist || fReportDVolHist)
					CreateHistogram(histogramName, true);
			}
		}
#ifdef TOPAS_MT
	} else {
		fVHistogramID = masterScorer->fVHistogramID;
		fHistogramID  = masterScorer->fHistogramID;
	}
#endif
}


G4bool TsVBinnedScorer::HasUnsatisfiedLimits() {
	G4bool fNeedToCalculateOneValue = true;

	if (fSumLimit > 0.) {
		CalculateOneValue(fRepeatSequenceTestBin);
		fNeedToCalculateOneValue = false;
		if ((fSumLimit / GetUnitValue()) > fSum) {
			G4cout << "\nRepeating Sequence due to test set in parameter:" << G4endl;
			G4cout << GetFullParmName("RepeatSequenceUntilSumGreaterThan") << G4endl;
			G4cout << "Limit is: " << fSumLimit / GetUnitValue() << ", current Sum is: " << fSum << G4endl;
			return true;
		}
	}

	if (fStandardDeviationLimit > 0.) {
		if (fNeedToCalculateOneValue) {
			CalculateOneValue(fRepeatSequenceTestBin);
			fNeedToCalculateOneValue = false;
		}

		if ((fStandardDeviationLimit / GetUnitValue()) < fStandardDeviation) {
			G4cout << "\nRepeating Sequence due to test set in parameter:" << G4endl;
			G4cout << GetFullParmName("RepeatSequenceUntilStandardDeviationLessThan") << G4endl;
			G4cout << "Limit is: " << fStandardDeviationLimit / GetUnitValue() << ", current StandardDeviation is: " << fStandardDeviation << G4endl;
			return true;
		}
	}

	if (fRelativeSDLimit > 0.) {
		if (fNeedToCalculateOneValue) {
			CalculateOneValue(fRepeatSequenceTestBin);
			fNeedToCalculateOneValue = false;
		}

		if (fRelativeSDLimit * fMean < fStandardDeviation / sqrt(fScoredHistories)) {
			G4cout << "\nRepeating Sequence due to test set in parameter:" << G4endl;
			G4cout << GetFullParmName("RepeatSequenceUntilRelativeStandardDeviationLessThan") << G4endl;
			G4cout << "Limit, RelativeSDLimit * Mean is: " << fRelativeSDLimit * fMean << G4endl;
			G4cout << "Current value,  StandardDeviation / sqrt (nHistories) is: " << fStandardDeviation / sqrt(fScoredHistories) << G4endl;
			return true;
		}
	}

	if (fCountLimit > 0) {
		if (fNeedToCalculateOneValue)
			CalculateOneValue(fRepeatSequenceTestBin);
		if (fCountLimit > fCountInBin) {
			G4cout << "\nRepeating Sequence due to test set in parameter:" << G4endl;
			G4cout << GetFullParmName("RepeatSequenceUntilCountGreaterThan") << G4endl;
			G4cout << "Limit is: " << fCountLimit << ", current CountInBin is: " << fCountInBin << G4endl;
			return true;
		}
	}

	return false;
}


void TsVBinnedScorer::PostConstructor()
{
	if (!fUnitWasSet) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "The scorer named " << GetName() << " never called SetUnit." << G4endl;
		G4cerr << "Every scorer must make this call in its constructor." << G4endl;
		fPm->AbortSession(1);
	}

	TsVScorer::PostConstructor();
	ActuallySetUnit(fUnitName);

	 if (fOutputToDicom) {
		G4String compType = fPm->GetStringParameter(fComponent->GetFullParmName("Type"));
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(compType);
#else
		compType.toLower();
#endif
		if (compType != "tsbox" && compType != "tsdicompatient" && compType != "tsxiopatient" && compType != "tsimagecube") {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << GetName() << " has unsupported component type for DICOM output." << G4endl;
			G4cerr << "Supported component types are TsBox, TsDicomPatient, TsXiOPatient and TsImageCube." << G4endl;
			fPm->AbortSession(1);
		}

		// scorer is explicitly assigned a ReferencedDicomPatient
		if (fPm->ParameterExists(GetFullParmName("ReferencedDicomPatient"))) {
			G4String compName = fPm->GetStringParameter(GetFullParmName("ReferencedDicomPatient"));
			TsVGeometryComponent* tmpReferencedDicomPatient = fGm->GetComponent(compName);
			if (!tmpReferencedDicomPatient) {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << GetName() << " has unfound ReferencedDicomPatient: " << compName << G4endl;
				fPm->AbortSession(1);
			}
			fReferencedDicomPatient = dynamic_cast<TsDicomPatient*>(tmpReferencedDicomPatient);
			if (!fReferencedDicomPatient) {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << GetFullParmName("ReferencedDicomPatient") << " does not refer to a TsDicomPatient" << G4endl;
				fPm->AbortSession(1);
			}
			G4RotationMatrix* rotDicom = fReferencedDicomPatient->GetRotRelToWorld();
			G4RotationMatrix* rotScore = fComponent->GetRotRelToWorld();
			if (*rotScore != *rotDicom) {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << "Scorer " << GetName() << " has a relative rotation between Component and ReferencedDicomPatient." << G4endl;
				G4cerr << "This is not allowed when outputting to DICOM format with a ReferencedDicomPatient." << G4endl;
				fPm->AbortSession(1);
			}
		} else {
			// infer ReferencedDicomPatient from component ancestry
			G4String compName = fPm->GetStringParameter(GetFullParmName("Component"));  // cannot use fComponent since it may be a copy
			TsVGeometryComponent* tmpComponent = fGm->GetComponent(compName);

			while (!fReferencedDicomPatient && tmpComponent && fPm->ParameterExists(tmpComponent->GetFullParmName("Parent"))) {
				fReferencedDicomPatient = dynamic_cast<TsDicomPatient*>(tmpComponent);
				G4String parentName = fPm->GetStringParameter(tmpComponent->GetFullParmName("Parent"));
				tmpComponent = fGm->GetComponent(parentName);
			}
		}
	}

	if (fSuppressStandardOutputHandling) {
		fNeedToUpdateFileSpecs = false;
	} else {
		if (!fOutputToBinary && !fOutputToCsv && !fOutputToRoot && !fOutputToXml && !fOutputToDicom) {
			G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
			G4cerr << "The scorer named " << GetName() << " has unsupported OutputType: "
			<< fPm->GetStringParameter(GetFullParmName("OutputType")) << G4endl;

			G4String quantityNameLower = fQuantity;
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(quantityNameLower);
#else
			quantityNameLower.toLower();
#endif
			if (quantityNameLower == "phasespace")
				G4cerr << "OutputType must be either ASCII, Binary or Limited." << G4endl;
			else
				G4cerr << "OutputType must be either CSV, Binary, Root, XML or Dicom." << G4endl;
			fPm->AbortSession(1);
		}

		// If possible, set up output right away so that can immediately abort if can't open file.
		// When OutputAfterRun is true, we check output at the end of each run.
		// PhaseSpace scorers have their own logic since can't set filename until their own constructor is finished.
#ifdef TOPAS_MT
		if (!G4Threading::IsWorkerThread()) {
#endif
			if ((!fOutputAfterRun || fPm->IsRandomMode()) && !fIsSubScorer) {
				G4int increment = 0;
				if (fOutputToBinary) {
					G4String outFileExt1 = ".binheader";
					G4String outFileExt2 = ".bin";
					if (fNumberOfOutputColumns > 0) {
						fOutFileSpec1 = ConfirmCanOpen(fOutFileName, outFileExt1, increment);
						fOutFileSpec2 = ConfirmCanOpen(fOutFileName, outFileExt2, increment);
					}
					if (fReportCVolHist || fReportDVolHist) {
						fVHOutFileSpec1 = ConfirmCanOpen(fOutFileName+"_VolHist", outFileExt1, increment);
						fVHOutFileSpec2 = ConfirmCanOpen(fOutFileName+"_VolHist", outFileExt2, increment);
					}
				} else if (fOutputToCsv) {
					G4String outFileExt1 = ".csv";
					if (fNumberOfOutputColumns > 0)
						fOutFileSpec1 = ConfirmCanOpen(fOutFileName, outFileExt1, increment);
					if (fReportCVolHist || fReportDVolHist)
						fVHOutFileSpec1 = ConfirmCanOpen(fOutFileName+"_VolHist", outFileExt1, increment);
				} else if (fOutputToDicom) {
					G4String outFileExt1 = ".dcm";
					fOutFileSpec1 = ConfirmCanOpen(fOutFileName, outFileExt1, increment);
				}

				fNeedToUpdateFileSpecs = false;
			}
#ifdef TOPAS_MT
		}
#endif
	}
}


void TsVBinnedScorer::SetUnit(const G4String& theUnitName)
{
	fUnitName = theUnitName;
	fUnitWasSet = true;
}

void TsVBinnedScorer::ActuallySetUnit(const G4String& theUnitName)
{
	if (fOutputToRoot || fOutputToXml) {
		if (fPm->ParameterExists(GetFullParmName("HistogramMin"))) {
			if (fPm->GetUnitCategoryOfParameter(GetFullParmName("HistogramMin")) != fPm->GetUnitCategory(theUnitName)) {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << "The scorer named: " << GetName() << " has inappropriate unit category in HistogramMin." << G4endl;
				fPm->AbortSession(1);
			}
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "The scorer named: " << GetName() << " specifies root or xml output but has no HistogramMin defined" << G4endl;
			fPm->AbortSession(1);
		}

		if (fPm->ParameterExists(GetFullParmName("HistogramMax"))) {
			if (fPm->GetUnitCategoryOfParameter(GetFullParmName("HistogramMax")) != fPm->GetUnitCategory(theUnitName)) {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << "The scorer named: " << GetName() << " has inappropriate unit category in HistogramMax." << G4endl;
				fPm->AbortSession(1);
			}
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "The scorer named: " << GetName() << " specifies root or xml output but has no HistogramMax defined" << G4endl;
			fPm->AbortSession(1);
		}
	}

	if (theUnitName=="") {
		unitValue = 1.;
	} else {
		unitValue = fPm->GetUnitValue(theUnitName);
		if (unitValue==0) {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "Scorer name: " << GetName() << " is using an unknown unit: " << theUnitName << G4endl;
			fPm->AbortSession(1);
		}
	}

	G4VPrimitiveScorer::SetUnit(theUnitName);

	// Since this section requires knowing the unit category, this could not be done in the ctor.
	if (fOutputToRoot || fOutputToXml || fReportCVolHist || fReportDVolHist) {
		fHistogramBins = fPm->GetIntegerParameter(GetFullParmName("HistogramBins"));
		fHistogramMin = fPm->GetDoubleParameter(GetFullParmName("HistogramMin"), fPm->GetUnitCategory(theUnitName)) / GetUnitValue();
		fHistogramMax = fPm->GetDoubleParameter(GetFullParmName("HistogramMax"), fPm->GetUnitCategory(theUnitName)) / GetUnitValue();
		G4double binWidth = ( fHistogramMax - fHistogramMin ) / fHistogramBins;
		for (G4int i=0; i < fHistogramBins; i++)
			fHistogramLowerValues.push_back(fHistogramMin + i * binWidth);
	}

	// Now that know binning, units and reporting options,
	// can know whether to open 1 or 2D histogram and can create histogram
	if (fOutputToRoot || fOutputToXml) {
		if (!fPm->ParameterExists(GetFullParmName("HistogramBins"))) {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "The scorer named: " << GetName() << " specifies root or xml output but has no HistogramBins defined" << G4endl;
			fPm->AbortSession(1);
		}
	}

	// ColorBy option will trigger extra reporting option if needed.
	if (fPm->ParameterExists(GetFullParmName("ColorBy"))) {
		fColorBy = fPm->GetStringParameter(GetFullParmName("ColorBy"));
#if GEANT4_VERSION_MAJOR >= 11
		G4StrUtil::to_lower(fColorBy);
#else
		fColorBy.toLower();
#endif

		if (!fPm->ParameterExists(GetFullParmName("ColorNames"))) {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "You have set the parameter Sc/" << GetName() << "/ColorBy" << G4endl;
			G4cerr << "but have not set the associated ColorNames parameter." << G4endl;
			fPm->AbortSession(1);
		}

		if (!fPm->ParameterExists(GetFullParmName("ColorValues"))) {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "You have set the parameter Sc/" << GetName() << "/ColorBy" << G4endl;
			G4cerr << "but have not set the associated ColorValues parameter." << G4endl;
			fPm->AbortSession(1);
		}

		if (fPm->GetVectorLength(GetFullParmName("ColorNames")) != ( fPm->GetVectorLength(GetFullParmName("ColorValues")) + 1) ) {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "The number of Sc/" << GetName() << "/ColorNames must be exactly one more than the number of Sc/" << GetName() << "/ColorValues." << G4endl;
			fPm->AbortSession(1);
		}

		G4String* colorNames = fPm->GetStringVector(GetFullParmName("ColorNames"));
		for (G4int iName = 0; iName < fPm->GetVectorLength(GetFullParmName("ColorNames")); iName++)
			fColorNames.push_back(colorNames[iName]);

		G4double* colorValues = 0;

		if (fColorBy == "sum") {
			if (!fReportSum) {
				fReportSum = true;
				fNumberOfOutputColumns++;
			}
			colorValues = fPm->GetDoubleVector(GetFullParmName("ColorValues"), fPm->GetUnitCategory(theUnitName));
		} else if (fColorBy == "mean") {
			if (!fReportMean) {
				fReportMean = true;
				fNumberOfOutputColumns++;
			}
			colorValues = fPm->GetDoubleVector(GetFullParmName("ColorValues"), fPm->GetUnitCategory(theUnitName));
		} else if (fColorBy == "histories") {
			if (!fReportHistories) {
				fReportHistories = true;
				fNumberOfOutputColumns++;
			}
			colorValues = fPm->GetUnitlessVector(GetFullParmName("ColorValues"));
		} else if (fColorBy == "count_in_bin") {
			if (!fReportCountInBin) {
				fReportCountInBin = true;
				fNumberOfOutputColumns++;
			}
			colorValues = fPm->GetUnitlessVector(GetFullParmName("ColorValues"));
		} else if (fColorBy == "standard_deviation") {
			if (!fReportStandardDeviation) {
				fReportStandardDeviation = true;
				fNumberOfOutputColumns++;
			}
			colorValues = fPm->GetDoubleVector(GetFullParmName("ColorValues"), fPm->GetUnitCategory(theUnitName));
		} else if (fColorBy == "min") {
			if (!fReportMin) {
				fReportMin = true;
				fNumberOfOutputColumns++;
			}
			colorValues = fPm->GetDoubleVector(GetFullParmName("ColorValues"), fPm->GetUnitCategory(theUnitName));
		} else if (fColorBy == "max") {
			if (!fReportMax) {
				fReportMax = true;
				fNumberOfOutputColumns++;
			}
			colorValues = fPm->GetDoubleVector(GetFullParmName("ColorValues"), fPm->GetUnitCategory(theUnitName));
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "The parameter Sc/" << GetName() << "/ColorBy has an unknown option: " << fColorBy << G4endl;
			G4cerr << "Should be sum, mean, histories, count_in_bin, standard_deviation, min or max" << G4endl;
			fPm->AbortSession(1);
		}

		for (G4int iName = 0; iName < fPm->GetVectorLength(GetFullParmName("ColorValues")); iName++)
			fColorValues.push_back(colorValues[iName]/GetUnitValue());
	}

	if (fReportSecondMoment || fReportVariance || fReportStandardDeviation)
		fAccumulateSecondMoment = true;

	if (fAccumulateSecondMoment || fReportMean)
		fAccumulateMean = true;

	if (fAccumulateMean || fReportCountInBin)
		fAccumulateCount = true;

	// Now that know binning and reporting options,
	// can initialize accumulation vectors appropriately
	if (fReportCountInBin || fReportMean || fAccumulateSecondMoment)
		fCountMap.assign(fNBins, 0);

	if (fReportMin)
		fMinMap.assign(fNBins,  9.e+99);

	if (fReportMax)
		fMaxMap.assign(fNBins, -9.e+99);

	if (fReportSum || fReportMean || fAccumulateSecondMoment || fReportCVolHist || fReportDVolHist)
		fFirstMomentMap.assign(fNBins, 0.);

	if (fReportMean || fAccumulateSecondMoment)
		fSecondMomentMap.assign(fNBins, 0.);

	// Setup outcome modeling only if model name is given and the unitName is Gy
	if ( fPm-> ParameterExists( GetFullParmNameLower( "OutcomeModelName" ) ) ) {
		fOm = new TsOutcomeModelList(fPm, fScm->GetExtensionManager(), GetName(), theUnitName);
		fReportOutcome = true;
		if (fPm->ParameterExists(GetFullParmNameLower("OutcomeOutputScaleFactor")))
			fNormFactor = fPm->GetUnitlessParameter(GetFullParmNameLower("OutcomeOutputScaleFactor"));
	}
}


void TsVBinnedScorer::CreateHistogram(G4String title, G4bool volumeHistogram) {
	if (volumeHistogram)
		title += "_VolHist";

	G4VAnalysisManager* analysisManager;
	if (fOutputToRoot)
		analysisManager = fScm->GetRootAnalysisManager();
	else
		analysisManager = fScm->GetXmlAnalysisManager();

	G4int ID = 0;

	if (fNEorTBins==0) {
		if (fNDivisions > 1 && !volumeHistogram) {
			// Binned by component division
			ID = analysisManager->CreateH2(title, fQuantity, fHistogramBins, fHistogramMin, fHistogramMax, fNDivisions, 0., fNDivisions);
		} else {
			// Unbinned
			ID = analysisManager->CreateH1(title, fQuantity, fHistogramBins, fHistogramMin, fHistogramMax);
		}
	} else {
		if (fNDivisions > 1 && !volumeHistogram) {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "Histogramming has been requested for scorer name: " << GetName() << G4endl;
			G4cerr << "But the scorer has binning both by component division and by either energy or time." << G4endl;
			G4cerr << "Topas can histogram by component division or by energy or time, but not by both." << G4endl;
			G4cerr << "Suggest you instead output to csv or binary and then use an external plotting tool." << G4endl;
			fPm->AbortSession(1);
		} else {
			// Binned by energy or time
			ID = analysisManager->CreateH2(title, fQuantity, fHistogramBins, fHistogramMin, fHistogramMax, fNEorTBins, fBinMin, fBinMax);
		}
	}

	if (volumeHistogram)
		fVHistogramID = ID;
	else
		fHistogramID = ID;
}


void TsVBinnedScorer::AccumulateHit(G4Step* aStep, G4double value)
{
	AccumulateHit(aStep, value, fComponent->GetIndex(aStep));
}


void TsVBinnedScorer::AccumulateHit(G4Step* aStep, G4double value, G4int index)
{
	if (index >= 0) {
		if (fBinByIncidentEnergy) {
			// Number of bins will be fNEorTBins plus 3 more bins for underflow, overflow and case of no incident track
			// Bin indexing starts from zero.
			G4int iBin = 0;

			if (fHaveIncidentParticle) {
				if (fIncidentParticleEnergy < fBinMin)
					iBin = 0;
				else if (fIncidentParticleEnergy > fBinMax)
					iBin = fNEorTBins + 1;
				else {
					if (!fBinLog )
						iBin = (int)( (fIncidentParticleEnergy - fBinMin) / fBinWidth ) + 1;
					else
						for ( iBin = 0; fIncidentParticleEnergy >= fTempEBins[iBin]; iBin++ );
				}
			} else {
				iBin = fNEorTBins + 2;
			}

			index = index * (fNEorTBins + 3) + iBin;
		} else if (fBinByPreStepEnergy || fBinByStepDepositEnergy || fBinByPrimaryEnergy) {
			// Number of bins will be fNEorTBins plus 2 more bins for underflow and overflow.
			// Bin indexing starts from zero.
			G4int iBin = 0;

			G4double edep;
			if (fBinByPrimaryEnergy) {
				TsTrackInformation* parentInformation = (TsTrackInformation*)(aStep->GetTrack()->GetUserInformation());
				if (parentInformation)
					edep = parentInformation->GetParentTrackVertexKineticEnergies().back();
				else
					edep = aStep->GetTrack()->GetVertexKineticEnergy();
			} else if (fBinByPreStepEnergy)
				edep = aStep->GetTrack()->GetKineticEnergy();
			else
				edep = aStep->GetTotalEnergyDeposit();

			if (edep < fBinMin)
				iBin = 0;
			else if (edep > fBinMax)
				iBin = fNEorTBins + 1;
			else {
				if (!fBinLog )
					iBin = (int)( (edep - fBinMin) / fBinWidth ) + 1;
				else
					for ( iBin = 0; edep >= fTempEBins[iBin]; iBin++ );
			}

			index = index * (fNEorTBins + 2) + iBin;
		} else if (fBinByTime) {
			// Number of bins will be fNEorTBins plus 2 more bins for underflow and overflow.
			G4int iBin = 0;
			G4double time = aStep->GetTrack()->GetGlobalTime()/ns;

			if (time < fBinMin)
				iBin = 0;
			else if (time > fBinMax) {
				iBin = fNEorTBins + 1;

				if (fTrackingVerbosity > 0)
					G4cout << "Track number " << aStep->GetTrack()->GetTrackID() <<
					" has GlobalTime in overflow bin. Type: " << aStep->GetTrack()->GetParticleDefinition()->GetParticleName() <<
					", Time: " << time << " ns = " << time / 3600000000000. << " hours." << G4endl;
			}
			else
				iBin = (int)( (time - fBinMin) / fBinWidth ) + 1;

			index = index * (fNEorTBins + 2) + iBin;
		}

		(*fEvtMap)[index] += value;

		if (fTrackingVerbosity > 0) {
			G4cout << "PreStep x,y,z: " << aStep->GetPreStepPoint()->GetPosition().x() << ", "
			<< aStep->GetPreStepPoint()->GetPosition().y() << ", "
			<< aStep->GetPreStepPoint()->GetPosition().z() << G4endl;
			G4cout << "Step Energy Deposit: " << aStep->GetTotalEnergyDeposit() << G4endl;
			G4cout << "Step Length: " << aStep->GetStepLength() << G4endl;

			const G4VTouchable* touchable = aStep->GetPreStepPoint()->GetTouchable();
			if (touchable->GetHistoryDepth() > 1) G4cout << "PreStep Touchable(2): " << touchable->GetVolume(2)->GetName() << " has replica number: " << touchable->GetReplicaNumber(2) << G4endl;
			if (touchable->GetHistoryDepth() > 0) G4cout << "PreStep Touchable(1): " << touchable->GetVolume(1)->GetName() << " has replica number: " << touchable->GetReplicaNumber(1) << G4endl;
			G4cout << "PreStep Touchable(0): " << touchable->GetVolume(0)->GetName() << " has replica number: " << touchable->GetReplicaNumber(0) << G4endl;
			touchable = aStep->GetPostStepPoint()->GetTouchable();
			if (touchable->GetHistoryDepth() > 1) G4cout << "PostStep Touchable(2): " << touchable->GetVolume(2)->GetName() << " has replica number: " << touchable->GetReplicaNumber(2) << G4endl;
			if (touchable->GetHistoryDepth() > 0) G4cout << "PostStep Touchable(1): " << touchable->GetVolume(1)->GetName() << " has replica number: " << touchable->GetReplicaNumber(1) << G4endl;
			G4cout << "PostStep Touchable(0): " << touchable->GetVolume(0)->GetName() << " has replica number: " << touchable->GetReplicaNumber(0) << G4endl;
		}
	} else {
		fUnscoredSteps++;
		fUnscoredEnergy += aStep->GetTotalEnergyDeposit();
	}

	return;
}


void TsVBinnedScorer::AccumulateEvent()
{
	if (fHaveIncidentParticle) {
		UserHookForEndOfIncidentParticle();
		fHaveIncidentParticle = false;
	}

	UserHookForEndOfEvent();

	fScoredHistories++;

	// Iterator to use with fEvtMap, the map of hits in the event
	std::map<G4int, G4double>::iterator itrX;

	// Bin index
	G4int index;

	// Value from one specific bin
	G4double x;
	G4double mean;
	G4double delta;
	G4double mom2;

	// Iterate over all the hits in this event
	for (itrX = fEvtMap->begin(); itrX != fEvtMap->end(); itrX++) {
		index = itrX->first;
		x = itrX->second;

		if (fAccumulateCount)
			fCountMap[index]++;

		if (fReportMin)
			if (x < fMinMap[index])
				fMinMap[index] = x;

		if (fReportMax)
			if (x > fMaxMap[index])
				fMaxMap[index] = x;

		// Use numerically stable algoritm from Donald E. Knuth (1998).
		// The Art of Computer Programming, volume 2: Seminumerical Algorithms,
		// 3rd edn., p. 232. Boston: Addison-Wesley.
		// for x in data:
		//   n = n + 1
		//   delta = x - mean
		//   mean = mean + delta/n
		//   mom2 = mom2 + delta*(x - mean)
		// variance = mom2/(n - 1)
		if (fAccumulateMean) {
			if (fAccumulateSecondMoment && fCountMap[index]==1) {
				// Initialize values to account for all previous histories having zero value
				// If we want Mean but don't want SecondMoment, can use a faster method at end of scoring.
				mean = x/fScoredHistories;
				fFirstMomentMap[index] = mean;
				mom2 = (fScoredHistories-1)*mean*mean + (x - mean)*(x - mean);
				fSecondMomentMap[index] = mom2;
			} else {
				mean = fFirstMomentMap[index];
				delta = x - mean;

				if (fAccumulateSecondMoment) {
					mean += delta/fScoredHistories;
					mom2 = fSecondMomentMap[index];
					mom2 += delta*(x-mean);
					fSecondMomentMap[index] = mom2;
				} else {
					mean += delta/fCountMap[index];
				}
				fFirstMomentMap[index] = mean;
			}
		} else if (fReportSum || fReportCVolHist || fReportDVolHist) {
			// Only need to accumulate sum
			fFirstMomentMap[index] += x;
		}
	}

	if (fAccumulateSecondMoment) {
		// Any bins hit in previous events but not hit in this event must accumulate a zero.
		// If fNDivisions is very large and many of the divisions have already been hit,
		// this loop becomes very time consuming. But I don't see a way around this.
		// If we want Mean but don't want SecondMoment, can use a faster method at end of scoring.
		for (index = 0; index < fNBins; index++) {
			if (fCountMap[index] > 0) {
				itrX = fEvtMap->find(index);
				if (itrX == fEvtMap->end()) {
					mean = fFirstMomentMap[index];
					fFirstMomentMap[index] -=  mean /fScoredHistories;
					fSecondMomentMap[index] += mean * mean;
				}
			}
		}
	}

	// Fill histograms unless doing volume histogram
	if ((fOutputToRoot || fOutputToXml) && fNumberOfOutputColumns > 0) {
		G4VAnalysisManager* analysisManager;
		if (fOutputToRoot)
			analysisManager = fScm->GetRootAnalysisManager();
		else
			analysisManager = fScm->GetXmlAnalysisManager();

		for (itrX = fEvtMap->begin(); itrX != fEvtMap->end(); itrX++) {
			index = itrX->first;
			x = itrX->second;

			if (fNDivisions==1 && fNEorTBins==0)
				analysisManager->FillH1(fHistogramID, x / GetUnitValue(), 1.);
			else
				analysisManager->FillH2(fHistogramID, x / GetUnitValue(), index, 1.);
		}
	}

	// Clear event total
	fEvtMap->clear();
}


void TsVBinnedScorer::RestoreResultsFromFile()
{
	fRestoreResultsFromFile = true;

	G4String inputType = fPm->GetStringParameter(GetFullParmName("InputType"));
#if GEANT4_VERSION_MAJOR >= 11
	G4StrUtil::to_lower(inputType);
#else
	inputType.toLower();
#endif

	G4String inputFileSpec;

	if (inputType == "csv")
		inputFileSpec = fPm->GetStringParameter(GetFullParmName("InputFile")) + ".csv";
	else if (inputType == "binary")
		inputFileSpec = fPm->GetStringParameter(GetFullParmName("InputFile")) + ".binheader";
	else if (inputType == "dicom") {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Called RestoreResultsFromFile for Scorer name: " << GetName() << G4endl;
		G4cerr << "but inputType is neither to Csv nor Binary" << G4endl;
		fPm->AbortSession(1);
	}

	std::ifstream inFile(inputFileSpec);
	if (!inFile) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Ts/RestoreResultsFromFile has been set true," << G4endl;
		G4cerr << "but unable to open input file: " << inputFileSpec << " for Scorer name: " << GetName() << G4endl;
		fPm->AbortSession(1);
	}

	fReadLine = "#";

	// Get past initial comment lines
	while (fReadLine.substr(0,9)!="# Scored ") {
		getline(inFile,fReadLine);
		if (inFile.eof()) {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "Ts/RestoreResultsFromFile has been set true," << G4endl;
			G4cerr << "but got end of file too early in input file: " << inputFileSpec << " for Scorer name: " << GetName() << G4endl;
			fPm->AbortSession(1);
		}
	}

	if (fNDivisions > 1) {
		getline(inFile,fReadLine);
		getline(inFile,fReadLine);
		getline(inFile,fReadLine);
	}

	// Study header to see what columns of data are available
	getline(inFile,fReadLine);

	// Check that quantity in header matches expected quantity
	G4int quantityLength = fQuantity.length();
	if (fReadLine.substr(2, quantityLength) != fQuantity) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Ts/RestoreResultsFromFile has been set true," << G4endl;
		G4cerr << "but input file: " << inputFileSpec << " has wrong quantity for Scorer name: " << GetName() << G4endl;
		G4cerr << "Expected: " << fQuantity << G4endl;
		fPm->AbortSession(1);
	}

	std::string::size_type delcharPos = fReadLine.find( ":" );
	fReadLine = fReadLine.substr(delcharPos+1);

	G4Tokenizer next(fReadLine);
	fReadBackValues = new G4int[9];
	G4String oneToken;
	while ((oneToken = next()) != "") {
		if (oneToken == "Sum") {
			fReadBackHasSum = true;
			fReadBackValues[fNReadBackValues] = 0;
			fNReadBackValues++;
		} else if (oneToken == "Mean") {
			fReadBackHasMean = true;
			fReadBackValues[fNReadBackValues] = 1;
			fNReadBackValues++;
		} else if (oneToken == "Histories_with_Scorer_Active") {
			fReadBackHasHistories = true;
			fReadBackValues[fNReadBackValues] = 2;
			fNReadBackValues++;
		} else if (oneToken == "Count_in_Bin") {
			fReadBackHasCountInBin = true;
			fReadBackValues[fNReadBackValues] = 3;
			fNReadBackValues++;
		} else if (oneToken == "Second_Moment") {
			fReadBackHasSecondMoment = true;
			fReadBackValues[fNReadBackValues] = 4;
			fNReadBackValues++;
		} else if (oneToken == "Variance") {
			fReadBackHasVariance = true;
			fReadBackValues[fNReadBackValues] = 5;
			fNReadBackValues++;
		} else if (oneToken == "Standard_Deviation") {
			fReadBackHasStandardDeviation = true;
			fReadBackValues[fNReadBackValues] = 6;
			fNReadBackValues++;
		} else if (oneToken == "Min") {
			fReadBackHasMin = true;
			fReadBackValues[fNReadBackValues] = 7;
			fNReadBackValues++;
		}  else if (oneToken == "Max") {
			fReadBackHasMax = true;
			fReadBackValues[fNReadBackValues] = 8;
			fNReadBackValues++;
		}
	}

	// Evaluate whether we have appropriate data to satisfy report options.
	// In some cases, we will need to perform new calculations to produce the data to report.
	if (fReportSum && (!fReadBackHasSum && (!fReadBackHasMean || !fReadBackHasHistories))) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Unable to restore requested report options for Scorer name: " << GetName() << G4endl;
		G4cerr << "Reporting Sum requires input of Sum or of Mean plus Number of Histories" << G4endl;
		fPm->AbortSession(1);
	}

	if (fReadBackHasSum && fReadBackHasMean && !fReadBackHasHistories) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Unable to restore requested report options for Scorer name: " << GetName() << G4endl;
		G4cerr << "When reading back both Sum and Mean we also require Number of Histories" << G4endl;
		fPm->AbortSession(1);
	}

	if (fReportMean && (!fReadBackHasMean && (!fReadBackHasSum || !fReadBackHasHistories))) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Unable to restore requested report options for Scorer name: " << GetName() << G4endl;
		G4cerr << "Reporting Mean requires input of Mean or of Sum plus Number of Histories" << G4endl;
		fPm->AbortSession(1);
	}

	if (fReportHistories && (!fReadBackHasHistories && (!fReadBackHasSum || !fReadBackHasMean))) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Unable to restore requested report options for Scorer name: " << GetName() << G4endl;
		G4cerr << "Reporting Number of Histories requires input of Number of Histories or of Sum plus Mean" << G4endl;
		fPm->AbortSession(1);
	}

	if (fReportCountInBin && !fReadBackHasCountInBin) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Unable to restore requested report options for Scorer name: " << GetName() << G4endl;
		G4cerr << "Reporting Count_In_Bin requires input of Count_In_Bin" << G4endl;
		fPm->AbortSession(1);
	}

	if (fReportSecondMoment && !fReadBackHasSecondMoment && !fReadBackHasVariance  && !fReadBackHasStandardDeviation) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Unable to restore requested report options for Scorer name: " << GetName() << G4endl;
		G4cerr << "Reporting Second_Moment requires input of Second_Moment, Variance or Standard_Deviation" << G4endl;
		fPm->AbortSession(1);
	}

	if (fReportSecondMoment && !fReadBackHasSecondMoment && !fReadBackHasHistories) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Unable to restore requested report options for Scorer name: " << GetName() << G4endl;
		G4cerr << "Reporting Second_Moment requires input of Second_Moment or Number of Histories" << G4endl;
		fPm->AbortSession(1);
	}

	if (fReportVariance && !fReadBackHasVariance && !fReadBackHasSecondMoment && !fReadBackHasStandardDeviation) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Unable to restore requested report options for Scorer name: " << GetName() << G4endl;
		G4cerr << "Reporting Variance requires input of Variance, Second_Moment or Standard_Deviation" << G4endl;
		fPm->AbortSession(1);
	}

	if (fReportVariance && !fReadBackHasVariance && !fReadBackHasHistories && !fReadBackHasStandardDeviation) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Unable to restore requested report options for Scorer name: " << GetName() << G4endl;
		G4cerr << "Reporting Variance requires input of Variance, Standard_Deviation or Number of Histories" << G4endl;
		fPm->AbortSession(1);
	}

	if (fReportStandardDeviation && !fReadBackHasStandardDeviation && !fReadBackHasSecondMoment && !fReadBackHasVariance) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Unable to restore requested report options for Scorer name: " << GetName() << G4endl;
		G4cerr << "Reporting Standard_Deviation requires input of Standard_Deviation, Second_Moment or Variance" << G4endl;
		fPm->AbortSession(1);
	}

	if (fReportStandardDeviation && !fReadBackHasStandardDeviation && !fReadBackHasHistories && !fReadBackHasVariance) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Unable to restore requested report options for Scorer name: " << GetName() << G4endl;
		G4cerr << "Reporting Standard_Deviation requires input of Standard_Deviation, Variance or Number of Histories" << G4endl;
		fPm->AbortSession(1);
	}

	if (fReportMin && !fReadBackHasMin) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Unable to restore requested report options for Scorer name: " << GetName() << G4endl;
		G4cerr << "Reporting Min requires input of Min" << G4endl;
		fPm->AbortSession(1);
	}

	if (fReportMax && !fReadBackHasMax) {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Unable to restore requested report options for Scorer name: " << GetName() << G4endl;
		G4cerr << "Reporting Max requires input of Max" << G4endl;
		fPm->AbortSession(1);
	}

	getline(inFile,fReadLine);

	if (fReadLine.substr(0,33) == "# Binned by incident track energy") {
		if (fBinByIncidentEnergy) {
			G4String expectedLine = "# Binned by incident track energy in " + G4UIcommand::ConvertToString(fNEorTBins)
			+ " bins of " + G4UIcommand::ConvertToString(fBinWidth) + " MeV"
			+ " from " + G4UIcommand::ConvertToString(fBinMin) + " MeV to " + G4UIcommand::ConvertToString(fBinMax) + " MeV";
			if (fReadLine != expectedLine) {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << "Ts/RestoreResultsFromFile has been set true," << G4endl;
				G4cerr << "but in input file: " << inputFileSpec << " for Scorer name: " << GetName() << G4endl;
				G4cerr << "has different energy binning than this scorer." << G4endl;
				fPm->AbortSession(1);
			}
			getline(inFile,fReadLine);
			getline(inFile,fReadLine);
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "Ts/RestoreResultsFromFile has been set true," << G4endl;
			G4cerr << "but in input file: " << inputFileSpec << " for Scorer name: " << GetName() << G4endl;
			G4cerr << "has energy binning while this scorer does not." << G4endl;
			fPm->AbortSession(1);
		}
	}

	if (fReadLine.substr(0,27) == "# Binned by pre-step energy") {
		if (fBinByPreStepEnergy) {
			G4String expectedLine = "# Binned by pre-step energy in " + G4UIcommand::ConvertToString(fNEorTBins)
			+ " bins of " + G4UIcommand::ConvertToString(fBinWidth) + " MeV"
			+ " from " + G4UIcommand::ConvertToString(fBinMin) + " MeV to " + G4UIcommand::ConvertToString(fBinMax) + " MeV";
			if (fReadLine != expectedLine) {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << "Ts/RestoreResultsFromFile has been set true," << G4endl;
				G4cerr << "but in input file: " << inputFileSpec << " for Scorer name: " << GetName() << G4endl;
				G4cerr << "has different energy binning than this scorer." << G4endl;
				fPm->AbortSession(1);
			}
			getline(inFile,fReadLine);
			getline(inFile,fReadLine);
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "Ts/RestoreResultsFromFile has been set true," << G4endl;
			G4cerr << "but in input file: " << inputFileSpec << " for Scorer name: " << GetName() << G4endl;
			G4cerr << "has energy binning while this scorer does not." << G4endl;
			fPm->AbortSession(1);
		}
	}

	if (fReadLine.substr(0,36) == "# Binned by energy deposited in step") {
		if (fBinByStepDepositEnergy) {
			G4String expectedLine = "# Binned by energy deposited in step " + G4UIcommand::ConvertToString(fNEorTBins)
			+ " bins of " + G4UIcommand::ConvertToString(fBinWidth) + " MeV"
			+ " from " + G4UIcommand::ConvertToString(fBinMin) + " MeV to " + G4UIcommand::ConvertToString(fBinMax) + " MeV";
			if (fReadLine != expectedLine) {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << "Ts/RestoreResultsFromFile has been set true," << G4endl;
				G4cerr << "but in input file: " << inputFileSpec << " for Scorer name: " << GetName() << G4endl;
				G4cerr << "has different energy binning than this scorer." << G4endl;
				fPm->AbortSession(1);
			}
			getline(inFile,fReadLine);
			getline(inFile,fReadLine);
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "Ts/RestoreResultsFromFile has been set true," << G4endl;
			G4cerr << "but in input file: " << inputFileSpec << " for Scorer name: " << GetName() << G4endl;
			G4cerr << "has energy binning while this scorer does not." << G4endl;
			fPm->AbortSession(1);
		}
	}

	if (fReadLine.substr(0,33) == "# Binned by primary track energy") {
		if (fBinByPrimaryEnergy) {
			G4String expectedLine = "# Binned by primary track energy in " + G4UIcommand::ConvertToString(fNEorTBins)
			+ " bins of " + G4UIcommand::ConvertToString(fBinWidth) + " MeV"
			+ " from " + G4UIcommand::ConvertToString(fBinMin) + " MeV to " + G4UIcommand::ConvertToString(fBinMax) + " MeV";
			if (fReadLine != expectedLine) {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << "Ts/RestoreResultsFromFile has been set true," << G4endl;
				G4cerr << "but in input file: " << inputFileSpec << " for Scorer name: " << GetName() << G4endl;
				G4cerr << "has different energy binning than this scorer." << G4endl;
				fPm->AbortSession(1);
			}
			getline(inFile,fReadLine);
			getline(inFile,fReadLine);
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "Ts/RestoreResultsFromFile has been set true," << G4endl;
			G4cerr << "but in input file: " << inputFileSpec << " for Scorer name: " << GetName() << G4endl;
			G4cerr << "has energy binning while this scorer does not." << G4endl;
			fPm->AbortSession(1);
		}
	}

	if (fReadLine.substr(0,16) == "# Binned by time") {
		if (fBinByTime) {
			G4String expectedLine = "# Binned by time in " + G4UIcommand::ConvertToString(fNEorTBins)
			+ " bins of " + G4UIcommand::ConvertToString(fBinWidth) + " ns"
			+ " from " + G4UIcommand::ConvertToString(fBinMin) + " ns to " + G4UIcommand::ConvertToString(fBinMax) + " ns";
			if (fReadLine != expectedLine) {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << "Ts/RestoreResultsFromFile has been set true," << G4endl;
				G4cerr << "but in input file: " << inputFileSpec << " for Scorer name: " << GetName() << G4endl;
				G4cerr << "has different time binning that this scorer." << G4endl;
				fPm->AbortSession(1);
			}
			getline(inFile,fReadLine);
			getline(inFile,fReadLine);
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "Ts/RestoreResultsFromFile has been set true," << G4endl;
			G4cerr << "but in input file: " << inputFileSpec << " for Scorer name: " << GetName() << G4endl;
			G4cerr << "has time binning while this scorer does not." << G4endl;
			fPm->AbortSession(1);
		}
	}

	if (inputType == "csv") {
		for (int i = 0; i < fNi; i++) {
			for (int j = 0; j < fNj; j++) {
				for (int k = 0; k < fNk; k++) {
					if (inFile.eof()) {
						G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
						G4cerr << "Ts/RestoreResultsFromFile has been set true," << G4endl;
						G4cerr << "but got end of file too early in input file: " << inputFileSpec << " for Scorer name: " << GetName() << G4endl;
						fPm->AbortSession(1);
					}

					if (fNDivisions > 1) {
						GetOneTokenFromReadLine();
						GetOneTokenFromReadLine();
						GetOneTokenFromReadLine();
					}

					if (fNEorTBins == 0) {
						// Not binning by energy, just read one value
						G4int idx = i*fNj*fNk+j*fNk+k;
						ReadOneValueFromASCII(idx);
					} else if (fBinByIncidentEnergy) {
						// Binning by incident track energy, read underflow, energy bins, overflow and no incident track
						G4int idx = (fNEorTBins + 3) * (i*fNj*fNk+j*fNk+k);
						ReadOneValueFromASCII(idx);

						for (int e = 0; e < fNEorTBins; e++) {
							idx = (fNEorTBins + 3) * (i*fNj*fNk+j*fNk+k) + e + 1;
							ReadOneValueFromASCII(idx);
						}

						idx = (fNEorTBins + 3) * (i*fNj*fNk+j*fNk+k) + fNEorTBins + 1;
						ReadOneValueFromASCII(idx);
						idx++;
						ReadOneValueFromASCII(idx);
					} else {
						// Binning by other energy or by time, read underflow, energy or time bins and overflow
						G4int idx = (fNEorTBins + 2) * (i*fNj*fNk+j*fNk+k);
						ReadOneValueFromASCII(idx);

						for (int e = 0; e < fNEorTBins; e++) {
							idx = (fNEorTBins + 2) * (i*fNj*fNk+j*fNk+k) + e + 1;
							ReadOneValueFromASCII(idx);
						}

						idx = (fNEorTBins + 2) * (i*fNj*fNk+j*fNk+k) + fNEorTBins + 1;
						ReadOneValueFromASCII(idx);
					}

					getline(inFile,fReadLine);
				}
			}
		}
	} else if (inputType == "binary") {
		G4String dataFileSpec = fPm->GetStringParameter(GetFullParmName("InputFile")) + ".bin";
		fReadFile.open(dataFileSpec);
		if (!fReadFile) {
			G4cout << "Error opening binary data file:" << dataFileSpec << G4endl;
			fPm->AbortSession(1);
		}

		for (int i = 0; i < fNi; i++) {
			for (int j = 0; j < fNj; j++) {
				for (int k = 0; k < fNk; k++) {
					if (fReadFile.eof()) {
						G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
						G4cerr << "Ts/RestoreResultsFromFile has been set true," << G4endl;
						G4cerr << "but got end of file too early in input file: " << dataFileSpec << " for Scorer name: " << GetName() << G4endl;
						fPm->AbortSession(1);
					}

					if (fNEorTBins == 0) {
						// Not binning by energy, just read one value
						G4int idx = i*fNj*fNk+j*fNk+k;
						ReadOneValueFromBinary(idx);
					} else if (fBinByIncidentEnergy) {
						// Binning by incident track energy, read underflow, energy bins, overflow and no incident track
						G4int idx = (fNEorTBins + 3) * (i*fNj*fNk+j*fNk+k);
						ReadOneValueFromBinary(idx);

						for (int e = 0; e < fNEorTBins; e++) {
							idx = (fNEorTBins + 3) * (i*fNj*fNk+j*fNk+k) + e + 1;
							ReadOneValueFromBinary(idx);
						}

						idx = (fNEorTBins + 3) * (i*fNj*fNk+j*fNk+k) + fNEorTBins + 1;
						ReadOneValueFromBinary(idx);
						idx++;
						ReadOneValueFromBinary(idx);
					} else {
						// Binning by other energy or by time, read underflow, energy or time bins and overflow
						G4int idx = (fNEorTBins + 2) * (i*fNj*fNk+j*fNk+k);
						ReadOneValueFromBinary(idx);

						for (int e = 0; e < fNEorTBins; e++) {
							idx = (fNEorTBins + 2) * (i*fNj*fNk+j*fNk+k) + e + 1;
							ReadOneValueFromBinary(idx);
						}

						idx = (fNEorTBins + 2) * (i*fNj*fNk+j*fNk+k) + fNEorTBins + 1;
						ReadOneValueFromBinary(idx);
					}
				}
			}
		}
	}
}


void TsVBinnedScorer::ReadOneValueFromASCII(G4int idx) {
	G4String token;

	for (G4int i = 0; i < fNReadBackValues; i++) {
		token = GetOneTokenFromReadLine();
		if (fReadBackValues[i] == 0) fSum = G4UIcommand::ConvertToDouble(token);
		else if (fReadBackValues[i] == 1) fMean = G4UIcommand::ConvertToDouble(token);
		else if (fReadBackValues[i] == 2) fScoredHistories = G4UIcommand::ConvertToInt(token);
		else if (fReadBackValues[i] == 3) fCountInBin = G4UIcommand::ConvertToInt(token);
		else if (fReadBackValues[i] == 4) fSecondMoment = G4UIcommand::ConvertToDouble(token);
		else if (fReadBackValues[i] == 5) fVariance = G4UIcommand::ConvertToDouble(token);
		else if (fReadBackValues[i] == 6) fStandardDeviation = G4UIcommand::ConvertToDouble(token);
		else if (fReadBackValues[i] == 7) fMin = G4UIcommand::ConvertToDouble(token);
		else if (fReadBackValues[i] == 8) fMax = G4UIcommand::ConvertToDouble(token);
	}

	StoreOneValue(idx);
}


void TsVBinnedScorer::ReadOneValueFromBinary(G4int idx)
{
	G4double oneDouble;

	for (G4int i = 0; i < fNReadBackValues; i++) {
		fReadFile.read(reinterpret_cast<char*>(&oneDouble), sizeof oneDouble);
		if (fReadBackValues[i] == 0) fSum = oneDouble;
		else if (fReadBackValues[i] == 1) fMean = oneDouble;
		else if (fReadBackValues[i] == 2) fScoredHistories = (G4long)oneDouble;
		else if (fReadBackValues[i] == 3) fCountInBin = (G4long)oneDouble;
		else if (fReadBackValues[i] == 4) fSecondMoment = oneDouble;
		else if (fReadBackValues[i] == 5) fVariance = oneDouble;
		else if (fReadBackValues[i] == 6) fStandardDeviation = oneDouble;
		else if (fReadBackValues[i] == 7) fMin = oneDouble;
		else if (fReadBackValues[i] == 8) fMax = oneDouble;
	}

	StoreOneValue(idx);
}


void TsVBinnedScorer::StoreOneValue(G4int idx) {
	// What we store in fFirstMomentMap depends on what we are reporting
	if (fReportMean || fAccumulateSecondMoment) {
		// fFirstMomentMap will store Mean
		if (fReadBackHasMean) fFirstMomentMap[idx] = fMean * GetUnitValue();
		else if (fScoredHistories == 0)
			fFirstMomentMap[idx] = 0.;
		else fFirstMomentMap[idx] = fSum / fScoredHistories * GetUnitValue();

		if (fAccumulateSecondMoment) {
			if (fReadBackHasSecondMoment) fSecondMomentMap[idx] = fSecondMoment * GetUnitValue() * GetUnitValue();
			else if (fReadBackHasVariance) fSecondMomentMap[idx] = fVariance * (fScoredHistories-1) * GetUnitValue() * GetUnitValue();
			else fSecondMomentMap[idx] = fStandardDeviation * fStandardDeviation * (fScoredHistories-1) * GetUnitValue() * GetUnitValue();
		}
	} else if (fReportSum) {
		// fFirstMomentMap will store Sum
		if (fReadBackHasSum) fFirstMomentMap[idx] = fSum * GetUnitValue();
		else fFirstMomentMap[idx] = fMean * fScoredHistories * GetUnitValue();
	}

	if (fReportCountInBin) fCountMap[idx] = fCountInBin;

	if (fReportMin) fMinMap[idx] = fMin * GetUnitValue();

	if (fReportMax) fMaxMap[idx] = fMax * GetUnitValue();

	if (fReportHistories) {
		if (fReadBackHasHistories) fReportHistories = fScoredHistories;
		else fReportHistories = fSum / fMean;
	}
}


G4String TsVBinnedScorer::GetOneTokenFromReadLine() {
	// Parse the first comma separated token from fReadLine.
	const std::string& delchar = ",";
	std::string::size_type delcharPos = fReadLine.find( delchar );
	G4String oneToken = fReadLine.substr(0, delcharPos);
	fReadLine = fReadLine.substr(delcharPos+1);
	return oneToken;
}


void TsVBinnedScorer::AbsorbResultsFromWorkerScorer(TsVScorer* workerScorer)
{
	TsVBinnedScorer* workerHistScorer = dynamic_cast<TsVBinnedScorer*>(workerScorer);

	// Absorb counts per bin
	if (fAccumulateCount) {
		std::vector<G4long>::iterator itrMaster = fCountMap.begin();
		std::vector<G4long>::iterator itrWorker = workerHistScorer->fCountMap.begin();
		while (itrMaster != fCountMap.end()) {
			*itrMaster += *itrWorker;
			itrMaster++;
			itrWorker++;
		}

		workerHistScorer->fCountMap.assign(fNBins, 0);
	}

	// Absorb min
	if (fReportMin) {
		std::vector<G4double>::iterator itrMinMaster = fMinMap.begin();
		std::vector<G4double>::iterator itrMinWorker = workerHistScorer->fMinMap.begin();

		while (itrMinMaster != fMinMap.end()) {
			if (*itrMinWorker < *itrMinMaster)
				*itrMinMaster = *itrMinWorker;
			itrMinMaster++;
			itrMinWorker++;
		}

		workerHistScorer->fMinMap.assign(fNBins, 9.e+99);
	}

	// Absorb max
	if (fReportMax) {
		std::vector<G4double>::iterator itrMaxMaster = fMaxMap.begin();
		std::vector<G4double>::iterator itrMaxWorker = workerHistScorer->fMaxMap.begin();

		while (itrMaxMaster != fMaxMap.end()) {
			if (*itrMaxWorker > *itrMaxMaster)
				*itrMaxMaster = *itrMaxWorker;
			itrMaxMaster++;
			itrMaxWorker++;
		}

		workerHistScorer->fMaxMap.assign(fNBins, -9.e+99);
	}

	if (fReportMean || fAccumulateSecondMoment) {
		std::vector<G4double>::iterator itrMaster = fFirstMomentMap.begin();
		std::vector<G4double>::iterator itrWorker = workerHistScorer->fFirstMomentMap.begin();
		while (itrMaster != fFirstMomentMap.end()) {
			*itrMaster = ( *itrMaster * fScoredHistories + *itrWorker * workerHistScorer->fScoredHistories ) /
				(fScoredHistories + workerHistScorer->fScoredHistories);
			itrMaster++;
			itrWorker++;
		}

		workerHistScorer->fFirstMomentMap.assign(fNBins, 0);

		if (fAccumulateSecondMoment) {
			std::vector<G4double>::iterator itrSecondMaster = fSecondMomentMap.begin();
			std::vector<G4double>::iterator itrSecondWorker = workerHistScorer->fSecondMomentMap.begin();
			while (itrSecondMaster != fSecondMomentMap.end()) {
				*itrSecondMaster += *itrSecondWorker;
				itrSecondMaster++;
				itrSecondWorker++;
			}

			workerHistScorer->fSecondMomentMap.assign(fNBins, 0);
		}

	} else if (fReportSum || fReportCVolHist || fReportDVolHist) {
		// Only need to absorb sum
		std::vector<G4double>::iterator itrMaster = fFirstMomentMap.begin();
		std::vector<G4double>::iterator itrWorker = workerHistScorer->fFirstMomentMap.begin();
		while (itrMaster != fFirstMomentMap.end()) {
			*itrMaster += *itrWorker;
			itrMaster++;
			itrWorker++;
		}

		workerHistScorer->fFirstMomentMap.assign(fNBins, 0);
	}

	fScoredHistories += workerHistScorer->fScoredHistories;
	fHitsWithNoIncidentParticle += workerHistScorer->fHitsWithNoIncidentParticle;
	fUnscoredSteps += workerHistScorer->fUnscoredSteps;
	fUnscoredEnergy += workerHistScorer->fUnscoredEnergy;

	workerHistScorer->fScoredHistories = 0;
	workerHistScorer->fHitsWithNoIncidentParticle = 0;
	workerHistScorer->fUnscoredSteps = 0;
	workerHistScorer->fUnscoredEnergy = 0.;
}


void TsVBinnedScorer::Output()
{
#ifdef TOPAS_MT
	if (G4Threading::IsWorkerThread()) return;
#endif
	if (fIsSubScorer) return;

	// Account for empty bins in Mean (handled earlier if accumulating second moment)
	if (fAccumulateMean && !fAccumulateSecondMoment)
		for (G4int index = 0; index < fNBins; index++)
			if (fCountMap[index] > 0)
				fFirstMomentMap[index] *= (G4double)fCountMap[index] / fScoredHistories;

	fMissedSubScorerVoxels = CallCombineOnSubScorers();

	if (fNeedToUpdateFileSpecs) {  // implies OutputAfterRun = true
		G4String runString;

		if (fRestoreResultsFromFile) {
			runString = "";
		} else {
			// TsVBinnedScorer updates the filename immediately before writing to disk
			// (not possible for TsVNtupleScorer since output is buffered)
			// The hope is one day for TsVBinnedScorer to use IO classes, and this can be made consistent
			G4int runID = GetRunID();
			G4int nPadding = fPm->GetIntegerParameter("Ts/RunIDPadding");

			std::ostringstream ss;
			ss << "_Run_" << std::setw(nPadding) << std::setfill('0') << runID;
			runString = ss.str();
		}

		G4int increment = 0;
		G4String outFileNameWithRun = fOutFileName + runString;
		if (fOutputToBinary) {
			G4String outFileExt1 = ".binheader";
			G4String outFileExt2 = ".bin";
			if (fNumberOfOutputColumns > 0) {
				fOutFileSpec1 = ConfirmCanOpen(outFileNameWithRun, outFileExt1, increment);
				fOutFileSpec2 = ConfirmCanOpen(outFileNameWithRun, outFileExt2, increment);
			}
			if (fReportCVolHist || fReportDVolHist) {
				fVHOutFileSpec1 = ConfirmCanOpen(outFileNameWithRun+"_VolHist", outFileExt1, increment);
				fVHOutFileSpec2 = ConfirmCanOpen(outFileNameWithRun+"_VolHist", outFileExt2, increment);
			}
		} else if (fOutputToCsv) {
			G4String outFileExt1 = ".csv";
			if (fNumberOfOutputColumns > 0)
				fOutFileSpec1 = ConfirmCanOpen(outFileNameWithRun, outFileExt1, increment);
			if (fReportCVolHist || fReportDVolHist)
				fVHOutFileSpec1 = ConfirmCanOpen(outFileNameWithRun+"_VolHist", outFileExt1, increment);
		} else if (fOutputToDicom) {
			G4String outFileExt1 = ".dcm";
			fOutFileSpec1 = ConfirmCanOpen(outFileNameWithRun, outFileExt1, increment);
		}
	}


	G4cout << "\nScorer: " << GetNameWithSplitId() << G4endl;

	if (fOutFileSpec2!="")
		G4cout << "Results have been written to file: " << fOutFileSpec2 << " with header file: " << fOutFileSpec1 << G4endl;
	else if (fOutFileSpec1!="")
		G4cout << "Results have been written to file: " << fOutFileSpec1 << G4endl;

	if (fSkippedWhileInactive > 0) {
		G4cout << "Warning: Some histories were not scored due to scorer being inactive at least part of the time." << G4endl;
		G4cout << "Total number of histories not scored for this reason: " << fSkippedWhileInactive << G4endl;
	}

	if (fUnscoredSteps > 0) {
		G4cout << "Warning: Some steps were not scored due to scorer being called with an invalid step." << G4endl;
		G4cout << "See console log for messages starting with: Topas experienced a potentially serious error in scoring." << G4endl;
		G4cout << "We believe this error is due to an unsolved bug in Geant4 parallel world handling." << G4endl;
		G4cout << "Total number of steps not scored for this reason: " << fUnscoredSteps << G4endl;
		G4cout << "Total amount of energy not scored for this reason: " << fUnscoredEnergy << " MeV" << G4endl;
	}

	if (fMissedSubScorerVoxels > 0) {
		G4cout << "Warning: Some voxels/bins in the primary scorer did not correspond to an entry in the sub-scorer (or sub-sub-scorers)." << G4endl;
		G4cout << "The value of the combined scorer for those voxels/bins has been set to 0." << G4endl;
		G4cout << "Total number of voxels/bins set to 0 due to this mismatch: " << fMissedSubScorerVoxels << G4endl;
	}

	if (fHitsWithNoIncidentParticle > 0) {
		G4cout << "Warning: One of your filters was based on the energy or momentum of the parent particle that was incident on the scoring component," << G4endl;
		G4cout << "but at least one hit resulted from a primary that was already inside the scoring component when it was generated." << G4endl;
		G4cout << "Such hits are always left out by this incident particle filter." << G4endl;
		G4cout << "Total number of hits not scored for this reason:" << fHitsWithNoIncidentParticle << G4endl;
	}

	if (!fSuppressStandardOutputHandling) {
		if (fSparsify && fPm->ParameterExists(GetFullParmName("SparsifyFactor")))
		{
			G4double sparsifyMaxValue = 0.;
			for (int i = 0; i < fNi; i++) {
				for (int j = 0; j < fNj; j++) {
					for (int k = 0; k < fNk; k++) {
						G4int idx = i*fNj*fNk+j*fNk+k;
						CalculateOneValue(idx);
						if (fSum > sparsifyMaxValue)
							sparsifyMaxValue = fSum;
					}
				}
			}
			fSparsifyThreshold = fPm->GetUnitlessParameter(GetFullParmName("SparsifyFactor")) * sparsifyMaxValue;
		}

		G4String consoleParmName = "Sc/" + GetName() + "/OutputToConsole";
		if ( fPm->ParameterExists(consoleParmName) && fPm->GetBooleanParameter(consoleParmName)) {
			PrintHeader();
			PrintASCII();
		}

		if (fNumberOfOutputColumns > 0) {
			if (fOutputToCsv) {
				std::ofstream ofile(fOutFileSpec1);
				if (ofile) {
					PrintHeader(ofile);
					PrintASCII(ofile);
					ofile.close();
				} else {
					G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
					G4cerr << "Output file: " << fOutFileSpec1 << " cannot be opened for Scorer name: " << GetName() << G4endl;
					fPm->AbortSession(1);
				}
			} else if (fOutputToBinary) {
				std::ofstream hfile(fOutFileSpec1);
				if (hfile) {
					PrintHeader(hfile);
					hfile << "# Binary file: " << fOutFileSpec2 << G4endl;
					hfile.close();
				} else {
					G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
					G4cerr << "Output file: " << fOutFileSpec1 << " cannot be opened for Scorer name: " << GetName() << G4endl;
					fPm->AbortSession(1);
				}

				std::ofstream ofile(fOutFileSpec2, std::ios::out | std::ios::binary);
				if (ofile) {
					PrintBinary(ofile);
					ofile.close();
				} else {
					G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
					G4cerr << "Output file: " << fOutFileSpec2 << " cannot be opened for Scorer name: " << GetName() << G4endl;
					fPm->AbortSession(1);
				}
			} else if (fOutputToDicom) {
				gdcm::ImageReader reader;
				gdcm::SmartPointer<gdcm::File> output_file = new gdcm::File;
				if (fReferencedDicomPatient) {
					G4String readerFileName = fReferencedDicomPatient->GetFirstFileName();
					reader.SetFileName(readerFileName);
					if( !reader.Read() ) {
						G4cout << "At output time, failed to read input DICOM: " << readerFileName << G4endl;
						fPm->AbortSession(1);
					}
					output_file = &reader.GetFile();
				}

				gdcm::Anonymizer anon;
				gdcm::UIDGenerator genUID;
				genUID.SetRoot(fPm->GetDicomRootUID());
				anon.SetFile(*output_file);
				gdcm::MediaStorage mst = gdcm::MediaStorage::RTDoseStorage;
				anon.Replace(gdcm::Tag(0x0008,0x0016), mst.GetString());            // SOP Class UID
				anon.Replace(gdcm::Tag(0x0008,0x0060), mst.GetModality());          // Modality
				anon.Empty(gdcm::Tag(0x0008,0x0008));                               // Image Type
				anon.Replace(gdcm::Tag(0x0008,0x0018), genUID.Generate());          // SOP Instance UID
				anon.Replace(gdcm::Tag(0x0020,0x000e), genUID.Generate());          // Series Instance UID
				anon.Remove(gdcm::Tag(0x0028, 0x1052));								// Rescale intercept
				anon.Remove(gdcm::Tag(0x0028, 0x1053));								// Rescale slope

				G4String seriesDescription = "";
				if (fPm->ParameterExists(GetFullParmName("SeriesDescription"))) {
					seriesDescription = fPm->GetStringParameter(GetFullParmName("SeriesDescription"));
				} else {
					seriesDescription = GetNameWithSplitId();
					if (!GetUnit().empty())
						seriesDescription = seriesDescription + " [" + GetUnit() + "]";
				}
				anon.Replace(gdcm::Tag(0x0008,0x103e), seriesDescription);          // Series Description
				anon.Replace(gdcm::Tag(0x0008,0x0070), "TOPAS");                    // Manufacturer
				G4String topasVersionString = "TOPAS " + fPm->GetTOPASVersion();
				anon.Replace(gdcm::Tag(0x0008,0x1090), topasVersionString);         // Manufacturer's Model Name

				char date[22];
				if (gdcm::System::GetCurrentDateTime(date)) {
					const size_t datelen = 8;
					const size_t timelen = 6;
					std::string tmp(date, datelen);
					anon.Replace(gdcm::Tag(0x0008,0x0012), tmp.c_str());  // Instance Creation Date
					anon.Replace(gdcm::Tag(0x0008,0x0021), tmp.c_str());  // Series Date
					anon.Replace(gdcm::Tag(0x0008,0x0023), tmp.c_str());  // Content Date
					tmp = std::string(date+datelen, timelen);
					anon.Replace(gdcm::Tag(0x0008,0x0013), tmp.c_str());  // Instance Creation Time
					anon.Replace(gdcm::Tag(0x0008,0x0031), tmp.c_str());  // Series Time
					anon.Replace(gdcm::Tag(0x0008,0x0033), tmp.c_str());  // Content Time
				}

				if (!fReferencedDicomPatient) {
					anon.Replace(gdcm::Tag(0x0020,0x000d), fPm->GetStudyInstanceUID());          // Study Instance UID
					anon.Replace(gdcm::Tag(0x0020,0x0052), fPm->GetWorldFrameOfReferenceUID());  // Frame of Reference UID
				}

				gdcm::ImageWriter writer;
				writer.SetFile(*output_file);
				writer.SetFileName(fOutFileSpec1);


				G4double voxelSizeX = fComponent->GetFullWidth(0) / fComponent->GetDivisionCount(0);
				G4double voxelSizeY = fComponent->GetFullWidth(1) / fComponent->GetDivisionCount(1);
				G4double voxelSizeZ = fComponent->GetFullWidth(2) / fComponent->GetDivisionCount(2);
				TsBox* scorerBox = dynamic_cast<TsBox*>(fComponent);
				if (!scorerBox) {
					G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
					G4cerr << "Scorer: " << GetName() << " tried to write DICOM file, but is not attached to TsBox or Patient component." << G4endl;
					fPm->AbortSession(1);
				}
				G4Point3D scorerCornerPos = scorerBox->GetTransFirstVoxelCenterRelToComponentCenter();

				// Transform to DICOM coordinate system
				if (fReferencedDicomPatient) {
					G4Point3D scorerCornerRelToScorer = scorerCornerPos;
					G4Point3D scorerCenterRelToWorld = *(fComponent->GetTransRelToWorld());
					G4Point3D patientCornerRelToDicom = fReferencedDicomPatient->GetTransFirstVoxelCenterRelToDicom();
					G4Point3D patientCenterRelToWorld = *(fReferencedDicomPatient->GetTransRelToWorld());
					G4Point3D patientCornerRelToPatient = fReferencedDicomPatient->GetTransFirstVoxelCenterRelToComponentCenter();

					// transform coordinate system from scorer to World
					G4Translate3D scorerToWorld(scorerCenterRelToWorld.x(), scorerCenterRelToWorld.y(), scorerCenterRelToWorld.z());

					// transform coordinate system from World to DICOM
					G4Translate3D worldToDicom(patientCornerRelToDicom.x() - patientCenterRelToWorld.x() - patientCornerRelToPatient.x(),
											   patientCornerRelToDicom.y() - patientCenterRelToWorld.y() - patientCornerRelToPatient.y(),
											   patientCornerRelToDicom.z() - patientCenterRelToWorld.z() - patientCornerRelToPatient.z());

					G4Point3D scorerCornerRelToDicom = worldToDicom * scorerToWorld * scorerCornerRelToScorer;

					scorerCornerPos = scorerCornerRelToDicom;
				}

				gdcm::SmartPointer<gdcm::Image> image = new gdcm::Image;

				image->SetNumberOfDimensions( 3 );
				image->SetDimension( 0, fNi );
				image->SetDimension( 1, fNj );
				image->SetDimension( 2, fNk );

				double origin[3] = {scorerCornerPos.x(), scorerCornerPos.y(), scorerCornerPos.z()};
				double spacing[3] = {voxelSizeX,voxelSizeY,voxelSizeZ};
				double directionCosines[9] = {1., 0., 0., 0., 1., 0., 0., 0., 1.};

				image->SetSpacing(spacing);
				image->SetOrigin(origin);
				image->SetDirectionCosines(directionCosines);

				image->GetPixelFormat().SetSamplesPerPixel(1);
				image->SetPhotometricInterpretation(gdcm::PhotometricInterpretation::MONOCHROME2);
				gdcm::DataElement pixel_data( gdcm::Tag(0x7fe0,0x0010) );


				// Determine scaling
				G4double maxAbsScoredValue = 0;
				G4double minScoredValue = 0;
				for (int i = 0; i < fNi; i++) {
					for (int j = 0; j < fNj; j++) {
						for (int k = 0; k < fNk; k++) {
							G4int idx = i*fNj*fNk + j*fNk + k;
							maxAbsScoredValue = fmax(maxAbsScoredValue, fabs(fFirstMomentMap[idx]));
							minScoredValue = fmin(minScoredValue, fFirstMomentMap[idx]);
						}
					}
				}

				G4bool useSigned = minScoredValue < 0;
				G4bool use32Bit = false;
				if (fPm->ParameterExists(GetFullParmName("DICOMOutput32BitsPerPixel")))
					use32Bit = fPm->GetBooleanParameter(GetFullParmName("DICOMOutput32BitsPerPixel"));

				G4double outputScaleFactor = maxAbsScoredValue / GetUnitValue();
				if (useSigned && use32Bit)
					outputScaleFactor /= INT32_MAX;
				else if (useSigned && !use32Bit)
					outputScaleFactor /= INT16_MAX;
				else if (!useSigned && use32Bit)
					outputScaleFactor /= UINT32_MAX;
				else if (!useSigned && !use32Bit)
					outputScaleFactor /= UINT16_MAX;
				image->SetSlope(outputScaleFactor);

				image->GetPixelFormat().SetPixelRepresentation(useSigned);
				if (use32Bit) {
					image->GetPixelFormat().SetBitsAllocated(32);
					std::vector<uint32_t> data;
					data.assign(fNDivisions, 0);

					for (int i = 0; i < fNi; i++) {
						for (int j = 0; j < fNj; j++) {
							for (int k = 0; k < fNk; k++) {
								G4int idx = i*fNj*fNk + j*fNk + k;
								G4int idx2 = fNi*fNj*k + fNi*j + i;
								data[idx2] = (uint32_t) (fFirstMomentMap[idx] / outputScaleFactor / GetUnitValue());
							}
						}
					}

					char* buffer = reinterpret_cast<char*>(&data.front());
					unsigned int buffer_size = data.size()*sizeof(uint32_t);
					pixel_data.SetByteValue(buffer, (uint32_t)buffer_size);
				} else {
					image->GetPixelFormat().SetBitsAllocated(16);
					std::vector<uint16_t> data;
					data.assign(fNDivisions, 0);

					for (int i = 0; i < fNi; i++) {
						for (int j = 0; j < fNj; j++) {
							for (int k = 0; k < fNk; k++) {
								G4int idx = i*fNj*fNk + j*fNk + k;
								G4int idx2 = fNi*fNj*k + fNi*j + i;
								data[idx2] = (uint16_t) (fFirstMomentMap[idx] / outputScaleFactor / GetUnitValue());
							}
						}
					}

					char* buffer = reinterpret_cast<char*>(&data.front());
					unsigned int buffer_size = data.size()*sizeof(uint16_t);
					pixel_data.SetByteValue(buffer, (uint32_t)buffer_size);
				}

				image->SetDataElement(pixel_data);
				writer.SetImage(*image);

				if( !writer.Write() )
				{
					G4cout << "Failed on attempt to write output to DICOM file: " << fOutFileSpec1 << G4endl;
					fPm->AbortSession(1);
				}
			}
		}

		// Handle volume histograms
		if (fReportCVolHist || fReportDVolHist) {
			fVolumeHistogramBinCounts = new G4int[fHistogramBins];
			for (int j = 0; j < fHistogramBins; j++)
				fVolumeHistogramBinCounts[j] = 0;

			fNVoxelsInVolumeHistogram = 0;
			for (int idx = 0; idx < fNDivisions; idx++) {
				CalculateOneValue(idx);
				if (fSum >= 0.) {
					fNVoxelsInVolumeHistogram++;
					for (int j = fHistogramBins-1; j >= 0; j--) {
						if (fSum >= fHistogramLowerValues[j]) {
							fVolumeHistogramBinCounts[j]++;
							if (fReportDVolHist) break;
						}
					}
				}
			}

			if ( fPm->ParameterExists(consoleParmName) && fPm->GetBooleanParameter(consoleParmName)) {
				G4cout << G4endl;
				PrintVHHeader();
				PrintVHASCII();
			}

			if (fOutputToCsv) {
				std::ofstream ofile(fVHOutFileSpec1);
				if (ofile) {
					PrintVHHeader(ofile);
					PrintVHASCII(ofile);
					ofile.close();
				} else {
					G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
					G4cerr << "Output file: " << fVHOutFileSpec1 << " cannot be opened for Scorer name: " << GetName() << G4endl;
					fPm->AbortSession(1);
				}
			} else if (fOutputToBinary) {
				std::ofstream hfile(fVHOutFileSpec1);
				if (hfile) {
					PrintVHHeader(hfile);
					hfile << "# Binary file: " << fVHOutFileSpec2 << G4endl;
					hfile.close();
				} else {
					G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
					G4cerr << "Output file: " << fVHOutFileSpec1 << " cannot be opened for Scorer name: " << GetName() << G4endl;
					fPm->AbortSession(1);
				}

				std::ofstream ofile(fVHOutFileSpec2, std::ios::out | std::ios::binary);
				if (ofile) {
					PrintVHBinary(ofile);
					ofile.close();
				} else {
					G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
					G4cerr << "Output file: " << fVHOutFileSpec2 << " cannot be opened for Scorer name: " << GetName() << G4endl;
					fPm->AbortSession(1);
				}
			} else if (fOutputToRoot || fOutputToXml) {
				G4VAnalysisManager* analysisManager;
				if (fOutputToRoot)
					analysisManager = fScm->GetRootAnalysisManager();
				else
					analysisManager = fScm->GetXmlAnalysisManager();

				G4double scaleFactor = 1. / fNVoxelsInVolumeHistogram;

				for (int j = 0; j < fHistogramBins; j++)
					for (int k = 0; k < fVolumeHistogramBinCounts[j]; k++)
						analysisManager->FillH1(fVHistogramID, fHistogramLowerValues[j], scaleFactor);
			}
		}

		if ( fReportOutcome ) {
			G4String modelSource;
			std::vector<G4double> volume;
			if ( fReportCVolHist || fReportDVolHist ) {
				modelSource = "DVH";
				for ( int i = 0; i < fHistogramBins; i++ ) {
					volume.push_back( fVolumeHistogramBinCounts[i] );
					fHistogramLowerValues[i] *= fNormFactor;
				}
				fProbOfOutcome = fOm->CalculateOutcome(fHistogramLowerValues, volume, fReportDVolHist);
			} else {
				modelSource = "Full Dose Distribution";
				std::vector<G4double> dose;
				// Here we assume that voxels have the same size.
				// The model works with relative volume, then
				// model can works with volume vector of 1.0
				for (int idx = 0; idx < fNDivisions; idx++) {
					CalculateOneValue(idx);
					if (fSum >= 0.) {
						dose.push_back(fSum*fNormFactor);
						volume.push_back(1.0);
					}
				}
				fProbOfOutcome = fOm->CalculateOutcome(dose, volume, true);
			}
			if (fPm->ParameterExists(GetFullParmName("Surface")))
				modelSource += " on surface " + fComponent->GetName() + "/" + fPm->GetStringParameter(GetFullParmName("Surface"));
			else
				modelSource += " in component " + fComponent->GetName();

			fOm->Print(modelSource);
		}
	}

	if (fColorBy == "sum")
		ColorBy(fSum);
	else if (fColorBy == "mean")
		ColorBy(fMean);
	else if (fColorBy == "histories")
		ColorBy(fScoredHistories);
	else if (fColorBy == "count_in_bin")
		ColorBy(fCountInBin);
	else if (fColorBy == "standard_deviation")
		ColorBy(fStandardDeviation);
	else if (fColorBy == "min")
		ColorBy(fMin);
	else if (fColorBy == "max")
		ColorBy(fMax);
}


void TsVBinnedScorer::Clear()
{
	fSkippedWhileInactive = 0;
	fUnscoredSteps = 0;
	fUnscoredEnergy = 0.;
	fMissedSubScorerVoxels = 0;
	fHitsWithNoIncidentParticle = 0;
	fOutputPosition = 0;

	fScoredHistories = 0;

	if (fReportCountInBin || fReportMean || fAccumulateSecondMoment)
		fCountMap.assign(fNBins, 0);

	if (fReportMin)
		fMinMap.assign(fNBins,  9.e+99);

	if (fReportMax)
		fMaxMap.assign(fNBins, -9.e+99);

	if (fReportSum || fReportMean || fAccumulateSecondMoment || fReportCVolHist || fReportDVolHist)
		fFirstMomentMap.assign(fNBins, 0.);

	if (fReportMean || fAccumulateSecondMoment)
		fSecondMomentMap.assign(fNBins, 0.);
}


G4String TsVBinnedScorer::ConfirmCanOpen(G4String fileName, G4String fileExt, G4int& increment) {
	// Check whether the file already exists
	G4String fileSpec;
	if (increment==0)
		fileSpec = fileName + fileExt;
	else
		fileSpec = fileName + "_" + G4UIcommand::ConvertToString(increment) + fileExt;

	std::fstream fin;
	fin.open(fileSpec,std::ios::in);

	if (fin.is_open()) {
		if (fPm->ParameterExists(GetFullParmName("IfOutputFileAlreadyExists"))) {
			G4String howToHandle = fPm->GetStringParameter(GetFullParmName("IfOutputFileAlreadyExists"));
#if GEANT4_VERSION_MAJOR >= 11
			G4StrUtil::to_lower(howToHandle);
#else
			howToHandle.toLower();
#endif
			if (howToHandle == "overwrite") {
				// Continue to use this file spec
			} else if (howToHandle == "increment") {
				// Iterate with increased increment as many times as necessary to find an unused filespec
				increment++;
				fileSpec = ConfirmCanOpen(fileName, fileExt, increment);
			} else if (howToHandle == "exit") {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << "Output file: " << fileSpec << " already exists for Scorer name: " << GetName() << G4endl;
				G4cerr << "If you really want to allow this, specify: " << G4endl;
				G4cerr << GetFullParmName("IfOutputFileAlreadyExists") << " as Overwrite or Increment." << G4endl;
				fPm->AbortSession(1);
			} else {
				G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
				G4cerr << "Scorer name: " << GetName() << " has invalid value for parameter: " << GetFullParmName("IfOutputFileAlreadyExists") << G4endl;
				G4cerr << "Allowed values are Exit, Overwrite or Increment" << G4endl;
				fPm->AbortSession(1);
			}
		} else {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "Output file: " << fileSpec << " already exists for Scorer name: " << GetName() << G4endl;
			G4cerr << "If you really want to allow this, specify: " << G4endl;
			G4cerr << GetFullParmName("IfOutputFileAlreadyExists") << " as Overwrite or Increment." << G4endl;
			fPm->AbortSession(1);
		}
	}
	fin.close();

	// Check whether the file can be opened
	std::ofstream ofile(fileSpec);
	if (ofile) {
		// Confirmed the file can be opened. Close it for now.
		ofile.close();
	} else {
		G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
		G4cerr << "Output file: " << fileSpec << " cannot be opened for Scorer name: " << GetName() << G4endl;
		fPm->AbortSession(1);
	}

	return fileSpec;
}


void TsVBinnedScorer::PrintHeader()
{
	G4String filterName;
	G4String parmName;
	std::vector<G4String>* filterNames = fPm->GetFilterNames();
	for (G4int iFilter = 0; iFilter < (G4int)filterNames->size(); iFilter++) {
		filterName = (*filterNames)[iFilter];
		parmName = "Sc/" + GetName() + "/" + filterName;
		if (fPm->ParameterExists(parmName))
			G4cout << "Filtered by: " << filterName << " = " << fPm->GetParameterValueAsString(fPm->GetTypeOfParameter(parmName), parmName) << G4endl;
	}

	if (fPm->ParameterExists(GetFullParmName("Surface")))
		G4cout << "Scored on surface: " << fPm->GetStringParameter(GetFullParmName("Surface")) << G4endl;
	else
		G4cout << "Scored in component: " << fComponent->GetName() << G4endl;

	if (fNDivisions > 1)
	{
		G4cout << fComponent->GetBinHeader(0, fNi) << G4endl;
		G4cout << fComponent->GetBinHeader(1, fNj) << G4endl;
		G4cout << fComponent->GetBinHeader(2, fNk) << G4endl;
	}

	G4cout << fQuantity;

	if (GetUnit()!="")
		G4cout << " ( " << GetUnit() << " )";
	else if (fScm->AddUnitEvenIfItIsOne())
		G4cout << " ( 1 )";

	G4cout << " : ";

	for (G4int i = 0; i < fNReportValues; i++) {
		if (fReportValues[i] == 0)
			G4cout << "Sum   ";
		else if (fReportValues[i] == 1)
			G4cout << "Mean   ";
		else if (fReportValues[i] == 2)
			G4cout << "Histories_with_Scorer_Active   ";
		else if (fReportValues[i] == 3)
			G4cout << "Count_in_Bin   ";
		else if (fReportValues[i] == 4)
			G4cout << "Second_Moment   ";
		else if (fReportValues[i] == 5)
			G4cout << "Variance   ";
		else if (fReportValues[i] == 6)
			G4cout << "Standard_Deviation   ";
		else if (fReportValues[i] == 7)
			G4cout << "Min   ";
		else if (fReportValues[i] == 8)
			G4cout << "Max   ";
	}

	G4cout << G4endl;

	if (fBinByIncidentEnergy) {
		G4cout << "Binned by incident track energy in " << fNEorTBins << " bins of " << fBinWidth << " MeV"
		<< " from " << fBinMin << " MeV to " << fBinMax << " MeV" << G4endl;
		G4cout << "First bin is underflow, next to last bin is overflow, final bin is for case of no incident track." << G4endl;
	}

	if (fBinByPreStepEnergy) {
		G4cout << "Binned by pre-step energy in " << fNEorTBins << " bins of " << fBinWidth << " MeV"
		<< " from " << fBinMin << " MeV to " << fBinMax << " MeV" << G4endl;
		G4cout << "First bin is underflow, last bin is overflow." << G4endl;
	}

	if (fBinByStepDepositEnergy) {
		G4cout << "Binned by energy deposited in step in " << fNEorTBins << " bins of " << fBinWidth << " MeV"
		<< " from " << fBinMin << " MeV to " << fBinMax << " MeV" << G4endl;
		G4cout << "First bin is underflow, last bin is overflow." << G4endl;
	}

	if (fBinByPrimaryEnergy) {
		G4cout << "Binned by primary track energy in " << fNEorTBins << " bins of " << fBinWidth << " MeV"
		<< " from " << fBinMin << " MeV to " << fBinMax << " MeV" << G4endl;
		G4cout << "First bin is underflow, last bin is overflow." << G4endl;
	}

	if (fBinByTime) {
		G4cout << "Binned by time in " << fNEorTBins << " bins of " << fBinWidth << " ns"
		<< " from " << fBinMin << " ns to " << fBinMax << " ns" << G4endl;
		G4cout << "First bin is underflow, last bin is overflow." << G4endl;
	}

	if (!fOutputToBinary) {
		if (fSparsify)
			G4cout << "Showing only rows that have sum > " << fSparsifyThreshold << " " << GetUnit() << G4endl;

		if (fSingleIndex && (fNDivisions > 1))
			G4cout << "Using single index calculated as " <<
			fComponent->GetDivisionName(0) << " * N" << fComponent->GetDivisionName(1) <<
			" * N" << fComponent->GetDivisionName(2) << " + " <<
			fComponent->GetDivisionName(1) << " * N" << fComponent->GetDivisionName(2) <<
			" + " << fComponent->GetDivisionName(2) << G4endl;
	}
}


void TsVBinnedScorer::PrintHeader(std::ostream& ofile)
{
	ofile << "# TOPAS Version: " << fPm->GetTOPASVersion() << G4endl;
	ofile << "# Parameter File: " << fPm->GetTopParameterFileSpec() << G4endl;
	ofile << "# Results for scorer: " << GetNameWithSplitId() << G4endl;

	G4String filterName;
	G4String parmName;
	std::vector<G4String>* filterNames = fPm->GetFilterNames();
	for (G4int iFilter = 0; iFilter < (G4int)filterNames->size(); iFilter++) {
		filterName = (*filterNames)[iFilter];
		parmName = "Sc/" + GetName() + "/" + filterName;
		if (fPm->ParameterExists(parmName))
			ofile << "# Filtered by: " << filterName << " = " << fPm->GetParameterValueAsString(fPm->GetTypeOfParameter(parmName), parmName) << G4endl;
	}

	if (fPm->ParameterExists(GetFullParmName("Surface")))
		ofile << "# Scored on surface: " << fPm->GetStringParameter(GetFullParmName("Surface")) << G4endl;
	else
		ofile << "# Scored in component: " << fComponent->GetName() << G4endl;

	if (fSkippedWhileInactive > 0) {
		ofile << "# Warning: Some histories were not scored due to scorer being inactive at least part of the time." << G4endl;
		ofile << "# Total number of histories not scored for this reason: " << fSkippedWhileInactive << G4endl;
	}

	if (fUnscoredSteps > 0) {
		ofile << "# Warning: Some steps were not scored due to touchable returning an invalid index number." << G4endl;
		ofile << "# See console log for messages starting with: Topas experienced a potentially serious error in scoring." << G4endl;
		ofile << "# We believe this error is due to an unsolved bug in Geant4 parallel world handling." << G4endl;
		ofile << "# Total number of steps not scored for this reason: " << fUnscoredSteps << G4endl;
		ofile << "# Total amount of energy not scored for this reason: " << fUnscoredEnergy << " MeV" << G4endl;
	}

	if (fMissedSubScorerVoxels > 0) {
		ofile << "# Warning: Some voxels/bins in the primary scorer did not correspond to an entry in the sub-scorer (or sub-sub-scorers)." << G4endl;
		ofile << "# The value of the combined scorer for those voxels/bins has been set to 0." << G4endl;
		ofile << "# Total number of voxels/bins set to 0 due to this mismatch: " << fMissedSubScorerVoxels << G4endl;
	}

	if (fHitsWithNoIncidentParticle > 0) {
		ofile << "# Warning: One of your filters was based on the energy or momentum of the parent particle that was incident on the scoring component," << G4endl;
		ofile << "# but at least one hit resulted from a primary that was already inside the scoring component when it was generated." << G4endl;
		ofile << "# Such hits are always left out by this incident particle filter." << G4endl;
		ofile << "# Total number of hits not scored for this reason:" << fHitsWithNoIncidentParticle << G4endl;
	}

	if (fNDivisions > 1)
	{
		ofile << "# " << fComponent->GetBinHeader(0, fNi) << G4endl;
		ofile << "# " << fComponent->GetBinHeader(1, fNj) << G4endl;
		ofile << "# " << fComponent->GetBinHeader(2, fNk) << G4endl;
	}

	ofile << "# " << fQuantity;

	if (GetUnit()!="")
		ofile << " ( " << GetUnit() << " )";
	else if (fScm->AddUnitEvenIfItIsOne())
		ofile << " ( 1 )";

	ofile << " : ";

	for (G4int i = 0; i < fNReportValues; i++) {
		if (fReportValues[i] == 0)
			ofile << "Sum   ";
		else if (fReportValues[i] == 1)
			ofile << "Mean   ";
		else if (fReportValues[i] == 2)
			ofile << "Histories_with_Scorer_Active   ";
		else if (fReportValues[i] == 3)
			ofile << "Count_in_Bin   ";
		else if (fReportValues[i] == 4)
			ofile << "Second_Moment   ";
		else if (fReportValues[i] == 5)
			ofile << "Variance   ";
		else if (fReportValues[i] == 6)
			ofile << "Standard_Deviation   ";
		else if (fReportValues[i] == 7)
			ofile << "Min   ";
		else if (fReportValues[i] == 8)
			ofile << "Max   ";
	}

	ofile << G4endl;

	if (fBinByIncidentEnergy) {
		ofile << "# " << "Binned by incident track energy in " << fNEorTBins << " bins of " << fBinWidth << " MeV"
		<< " from " << fBinMin << " MeV to " << fBinMax << " MeV" << G4endl;
		ofile << "# " << "First bin is underflow, next to last bin is overflow, last bin is for case of no incident track." << G4endl;
	}

	if (fBinByPreStepEnergy) {
		ofile << "# " << "Binned by pre-step energy in " << fNEorTBins << " bins of " << fBinWidth << " MeV"
		<< " from " << fBinMin << " MeV to " << fBinMax << " MeV" << G4endl;
		ofile << "# " << "First bin is underflow, last bin is overflow." << G4endl;
	}

	if (fBinByStepDepositEnergy) {
		ofile << "# " << "Binned by energy deosited in step in " << fNEorTBins << " bins of " << fBinWidth << " MeV"
		<< " from " << fBinMin << " MeV to " << fBinMax << " MeV" << G4endl;
		ofile << "# " << "First bin is underflow, last bin is overflow." << G4endl;
	}

	if (fBinByPrimaryEnergy) {
		ofile << "# " << "Binned by primary track energy in " << fNEorTBins << " bins of " << fBinWidth << " MeV"
		<< " from " << fBinMin << " MeV to " << fBinMax << " MeV" << G4endl;
		ofile << "# " << "First bin is underflow, next to last bin is overflow." << G4endl;
	}

	if (fBinByTime) {
		ofile << "# " << "Binned by time in " << fNEorTBins << " bins of " << fBinWidth << " ns"
		<< " from " << fBinMin << " ns to " << fBinMax << " ns" << G4endl;
		ofile << "# " << "First bin is underflow, last bin is overflow." << G4endl;
	}

	if (!fOutputToBinary) {
		if (fSparsify)
			ofile << "# Showing only rows that have sum > " << fSparsifyThreshold << " " << GetUnit() << G4endl;

		if (fSingleIndex && (fNDivisions > 1))
			ofile << "# Using single index calculated as " <<
			fComponent->GetDivisionName(0) << " * N" << fComponent->GetDivisionName(1) <<
			" * N" << fComponent->GetDivisionName(2) << " + " <<
			fComponent->GetDivisionName(1) << " * N" << fComponent->GetDivisionName(2) <<
			" + " << fComponent->GetDivisionName(2) << G4endl;
	}
}


void TsVBinnedScorer::PrintASCII(std::ostream& ofile)
{
	ofile << std::setprecision(16); // for double value with 8 bytes

	for (int i = 0; i < fNi; i++) {
		for (int j = 0; j < fNj; j++) {
			for (int k = 0; k < fNk; k++) {
				fNeedComma = false;

				if (fNEorTBins == 0) {
					// Not binning by energy, just print one value
					G4int idx = i*fNj*fNk+j*fNk+k;
					CalculateOneValue(idx);
					if (!fSparsify || fSum > fSparsifyThreshold) {
						if (fNDivisions > 1) {
							if (fSingleIndex)
								ofile << idx << ", ";
							else
								ofile << i << ", " << j << ", " << k << ", ";
						}
						PrintOneValueToASCII(ofile);
						ofile << G4endl;
					}
				} else if (fBinByIncidentEnergy) {
					// Binning by energy, print underflow, energy bins, overflow and no incident particle
					G4int idx = (fNEorTBins + 3) * (i*fNj*fNk+j*fNk+k);
					CalculateOneValue(idx);
					PrintOneValueToASCII(ofile);
					fNeedComma = true;

					for (int e = 0; e < fNEorTBins; e++) {
						idx = (fNEorTBins + 3) * (i*fNj*fNk+j*fNk+k) + e + 1;
						CalculateOneValue(idx);
						PrintOneValueToASCII(ofile);
					}

					idx = (fNEorTBins + 3) * (i*fNj*fNk+j*fNk+k) + fNEorTBins + 1;
					CalculateOneValue(idx);
					PrintOneValueToASCII(ofile);
					idx++;
					CalculateOneValue(idx);
					PrintOneValueToASCII(ofile);
					ofile << G4endl;
				} else {
					// Binning by other energy or by time, print underflow, energy or time bins and overflow
					G4int idx = (fNEorTBins + 2) * (i*fNj*fNk+j*fNk+k);
					CalculateOneValue(idx);
					PrintOneValueToASCII(ofile);
					fNeedComma = true;

					for (int e = 0; e < fNEorTBins; e++) {
						idx = (fNEorTBins + 2) * (i*fNj*fNk+j*fNk+k) + e + 1;
						CalculateOneValue(idx);
						PrintOneValueToASCII(ofile);
					}

					idx = (fNEorTBins + 2) * (i*fNj*fNk+j*fNk+k) + fNEorTBins + 1;
					CalculateOneValue(idx);
					PrintOneValueToASCII(ofile);
					ofile << G4endl;
				}
			}
		}
	}
	ofile << std::setprecision(6);
}


void TsVBinnedScorer::PrintBinary(std::ostream& ofile)
{
	G4int size = fNumberOfOutputColumns*fNDivisions;
	// If binning by energy, every row needs energy bins plus underflow, overflow and no incident track bins
	if (fBinByIncidentEnergy) size = size * (fNEorTBins + 3);
	// If binning by time, every row needs energy bins plus underflow and overflow bins
	if (fBinByPreStepEnergy || fBinByStepDepositEnergy || fBinByPrimaryEnergy || fBinByTime) size = size * (fNEorTBins + 2);

	G4double* data = new G4double[size];

	for (int k = 0; k < fNk; k++) {
		for (int j = 0; j < fNj; j++) {
			for (int i = 0; i < fNi; i++) {
				if (fNEorTBins == 0) {
					// Not binning by energy, just print one value
					G4int idx = i*fNj*fNk+j*fNk+k;
					PrintOneValueToBinary(idx, data);
				} else if (fBinByIncidentEnergy) {
					// Binning by energy, print underflow, energy bins, overflow and no incident track
					G4int idx = (fNEorTBins + 3) * (i*fNj*fNk+j*fNk+k);
					PrintOneValueToBinary(idx, data);

					for (int e = 0; e < fNEorTBins; e++) {
						idx = (fNEorTBins + 3) * (i*fNj*fNk+j*fNk+k) + e + 1;
						PrintOneValueToBinary(idx, data);
					}

					idx = (fNEorTBins + 3) * (i*fNj*fNk+j*fNk+k) + fNEorTBins + 1;
					PrintOneValueToBinary(idx, data);
					idx++;
					PrintOneValueToBinary(idx, data);
				} else {
					// Binning by other energy time, print underflow, energy or time bins and overflow
					G4int idx = (fNEorTBins + 2) * (i*fNj*fNk+j*fNk+k);
					PrintOneValueToBinary(idx, data);

					for (int e = 0; e < fNEorTBins; e++) {
						idx = (fNEorTBins + 2) * (i*fNj*fNk+j*fNk+k) + e + 1;
						PrintOneValueToBinary(idx, data);
					}

					idx = (fNEorTBins + 2) * (i*fNj*fNk+j*fNk+k) + fNEorTBins + 1;
					PrintOneValueToBinary(idx, data);
				}
			}
		}
	}
	ofile.write( (char*) data, size*sizeof(G4double));
	delete[] data;
}


void TsVBinnedScorer::PrintOneValueToASCII(std::ostream& ofile)
{
	for (G4int i = 0; i < fNReportValues; i++) {
		if (fReportValues[i] == 0) {
			if (fNeedComma) ofile << ", ";
			ofile << fSum;
			fNeedComma = true;
		} else if (fReportValues[i] == 1) {
			if (fNeedComma) ofile << ", ";
			ofile << fMean;
			fNeedComma = true;
		} else if (fReportValues[i] == 2) {
			if (fNeedComma) ofile << ", ";
			ofile << fScoredHistories;
			fNeedComma = true;
		} else if (fReportValues[i] == 3) {
			if (fNeedComma) ofile << ", ";
			ofile << fCountInBin;
			fNeedComma = true;
		} else if (fReportValues[i] == 4) {
			if (fNeedComma) ofile << ", ";
			ofile << fSecondMoment;
			fNeedComma = true;
		} else if (fReportValues[i] == 5) {
			if (fNeedComma) ofile << ", ";
			ofile << fVariance;
			fNeedComma = true;
		} else if (fReportValues[i] == 6) {
			if (fNeedComma) ofile << ", ";
			ofile << fStandardDeviation;
			fNeedComma = true;
		} else if (fReportValues[i] == 7) {
			if (fNeedComma) ofile << ", ";
			ofile << fMin;
			fNeedComma = true;
		} else if (fReportValues[i] == 8) {
			if (fNeedComma) ofile << ", ";
			ofile << fMax;
			fNeedComma = true;
		}
	}
}


void TsVBinnedScorer::PrintOneValueToBinary(G4int idx, G4double* data)
{
	CalculateOneValue(idx);

	for (G4int i = 0; i < fNReportValues; i++) {
		if (fReportValues[i] == 0)
			data[fOutputPosition++] = fSum;
		else if (fReportValues[i] == 1)
			data[fOutputPosition++] = fMean;
		else if (fReportValues[i] == 2)
			data[fOutputPosition++] = fScoredHistories;
		else if (fReportValues[i] == 3)
			data[fOutputPosition++] = fCountInBin;
		else if (fReportValues[i] == 4)
			data[fOutputPosition++] = fSecondMoment;
		else if (fReportValues[i] == 5)
			data[fOutputPosition++] = fVariance;
		else if (fReportValues[i] == 6)
			data[fOutputPosition++] = fStandardDeviation;
		else if (fReportValues[i] == 7)
			data[fOutputPosition++] = fMin;
		else if (fReportValues[i] == 8)
			data[fOutputPosition++] = fMax;
	}
}


void TsVBinnedScorer::PrintVHHeader()
{
	G4cout << "\nScorer: " << GetNameWithSplitId() << G4endl;

	if (fVHOutFileSpec2!="")
		G4cout << "Results have been written to file: " << fVHOutFileSpec2 << " with header file: " << fVHOutFileSpec1 << G4endl;
	else if (fVHOutFileSpec1!="")
		G4cout << "Results have been written to file: " << fVHOutFileSpec1 << G4endl;

	if (fSkippedWhileInactive > 0) {
		G4cout << "Warning: Some histories were not scored due to scorer being inactive at least part of the time." << G4endl;
		G4cout << "Total number of histories not scored for this reason: " << fSkippedWhileInactive << G4endl;
	}

	if (fUnscoredSteps > 0) {
		G4cout << "Warning: Some steps were not scored due to touchable returning an invalid index number." << G4endl;
		G4cout << "See console log for messages starting with: Topas experienced a potentially serious error in scoring." << G4endl;
		G4cout << "We believe this error is due to an unsolved bug in Geant4 parallel world handling." << G4endl;
		G4cout << "Total number of steps not scored for this reason: " << fUnscoredSteps << G4endl;
		G4cout << "Total amount of energy not scored for this reason: " << fUnscoredEnergy << " MeV" << G4endl;
	}

	if (fMissedSubScorerVoxels > 0) {
		G4cout << "Warning: Some voxels/bins in the primary scorer did not correspond to an entry in the sub-scorer (or sub-sub-scorers)." << G4endl;
		G4cout << "The value of the combined scorer for those voxels/bins has been set to 0." << G4endl;
		G4cout << "Total number of voxels/bins set to 0 due to this mismatch: " << fMissedSubScorerVoxels << G4endl;
	}

	if (fHitsWithNoIncidentParticle > 0) {
		G4cout << "Warning: One of your filters was based on the energy or momentum of the parent particle that was incident on the scoring component," << G4endl;
		G4cout << "but at least one hit resulted from a primary that was already inside the scoring component when it was generated." << G4endl;
		G4cout << "Such hits are always left out by this incident particle filter." << G4endl;
		G4cout << "Total number of hits not scored for this reason:" << fHitsWithNoIncidentParticle << G4endl;
	}

	if (fPm->ParameterExists(GetFullParmName("Surface")))
		G4cout << "Scored on surface: " << fPm->GetStringParameter(GetFullParmName("Surface")) << G4endl;
	else
		G4cout << "Scored in component: " << fComponent->GetName() << G4endl;

	if (fReportCVolHist)
		G4cout << "Cumulative Volume Histogram over number of voxels: " << fNVoxelsInVolumeHistogram << G4endl;
	else
		G4cout << "Differential Volume Histogram over number of voxels: " << fNVoxelsInVolumeHistogram << G4endl;

	G4cout << "BinNumber, LowerLimit of " << fQuantity;

	if (GetUnit()!="")
		G4cout << " ( " << GetUnit() << " )";
	else if (fScm->AddUnitEvenIfItIsOne())
		G4cout << " ( 1 )";

	G4cout << ", Value" << G4endl;
}


void TsVBinnedScorer::PrintVHHeader(std::ostream& ofile)
{
	ofile << "# TOPAS Version: " << fPm->GetTOPASVersion() << G4endl;
	ofile << "# Parameter File: " << fPm->GetTopParameterFileSpec() << G4endl;
	ofile << "# Results for Scorer: " << GetNameWithSplitId() << G4endl;

	if (fSkippedWhileInactive > 0) {
		ofile << "# Warning: Some histories were not scored due to scorer being inactive at least part of the time." << G4endl;
		ofile << "# Total number of histories not scored for this reason: " << fSkippedWhileInactive << G4endl;
	}

	if (fUnscoredSteps > 0) {
		ofile << "# Warning: Some steps were not scored due to touchable returning an invalid index number." << G4endl;
		ofile << "# See console log for messages starting with: Topas experienced a potentially serious error in scoring." << G4endl;
		ofile << "# We believe this error is due to an unsolved bug in Geant4 parallel world handling." << G4endl;
		ofile << "# Total number of steps not scored for this reason: " << fUnscoredSteps << G4endl;
		ofile << "# Total amount of energy not scored for this reason: " << fUnscoredEnergy << " MeV" << G4endl;
	}

	if (fMissedSubScorerVoxels > 0) {
		ofile << "# Warning: Some voxels/bins in the primary scorer did not correspond to an entry in the sub-scorer (or sub-sub-scorers)." << G4endl;
		ofile << "# The value of the combined scorer for those voxels/bins has been set to 0." << G4endl;
		ofile << "# Total number of voxels/bins set to 0 due to this mismatch: " << fMissedSubScorerVoxels << G4endl;
	}

	if (fHitsWithNoIncidentParticle > 0) {
		ofile << "# Warning: One of your filters was based on the energy or momentum of the parent particle that was incident on the scoring component," << G4endl;
		ofile << "# but at least one hit resulted from a primary that was already inside the scoring component when it was generated." << G4endl;
		ofile << "# Such hits are always left out by this incident particle filter." << G4endl;
		ofile << "# Total number of hits not scored for this reason:" << fHitsWithNoIncidentParticle << G4endl;
	}

	if (fPm->ParameterExists(GetFullParmName("Surface")))
		ofile << "# Scored on surface: " << fPm->GetStringParameter(GetFullParmName("Surface")) << G4endl;
	else
		ofile << "# Scored in component: " << fComponent->GetName() << G4endl;

	if (fReportCVolHist)
		ofile << "# Cumulative Volume Histogram over number of voxels: " << fNVoxelsInVolumeHistogram << G4endl;
	else
		ofile << "# Differential Volume Histogram over number of voxels: " << fNVoxelsInVolumeHistogram << G4endl;

	ofile << "# BinNumber, LowerLimit of " << fQuantity;

	if (GetUnit()!="")
		ofile << " ( " << GetUnit() << " )";
	else if (fScm->AddUnitEvenIfItIsOne())
		ofile << " ( 1 )";

	ofile << ", Value" << G4endl;
}


void TsVBinnedScorer::PrintVHASCII(std::ostream& ofile)
{
	ofile << std::setprecision(16); // for double value with 8 bytes
	G4double scaleFactor = 1. / fNVoxelsInVolumeHistogram;

	for (int j = 0; j < fHistogramBins; j++)
		ofile << j << ", " << fHistogramLowerValues[j] << ", " << fVolumeHistogramBinCounts[j] * scaleFactor << G4endl;

	ofile << std::setprecision(6);
}


void TsVBinnedScorer::PrintVHBinary(std::ostream& ofile)
{
	G4double* data = new G4double[fHistogramBins];

	for (int j = 0; j < fHistogramBins; j++)
		data[j] = fVolumeHistogramBinCounts[j]/fNVoxelsInVolumeHistogram;

	ofile.write( (char*) data, fHistogramBins*sizeof(G4double));
	delete[] data;
}


void TsVBinnedScorer::CalculateOneValue(G4int idx)
{
	// None of this is required if all we are doing is reporting number of scored histories
	if (fReportCountInBin || fReportMin || fReportMax || fReportSum || fReportMean || fAccumulateSecondMoment) {
		G4bool excluded = ExcludedByRTStructFilter(idx);

		if (fReportCountInBin) {
			if (excluded)
				fCountInBin = -1;
			else
				fCountInBin = fCountMap[idx];
		}

		if (fReportMin) {
			if (excluded)
				fMin = -1;
			else
				fMin = fMinMap[idx] / GetUnitValue();
		}

		if (fReportMax) {
			if (excluded)
				fMax = -1;
			else
				fMax = fMaxMap[idx] / GetUnitValue();
		}

		if (fReportSum || fReportMean || fAccumulateSecondMoment || fReportCVolHist || fReportDVolHist) {
			if (excluded) {
				fMean = -1;
				fSum = -1;
				fSecondMoment = -1;
				fVariance = -1;
				fStandardDeviation = -1;
			} else {
				if (fFirstMomentMap[idx]==0.) {
					fMean = 0.;
					fSum = 0.;
					fSecondMoment = 0.;
					fVariance = 0.;
					fStandardDeviation = 0.;
				} else {
					if (fReportMean || fAccumulateSecondMoment) {
						fMean = fFirstMomentMap[idx] / GetUnitValue();
						fSum = fScoredHistories * fMean;

						if (fAccumulateSecondMoment) {
							fSecondMoment = fSecondMomentMap[idx] / GetUnitValue() / GetUnitValue();
							if (fScoredHistories > 1)
								fVariance = fSecondMoment/(fScoredHistories-1);
							else
								fVariance = 0.;
							fStandardDeviation = sqrt(fVariance);
						}
					} else {
						fSum = fFirstMomentMap[idx] / GetUnitValue();
					}
				}
			}
		}
	}
}


void TsVBinnedScorer::ColorBy(G4double value) {
	fColorByTotal += value;
	G4int iValue;
	for (iValue = 0; iValue < (G4int)fColorValues.size() && fColorByTotal > fColorValues[iValue]; iValue++)
	{;}

	G4VisAttributes* visAtt = new G4VisAttributes();
	visAtt->SetForceSolid(true);
	visAtt->SetColor( fPm->GetColor(fColorNames[iValue])->GetColor() );
	fComponent->SetVisAttribute(visAtt);
}
