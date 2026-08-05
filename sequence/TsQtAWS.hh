//
// ********************************************************************
// *                                                                  *
// * Copyright 2025 The TOPAS Collaboration                           *
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
// * EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE WARRANTIES  *
// * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND         *
// * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT      *
// * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,     *
// * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
// * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR    *
// * OTHER DEALINGS IN THE SOFTWARE.                                  *
// *                                                                  *
// ********************************************************************
//

#ifndef TsQtAWS_hh
#define TsQtAWS_hh

#if defined(G4UI_BUILD_QT_SESSION) || defined(G4UI_USE_QT)

#include <QObject>
#include <QMap>
#include <QJsonValue>

class TsParameterManager;
class QWidget;
class QDialog;
class QStackedWidget;
class QProgressBar;
class QLabel;
class QFrame;
class QPlainTextEdit;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QScrollArea;
class QTextEdit;
class QTimer;
class QVBoxLayout;
class QGridLayout;
class QComboBox;

class TsQtAWS : public QObject {
    Q_OBJECT

public:
    TsQtAWS(TsParameterManager* pm, QObject* parent = nullptr);
    void ShowWizard(QWidget* parentWidget);

private slots:
    void OnPollJobs();

private:
    TsParameterManager* fPm;

    QString fAwsDir;
    QString fProfile;
    QString fRegion;
    QString fSnapshotRoot;

    QString fComputeJsonPath;
    QString fJobDefJsonPath;
    QString fJobDefPostJsonPath;
    QString fTopasSubmitPath;
    QString fPostprocessSubmitPath;

    QString fComputeJsonText;
    QString fJobDefJsonText;
    QString fJobDefPostJsonText;
    bool fComputeApplied;
    bool fComputeCreated;
    bool fComputeNeedsApply;
    bool fJobDefApplied;
    bool fJobDefCreated;
    bool fJobDefNeedsApply;

    QStringList fJobIds;
    QString fPostprocessJobId;

    QString fProjectName;
    QString fRunDate;
    QString fOutputBucket;

    QWidget* fWizardParent;
    QDialog* fWizard;
    QStackedWidget* fStack;
    QProgressBar* fStepProgress;
    QLabel* fStepLabel;
    QLabel* fComputeApplyDesc;
    QLabel* fJobDefApplyDesc;
    QLineEdit* fEditComputeSnapshotFolder;
    QPlainTextEdit* fComputeEdit;
    QPlainTextEdit* fJobDefEdit;
    QLineEdit* fEditJobDefSnapshotFolder;
    QLineEdit* fEditWizardJobQueueName;
    QLineEdit* fAwsDirEdit;
    QLineEdit* fProfileEdit;
    QLineEdit* fEditAwsRegion;
    QPushButton* fBtnCancel;
    QPushButton* fBtnReset;
    QPushButton* fBtnNext;
    QPushButton* fBtnDownloadS3;
    QPushButton* fBtnSubmitPostprocess;
    QTextEdit* fMonitorText;
    QLabel* fMonitorLastUpdated;
    QMap<QString, QFrame*> fMonitorStateDots;
    QMap<QString, QLabel*> fMonitorStateLabels;
    QTimer* fPollTimer;

    QLineEdit* fEditProjectName;
    QLineEdit* fEditRunDate;
    QLineEdit* fEditNumJobs;
    QLineEdit* fEditJobName;
    QLineEdit* fEditInputBucket;
    QLineEdit* fEditOutputBucket;
    QLineEdit* fEditLocalSimDir;
    QLineEdit* fEditFileToRun;
    QLineEdit* fEditJobQueue;
    QLineEdit* fEditJobDefinitionName;
    QLineEdit* fEditPostprocessJobDefinitionName;

