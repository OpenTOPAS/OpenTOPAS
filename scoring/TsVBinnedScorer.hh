//
// ********************************************************************
// *                                                                  *
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

#ifndef TsVBinnedScorer_hh
#define TsVBinnedScorer_hh

#include "TsVScorer.hh"

class TsDicomPatient;
class TsOutcomeModelList;

class TsVBinnedScorer : public TsVScorer
{
public:
	TsVBinnedScorer(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
			  		   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer);

	virtual ~TsVBinnedScorer();

protected:
	// When results are written out, they are converted from Geant4 internal units to your specified unit.
	// Unit string must be a valid Geant4 unit or an additional unit string specifically defined by TOPAS.
	void SetUnit(const G4String& unitString);

	// May be useful for expert custom scorers
	G4bool fNeedToUpdateFileSpecs;
	G4String fOutFileSpec1;
	G4String fOutFileSpec2;
	G4bool fOutputToBinary;
	G4bool fOutputToCsv;
	G4bool fOutputToRoot;
	G4bool fOutputToXml;
	G4bool fOutputToDicom;
	G4String fVHOutFileSpec1;
	G4String fVHOutFileSpec2;

// User classes should not access any methods or data beyond this point
public:
	void UpdateForNewRun(G4bool rebuiltSomeComponents);
	G4bool HasUnsatisfiedLimits();

	void PostConstructor();
	void AccumulateHit(G4Step* aStep, G4double value);
	void AccumulateHit(G4Step* aStep, G4double value, G4int index);
	void AccumulateEvent();
	virtual void AbsorbResultsFromWorkerScorer(TsVScorer* workerScorer);
	void RestoreResultsFromFile();

	std::vector <G4double> fFirstMomentMap;

protected:
	void GetAppropriatelyBinnedCopyOfComponent(G4String componentName);

	G4String ConfirmCanOpen(G4String fileName, G4String fileExt, G4int& increment);
	virtual void Output();
	virtual void Clear();

	std::map<G4int, G4double>* fEvtMap;

	std::vector <G4double> fSecondMomentMap;
	std::vector <G4long> fCountMap;
	std::vector <G4double> fMinMap;
	std::vector <G4double> fMaxMap;

private:
	void ActuallySetUnit(const G4String& unitName);
	void CreateHistogram(G4String title, G4bool volumeHistogram);
	void ReadOneValueFromASCII(G4int idx);
	void ReadOneValueFromBinary(G4int idx);
	void StoreOneValue(G4int idx);
	G4String GetOneTokenFromReadLine();
	void PrintHeader();
	void PrintHeader(std::ostream&);
	void PrintASCII(std::ostream& a=G4cout);
	void PrintBinary(std::ostream& a=G4cout);
	void PrintOneValueToASCII(std::ostream& ofile);
	void PrintOneValueToBinary(G4int idx, G4double* data);
	void PrintVHHeader();
	void PrintVHHeader(std::ostream&);
	void PrintVHASCII(std::ostream& a=G4cout);
	void PrintVHBinary(std::ostream& a=G4cout);
	void CalculateOneValue(G4int idx);
	void ColorBy(G4double value);

	TsOutcomeModelList* fOm;

	TsDicomPatient* fReferencedDicomPatient;

	G4String fUnitName;
	G4bool fUnitWasSet;
	G4double fNormFactor;
	G4int fNBins;

	G4int fHistogramID;
	G4int fVHistogramID;
	G4int fHistogramBins;
	G4double fHistogramMin;
	G4double fHistogramMax;
	std::vector<G4double> fHistogramLowerValues;
	G4int fNVoxelsInVolumeHistogram;
	G4int* fVolumeHistogramBinCounts;

	G4bool fBinByIncidentEnergy;
	G4bool fBinByPreStepEnergy;
	G4bool fBinByStepDepositEnergy;
	G4bool fBinByPrimaryEnergy;
	G4bool fBinByTime;
	G4bool fBinLog;
	G4int fNEorTBins;
	G4double fBinMin;
	G4double fBinMax;
	G4double fBinWidth;
	std::vector<G4double> fTempEBins;

	G4int* fReportValues;
	G4int fNReportValues;
	G4bool fReportSum;
	G4bool fReportMean;
	G4bool fReportHistories;
	G4bool fReportCountInBin;
	G4bool fReportSecondMoment;
	G4bool fReportVariance;
	G4bool fReportStandardDeviation;
	G4bool fReportMin;
	G4bool fReportMax;
	G4bool fReportCVolHist;
	G4bool fReportDVolHist;
	G4bool fReportOutcome;
	G4bool fAccumulateSecondMoment;
	G4bool fAccumulateMean;
	G4bool fAccumulateCount;
 	G4int fNumberOfOutputColumns;
	G4int fOutputPosition;
	G4bool fNeedComma;

	G4bool fRestoreResultsFromFile;
	G4String fReadLine;
	std::ifstream fReadFile;
	G4int* fReadBackValues;
	G4int fNReadBackValues;
	G4bool fReadBackHasSum;
	G4bool fReadBackHasMean;
	G4bool fReadBackHasHistories;
	G4bool fReadBackHasCountInBin;
	G4bool fReadBackHasSecondMoment;
	G4bool fReadBackHasVariance;
	G4bool fReadBackHasStandardDeviation;
	G4bool fReadBackHasMin;
	G4bool fReadBackHasMax;

	G4String fColorBy;
	std::vector<G4String> fColorNames;
	std::vector<G4double> fColorValues;
	G4double fColorByTotal;

	G4bool fSparsify;
	G4double fSparsifyThreshold;
	G4bool fSingleIndex;

	G4double fMean;
	G4int fCountInBin;
	G4float fSum;
	G4double fSecondMoment;
	G4double fVariance;
	G4double fStandardDeviation;
	G4double fMin;
	G4double fMax;

	G4double fSumLimit;
	G4double fStandardDeviationLimit;
	G4double fRelativeSDLimit;
	G4int fCountLimit;
	G4int fRepeatSequenceTestBin;

	std::map<G4String, G4double> fProbOfOutcome;
};

#endif