    QLineEdit* fEditPpLocalScript;
    QLineEdit* fEditPpExtraPip;
    QLineEdit* fEditDownloadDir;
    QWidget* fPostprocessSectionWidget;
    QLineEdit* fEditSubmitSnapshotFolder;
    QGridLayout* fSubmitVarsGrid;
    QGridLayout* fPostprocessVarsGrid;
    QMap<QString, QLineEdit*> fSubmitVarEdits;
    QStringList fSubmitVarOrder;
    QMap<QString, QString> fSubmitVarDefaults;
    QMap<QString, QLineEdit*> fPostprocessVarEdits;
    QStringList fPostprocessVarOrder;
    QMap<QString, QString> fPostprocessVarDefaults;
    QString fSubmitScriptSnapshotPath;
    bool fSubmitApplied;
    QScrollArea* fStep2Scroll;
    QScrollArea* fStep3Scroll;
    QStackedWidget* fComputeModeStack;
    QWidget* fComputeFormPage;
    QPlainTextEdit* fComputeTextEditor;
    bool fComputeTextMode;
    QMap<QString, QLineEdit*> fComputeScalarEdits;
    QMap<QString, QJsonValue::Type> fComputeScalarTypes;
    QMap<QString, QList<QLineEdit*>> fComputeArrayEdits;
    QMap<QString, QJsonValue::Type> fComputeArrayItemTypes;
    QMap<QString, QVBoxLayout*> fComputeArrayRowsLayouts;
    QMap<QString, QVBoxLayout*> fComputeFlatArrayRowsLayouts;
    QMap<QString, QStringList> fComputeFlatArrayKeys;
    QMap<QString, QList<QWidget*>> fComputeFlatArrayRowWidgets;
    QStackedWidget* fJobDefModeStack;
    QWidget* fJobDefFormPage;
    QPlainTextEdit* fJobDefTextEditor;
    bool fJobDefTextMode;
    QMap<QString, QLineEdit*> fJobDefScalarEdits;
    QMap<QString, QJsonValue::Type> fJobDefScalarTypes;
    QMap<QString, QList<QLineEdit*>> fJobDefArrayEdits;
    QMap<QString, QJsonValue::Type> fJobDefArrayItemTypes;
    QMap<QString, QVBoxLayout*> fJobDefArrayRowsLayouts;
    QMap<QString, QVBoxLayout*> fJobDefFlatArrayRowsLayouts;
    QMap<QString, QStringList> fJobDefFlatArrayKeys;
    QMap<QString, QList<QWidget*>> fJobDefFlatArrayRowWidgets;

    int fCurrentStep;
    bool fWizardAdvancedMode;
    QRadioButton* fRadioWizardDefault;
    QRadioButton* fRadioWizardAdvanced;

    QStackedWidget* fComputeStepStack;
    QRadioButton* fRadioComputeExisting;
    QRadioButton* fRadioComputeCreate;
    QLineEdit* fEditExistingComputeEnvName;

    QStackedWidget* fJobBatchStepStack;
    QRadioButton* fRadioBatchExisting;
    QRadioButton* fRadioBatchCreate;
    QRadioButton* fRadioCreateUseExistingBucketsLogs;
    QRadioButton* fRadioCreateNewBucketsLogs;
    QLineEdit* fEditExistingInputBucket;
    QLineEdit* fEditExistingOutputBucket;
    QLineEdit* fEditExistingJobQueue;
    QLineEdit* fEditExistingJobDefSim;
    QLineEdit* fEditExistingJobDefPost;
    QLineEdit* fEditProvisionInputBucket;
    QLineEdit* fEditProvisionOutputBucket;
    QLineEdit* fEditProvisionLogGroup;
    QRadioButton* fRadioLogGroupCreate;
    QRadioButton* fRadioLogGroupExisting;
    QRadioButton* fRadioSimJobDefExisting;
    QRadioButton* fRadioSimJobDefCreate;
    QRadioButton* fRadioProvisionPostYes;
    QRadioButton* fRadioProvisionPostNo;
    QRadioButton* fRadioPostJobDefExisting;
    QRadioButton* fRadioPostJobDefCreate;
    QRadioButton* fRadioCloudSubmitUseOpenParam;
    QRadioButton* fRadioCloudSubmitUseTemplateVars;
    QLineEdit* fEditPostprocessScriptPath;

    QWidget* fJobDefEditorHost;
    QVBoxLayout* fSimJobDefHostLayout;
    QVBoxLayout* fPostJobDefHostLayout;

    bool fComputeUseExisting;
    QString fExistingComputeEnvName;
    bool fBatchUseExisting;
    bool fBatchUseExistingJobDefs;
    bool fBatchUseExistingPostJobDef;
    bool fSubmitUseOpenedParameterFile;
    bool fPostprocessScriptSelectedByBrowse;
    bool fWantsPostprocessing;
    bool fMonitoringPostprocess;
    bool fDownloadPostprocessResults;
    bool fS3LogsProvisioned;
    bool fSimJobDefPageInitialized;
    bool fPostJobDefPageInitialized;
    int fActiveJobDefTabIndex;

    bool fSubmitNeedsReset;
    QString fDefaultExistingComputeEnvName;
    QString fComputeDefaultJsonText;
    QString fJobDefDefaultJsonText;
    QString fJobDefPostDefaultJsonText;
    QString AutoDetectAwsDir() const;
    QString ComputeEnvironmentNameFromCurrentJson() const;
    bool HasRequiredFiles(const QString& dir) const;
    QString SnapshotSubdir() const;
    void ReloadAwsFilesFromUi();
    void SaveAwsSessionOnly();

    void CreateWizardPagesAndNavigation();

    bool RunProcess(const QString& program, const QStringList& args, const QString& workingDir,
                    QString* out, QString* err, int* exitCode);
    bool RunAws(const QStringList& args, QString* out, QString* err, int* exitCode);
    void AwsCliArgs(QStringList* args) const;

    bool ApplyComputeJsonSnapshot(bool showSuccessMessage = true);
    bool CreateComputeEnvironmentFromSnapshot();
    bool ApplyJobDefJsonSnapshot();
    bool CreateJobDefinitionFromSnapshot();
    bool ApplySubmitScriptSnapshot(bool showSuccessMessage = true);
    bool WriteTextFile(const QString& path, const QString& text);
    bool RegisterJobDefinitionFile(const QString& jsonFilePath, QString* errOut);
    QMap<QString, QString> BuildSubmitOverridesFromUi() const;

    bool RunSubmitScript(const QString& scriptPath, QString* out, QString* err, int* exitCode,
                         const QMap<QString, QString>& extraEnv = QMap<QString, QString>());
    void ParseJobIdsFromSubmitOutput(const QString& out);

    void SetStep(int step);
    void UpdateNavButtons();
    void ShowAwsMessage(QWidget* parent, const QString& title, bool ok, const QString& detail);

    void StartMonitoring();
    void StopMonitoring();
    void RefreshJobStatus();

    void BuildPostprocessScriptToFile(const QString& path);

    void ApplyAwsConfigFromFile();
    void RebuildComputeFormFromCurrentJson();
    void BuildComputeFormForValue(const QString& path, const QString& label, const QJsonValue& value, QVBoxLayout* layout);
    void AddComputeArrayRow(const QString& path, const QString& valueText, QVBoxLayout* rowsLayout);
    void PromptAddArrayValue(const QString& path);
    void PromptRemoveArrayValue(const QString& path);
    bool BuildComputeJsonFromForm(QString* jsonOut, QString* errorOut) const;
    void SetJsonValueAtPath(QJsonObject& root, const QString& path, const QJsonValue& value) const;
    void MarkComputeNeedsApply();
    void RebuildJobDefFormFromCurrentJson();
    void BuildJobDefFormForValue(const QString& path, const QString& label, const QJsonValue& value, QVBoxLayout* layout);
    void AddJobDefArrayRow(const QString& path, const QString& valueText, QVBoxLayout* rowsLayout);
    void PromptAddJobDefArrayValue(const QString& path);
    void PromptRemoveJobDefArrayValue(const QString& path);
    bool BuildJobDefJsonFromForm(QString* jsonOut, QString* errorOut) const;
    void MarkJobDefNeedsApply();

    void CommitJobDefEditorToActiveString();
    void LoadActiveJobDefIntoEditor();
    void UpdateJobDefTabVisibility();
    void EnsureJobDefEditorHostForPage(bool simulationPage);
    void SyncSubmitStepFromBatchState();
    QString PatchJobDefJsonBucketsAndLog(const QString& jsonText, const QString& inputBucket, const QString& outputBucket,
                                         const QString& logGroup, const QString& region) const;
    bool RunProvisionS3AndLogs(QString* errOut);
    void SetWizardEditorModesFromStep2();
    int ComputeStepStackIndex() const;
    int JobBatchStepStackIndex() const;
};

#endif

#endif
