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

#ifdef G4UI_USE_QT

#include "TsQtAWS.hh"
#include "TsQtAWSUtils.hh"

#include "TsParameterManager.hh"

#include <QCoreApplication>
#include <QComboBox>
#include <QDate>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QProcessEnvironment>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSettings>
#include <QStandardItemModel>
#include <QSet>
#include <QStackedWidget>
#include <QStringList>
#include <QRegularExpression>
#include <QTextEdit>
#include <QTimer>
#include <QPixmap>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QVector>
#include <QVBoxLayout>
#include <QTextDocument>
#include <functional>

using namespace TsQtAWSUtils;

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

TsQtAWS::TsQtAWS(TsParameterManager* pm, QObject* parent)
    : QObject(parent), fPm(pm), fProfile(), fRegion(), fComputeApplied(false), fComputeCreated(false), fComputeNeedsApply(false),
      fJobDefApplied(false), fJobDefCreated(false), fJobDefNeedsApply(false),
      fWizardParent(nullptr),
      fWizard(nullptr), fStack(nullptr), fStepProgress(nullptr), fStepLabel(nullptr), fComputeApplyDesc(nullptr),
      fJobDefApplyDesc(nullptr), fEditComputeSnapshotFolder(nullptr), fComputeEdit(nullptr),
      fJobDefEdit(nullptr), fEditJobDefSnapshotFolder(nullptr), fEditWizardJobQueueName(nullptr),
      fAwsDirEdit(nullptr), fProfileEdit(nullptr),
      fEditAwsRegion(nullptr), fBtnCancel(nullptr), fBtnReset(nullptr), fBtnNext(nullptr), fBtnDownloadS3(nullptr),
      fBtnSubmitPostprocess(nullptr), fMonitorText(nullptr), fMonitorLastUpdated(nullptr),
      fPollTimer(nullptr), fEditProjectName(nullptr), fEditRunDate(nullptr),
      fEditNumJobs(nullptr), fEditJobName(nullptr), fEditInputBucket(nullptr), fEditOutputBucket(nullptr),
      fEditLocalSimDir(nullptr), fEditFileToRun(nullptr), fEditJobQueue(nullptr), fEditJobDefinitionName(nullptr),
      fEditPostprocessJobDefinitionName(nullptr), fEditPpLocalScript(nullptr), fEditPpExtraPip(nullptr), fEditDownloadDir(nullptr),
      fPostprocessSectionWidget(nullptr), fEditSubmitSnapshotFolder(nullptr), fSubmitVarsGrid(nullptr), fPostprocessVarsGrid(nullptr),
      fSubmitScriptSnapshotPath(), fSubmitApplied(true),
      fStep2Scroll(nullptr), fStep3Scroll(nullptr), fComputeModeStack(nullptr), fComputeFormPage(nullptr), fComputeTextEditor(nullptr),
      fComputeTextMode(false), fJobDefModeStack(nullptr), fJobDefFormPage(nullptr), fJobDefTextEditor(nullptr), fJobDefTextMode(false),
      fCurrentStep(0), fWizardAdvancedMode(false), fRadioWizardDefault(nullptr), fRadioWizardAdvanced(nullptr),
      fComputeStepStack(nullptr), fRadioComputeExisting(nullptr), fRadioComputeCreate(nullptr), fEditExistingComputeEnvName(nullptr),
      fJobBatchStepStack(nullptr), fRadioBatchExisting(nullptr), fRadioBatchCreate(nullptr),
      fRadioCreateUseExistingBucketsLogs(nullptr), fRadioCreateNewBucketsLogs(nullptr), fEditExistingInputBucket(nullptr),
      fEditExistingOutputBucket(nullptr), fEditExistingJobQueue(nullptr), fEditExistingJobDefSim(nullptr),
      fEditExistingJobDefPost(nullptr), fEditProvisionInputBucket(nullptr),
      fEditProvisionOutputBucket(nullptr), fEditProvisionLogGroup(nullptr), fRadioLogGroupCreate(nullptr),
      fRadioLogGroupExisting(nullptr), fRadioSimJobDefExisting(nullptr), fRadioSimJobDefCreate(nullptr), fRadioProvisionPostYes(nullptr),
      fRadioProvisionPostNo(nullptr), fRadioPostJobDefExisting(nullptr), fRadioPostJobDefCreate(nullptr),
      fRadioCloudSubmitUseOpenParam(nullptr), fRadioCloudSubmitUseTemplateVars(nullptr), fEditPostprocessScriptPath(nullptr),
      fJobDefEditorHost(nullptr), fSimJobDefHostLayout(nullptr),
      fPostJobDefHostLayout(nullptr), fComputeUseExisting(false), fBatchUseExisting(false), fBatchUseExistingJobDefs(false),
      fBatchUseExistingPostJobDef(false), fSubmitUseOpenedParameterFile(false), fPostprocessScriptSelectedByBrowse(false),
      fWantsPostprocessing(true), fMonitoringPostprocess(false), fDownloadPostprocessResults(false), fS3LogsProvisioned(false),
      fSimJobDefPageInitialized(false), fPostJobDefPageInitialized(false),
      fActiveJobDefTabIndex(0), fSubmitNeedsReset(false)
{
}

// ---------------------------------------------------------------------------
// Main entry: build the wizard dialog and run it
// ---------------------------------------------------------------------------

void TsQtAWS::ShowWizard(QWidget* parentWidget)
{
    fWizardParent = parentWidget;
    fComputeUseExisting = false;
    fBatchUseExisting = false;
    fBatchUseExistingJobDefs = false;
    fBatchUseExistingPostJobDef = false;
    fSubmitUseOpenedParameterFile = true;
    fPostprocessScriptSelectedByBrowse = false;
    fS3LogsProvisioned = false;
    fWantsPostprocessing = true;
    fMonitoringPostprocess = false;
    fDownloadPostprocessResults = false;
    fSimJobDefPageInitialized = false;
    fPostJobDefPageInitialized = false;
    fActiveJobDefTabIndex = 0;
    fExistingComputeEnvName.clear();
    fDefaultExistingComputeEnvName.clear();
    fProfile.clear();
    fRegion.clear();
    fAwsDir.clear();

    if (fProfile.isEmpty()) {
        QByteArray p = qgetenv("AWS_PROFILE");
        if (!p.isEmpty())
            fProfile = QString::fromUtf8(p);
    }
    if (fProfile.isEmpty()) {
        QByteArray p = qgetenv("AWS_DEFAULT_PROFILE");
        if (!p.isEmpty())
            fProfile = QString::fromUtf8(p);
    }
    if (fRegion.isEmpty()) {
        QByteArray r = qgetenv("AWS_REGION");
        if (r.isEmpty())
            r = qgetenv("AWS_DEFAULT_REGION");
        if (!r.isEmpty())
            fRegion = QString::fromUtf8(r);
    }
    ApplyAwsConfigFromFile();

    fAwsDir = AutoDetectAwsDir();

    fWizard = new QDialog(parentWidget);
    fWizard->setWindowTitle("TOPAS AWS cloud");
    fWizard->resize(900, 700);
    QFont wizardFont = fWizard->font();
    wizardFont.setPointSize(wizardFont.pointSize() + 1);
    fWizard->setFont(wizardFont);
    fWizard->setStyleSheet(
        "QLineEdit { border: 1px solid transparent; border-radius: 8px; padding: 2px 6px; }"
        "QLineEdit:focus { border: 2px solid rgb(10,132,255); border-radius: 8px; }");

    CreateWizardPagesAndNavigation();

    fAwsDirEdit->setText(fAwsDir);
    fProfileEdit->setText(fProfile);
    fEditAwsRegion->setText(fRegion);
    ReloadAwsFilesFromUi();

    SetStep(0); // welcome page
    fWizard->exec();

    StopMonitoring();

    delete fWizard;
    fWizard = nullptr;
    fStack = nullptr;
    fStepProgress = nullptr;
    fStepLabel = nullptr;
    fComputeApplyDesc = nullptr;
    fJobDefApplyDesc = nullptr;
    fEditComputeSnapshotFolder = nullptr;
    fComputeEdit = nullptr;
    fJobDefEdit = nullptr;
    fEditJobDefSnapshotFolder = nullptr;
    fEditWizardJobQueueName = nullptr;
    fAwsDirEdit = nullptr;
    fProfileEdit = nullptr;
    fBtnCancel = nullptr;
    fBtnDownloadS3 = nullptr;
    fBtnSubmitPostprocess = nullptr;
    fRadioWizardDefault = nullptr;
    fRadioWizardAdvanced = nullptr;
    fComputeStepStack = nullptr;
    fRadioComputeExisting = nullptr;
    fRadioComputeCreate = nullptr;
    fEditExistingComputeEnvName = nullptr;
    fJobBatchStepStack = nullptr;
    fRadioBatchExisting = nullptr;
    fRadioBatchCreate = nullptr;
    fRadioCreateUseExistingBucketsLogs = nullptr;
    fRadioCreateNewBucketsLogs = nullptr;
    fEditExistingInputBucket = nullptr;
    fEditExistingOutputBucket = nullptr;
    fEditExistingJobQueue = nullptr;
    fEditExistingJobDefSim = nullptr;
    fEditExistingJobDefPost = nullptr;
    fRadioPostJobDefExisting = nullptr;
    fRadioPostJobDefCreate = nullptr;
    fRadioCloudSubmitUseOpenParam = nullptr;
    fRadioCloudSubmitUseTemplateVars = nullptr;
    fEditPostprocessScriptPath = nullptr;
    fEditProvisionInputBucket = nullptr;
    fEditProvisionOutputBucket = nullptr;
    fEditProvisionLogGroup = nullptr;
    fRadioLogGroupCreate = nullptr;
    fRadioLogGroupExisting = nullptr;
    fRadioSimJobDefExisting = nullptr;
    fRadioSimJobDefCreate = nullptr;
    fRadioProvisionPostYes = nullptr;
    fRadioProvisionPostNo = nullptr;
    fJobDefEditorHost = nullptr;
    fSimJobDefHostLayout = nullptr;
    fPostJobDefHostLayout = nullptr;
    fBtnReset = nullptr;
    fBtnNext = nullptr;
    fMonitorText = nullptr;
    fMonitorLastUpdated = nullptr;
    fMonitorStateDots.clear();
    fMonitorStateLabels.clear();
    fEditProjectName = nullptr;
    fEditRunDate = nullptr;
    fEditNumJobs = nullptr;
    fEditJobName = nullptr;
    fEditInputBucket = nullptr;
    fEditOutputBucket = nullptr;
    fEditLocalSimDir = nullptr;
    fEditFileToRun = nullptr;
    fEditJobQueue = nullptr;
    fEditJobDefinitionName = nullptr;
    fEditPpLocalScript = nullptr;
    fEditPpExtraPip = nullptr;
    fPostprocessSectionWidget = nullptr;
    fEditDownloadDir = nullptr;
    fEditSubmitSnapshotFolder = nullptr;
    fSubmitVarsGrid = nullptr;
    fPostprocessVarsGrid = nullptr;
    fEditPostprocessJobDefinitionName = nullptr;
    fEditAwsRegion = nullptr;
    fStep2Scroll = nullptr;
    fStep3Scroll = nullptr;
    fComputeModeStack = nullptr;
    fComputeFormPage = nullptr;
    fComputeTextEditor = nullptr;
    fJobDefModeStack = nullptr;
    fJobDefFormPage = nullptr;
    fJobDefTextEditor = nullptr;
    fComputeScalarEdits.clear();
    fComputeScalarTypes.clear();
    fComputeArrayEdits.clear();
    fComputeArrayItemTypes.clear();
    fComputeFlatArrayRowsLayouts.clear();
    fComputeFlatArrayKeys.clear();
    fComputeFlatArrayRowWidgets.clear();
    fComputeArrayRowsLayouts.clear();
    fJobDefScalarEdits.clear();
    fJobDefScalarTypes.clear();
    fJobDefArrayEdits.clear();
    fJobDefArrayItemTypes.clear();
    fJobDefArrayRowsLayouts.clear();
    fJobDefFlatArrayRowsLayouts.clear();
    fJobDefFlatArrayKeys.clear();
    fJobDefFlatArrayRowWidgets.clear();
    fPostprocessVarEdits.clear();
    fPostprocessVarOrder.clear();
    fPostprocessVarDefaults.clear();
}

// ---------------------------------------------------------------------------
// Wizard step changes and button states
// ---------------------------------------------------------------------------

void TsQtAWS::SetStep(int step)
{
    fCurrentStep = step;
    if (fStepProgress) {
        fStepProgress->setValue(step);
        if (step == 0)
            fStepLabel->setText("Welcome");
        else
            fStepLabel->setText(QString("Step %1 of 9").arg(step));
    }
    if (fStack)
        fStack->setCurrentIndex(step);
    if (fStack && fStack->currentWidget())
        fStack->currentWidget()->setFocus(Qt::OtherFocusReason);

    if (step == 5 && fEditWizardJobQueueName) {
        const QSignalBlocker b(fEditWizardJobQueueName);
        if (fEditWizardJobQueueName->text().trimmed().isEmpty() && fSubmitVarDefaults.contains("JOB_QUEUE"))
            fEditWizardJobQueueName->setText(fSubmitVarDefaults.value("JOB_QUEUE"));
    }
    if (step == 6) {
        if (fRadioCloudSubmitUseOpenParam && fRadioCloudSubmitUseTemplateVars) {
            const QSignalBlocker b1(fRadioCloudSubmitUseOpenParam);
            const QSignalBlocker b2(fRadioCloudSubmitUseTemplateVars);
            fRadioCloudSubmitUseOpenParam->setChecked(fSubmitUseOpenedParameterFile);
            fRadioCloudSubmitUseTemplateVars->setChecked(!fSubmitUseOpenedParameterFile);
        }
    }
    if (step == 7)
        SyncSubmitStepFromBatchState();
    if (step == 8 && !fMonitoringPostprocess)
        fDownloadPostprocessResults = false;
    if (step == 9) {
        if (fBtnDownloadS3) {
            fBtnDownloadS3->setText(fDownloadPostprocessResults
                                        ? "Download post-processing job outputs from S3"
                                        : "Download simulation job outputs from S3");
        }
        if (fPostprocessSectionWidget) {
            fPostprocessSectionWidget->setVisible(fWantsPostprocessing && !fDownloadPostprocessResults);
            fPostprocessSectionWidget->setEnabled(true);
        }
        if (fBtnSubmitPostprocess)
            fBtnSubmitPostprocess->setEnabled(fWantsPostprocessing && !fDownloadPostprocessResults);
    }

    const bool onWelcomePage = (step == 0);
    if (fStepLabel)
        fStepLabel->setVisible(!onWelcomePage);
    if (fStepProgress)
        fStepProgress->setVisible(!onWelcomePage);
    if (fBtnCancel)
        fBtnCancel->setVisible(!onWelcomePage);
    if (fBtnReset)
        fBtnReset->setVisible(!onWelcomePage && (step == 3 || step == 4 || step == 5 || step == 7));
    if (fBtnNext)
        fBtnNext->setVisible(!onWelcomePage);
    if (step == 4 && !fComputeTextMode && ComputeStepStackIndex() == 1)
        RebuildComputeFormFromCurrentJson();
    if (step == 5 && !fJobDefTextMode) {
        const int jbi = JobBatchStepStackIndex();
        if ((jbi == 3 && fActiveJobDefTabIndex == 0) || (jbi == 6 && fActiveJobDefTabIndex == 1))
            RebuildJobDefFormFromCurrentJson();
    }
    if (step == 4 && fStep2Scroll && fStep2Scroll->verticalScrollBar())
        fStep2Scroll->verticalScrollBar()->setValue(0);
    if (step == 5 && fStep3Scroll && fStep3Scroll->verticalScrollBar())
        fStep3Scroll->verticalScrollBar()->setValue(0);

    if (step == 8) { // job monitoring
        StartMonitoring();
        OnPollJobs();
    } else {
        StopMonitoring();
    }

    UpdateNavButtons();
}

void TsQtAWS::UpdateNavButtons()
{
    if (fCurrentStep == 0) { // welcome page
        fBtnNext->setText("Next");
        fBtnNext->setEnabled(true);
    } else if (fCurrentStep == 1) { // AWS scripts / profile
        fBtnNext->setText("Next");
        const QString dirTry = fAwsDirEdit ? fAwsDirEdit->text() : fAwsDir;
        fBtnNext->setEnabled(HasRequiredFiles(dirTry));
    } else if (fCurrentStep == 2) { // default vs advanced
        fBtnNext->setText("Next");
        fBtnNext->setEnabled(true);
    } else if (fCurrentStep == 3) { // buckets
        fBtnNext->setText("Next");
        const QString a = fEditProvisionInputBucket ? fEditProvisionInputBucket->text().trimmed() : QString();
        const QString b = fEditProvisionOutputBucket ? fEditProvisionOutputBucket->text().trimmed() : QString();
        fBtnNext->setEnabled(!a.isEmpty() && !b.isEmpty());
    } else if (fCurrentStep == 4) { // compute nested
        const int csi = ComputeStepStackIndex();
        if (csi == 0) {
            fBtnNext->setText("Next");
            const bool ex = fRadioComputeExisting && fRadioComputeExisting->isChecked();
            const QString n = fEditExistingComputeEnvName ? fEditExistingComputeEnvName->text().trimmed() : QString();
            fBtnNext->setEnabled(!ex || !n.isEmpty());
        } else {
            fBtnNext->setText("Create compute environment");
            const QString folderName = fEditComputeSnapshotFolder ? fEditComputeSnapshotFolder->text().trimmed() : QString();
            fBtnNext->setEnabled(!folderName.isEmpty());
        }
    } else if (fCurrentStep == 5) { // batch nested
        const int jsi = JobBatchStepStackIndex();
        if (jsi == 0) {
            fBtnNext->setText("Next");
            const bool useExisting = fRadioBatchExisting && fRadioBatchExisting->isChecked();
            const QString jq = useExisting
                ? (fEditExistingJobQueue ? fEditExistingJobQueue->text().trimmed() : QString())
                : (fEditWizardJobQueueName ? fEditWizardJobQueueName->text().trimmed() : QString());
            fBtnNext->setEnabled(!jq.isEmpty());
        } else if (jsi == 1) {
            fBtnNext->setText("Next");
            const QString c = fEditProvisionLogGroup ? fEditProvisionLogGroup->text().trimmed() : QString();
            fBtnNext->setEnabled(!c.isEmpty());
        } else if (jsi == 2) {
            fBtnNext->setText("Next");
            const bool useExisting = fBatchUseExistingJobDefs;
            const QString existingName = fEditExistingJobDefSim ? fEditExistingJobDefSim->text().trimmed() : QString();
            fBtnNext->setEnabled(!useExisting || !existingName.isEmpty());
        } else if (jsi == 3) {
            fBtnNext->setText("Next");
            const QString folderName = fEditJobDefSnapshotFolder ? fEditJobDefSnapshotFolder->text().trimmed() : QString();
            fBtnNext->setEnabled(!folderName.isEmpty());
        } else if (jsi == 4) {
            fBtnNext->setText("Next");
            const bool wantsPost = fRadioProvisionPostYes && fRadioProvisionPostYes->isChecked();
            fBtnNext->setEnabled(!wantsPost || fPostprocessScriptSelectedByBrowse);
        } else if (jsi == 5) {
            fBtnNext->setText("Next");
            const bool useExistingPost = fRadioPostJobDefExisting && fRadioPostJobDefExisting->isChecked();
            const QString postName = fEditExistingJobDefPost ? fEditExistingJobDefPost->text().trimmed() : QString();
            fBtnNext->setEnabled(!useExistingPost || !postName.isEmpty());
        } else {
            fBtnNext->setText("Create job definition");
            const QString folderName = fEditJobDefSnapshotFolder ? fEditJobDefSnapshotFolder->text().trimmed() : QString();
            fBtnNext->setEnabled(!folderName.isEmpty());
        }
    } else if (fCurrentStep == 6) { // cloud submission mode
        fBtnNext->setText("Next");
        fBtnNext->setEnabled(true);
    } else if (fCurrentStep == 7) { // submit
        fBtnNext->setText("Submit to cloud");
        const QString folderName = fEditSubmitSnapshotFolder ? fEditSubmitSnapshotFolder->text().trimmed() : QString();
        bool allFilled = true;
        for (const QString& key : fSubmitVarOrder) {
            QLineEdit* edit = fSubmitVarEdits.value(key, nullptr);
            if (edit && edit->text().trimmed().isEmpty()) {
                allFilled = false;
                break;
            }
        }
        fBtnNext->setEnabled(!folderName.isEmpty() && allFilled);
    } else if (fCurrentStep == 8) { // monitoring
        fBtnNext->setText("Next");
    } else if (fCurrentStep == 9) { // download / close
        fBtnNext->setText("Close");
        fBtnNext->setEnabled(true);
    }
    if (fCurrentStep > 0) {
        ApplyPrimaryButtonStyle(fBtnNext);
    } else {
        fBtnNext->setStyleSheet("");
    }

    if (fBtnReset) {
        bool enableReset = false;
        if (fCurrentStep == 3) {
            const bool exIn = fEditProvisionInputBucket
                && fEditProvisionInputBucket->text().trimmed() != QStringLiteral("topas-nbio-input");
            const bool exOut = fEditProvisionOutputBucket
                && fEditProvisionOutputBucket->text().trimmed() != QStringLiteral("topas-nbio-output");
            enableReset = exIn || exOut;
        } else if (fCurrentStep == 4) {
            const int csi = ComputeStepStackIndex();
            if (csi == 0) {
                const bool ex = fRadioComputeExisting && fRadioComputeExisting->isChecked();
                const bool nameDiff =
                    fEditExistingComputeEnvName
                    && fEditExistingComputeEnvName->text().trimmed() != fDefaultExistingComputeEnvName.trimmed();
                enableReset = ex && nameDiff;
            } else {
                const QString folderName = fEditComputeSnapshotFolder ? fEditComputeSnapshotFolder->text().trimmed() : QString();
                enableReset = fComputeNeedsApply || !folderName.isEmpty();
            }
        } else if (fCurrentStep == 5) {
            const int jsi = JobBatchStepStackIndex();
            if (jsi == 0) {
                const QString dQueue = fSubmitVarDefaults.value(QStringLiteral("JOB_QUEUE")).trimmed();
                const bool useExistingQ = fRadioBatchExisting && fRadioBatchExisting->isChecked();
                if (useExistingQ)
                    enableReset = fEditExistingJobQueue && fEditExistingJobQueue->text().trimmed() != dQueue;
                else
                    enableReset = fEditWizardJobQueueName && fEditWizardJobQueueName->text().trimmed() != dQueue;
            } else if (jsi == 1) {
                const bool exLog = fEditProvisionLogGroup
                    && fEditProvisionLogGroup->text().trimmed() != QStringLiteral("/aws/batch/topas-nbio");
                enableReset = exLog;
            } else if (jsi == 2) {
                QString defaultSim = fSubmitVarDefaults.value(QStringLiteral("JOB_DEFINITION")).trimmed();
                if (defaultSim.isEmpty())
                    defaultSim = fSubmitVarDefaults.value(QStringLiteral("JOB_DEFINITION_NAME")).trimmed();
                const bool nameDiff = fEditExistingJobDefSim && fEditExistingJobDefSim->text().trimmed() != defaultSim;
                enableReset = fBatchUseExistingJobDefs && nameDiff;
            } else if (jsi == 3) {
                const QString folderName = fEditJobDefSnapshotFolder ? fEditJobDefSnapshotFolder->text().trimmed() : QString();
                enableReset = fJobDefNeedsApply || !folderName.isEmpty();
            } else if (jsi == 4) {
                const bool exPost = fRadioProvisionPostNo && fRadioProvisionPostNo->isChecked();
                enableReset = exPost;
            } else if (jsi == 5) {
                const bool useExisting = fRadioPostJobDefExisting && fRadioPostJobDefExisting->isChecked();
                const bool nameDiff = fEditExistingJobDefPost
                    && fEditExistingJobDefPost->text().trimmed() != QStringLiteral("topas-nbio-postprocess-job");
                enableReset = useExisting && nameDiff;
            } else if (jsi == 6) {
                const QString folderName = fEditJobDefSnapshotFolder ? fEditJobDefSnapshotFolder->text().trimmed() : QString();
                enableReset = fJobDefNeedsApply || !folderName.isEmpty();
            }
        } else if (fCurrentStep == 7) {
            const QString folderName = fEditSubmitSnapshotFolder ? fEditSubmitSnapshotFolder->text().trimmed() : QString();
            enableReset = fSubmitNeedsReset || !folderName.isEmpty();
        }
        fBtnReset->setEnabled(enableReset);
    }
}

// ---------------------------------------------------------------------------
// Paths and loading JSON from the aws folder
// ---------------------------------------------------------------------------

void TsQtAWS::ApplyAwsConfigFromFile()
{
    const QString path = QDir::homePath() + QStringLiteral("/.aws/config");
    ApplyAwsSharedConfig(path, &fProfile, &fRegion);
}

void TsQtAWS::RebuildComputeFormFromCurrentJson()
{
    if (!fComputeFormPage || !fComputeTextEditor)
        return;

    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(fComputeFormPage->layout());
    if (!layout)
        return;
    QLayoutItem* item = nullptr;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget())
            delete item->widget();
        delete item;
    }

    fComputeScalarEdits.clear();
    fComputeScalarTypes.clear();
    fComputeArrayEdits.clear();
    fComputeArrayItemTypes.clear();

    QJsonParseError pe;
    QJsonDocument doc = QJsonDocument::fromJson(fComputeTextEditor->toPlainText().toUtf8(), &pe);
    if (pe.error != QJsonParseError::NoError || !doc.isObject()) {
        QLabel* msg = new QLabel(
            "JSON is invalid. If you are in advanced mode, fix the text. Otherwise reload the template from the AWS folder.");
        msg->setWordWrap(true);
        layout->addWidget(msg);
        layout->addStretch(1);
        return;
    }

    const QJsonObject obj = doc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it)
        BuildComputeFormForValue(it.key(), it.key(), it.value(), layout);
    layout->addStretch(1);
}

void TsQtAWS::BuildComputeFormForValue(const QString& path, const QString& label, const QJsonValue& value, QVBoxLayout* layout)
{
    if (!layout)
        return;

    if (value.isObject()) {
        QLabel* heading = new QLabel(QString("<b>%1</b>").arg(label));
        heading->setTextFormat(Qt::RichText);
        heading->setToolTip(AwsFieldTooltipForField(path));
        layout->addWidget(heading);
        QWidget* group = new QWidget();
        QVBoxLayout* groupLayout = new QVBoxLayout(group);
        groupLayout->setContentsMargins(24, 2, 0, 6);
        groupLayout->setSpacing(12);
        const QJsonObject obj = value.toObject();
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            const QString childPath = path.isEmpty() ? it.key() : (path + "." + it.key());
            BuildComputeFormForValue(childPath, it.key(), it.value(), groupLayout);
        }
        layout->addWidget(group);
        return;
    }

    if (value.isArray()) {
        const QJsonArray arr = value.toArray();
        const JsonArrayShape shape = ClassifyArrayShape(arr);
        if (shape == JsonArrayShape::Complex) {
            QLabel* unsupported = new QLabel(QString("%1: complex arrays are editable in text editor mode.").arg(label));
            unsupported->setWordWrap(true);
            unsupported->setToolTip(AwsFieldTooltipForField(path));
            layout->addWidget(unsupported);
            return;
        }

        QLabel* arrayLabel = new QLabel(QString("<b>%1</b>").arg(label));
        arrayLabel->setTextFormat(Qt::RichText);
        arrayLabel->setToolTip(AwsFieldTooltipForField(path));
        layout->addWidget(arrayLabel);

        if (shape == JsonArrayShape::FlatObject) {
            AddFlatObjectArrayRows(path, arr, layout, fWizard, &fComputeScalarEdits, &fComputeScalarTypes,
                                   &fComputeFlatArrayRowsLayouts, &fComputeFlatArrayKeys, &fComputeFlatArrayRowWidgets,
                                   [this]() { MarkComputeNeedsApply(); });
            return;
        }

        QHBoxLayout* btnRow = new QHBoxLayout();
        btnRow->setSpacing(8);
        QPushButton* addBtn = new QPushButton("+");
        addBtn->setFixedWidth(30);
        QObject::connect(addBtn, &QPushButton::clicked, fWizard, [this, path]() { PromptAddArrayValue(path); });
        QPushButton* removeBtn = new QPushButton("-");
        removeBtn->setFixedWidth(30);
        QObject::connect(removeBtn, &QPushButton::clicked, fWizard, [this, path]() { PromptRemoveArrayValue(path); });
        btnRow->addWidget(addBtn);
        btnRow->addWidget(removeBtn);
        btnRow->addStretch(1);
        QWidget* rowsWidget = new QWidget();
        QVBoxLayout* rowsLayout = new QVBoxLayout(rowsWidget);
        rowsLayout->setContentsMargins(0, 0, 0, 0);
        rowsLayout->setSpacing(9);
        fComputeArrayEdits[path].clear();
        fComputeArrayRowsLayouts[path] = rowsLayout;
        QJsonValue::Type itemType = arr.isEmpty() ? QJsonValue::String : arr[0].type();
        fComputeArrayItemTypes[path] = itemType;
        for (const QJsonValue& v : arr)
            AddComputeArrayRow(path, v.toVariant().toString(), rowsLayout);
        rowsLayout->addLayout(btnRow);
        layout->addWidget(rowsWidget);
        return;
    }

    QLabel* scalarLabel = new QLabel(QString("<b>%1</b>").arg(label));
    scalarLabel->setTextFormat(Qt::RichText);
    scalarLabel->setToolTip(AwsFieldTooltipForField(path));
    layout->addWidget(scalarLabel);
    QLineEdit* edit = new QLineEdit(value.toVariant().toString());
    edit->setFixedHeight(22);
    edit->setCursorPosition(0);
    edit->setToolTip(AwsFieldTooltipForField(path));
    QObject::connect(edit, &QLineEdit::textEdited, fWizard, [this](const QString&) { MarkComputeNeedsApply(); });
    layout->addWidget(edit);
    fComputeScalarEdits[path] = edit;
    fComputeScalarTypes[path] = value.type();
}

void TsQtAWS::AddComputeArrayRow(const QString& path, const QString& valueText, QVBoxLayout* rowsLayout)
{
    if (!rowsLayout)
        return;
    QLineEdit* edit = new QLineEdit(valueText);
    edit->setFixedHeight(22);
    edit->setCursorPosition(0);
    QObject::connect(edit, &QLineEdit::textEdited, fWizard, [this](const QString&) { MarkComputeNeedsApply(); });
    const int insertAt = std::max(0, rowsLayout->count() - 1); // keep +/- row at bottom
    rowsLayout->insertWidget(insertAt, edit);
    fComputeArrayEdits[path].append(edit);
}

void TsQtAWS::PromptAddArrayValue(const QString& path)
{
    QVBoxLayout* rowsLayout = fComputeArrayRowsLayouts.value(path, nullptr);
    if (!rowsLayout || !fWizard)
        return;

    QDialog dialog(fWizard);
    dialog.setWindowTitle("Add value");
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->addWidget(new QLabel("Enter value to add:"));
    QLineEdit* input = new QLineEdit();
    input->setFixedHeight(22);
    layout->addWidget(input);
    QDialogButtonBox* box = new QDialogButtonBox();
    QPushButton* addBtn = box->addButton("Add", QDialogButtonBox::AcceptRole);
    addBtn->setEnabled(false);
    ApplyPrimaryButtonStyle(addBtn);
    box->addButton("Cancel", QDialogButtonBox::RejectRole);
    layout->addWidget(box);
    QObject::connect(input, &QLineEdit::textChanged, &dialog, [addBtn](const QString& t) { addBtn->setEnabled(!t.trimmed().isEmpty()); });
    QObject::connect(box, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() == QDialog::Accepted)
    {
        AddComputeArrayRow(path, input->text().trimmed(), rowsLayout);
        MarkComputeNeedsApply();
    }
}

void TsQtAWS::PromptRemoveArrayValue(const QString& path)
{
    QList<QLineEdit*> edits = fComputeArrayEdits.value(path);
    if (edits.isEmpty() || !fWizard)
        return;

    QDialog dialog(fWizard);
    dialog.setWindowTitle("Remove value");
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->addWidget(new QLabel("Select value to remove:"));
    QComboBox* combo = new QComboBox();
    QVector<int> indices;
    for (int i = 0; i < edits.size(); ++i) {
        QLineEdit* e = edits[i];
        if (!e)
            continue;
        combo->addItem(e->text());
        indices.append(i);
    }
    layout->addWidget(combo);
    QDialogButtonBox* box = new QDialogButtonBox();
    QPushButton* removeBtn = box->addButton("Remove", QDialogButtonBox::AcceptRole);
    removeBtn->setEnabled(combo->count() > 0);
    ApplyPrimaryButtonStyle(removeBtn);
    box->addButton("Cancel", QDialogButtonBox::RejectRole);
    layout->addWidget(box);
    QObject::connect(box, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted || combo->currentIndex() < 0)
        return;
    const int idx = indices[combo->currentIndex()];
    if (idx < 0 || idx >= edits.size() || !edits[idx])
        return;
    QLineEdit* victim = edits[idx];
    fComputeArrayEdits[path].removeAt(idx);
    delete victim;
    MarkComputeNeedsApply();
}

void TsQtAWS::SetJsonValueAtPath(QJsonObject& root, const QString& path, const QJsonValue& value) const
{
    const QStringList parts = path.split('.', Qt::SkipEmptyParts);
    if (parts.isEmpty())
        return;

    std::function<void(QJsonValue&, int)> setAt = [&](QJsonValue& node, int idx) {
        bool isIndex = false;
        const int index = parts[idx].toInt(&isIndex);
        const bool last = (idx == parts.size() - 1);
        bool nextIsIndex = false;
        if (!last)
            parts[idx + 1].toInt(&nextIsIndex);

        if (isIndex) {
            QJsonArray arr = node.isArray() ? node.toArray() : QJsonArray();
            while (arr.size() <= index)
                arr.append(QJsonValue());
            if (last) {
                arr[index] = value;
            } else {
                QJsonValue child = arr[index];
                if (child.isUndefined() || child.isNull())
                    child = nextIsIndex ? QJsonValue(QJsonArray()) : QJsonValue(QJsonObject());
                setAt(child, idx + 1);
                arr[index] = child;
            }
            node = arr;
            return;
        }

        const QString& key = parts[idx];
        QJsonObject obj = node.isObject() ? node.toObject() : QJsonObject();
        if (last) {
            obj.insert(key, value);
            node = obj;
            return;
        }
        QJsonValue child = obj.value(key);
        if (child.isUndefined() || child.isNull())
            child = nextIsIndex ? QJsonValue(QJsonArray()) : QJsonValue(QJsonObject());
        setAt(child, idx + 1);
        obj.insert(key, child);
        node = obj;
    };

    QJsonValue rootVal(root);
    setAt(rootVal, 0);
    root = rootVal.toObject();
}

bool TsQtAWS::BuildComputeJsonFromForm(QString* jsonOut, QString* errorOut) const
{
    if (!jsonOut || !fComputeTextEditor) {
        if (errorOut)
            *errorOut = "Internal form editor error.";
        return false;
    }
    QJsonParseError pe;
    QJsonDocument doc = QJsonDocument::fromJson(fComputeTextEditor->toPlainText().toUtf8(), &pe);
    if (pe.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorOut)
            *errorOut = "Current JSON is invalid. Switch to text editor mode to fix it.";
        return false;
    }
    QJsonObject root = doc.object();
    QStringList orderedScalarPaths;
    QStringList orderedArrayPaths;
    CollectFormPathsInJsonOrder(root, "", &orderedScalarPaths, &orderedArrayPaths);
    ApplyScalarEditsInOrder(orderedScalarPaths, fComputeScalarEdits, fComputeScalarTypes,
                            [&](const QString& path, const QJsonValue& v) { SetJsonValueAtPath(root, path, v); });
    ApplyPrimitiveArrayEditsInOrder(orderedArrayPaths, fComputeArrayEdits, fComputeArrayItemTypes,
                                    [&](const QString& path, const QJsonArray& arr) { SetJsonValueAtPath(root, path, arr); });

    QJsonDocument outDoc(root);
    *jsonOut = QString::fromUtf8(outDoc.toJson(QJsonDocument::Indented));
    return true;
}

void TsQtAWS::MarkComputeNeedsApply()
{
    fComputeNeedsApply = true;
    fComputeApplied = false;
    fComputeCreated = false;
    UpdateNavButtons();
}

void TsQtAWS::RebuildJobDefFormFromCurrentJson()
{
    if (!fJobDefFormPage || !fJobDefTextEditor)
        return;
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(fJobDefFormPage->layout());
    if (!layout)
        return;
    QLayoutItem* item = nullptr;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget())
            delete item->widget();
        delete item;
    }
    fJobDefScalarEdits.clear();
    fJobDefScalarTypes.clear();
    fJobDefArrayEdits.clear();
    fJobDefArrayItemTypes.clear();
    fJobDefArrayRowsLayouts.clear();

    QJsonParseError pe;
    QJsonDocument doc = QJsonDocument::fromJson(fJobDefTextEditor->toPlainText().toUtf8(), &pe);
    if (pe.error != QJsonParseError::NoError || (!doc.isObject() && !doc.isArray())) {
        QLabel* msg = new QLabel("JSON is invalid. Switch to text editor mode to fix syntax.");
        msg->setWordWrap(true);
        layout->addWidget(msg);
        layout->addStretch(1);
        return;
    }
    if (doc.isObject()) {
        const QJsonObject obj = doc.object();
        QStringList orderedKeys = ExtractTopLevelJobDefKeys(fJobDefTextEditor ? fJobDefTextEditor->toPlainText() : QString(), "");
        for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
            if (!orderedKeys.contains(it.key()))
                orderedKeys.append(it.key());
        }
        for (const QString& key : orderedKeys)
            BuildJobDefFormForValue(key, key, obj.value(key), layout);
    } else {
        const QJsonArray arr = doc.array();
        for (int i = 0; i < arr.size(); ++i) {
            if (!arr[i].isObject()) {
                QLabel* msg = new QLabel("JSON array entries must be objects for field editor mode.");
                msg->setWordWrap(true);
                layout->addWidget(msg);
                layout->addStretch(1);
                return;
            }
            BuildJobDefFormForValue(QString::number(i), QString("Definition %1").arg(i + 1), arr[i], layout);
        }
    }
    layout->addStretch(1);
}

void TsQtAWS::BuildJobDefFormForValue(const QString& path, const QString& label, const QJsonValue& value, QVBoxLayout* layout)
{
    if (!layout)
        return;
    if (value.isObject()) {
        QLabel* heading = new QLabel(QString("<b>%1</b>").arg(label));
        heading->setTextFormat(Qt::RichText);
        heading->setToolTip(AwsFieldTooltipForField(path));
        layout->addWidget(heading);
        QWidget* group = new QWidget();
        QVBoxLayout* groupLayout = new QVBoxLayout(group);
        groupLayout->setContentsMargins(24, 2, 0, 6);
        groupLayout->setSpacing(12);
        const QJsonObject obj = value.toObject();
        QStringList orderedKeys = ExtractTopLevelJobDefKeys(fJobDefTextEditor ? fJobDefTextEditor->toPlainText() : QString(), path);
        for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
            if (!orderedKeys.contains(it.key()))
                orderedKeys.append(it.key());
        }
        for (const QString& key : orderedKeys) {
            const QString childPath = path.isEmpty() ? key : (path + "." + key);
            BuildJobDefFormForValue(childPath, key, obj.value(key), groupLayout);
        }
        layout->addWidget(group);
        return;
    }
    if (value.isArray()) {
        const QJsonArray arr = value.toArray();
        const JsonArrayShape shape = ClassifyArrayShape(arr);
        if (shape == JsonArrayShape::Complex) {
            QLabel* unsupported = new QLabel(QString("%1: complex arrays are editable in text editor mode.").arg(label));
            unsupported->setWordWrap(true);
            unsupported->setToolTip(AwsFieldTooltipForField(path));
            layout->addWidget(unsupported);
            return;
        }

        QLabel* arrayLabel = new QLabel(QString("<b>%1</b>").arg(label));
        arrayLabel->setTextFormat(Qt::RichText);
        arrayLabel->setToolTip(AwsFieldTooltipForField(path));
        layout->addWidget(arrayLabel);

        if (shape == JsonArrayShape::FlatObject) {
            AddFlatObjectArrayRows(path, arr, layout, fWizard, &fJobDefScalarEdits, &fJobDefScalarTypes,
                                   &fJobDefFlatArrayRowsLayouts, &fJobDefFlatArrayKeys, &fJobDefFlatArrayRowWidgets,
                                   [this]() { MarkJobDefNeedsApply(); });
            return;
        }

        QHBoxLayout* btnRow = new QHBoxLayout();
        btnRow->setSpacing(8);
        QPushButton* addBtn = new QPushButton("+");
        addBtn->setFixedWidth(30);
        QObject::connect(addBtn, &QPushButton::clicked, fWizard, [this, path]() { PromptAddJobDefArrayValue(path); });
        QPushButton* removeBtn = new QPushButton("-");
        removeBtn->setFixedWidth(30);
        QObject::connect(removeBtn, &QPushButton::clicked, fWizard, [this, path]() { PromptRemoveJobDefArrayValue(path); });
        btnRow->addWidget(addBtn);
        btnRow->addWidget(removeBtn);
        btnRow->addStretch(1);
        QWidget* rowsWidget = new QWidget();
        QVBoxLayout* rowsLayout = new QVBoxLayout(rowsWidget);
        rowsLayout->setContentsMargins(0, 0, 0, 0);
        rowsLayout->setSpacing(9);
        fJobDefArrayEdits[path].clear();
        fJobDefArrayRowsLayouts[path] = rowsLayout;
        QJsonValue::Type itemType = arr.isEmpty() ? QJsonValue::String : arr[0].type();
        fJobDefArrayItemTypes[path] = itemType;
        for (const QJsonValue& v : arr)
            AddJobDefArrayRow(path, v.toVariant().toString(), rowsLayout);
        rowsLayout->addLayout(btnRow);
        layout->addWidget(rowsWidget);
        return;
    }
    QLabel* scalarLabel = new QLabel(QString("<b>%1</b>").arg(label));
    scalarLabel->setTextFormat(Qt::RichText);
    scalarLabel->setToolTip(AwsFieldTooltipForField(path));
    layout->addWidget(scalarLabel);
    QLineEdit* edit = new QLineEdit(value.toVariant().toString());
    edit->setFixedHeight(22);
    edit->setCursorPosition(0);
    edit->setToolTip(AwsFieldTooltipForField(path));
    QObject::connect(edit, &QLineEdit::textEdited, fWizard, [this](const QString&) { MarkJobDefNeedsApply(); });
    layout->addWidget(edit);
    fJobDefScalarEdits[path] = edit;
    fJobDefScalarTypes[path] = value.type();
}

void TsQtAWS::AddJobDefArrayRow(const QString& path, const QString& valueText, QVBoxLayout* rowsLayout)
{
    if (!rowsLayout)
        return;
    QLineEdit* edit = new QLineEdit(valueText);
    edit->setFixedHeight(22);
    edit->setCursorPosition(0);
    QObject::connect(edit, &QLineEdit::textEdited, fWizard, [this](const QString&) { MarkJobDefNeedsApply(); });
    const int insertAt = std::max(0, rowsLayout->count() - 1);
    rowsLayout->insertWidget(insertAt, edit);
    fJobDefArrayEdits[path].append(edit);
}

void TsQtAWS::PromptAddJobDefArrayValue(const QString& path)
{
    QVBoxLayout* rowsLayout = fJobDefArrayRowsLayouts.value(path, nullptr);
    if (!rowsLayout || !fWizard)
        return;
    QDialog dialog(fWizard);
    dialog.setWindowTitle("Add value");
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->addWidget(new QLabel("Enter value to add:"));
    QLineEdit* input = new QLineEdit();
    input->setFixedHeight(22);
    layout->addWidget(input);
    QDialogButtonBox* box = new QDialogButtonBox();
    QPushButton* addBtn = box->addButton("Add", QDialogButtonBox::AcceptRole);
    addBtn->setEnabled(false);
    ApplyPrimaryButtonStyle(addBtn);
    box->addButton("Cancel", QDialogButtonBox::RejectRole);
    layout->addWidget(box);
    QObject::connect(input, &QLineEdit::textChanged, &dialog, [addBtn](const QString& t) { addBtn->setEnabled(!t.trimmed().isEmpty()); });
    QObject::connect(box, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() == QDialog::Accepted) {
        AddJobDefArrayRow(path, input->text().trimmed(), rowsLayout);
        MarkJobDefNeedsApply();
    }
}

void TsQtAWS::PromptRemoveJobDefArrayValue(const QString& path)
{
    QList<QLineEdit*> edits = fJobDefArrayEdits.value(path);
    if (edits.isEmpty() || !fWizard)
        return;
    QDialog dialog(fWizard);
    dialog.setWindowTitle("Remove value");
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->addWidget(new QLabel("Select value to remove:"));
    QComboBox* combo = new QComboBox();
    QVector<int> indices;
    for (int i = 0; i < edits.size(); ++i) {
        QLineEdit* e = edits[i];
        if (!e)
            continue;
        combo->addItem(e->text());
        indices.append(i);
    }
    layout->addWidget(combo);
    QDialogButtonBox* box = new QDialogButtonBox();
    QPushButton* removeBtn = box->addButton("Remove", QDialogButtonBox::AcceptRole);
    removeBtn->setEnabled(combo->count() > 0);
    ApplyPrimaryButtonStyle(removeBtn);
    box->addButton("Cancel", QDialogButtonBox::RejectRole);
    layout->addWidget(box);
    QObject::connect(box, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted || combo->currentIndex() < 0)
        return;
    const int idx = indices[combo->currentIndex()];
    if (idx < 0 || idx >= edits.size() || !edits[idx])
        return;
    QLineEdit* victim = edits[idx];
    fJobDefArrayEdits[path].removeAt(idx);
    delete victim;
    MarkJobDefNeedsApply();
}

bool TsQtAWS::BuildJobDefJsonFromForm(QString* jsonOut, QString* errorOut) const
{
    if (!jsonOut || !fJobDefTextEditor) {
        if (errorOut)
            *errorOut = "Internal form editor error.";
        return false;
    }
    QJsonParseError pe;
    QJsonDocument doc = QJsonDocument::fromJson(fJobDefTextEditor->toPlainText().toUtf8(), &pe);
    if (pe.error != QJsonParseError::NoError || (!doc.isObject() && !doc.isArray())) {
        if (errorOut)
            *errorOut = "Current JSON is invalid. Switch to text editor mode to fix it.";
        return false;
    }

    if (doc.isObject()) {
        QJsonObject root = doc.object();
        QStringList orderedScalarPaths;
        QStringList orderedArrayPaths;
        CollectFormPathsInJsonOrder(root, "", &orderedScalarPaths, &orderedArrayPaths);
        ApplyScalarEditsInOrder(orderedScalarPaths, fJobDefScalarEdits, fJobDefScalarTypes,
                                [&](const QString& path, const QJsonValue& v) { SetJsonValueAtPath(root, path, v); });
        ApplyPrimitiveArrayEditsInOrder(orderedArrayPaths, fJobDefArrayEdits, fJobDefArrayItemTypes,
                                        [&](const QString& path, const QJsonArray& arr) { SetJsonValueAtPath(root, path, arr); });
        QJsonDocument outDoc(root);
        *jsonOut = SerializeJobDefRootPreservingTopOrder(outDoc, fJobDefTextEditor ? fJobDefTextEditor->toPlainText() : QString());
        return true;
    }

    QJsonArray rootArr = doc.array();
    QStringList orderedScalarPaths;
    QStringList orderedArrayPaths;
    for (int i = 0; i < rootArr.size(); ++i)
        CollectFormPathsInJsonOrder(rootArr[i], QString::number(i), &orderedScalarPaths, &orderedArrayPaths);
    auto setInIndexedObject = [&](const QString& fullPath, const QJsonValue& val) {
        const int dot = fullPath.indexOf('.');
        if (dot <= 0)
            return;
        bool ok = false;
        const int idx = fullPath.left(dot).toInt(&ok);
        if (!ok || idx < 0 || idx >= rootArr.size() || !rootArr[idx].isObject())
            return;
        const QString subPath = fullPath.mid(dot + 1);
        QJsonObject obj = rootArr[idx].toObject();
        SetJsonValueAtPath(obj, subPath, val);
        rootArr[idx] = obj;
    };

    ApplyScalarEditsInOrder(orderedScalarPaths, fJobDefScalarEdits, fJobDefScalarTypes,
                            [&](const QString& path, const QJsonValue& v) { setInIndexedObject(path, v); });
    ApplyPrimitiveArrayEditsInOrder(orderedArrayPaths, fJobDefArrayEdits, fJobDefArrayItemTypes,
                                    [&](const QString& path, const QJsonArray& arr) { setInIndexedObject(path, arr); });
    QJsonDocument outDoc(rootArr);
    *jsonOut = SerializeJobDefRootPreservingTopOrder(outDoc, fJobDefTextEditor ? fJobDefTextEditor->toPlainText() : QString());
    return true;
}

void TsQtAWS::MarkJobDefNeedsApply()
{
    fJobDefNeedsApply = true;
    fJobDefApplied = false;
    fJobDefCreated = false;
    UpdateNavButtons();
}

void TsQtAWS::SaveAwsSessionOnly()
{
    if (!fAwsDirEdit || !fProfileEdit || !fEditAwsRegion)
        return;
    fAwsDir = fAwsDirEdit->text();
    fProfile = fProfileEdit->text().trimmed();
    fRegion = fEditAwsRegion->text().trimmed();
}

void TsQtAWS::ReloadAwsFilesFromUi()
{
    if (!fAwsDirEdit || !fProfileEdit || !fEditAwsRegion || !fComputeEdit || !fJobDefEdit)
        return;
    fAwsDir = fAwsDirEdit->text();
    fProfile = fProfileEdit->text().trimmed();
    fRegion = fEditAwsRegion->text().trimmed();
    fComputeJsonPath = QDir(fAwsDir).filePath(kFileCompute);
    fJobDefJsonPath = QDir(fAwsDir).filePath(kFileJobDef);
    fJobDefPostJsonPath = QDir(fAwsDir).filePath(kFileJobDefPost);
    fTopasSubmitPath = QDir(fAwsDir).filePath(kFileSubmit);
    fPostprocessSubmitPath = QDir(fAwsDir).filePath(kFilePost);
    fProjectName.clear();
    fRunDate.clear();
    fOutputBucket.clear();
    QMap<QString, QString> postVarDefaults;
    QStringList postVarOrder;
    QString postJobDefDefault;
    QFile fc(fComputeJsonPath);
    if (fc.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString computeText = QString::fromUtf8(fc.readAll());
        if (fComputeTextEditor)
            fComputeTextEditor->setPlainText(computeText);
        fComputeDefaultJsonText = computeText;
        QJsonParseError computeErr;
        const QJsonDocument computeDoc = QJsonDocument::fromJson(computeText.toUtf8(), &computeErr);
        if (computeErr.error == QJsonParseError::NoError && computeDoc.isObject())
            fDefaultExistingComputeEnvName =
                computeDoc.object().value(QStringLiteral("computeEnvironmentName")).toString().trimmed();
        if (fEditExistingComputeEnvName && fEditExistingComputeEnvName->text().trimmed().isEmpty()) {
            if (!fDefaultExistingComputeEnvName.isEmpty())
                fEditExistingComputeEnvName->setText(fDefaultExistingComputeEnvName);
        }
    }
    QFile fj(fJobDefJsonPath);
    if (fj.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fJobDefJsonText = QString::fromUtf8(fj.readAll());
        fJobDefDefaultJsonText = fJobDefJsonText;
    }
    QFile fjPost(fJobDefPostJsonPath);
    if (fjPost.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fJobDefPostJsonText = QString::fromUtf8(fjPost.readAll());
        fJobDefPostDefaultJsonText = fJobDefPostJsonText;
    } else {
        fJobDefPostJsonText.clear();
        fJobDefPostDefaultJsonText.clear();
    }
    fActiveJobDefTabIndex = 0;
    EnsureJobDefEditorHostForPage(true);
    LoadActiveJobDefIntoEditor();

    // Build step-4 form dynamically from topas_submit.sh variables block.
    if (fSubmitVarsGrid) {
        QLayoutItem* item = nullptr;
        while ((item = fSubmitVarsGrid->takeAt(0)) != nullptr) {
            if (item->widget())
                delete item->widget();
            delete item;
        }
        fSubmitVarOrder.clear();
        fSubmitVarDefaults.clear();
        fSubmitVarEdits.clear();
        QFile fs(fTopasSubmitPath);
        if (fs.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const QString scriptText = QString::fromUtf8(fs.readAll());
            ParseSubmitVariablesBlock(scriptText, &fSubmitVarOrder, &fSubmitVarDefaults);
        }
        for (int i = 0; i < fSubmitVarOrder.size(); ++i) {
            const QString& key = fSubmitVarOrder[i];
            QLabel* lbl = new QLabel(key);
            QLineEdit* edit = new QLineEdit(fSubmitVarDefaults.value(key));
            edit->setFixedHeight(22);
            QObject::connect(edit, &QLineEdit::textEdited, fWizard, [this](const QString&) {
                fSubmitNeedsReset = true;
                fSubmitApplied = false;
                fSubmitScriptSnapshotPath.clear();
                UpdateNavButtons();
            });
            fSubmitVarsGrid->addWidget(lbl, i, 0);
            fSubmitVarsGrid->addWidget(edit, i, 1);
            if (key == QStringLiteral("LOCAL_SIM_DIR")) {
                QPushButton* browseDirBtn = new QPushButton("Browse...");
                QObject::connect(browseDirBtn, &QPushButton::clicked, fWizard, [this, edit]() {
                    const QString startDir = edit ? edit->text().trimmed() : QString();
                    const QString dir = QFileDialog::getExistingDirectory(
                        fWizard, "Select local simulation directory", startDir);
                    if (dir.isEmpty() || !edit)
                        return;
                    edit->setText(dir);
                    fSubmitNeedsReset = true;
                    fSubmitApplied = false;
                    fSubmitScriptSnapshotPath.clear();
                    UpdateNavButtons();
                });
                fSubmitVarsGrid->addWidget(browseDirBtn, i, 2);
            } else if (key == QStringLiteral("FILE_TO_RUN")) {
                QPushButton* browseFileBtn = new QPushButton("Browse...");
                QObject::connect(browseFileBtn, &QPushButton::clicked, fWizard, [this, edit]() {
                    QString startDir;
                    if (fEditLocalSimDir && !fEditLocalSimDir->text().trimmed().isEmpty())
                        startDir = fEditLocalSimDir->text().trimmed();
                    else if (edit && !edit->text().trimmed().isEmpty())
                        startDir = QFileInfo(edit->text().trimmed()).absolutePath();
                    const QString filePath = QFileDialog::getOpenFileName(
                        fWizard, "Select parameter file to run", startDir);
                    if (filePath.isEmpty() || !edit)
                        return;
                    edit->setText(QFileInfo(filePath).fileName());
                    fSubmitNeedsReset = true;
                    fSubmitApplied = false;
                    fSubmitScriptSnapshotPath.clear();
                    UpdateNavButtons();
                });
                fSubmitVarsGrid->addWidget(browseFileBtn, i, 2);
            }
            fSubmitVarEdits[key] = edit;
        }
        // Backward-compat aliases used by step 5/6 code paths.
        fEditProjectName = fSubmitVarEdits.value("PROJECT_NAME", nullptr);
        fEditRunDate = fSubmitVarEdits.value("RUN_DATE", nullptr);
        fEditNumJobs = fSubmitVarEdits.value("NUM_JOBS", nullptr);
        fEditJobName = fSubmitVarEdits.value("JOB_NAME", nullptr);
        fEditInputBucket = fSubmitVarEdits.value("INPUT_BUCKET", nullptr);
        fEditOutputBucket = fSubmitVarEdits.value("OUTPUT_BUCKET", nullptr);
        fEditLocalSimDir = fSubmitVarEdits.value("LOCAL_SIM_DIR", nullptr);
        fEditFileToRun = fSubmitVarEdits.value("FILE_TO_RUN", nullptr);
        fEditJobQueue = fSubmitVarEdits.value("JOB_QUEUE", nullptr);
        fEditJobDefinitionName = fSubmitVarEdits.value("JOB_DEFINITION", nullptr);
        if (!fEditJobDefinitionName)
            fEditJobDefinitionName = fSubmitVarEdits.value("JOB_DEFINITION_NAME", nullptr);
        fSubmitNeedsReset = false;
        fSubmitApplied = true;
        fSubmitScriptSnapshotPath = fTopasSubmitPath;
        if (fEditExistingInputBucket && fEditExistingInputBucket->text().trimmed().isEmpty())
            fEditExistingInputBucket->setText(fSubmitVarDefaults.value(QStringLiteral("INPUT_BUCKET")));
        if (fEditExistingOutputBucket && fEditExistingOutputBucket->text().trimmed().isEmpty())
            fEditExistingOutputBucket->setText(fSubmitVarDefaults.value(QStringLiteral("OUTPUT_BUCKET")));
        if (fEditExistingJobQueue && fEditExistingJobQueue->text().trimmed().isEmpty())
            fEditExistingJobQueue->setText(fSubmitVarDefaults.value(QStringLiteral("JOB_QUEUE")));
        if (fEditExistingJobDefSim && fEditExistingJobDefSim->text().trimmed().isEmpty()) {
            QString jds = fSubmitVarDefaults.value(QStringLiteral("JOB_DEFINITION"));
            if (jds.isEmpty())
                jds = fSubmitVarDefaults.value(QStringLiteral("JOB_DEFINITION_NAME"));
            if (!jds.isEmpty())
                fEditExistingJobDefSim->setText(jds);
        }
        postJobDefDefault = fSubmitVarDefaults.value(QStringLiteral("POSTPROCESS_JOB_DEFINITION")).trimmed();
    }

    QFile fp(fPostprocessSubmitPath);
    if (fp.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString postScriptText = QString::fromUtf8(fp.readAll());
        ParseSubmitVariablesBlock(postScriptText, &postVarOrder, &postVarDefaults);
        const QString postQueueDefault = postVarDefaults.value(QStringLiteral("JOB_QUEUE")).trimmed();
        if (fEditJobQueue && fEditJobQueue->text().trimmed().isEmpty() && !postQueueDefault.isEmpty())
            fEditJobQueue->setText(postQueueDefault);
        if (postJobDefDefault.isEmpty())
            postJobDefDefault = postVarDefaults.value(QStringLiteral("POSTPROCESS_JOB_DEFINITION")).trimmed();
        if (postJobDefDefault.isEmpty()) {
            const QRegularExpression postJdRe("\\-\\-job\\-definition\\s+((?:\"[^\"]+\")|(?:'[^']+')|(?:[^\\s\\\\]+))");
            const QRegularExpressionMatch m = postJdRe.match(postScriptText);
            if (m.hasMatch()) {
                postJobDefDefault = m.captured(1).trimmed();
                if (postJobDefDefault.startsWith('"') && postJobDefDefault.endsWith('"') && postJobDefDefault.size() >= 2)
                    postJobDefDefault = postJobDefDefault.mid(1, postJobDefDefault.size() - 2);
                else if (postJobDefDefault.startsWith('\'') && postJobDefDefault.endsWith('\'') && postJobDefDefault.size() >= 2)
                    postJobDefDefault = postJobDefDefault.mid(1, postJobDefDefault.size() - 2);
            }
        }
    }
    fPostprocessVarOrder = postVarOrder;
    fPostprocessVarDefaults = postVarDefaults;
    if (fPostprocessVarsGrid) {
        QLayoutItem* item = nullptr;
        while ((item = fPostprocessVarsGrid->takeAt(0)) != nullptr) {
            if (item->widget())
                delete item->widget();
            delete item;
        }
        fPostprocessVarEdits.clear();
        int row = 0;
        for (const QString& key : postVarOrder) {
            QLabel* lbl = new QLabel(key);
            QLineEdit* edit = new QLineEdit(postVarDefaults.value(key));
            edit->setFixedHeight(22);
            fPostprocessVarsGrid->addWidget(lbl, row, 0);
            fPostprocessVarsGrid->addWidget(edit, row, 1);
            fPostprocessVarEdits[key] = edit;
            row++;
        }
    }
    fEditPpLocalScript = fPostprocessVarEdits.value(QStringLiteral("LOCAL_SCRIPT"), nullptr);
    fEditPpExtraPip = fPostprocessVarEdits.value(QStringLiteral("EXTRA_PIP_PACKAGES"), nullptr);
    fEditPostprocessJobDefinitionName = fPostprocessVarEdits.value(QStringLiteral("POSTPROCESS_JOB_DEFINITION"), nullptr);
    if (!fEditPostprocessJobDefinitionName)
        fEditPostprocessJobDefinitionName = fPostprocessVarEdits.value(QStringLiteral("JOB_DEFINITION"), nullptr);
    if (!fEditJobQueue || fEditJobQueue->text().trimmed().isEmpty())
        fEditJobQueue = fPostprocessVarEdits.value(QStringLiteral("JOB_QUEUE"), fEditJobQueue);
    if (!fEditProjectName || fEditProjectName->text().trimmed().isEmpty())
        fEditProjectName = fPostprocessVarEdits.value(QStringLiteral("PROJECT_NAME"), fEditProjectName);
    if (!fEditRunDate || fEditRunDate->text().trimmed().isEmpty())
        fEditRunDate = fPostprocessVarEdits.value(QStringLiteral("RUN_DATE"), fEditRunDate);
    if (!fEditOutputBucket || fEditOutputBucket->text().trimmed().isEmpty())
        fEditOutputBucket = fPostprocessVarEdits.value(QStringLiteral("OUTPUT_BUCKET"), fEditOutputBucket);
    if (fEditExistingJobDefPost && fEditExistingJobDefPost->text().trimmed().isEmpty() && !postJobDefDefault.isEmpty())
        fEditExistingJobDefPost->setText(postJobDefDefault);

    QFileInfo fiTop(QString::fromUtf8(fPm->GetTopParameterFileSpec().c_str()));
    QString defaultDir = fiTop.absolutePath();
    QString ppLocal = postVarDefaults.value(QStringLiteral("LOCAL_SCRIPT")).trimmed();
    const QString awsBaseDir = (fAwsDirEdit && !fAwsDirEdit->text().trimmed().isEmpty())
        ? fAwsDirEdit->text().trimmed()
        : fAwsDir;
    if (QDir::isRelativePath(ppLocal) && !awsBaseDir.isEmpty())
        ppLocal = QDir(awsBaseDir).filePath(ppLocal);
    ppLocal = QDir::cleanPath(ppLocal);
    QString ppPip = postVarDefaults.value(QStringLiteral("EXTRA_PIP_PACKAGES")).trimmed();
    if (fEditPpLocalScript && fEditPpLocalScript->text().trimmed().isEmpty())
        fEditPpLocalScript->setText(ppLocal);
    if (fEditPpExtraPip && fEditPpExtraPip->text().trimmed().isEmpty())
        fEditPpExtraPip->setText(ppPip);
    if (fEditPostprocessJobDefinitionName && fEditPostprocessJobDefinitionName->text().trimmed().isEmpty() && !postJobDefDefault.isEmpty())
        fEditPostprocessJobDefinitionName->setText(postJobDefDefault);
    if (fEditPostprocessScriptPath && fEditPostprocessScriptPath->text().trimmed().isEmpty())
        fEditPostprocessScriptPath->setText(QDir::cleanPath(awsBaseDir));
    fEditDownloadDir->setText(QDir(defaultDir).filePath("aws_outputs"));

    fComputeJsonText = fComputeTextEditor ? fComputeTextEditor->toPlainText() : QString();
    fJobDefJsonText = fJobDefTextEditor ? fJobDefTextEditor->toPlainText() : QString();
    fComputeApplied = true;
    fComputeCreated = false;
    fComputeNeedsApply = false;
    fJobDefApplied = true;
    fJobDefCreated = false;
    fJobDefNeedsApply = false;
    if (fComputeApplyDesc) {
        fComputeApplyDesc->setText("These settings will be saved locally in the AWS directory that you specified in step 1.");
    }
    if (fJobDefApplyDesc) {
        fJobDefApplyDesc->setText("These settings will be saved locally in the AWS directory that you specified in step 1.");
    }
    UpdateJobDefTabVisibility();
    RebuildComputeFormFromCurrentJson();
    RebuildJobDefFormFromCurrentJson();
    UpdateNavButtons();
}

bool TsQtAWS::HasRequiredFiles(const QString& dir) const
{
    if (dir.isEmpty())
        return false;
    QDir d(dir);
    if (!d.exists())
        return false;
    return d.exists(kFileCompute) && d.exists(kFileJobDef) && d.exists(kFileJobDefPost) && d.exists(kFileSubmit)
        && d.exists(kFilePost);
}

QString TsQtAWS::AutoDetectAwsDir() const
{
    const QString appDir = QCoreApplication::applicationDirPath();
    QStringList roots;
    roots << "/Applications/TOPAS-nBio/aws/"
          << QDir::homePath() + "/Applications/TOPAS-nBio/aws/"
          << appDir + "/../../OpenTOPAS/aws/"
          << appDir + "/../../TOPAS-nBio/aws/";

    for (const QString& r : roots) {
        const QString c = QDir::cleanPath(r);
        if (HasRequiredFiles(c))
            return c;
    }
    return QString();
}

QString TsQtAWS::SnapshotSubdir() const
{
    QString stamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    stamp.replace(':', "-");
    QDir d(fAwsDir);
    QString snap = d.filePath("snapshots/" + stamp);
    return snap;
}

// ---------------------------------------------------------------------------
// Run aws / bash and small file helpers
// ---------------------------------------------------------------------------

bool TsQtAWS::WriteTextFile(const QString& path, const QString& text)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    qint64 n = f.write(text.toUtf8());
    f.close();
    return n > 0;
}

void TsQtAWS::AwsCliArgs(QStringList* args) const
{
    if (!fProfile.isEmpty()) {
        args->append("--profile");
        args->append(fProfile);
    }
    if (!fRegion.isEmpty()) {
        args->append("--region");
        args->append(fRegion);
    }
}

bool TsQtAWS::RunProcess(const QString& program, const QStringList& args, const QString& workingDir, QString* out,
                         QString* err, int* exitCode)
{
    QProcess proc;
    if (!workingDir.isEmpty())
        proc.setWorkingDirectory(workingDir);
    proc.start(program, args);
    if (!proc.waitForFinished(1200000)) {
        proc.kill();
        if (err)
            *err = QString("Timed out waiting for: ") + program;
        if (exitCode)
            *exitCode = -1;
        return false;
    }
    if (out)
        *out = QString::fromUtf8(proc.readAllStandardOutput());
    if (err)
        *err = QString::fromUtf8(proc.readAllStandardError());
    if (exitCode)
        *exitCode = proc.exitCode();
    return proc.exitStatus() == QProcess::NormalExit;
}

bool TsQtAWS::RunAws(const QStringList& args, QString* out, QString* err, int* exitCode)
{
    if (fProfileEdit)
        fProfile = fProfileEdit->text().trimmed();
    if (fEditAwsRegion)
        fRegion = fEditAwsRegion->text().trimmed();
    QStringList full;
    AwsCliArgs(&full);
    full.append(args);
    return RunProcess("aws", full, QString(), out, err, exitCode);
}

void TsQtAWS::ShowAwsMessage(QWidget* parent, const QString& title, bool ok, const QString& detail)
{
    QMessageBox box(parent);
    box.setWindowTitle(title);
    if (ok) {
        box.setIcon(QMessageBox::Information);
        box.setText("Command finished successfully.");
    } else {
        box.setIcon(QMessageBox::Critical);
        box.setText("Command failed.");
    }
    box.setDetailedText(detail);
    box.exec();
}

// ---------------------------------------------------------------------------
// Apply JSON snapshots to AWS (compute environment, job definitions)
// ---------------------------------------------------------------------------

static void TopasAwsSetContainerEnvValue(QJsonObject& root, const QString& name, const QString& value)
{
    QJsonObject cp = root.value(QStringLiteral("containerProperties")).toObject();
    QJsonArray env = cp.value(QStringLiteral("environment")).toArray();
    bool found = false;
    for (int i = 0; i < env.size(); ++i) {
        QJsonObject e = env[i].toObject();
        if (e.value(QStringLiteral("name")).toString() == name) {
            e.insert(QStringLiteral("value"), value);
            env[i] = e;
            found = true;
            break;
        }
    }
    if (!found) {
        QJsonObject e;
        e.insert(QStringLiteral("name"), name);
        e.insert(QStringLiteral("value"), value);
        env.append(e);
    }
    cp.insert(QStringLiteral("environment"), env);
    root.insert(QStringLiteral("containerProperties"), cp);
}

static void TopasAwsSetAwsLogsOptions(QJsonObject& root, const QString& logGroup, const QString& region)
{
    QJsonObject cp = root.value(QStringLiteral("containerProperties")).toObject();
    QJsonObject lc = cp.value(QStringLiteral("logConfiguration")).toObject();
    QJsonObject opts = lc.value(QStringLiteral("options")).toObject();
    if (!logGroup.isEmpty())
        opts.insert(QStringLiteral("awslogs-group"), logGroup);
    if (!region.isEmpty())
        opts.insert(QStringLiteral("awslogs-region"), region);
    lc.insert(QStringLiteral("options"), opts);
    cp.insert(QStringLiteral("logConfiguration"), lc);
    root.insert(QStringLiteral("containerProperties"), cp);
}

bool TsQtAWS::ApplyComputeJsonSnapshot(bool showSuccessMessage)
{
    QJsonParseError pe;
    QJsonDocument doc = QJsonDocument::fromJson(fComputeJsonText.toUtf8(), &pe);
    if (pe.error != QJsonParseError::NoError) {
        ShowAwsMessage(fWizardParent, "Compute environment JSON", false, pe.errorString());
        return false;
    }
    const QString folderName = fEditComputeSnapshotFolder ? fEditComputeSnapshotFolder->text().trimmed() : QString();
    if (folderName.isEmpty()) {
        ShowAwsMessage(fWizardParent, "Compute environment JSON", false, "Please enter a folder name.");
        return false;
    }
    const QString localRoot = QDir(fAwsDir).filePath("snapshots/" + folderName);
    QDir().mkpath(localRoot);
    const QString localFile = QDir(localRoot).filePath(kFileCompute);
    if (!WriteTextFile(localFile, fComputeJsonText)) {
        ShowAwsMessage(fWizardParent, "Compute environment JSON", false, "Could not write local snapshot file.");
        return false;
    }
    fSnapshotRoot = localRoot;

    fComputeApplied = true;
    fComputeCreated = false;
    fComputeNeedsApply = false;
    if (showSuccessMessage) {
        QMessageBox box(fWizardParent);
        box.setWindowTitle("Apply changes");
        box.setIcon(QMessageBox::Information);
        box.setText("Changes saved successfully.");
        box.setDetailedText("Snapshot saved.");
        box.exec();
    }
    return true;
}

bool TsQtAWS::CreateComputeEnvironmentFromSnapshot()
{
    if (fComputeUseExisting)
        return true;
    if (fComputeTextMode) {
        fComputeJsonText = fComputeTextEditor ? fComputeTextEditor->toPlainText() : QString();
    } else {
        QString builtJson;
        QString error;
        if (!BuildComputeJsonFromForm(&builtJson, &error)) {
            ShowAwsMessage(fWizardParent, "Compute environment JSON", false, error);
            return false;
        }
        fComputeJsonText = builtJson;
        if (fComputeTextEditor)
            fComputeTextEditor->setPlainText(builtJson);
    }
    if (!ApplyComputeJsonSnapshot(false))
        return false;
    const QString snapFile = QDir(fSnapshotRoot).filePath(kFileCompute);
    QString out;
    QString err;
    int code = 0;
    QStringList args;
    args << "batch"
         << "create-compute-environment"
         << "--cli-input-json"
         << "file://" + snapFile;
    if (!RunAws(args, &out, &err, &code) || code != 0) {
        QString detail = err;
        if (!out.isEmpty())
            detail += "\n" + out;
        ShowAwsMessage(fWizardParent, "AWS create-compute-environment", false, detail);
        return false;
    }
    ShowAwsMessage(fWizardParent, "AWS create-compute-environment", true, out.isEmpty() ? QString("OK") : out);
    return true;
}

bool TsQtAWS::RegisterJobDefinitionFile(const QString& jsonFilePath, QString* errOut)
{
    QFile f(jsonFilePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errOut)
            *errOut = "Cannot read job definition JSON: " + jsonFilePath;
        return false;
    }
    QByteArray data = f.readAll();
    f.close();
    QJsonParseError pe;
    QJsonDocument doc = QJsonDocument::fromJson(data, &pe);
    if (pe.error != QJsonParseError::NoError) {
        if (errOut)
            *errOut = pe.errorString();
        return false;
    }

    QJsonArray arr;
    if (doc.isArray()) {
        arr = doc.array();
    } else if (doc.isObject()) {
        arr.append(doc.object());
    } else {
        if (errOut)
            *errOut = "Job definition JSON must be an object or array.";
        return false;
    }

    int idx = 0;
    for (const QJsonValue& v : arr) {
        if (!v.isObject()) {
            if (errOut)
                *errOut = "Array element is not an object.";
            return false;
        }
        QJsonDocument one(v.toObject());
        QString tmpPath = QDir(fSnapshotRoot).filePath(QString("jobdef_reg_%1.json").arg(idx));
        if (!WriteTextFile(tmpPath, QString::fromUtf8(one.toJson(QJsonDocument::Indented)))) {
            if (errOut)
                *errOut = "Failed to write temp job definition.";
            return false;
        }
        QString out;
        QString err;
        int code = 0;
        QStringList args;
        args << "batch"
             << "register-job-definition"
             << "--cli-input-json"
             << "file://" + tmpPath;
        if (!RunAws(args, &out, &err, &code) || code != 0) {
            if (errOut)
                *errOut = err + "\n" + out;
            return false;
        }
        idx++;
    }
    return true;
}

bool TsQtAWS::ApplyJobDefJsonSnapshot()
{
    QJsonParseError pe;
    QJsonDocument doc = QJsonDocument::fromJson(fJobDefJsonText.toUtf8(), &pe);
    if (pe.error != QJsonParseError::NoError || !doc.isObject()) {
        ShowAwsMessage(fWizardParent, "Job definition JSON", false,
                       QStringLiteral("Simulation job definition: %1").arg(pe.errorString()));
        return false;
    }
    if (fWantsPostprocessing) {
        QJsonDocument docP = QJsonDocument::fromJson(fJobDefPostJsonText.toUtf8(), &pe);
        if (pe.error != QJsonParseError::NoError || !docP.isObject()) {
            ShowAwsMessage(fWizardParent, "Job definition JSON", false,
                           QStringLiteral("Post-processing job definition: %1").arg(pe.errorString()));
            return false;
        }
    }
    const QString folderName = fEditJobDefSnapshotFolder ? fEditJobDefSnapshotFolder->text().trimmed() : QString();
    if (folderName.isEmpty()) {
        ShowAwsMessage(fWizardParent, "Job definition JSON", false, "Please enter a folder name.");
        return false;
    }
    fSnapshotRoot = QDir(fAwsDir).filePath("snapshots/" + folderName);
    QDir().mkpath(fSnapshotRoot);
    const QString snapSim = QDir(fSnapshotRoot).filePath(kFileJobDef);
    if (!WriteTextFile(snapSim, fJobDefJsonText)) {
        ShowAwsMessage(fWizardParent, "Job definition JSON", false, "Could not write simulation snapshot file.");
        return false;
    }
    if (fWantsPostprocessing) {
        const QString snapPost = QDir(fSnapshotRoot).filePath(kFileJobDefPost);
        if (!WriteTextFile(snapPost, fJobDefPostJsonText)) {
            ShowAwsMessage(fWizardParent, "Job definition JSON", false, "Could not write post-processing snapshot file.");
            return false;
        }
    }
    fJobDefApplied = true;
    fJobDefCreated = false;
    fJobDefNeedsApply = false;
    return true;
}

void TsQtAWS::CommitJobDefEditorToActiveString()
{
    if (!fJobDefTextEditor)
        return;
    if (fJobDefTextMode) {
        if (fActiveJobDefTabIndex == 0)
            fJobDefJsonText = fJobDefTextEditor->toPlainText();
        else
            fJobDefPostJsonText = fJobDefTextEditor->toPlainText();
    } else {
        QString builtJson;
        QString error;
        if (!BuildJobDefJsonFromForm(&builtJson, &error))
            return;
        if (fActiveJobDefTabIndex == 0)
            fJobDefJsonText = builtJson;
        else
            fJobDefPostJsonText = builtJson;
    }
}

void TsQtAWS::LoadActiveJobDefIntoEditor()
{
    if (!fJobDefTextEditor)
        return;
    const QString txt = (fActiveJobDefTabIndex == 0) ? fJobDefJsonText : fJobDefPostJsonText;
    const QSignalBlocker b(fJobDefTextEditor);
    fJobDefTextEditor->setPlainText(txt);
}

void TsQtAWS::UpdateJobDefTabVisibility()
{
    if (!fWantsPostprocessing && fActiveJobDefTabIndex == 1) {
        CommitJobDefEditorToActiveString();
        fActiveJobDefTabIndex = 0;
        LoadActiveJobDefIntoEditor();
    }
}

void TsQtAWS::EnsureJobDefEditorHostForPage(bool simulationPage)
{
    if (!fJobDefEditorHost)
        return;
    QVBoxLayout* target = simulationPage ? fSimJobDefHostLayout : fPostJobDefHostLayout;
    if (!target)
        return;
    if (QWidget* pw = fJobDefEditorHost->parentWidget()) {
        if (QLayout* pl = pw->layout())
            pl->removeWidget(fJobDefEditorHost);
    }
    target->addWidget(fJobDefEditorHost, 1);
}

static QString TopasAwsJobDefinitionNameFromJson(const QString& jsonText)
{
    QJsonParseError pe;
    const QJsonDocument d = QJsonDocument::fromJson(jsonText.toUtf8(), &pe);
    if (pe.error != QJsonParseError::NoError || !d.isObject())
        return QString();
    return d.object().value(QStringLiteral("jobDefinitionName")).toString().trimmed();
}

void TsQtAWS::SyncSubmitStepFromBatchState()
{
    const QString inB = fEditProvisionInputBucket ? fEditProvisionInputBucket->text().trimmed() : QString();
    const QString outB = fEditProvisionOutputBucket ? fEditProvisionOutputBucket->text().trimmed() : QString();
    if (fSubmitVarEdits.contains(QStringLiteral("INPUT_BUCKET")) && !inB.isEmpty())
        fSubmitVarEdits[QStringLiteral("INPUT_BUCKET")]->setText(inB);
    if (fSubmitVarEdits.contains(QStringLiteral("OUTPUT_BUCKET")) && !outB.isEmpty())
        fSubmitVarEdits[QStringLiteral("OUTPUT_BUCKET")]->setText(outB);
    const QString queueName = (fBatchUseExisting && fEditExistingJobQueue)
        ? fEditExistingJobQueue->text().trimmed()
        : (fEditWizardJobQueueName ? fEditWizardJobQueueName->text().trimmed() : QString());
    if (fSubmitVarEdits.contains(QStringLiteral("JOB_QUEUE")) && !queueName.isEmpty())
        fSubmitVarEdits[QStringLiteral("JOB_QUEUE")]->setText(queueName);
    if (fPostprocessVarEdits.contains(QStringLiteral("JOB_QUEUE")) && !queueName.isEmpty())
        fPostprocessVarEdits[QStringLiteral("JOB_QUEUE")]->setText(queueName);

    const QString projectName = fEditProjectName ? fEditProjectName->text().trimmed() : QString();
    const QString runDate = fEditRunDate ? fEditRunDate->text().trimmed() : QString();
    if (fPostprocessVarEdits.contains(QStringLiteral("PROJECT_NAME")) && !projectName.isEmpty())
        fPostprocessVarEdits[QStringLiteral("PROJECT_NAME")]->setText(projectName);
    if (fPostprocessVarEdits.contains(QStringLiteral("RUN_DATE")) && !runDate.isEmpty())
        fPostprocessVarEdits[QStringLiteral("RUN_DATE")]->setText(runDate);
    if (fPostprocessVarEdits.contains(QStringLiteral("OUTPUT_BUCKET")) && !outB.isEmpty())
        fPostprocessVarEdits[QStringLiteral("OUTPUT_BUCKET")]->setText(outB);

    QString jdSim;
    if (fBatchUseExistingJobDefs)
        jdSim = fEditExistingJobDefSim ? fEditExistingJobDefSim->text().trimmed() : QString();
    else
        jdSim = TopasAwsJobDefinitionNameFromJson(fJobDefJsonText);
    if (!jdSim.isEmpty()) {
        if (fSubmitVarEdits.contains(QStringLiteral("JOB_DEFINITION")))
            fSubmitVarEdits[QStringLiteral("JOB_DEFINITION")]->setText(jdSim);
        if (fSubmitVarEdits.contains(QStringLiteral("JOB_DEFINITION_NAME")))
            fSubmitVarEdits[QStringLiteral("JOB_DEFINITION_NAME")]->setText(jdSim);
    }

    QString jdPost = fEditPostprocessJobDefinitionName ? fEditPostprocessJobDefinitionName->text().trimmed() : QString();
    if (jdPost.isEmpty() && fWantsPostprocessing)
        jdPost = TopasAwsJobDefinitionNameFromJson(fJobDefPostJsonText);
    if (!jdPost.isEmpty() && fSubmitVarEdits.contains(QStringLiteral("POSTPROCESS_JOB_DEFINITION")))
        fSubmitVarEdits[QStringLiteral("POSTPROCESS_JOB_DEFINITION")]->setText(jdPost);
    if (!jdPost.isEmpty() && fPostprocessVarEdits.contains(QStringLiteral("POSTPROCESS_JOB_DEFINITION")))
        fPostprocessVarEdits[QStringLiteral("POSTPROCESS_JOB_DEFINITION")]->setText(jdPost);
    if (!jdPost.isEmpty() && fPostprocessVarEdits.contains(QStringLiteral("JOB_DEFINITION")))
        fPostprocessVarEdits[QStringLiteral("JOB_DEFINITION")]->setText(jdPost);

    auto setSubmitFieldText = [this](const QString& key, const QString& value) {
        QLineEdit* edit = fSubmitVarEdits.value(key, nullptr);
        if (edit)
            edit->setText(value);
    };
    if (fSubmitUseOpenedParameterFile) {
        const QFileInfo fiTop(QString::fromUtf8(fPm->GetTopParameterFileSpec().c_str()));
        const QString localSimDir = fiTop.absolutePath().trimmed();
        const QString fileToRun = fiTop.fileName().trimmed();
        setSubmitFieldText(QStringLiteral("PROJECT_NAME"), QString());
        setSubmitFieldText(QStringLiteral("NUM_JOBS"), QString());
        setSubmitFieldText(QStringLiteral("JOB_NAME"), QString());
        if (!localSimDir.isEmpty())
            setSubmitFieldText(QStringLiteral("LOCAL_SIM_DIR"), localSimDir);
        if (!fileToRun.isEmpty())
            setSubmitFieldText(QStringLiteral("FILE_TO_RUN"), fileToRun);
    } else {
        const QStringList restoreKeys = {
            QStringLiteral("PROJECT_NAME"),
            QStringLiteral("NUM_JOBS"),
            QStringLiteral("JOB_NAME"),
            QStringLiteral("LOCAL_SIM_DIR"),
            QStringLiteral("FILE_TO_RUN")
        };
        for (const QString& key : restoreKeys) {
            QLineEdit* edit = fSubmitVarEdits.value(key, nullptr);
            if (!edit)
                continue;
            if (edit->text().trimmed().isEmpty() && fSubmitVarDefaults.contains(key))
                edit->setText(fSubmitVarDefaults.value(key));
        }
    }

    const QString localScript = fEditPpLocalScript ? fEditPpLocalScript->text().trimmed() : QString();
    if (!localScript.isEmpty() && fPostprocessVarEdits.contains(QStringLiteral("LOCAL_SCRIPT")))
        fPostprocessVarEdits[QStringLiteral("LOCAL_SCRIPT")]->setText(localScript);
}

QString TsQtAWS::PatchJobDefJsonBucketsAndLog(const QString& jsonText, const QString& inputBucket, const QString& outputBucket,
                                             const QString& logGroup, const QString& region) const
{
    QJsonParseError pe;
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toUtf8(), &pe);
    if (pe.error != QJsonParseError::NoError || !doc.isObject())
        return jsonText;
    QJsonObject root = doc.object();
    const QString jn = root.value(QStringLiteral("jobDefinitionName")).toString();
    const bool isPost = jn.contains(QStringLiteral("postprocess"), Qt::CaseInsensitive)
        || jn.contains(QStringLiteral("postP"), Qt::CaseInsensitive);
    if (isPost)
        TopasAwsSetContainerEnvValue(root, QStringLiteral("OUTPUT_BUCKET"), outputBucket);
    else {
        TopasAwsSetContainerEnvValue(root, QStringLiteral("INPUT_BUCKET"), inputBucket);
        TopasAwsSetContainerEnvValue(root, QStringLiteral("OUTPUT_BUCKET"), outputBucket);
    }
    TopasAwsSetAwsLogsOptions(root, logGroup, region);
    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

static bool TopasAwsCliOkOrAlreadyExists(int code, const QString& err, const QString& out)
{
    if (code == 0)
        return true;
    const QString all = err + "\n" + out;
    return all.contains(QStringLiteral("BucketAlreadyOwnedByYou"), Qt::CaseInsensitive)
        || all.contains(QStringLiteral("bucketAlreadyExists"), Qt::CaseInsensitive)
        || all.contains(QStringLiteral("ResourceAlreadyExistsException"), Qt::CaseInsensitive)
        || all.contains(QStringLiteral("AlreadyExists"), Qt::CaseInsensitive);
}

bool TsQtAWS::RunProvisionS3AndLogs(QString* errOut)
{
    const QString inB = fEditProvisionInputBucket ? fEditProvisionInputBucket->text().trimmed() : QString();
    const QString outB = fEditProvisionOutputBucket ? fEditProvisionOutputBucket->text().trimmed() : QString();
    const QString lg = fEditProvisionLogGroup ? fEditProvisionLogGroup->text().trimmed() : QString();
    if (inB.isEmpty() || outB.isEmpty() || lg.isEmpty()) {
        if (errOut)
            *errOut = "Bucket and log group names are required.";
        return false;
    }
    QString out;
    QString err;
    int code = 0;
    const bool useExistingBuckets = fRadioCreateUseExistingBucketsLogs && fRadioCreateUseExistingBucketsLogs->isChecked();
    if (!useExistingBuckets) {
        QStringList mbIn;
        mbIn << QStringLiteral("s3") << QStringLiteral("mb") << (QStringLiteral("s3://") + inB);
        if (!RunAws(mbIn, &out, &err, &code)) {
            if (errOut)
                *errOut = QStringLiteral("aws s3 mb failed to run: ") + err;
            return false;
        }
        if (!TopasAwsCliOkOrAlreadyExists(code, err, out)) {
            if (errOut)
                *errOut = err + "\n" + out;
            return false;
        }
        QStringList mbOut;
        mbOut << QStringLiteral("s3") << QStringLiteral("mb") << (QStringLiteral("s3://") + outB);
        code = 0;
        if (!RunAws(mbOut, &out, &err, &code)) {
            if (errOut)
                *errOut = QStringLiteral("aws s3 mb failed to run: ") + err;
            return false;
        }
        if (!TopasAwsCliOkOrAlreadyExists(code, err, out)) {
            if (errOut)
                *errOut = err + "\n" + out;
            return false;
        }
    }
    const bool useExistingLogGroup = fRadioLogGroupExisting && fRadioLogGroupExisting->isChecked();
    if (!useExistingLogGroup) {
        QStringList lgArgs;
        lgArgs << QStringLiteral("logs") << QStringLiteral("create-log-group") << QStringLiteral("--log-group-name") << lg;
        code = 0;
        if (!RunAws(lgArgs, &out, &err, &code)) {
            if (errOut)
                *errOut = QStringLiteral("aws logs create-log-group failed to run: ") + err;
            return false;
        }
        if (!TopasAwsCliOkOrAlreadyExists(code, err, out)) {
            if (errOut)
                *errOut = err + "\n" + out;
            return false;
        }
    }
    return true;
}

void TsQtAWS::SetWizardEditorModesFromStep2()
{
    fWizardAdvancedMode = fRadioWizardAdvanced && fRadioWizardAdvanced->isChecked();
    fComputeTextMode = fWizardAdvancedMode;
    if (fComputeModeStack)
        fComputeModeStack->setCurrentIndex(fWizardAdvancedMode ? 1 : 0);
    fJobDefTextMode = fWizardAdvancedMode;
    if (fJobDefModeStack)
        fJobDefModeStack->setCurrentIndex(fWizardAdvancedMode ? 1 : 0);
}

int TsQtAWS::ComputeStepStackIndex() const
{
    return fComputeStepStack ? fComputeStepStack->currentIndex() : 0;
}

int TsQtAWS::JobBatchStepStackIndex() const
{
    return fJobBatchStepStack ? fJobBatchStepStack->currentIndex() : 0;
}

bool TsQtAWS::ApplySubmitScriptSnapshot(bool showSuccessMessage)
{
    if (fTopasSubmitPath.isEmpty() || !QFileInfo::exists(fTopasSubmitPath)) {
        ShowAwsMessage(fWizardParent, "Submit to cloud", false, "Could not find topas_submit.sh in the AWS directory.");
        return false;
    }
    QMap<QString, QString> submitEnv = BuildSubmitOverridesFromUi();

    QFile submitFile(fTopasSubmitPath);
    if (!submitFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ShowAwsMessage(fWizardParent, "Submit to cloud", false, "Could not read topas_submit.sh.");
        return false;
    }
    const QString submitText = QString::fromUtf8(submitFile.readAll());
    submitFile.close();

    QString patchedSubmitText;
    if (!BuildPatchedSubmitScript(submitText, submitEnv, &patchedSubmitText)) {
        ShowAwsMessage(fWizardParent, "Submit to cloud", false,
                       "Could not locate the editable variables block in topas_submit.sh.");
        return false;
    }

    const QString folderName = fEditSubmitSnapshotFolder ? fEditSubmitSnapshotFolder->text().trimmed() : QString();
    if (folderName.isEmpty()) {
        ShowAwsMessage(fWizardParent, "Submit to cloud", false, "Please enter a folder name.");
        return false;
    }
    fSnapshotRoot = QDir(fAwsDir).filePath("snapshots/" + folderName);
    QDir().mkpath(fSnapshotRoot);
    const QString scriptName = QFileInfo(fTopasSubmitPath).fileName();
    const QString snapScript = QDir(fSnapshotRoot).filePath(scriptName);
    if (!WriteTextFile(snapScript, patchedSubmitText)) {
        ShowAwsMessage(fWizardParent, "Submit to cloud", false, "Could not write local submit script.");
        return false;
    }
    QFile::setPermissions(snapScript, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner | QFile::ReadGroup | QFile::ExeGroup |
                                          QFile::ReadOther | QFile::ExeOther);

    fSubmitScriptSnapshotPath = snapScript;
    fSubmitApplied = true;
    fSubmitNeedsReset = false;
    Q_UNUSED(showSuccessMessage);
    return true;
}

QMap<QString, QString> TsQtAWS::BuildSubmitOverridesFromUi() const
{
    QMap<QString, QString> submitEnv;
    for (const QString& key : fSubmitVarOrder) {
        QLineEdit* edit = fSubmitVarEdits.value(key, nullptr);
        if (edit)
            submitEnv[key] = edit->text();
    }
    const QString queueFromWizard = (fBatchUseExisting && fEditExistingJobQueue)
        ? fEditExistingJobQueue->text().trimmed()
        : (fEditWizardJobQueueName ? fEditWizardJobQueueName->text().trimmed() : QString());
    if (!queueFromWizard.isEmpty())
        submitEnv[QStringLiteral("JOB_QUEUE")] = queueFromWizard;
    if (fEditJobDefinitionName) {
        const QString jd = fEditJobDefinitionName->text().trimmed();
        if (!jd.isEmpty()) {
            submitEnv[QStringLiteral("JOB_DEFINITION")] = jd;
            submitEnv[QStringLiteral("JOB_DEFINITION_NAME")] = jd;
        }
    }
    return submitEnv;
}

QString TsQtAWS::ComputeEnvironmentNameFromCurrentJson() const
{
    if (fComputeUseExisting)
        return fExistingComputeEnvName.trimmed();
    QString text = fComputeJsonText;
    if (text.trimmed().isEmpty() && fComputeTextEditor)
        text = fComputeTextEditor->toPlainText();
    QJsonParseError pe;
    const QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &pe);
    if (pe.error != QJsonParseError::NoError || !doc.isObject())
        return QString();
    return doc.object().value(QStringLiteral("computeEnvironmentName")).toString().trimmed();
}

bool TsQtAWS::CreateJobDefinitionFromSnapshot()
{
    const QString queueResolved = (fBatchUseExisting && fEditExistingJobQueue)
        ? fEditExistingJobQueue->text().trimmed()
        : (fEditWizardJobQueueName ? fEditWizardJobQueueName->text().trimmed() : QString());
    if (fBatchUseExisting) {
        SyncSubmitStepFromBatchState();
        return true;
    }
    CommitJobDefEditorToActiveString();
    if (!ApplyJobDefJsonSnapshot())
        return false;
    const QString snapSim = QDir(fSnapshotRoot).filePath(kFileJobDef);
    const QString queueName = queueResolved;
    if (queueName.isEmpty()) {
        ShowAwsMessage(fWizardParent, "Job queue", false,
                       "Enter a job queue name before creating the job definition.");
        return false;
    }
    const QString ceName = ComputeEnvironmentNameFromCurrentJson();
    if (ceName.isEmpty()) {
        ShowAwsMessage(fWizardParent, "Compute environment", false,
                       "Could not determine compute environment name. Go back to the compute environment step.");
        return false;
    }
    QString jqOut;
    QString jqErr;
    int jqCode = 0;
    QStringList jqArgs;
    jqArgs << QStringLiteral("batch") << QStringLiteral("create-job-queue") << QStringLiteral("--job-queue-name") << queueName
           << QStringLiteral("--state") << QStringLiteral("ENABLED") << QStringLiteral("--priority") << QStringLiteral("1")
           << QStringLiteral("--compute-environment-order")
           << QStringLiteral("order=1,computeEnvironment=%1").arg(ceName);
    if (!RunAws(jqArgs, &jqOut, &jqErr, &jqCode) || jqCode != 0) {
        QString detail = jqErr;
        if (!jqOut.isEmpty())
            detail += "\n" + jqOut;
        ShowAwsMessage(fWizardParent, "AWS create-job-queue", false, detail);
        return false;
    }
    QString err;
    if (!RegisterJobDefinitionFile(snapSim, &err)) {
        ShowAwsMessage(fWizardParent, "AWS register-job-definition", false, err);
        return false;
    }
    if (fWantsPostprocessing && !fBatchUseExistingPostJobDef) {
        const QString snapPost = QDir(fSnapshotRoot).filePath(kFileJobDefPost);
        if (!RegisterJobDefinitionFile(snapPost, &err)) {
            ShowAwsMessage(fWizardParent, "AWS register-job-definition (post)", false, err);
            return false;
        }
    }
    if (fSubmitVarEdits.contains(QStringLiteral("JOB_QUEUE")) && !queueName.isEmpty())
        fSubmitVarEdits[QStringLiteral("JOB_QUEUE")]->setText(queueName);
    SyncSubmitStepFromBatchState();
    ShowAwsMessage(fWizardParent, "AWS Batch", true,
                   QStringLiteral("Job queue \"%1\" was created (linked to compute environment \"%2\"). Job definition(s) were "
                                  "registered.")
                       .arg(queueName, ceName));
    return true;
}

// ---------------------------------------------------------------------------
// Script execution and postprocess script generation
// ---------------------------------------------------------------------------

void TsQtAWS::ParseJobIdsFromSubmitOutput(const QString& out)
{
    fJobIds.clear();
    QSet<QString> seen;
    const QRegularExpression uuidRe(
        "\\b[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\\b");
    const QStringList lines = out.split('\n');
    for (QString line : lines) {
        line = line.trimmed();
        if (line.isEmpty())
            continue;
        QRegularExpressionMatchIterator it = uuidRe.globalMatch(line);
        while (it.hasNext()) {
            const QString id = it.next().captured(0);
            if (!seen.contains(id)) {
                seen.insert(id);
                fJobIds.append(id);
            }
        }
    }
}

bool TsQtAWS::RunSubmitScript(const QString& scriptPath, QString* out, QString* err, int* exitCode,
                              const QMap<QString, QString>& extraEnv)
{
    if (fProfileEdit)
        fProfile = fProfileEdit->text().trimmed();
    if (fEditAwsRegion)
        fRegion = fEditAwsRegion->text().trimmed();
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (!fProfile.isEmpty())
        env.insert("TOPAS_AWS_PROFILE", fProfile);
    if (!fRegion.isEmpty())
        env.insert("TOPAS_AWS_REGION", fRegion);
    for (auto it = extraEnv.constBegin(); it != extraEnv.constEnd(); ++it)
        env.insert(it.key(), it.value());
    QProcess proc;
    proc.setProcessEnvironment(env);
    const QFileInfo scriptInfo(scriptPath);
    proc.setWorkingDirectory(scriptInfo.absolutePath());
    proc.start("/bin/bash", QStringList() << scriptInfo.fileName());
    if (!proc.waitForFinished(1200000)) {
        proc.kill();
        if (err)
            *err = "bash timed out";
        if (exitCode)
            *exitCode = -1;
        return false;
    }
    if (out)
        *out = QString::fromUtf8(proc.readAllStandardOutput());
    if (err)
        *err = QString::fromUtf8(proc.readAllStandardError());
    if (exitCode)
        *exitCode = proc.exitCode();
    return proc.exitStatus() == QProcess::NormalExit;
}

void TsQtAWS::BuildPostprocessScriptToFile(const QString& path)
{
    auto postValue = [this](const QString& key, QLineEdit* fallback) {
        QLineEdit* edit = fPostprocessVarEdits.value(key, nullptr);
        if (edit)
            return edit->text();
        return EditTextOrEmpty(fallback);
    };
    QString ppDef = postValue(QStringLiteral("POSTPROCESS_JOB_DEFINITION"), fEditPostprocessJobDefinitionName);
    if (ppDef.trimmed().isEmpty())
        ppDef = postValue(QStringLiteral("JOB_DEFINITION"), fEditPostprocessJobDefinitionName);
    QString script;
    script += "#!/bin/bash\n";
    script += "set -e\n";
    script += "AWSCLI=(aws)\n";
    script += "if [ -n \"$TOPAS_AWS_PROFILE\" ]; then\n";
    script += "  AWSCLI+=(--profile \"$TOPAS_AWS_PROFILE\")\n";
    script += "fi\n";
    script += "if [ -n \"$TOPAS_AWS_REGION\" ]; then\n";
    script += "  AWSCLI+=(--region \"$TOPAS_AWS_REGION\")\n";
    script += "fi\n";
    script += "PROJECT_NAME=\"" + postValue(QStringLiteral("PROJECT_NAME"), fEditProjectName).replace("\"", "\\\"") + "\"\n";
    script += "RUN_DATE=\"" + postValue(QStringLiteral("RUN_DATE"), fEditRunDate).replace("\"", "\\\"") + "\"\n";
    script += "OUTPUT_BUCKET=\"" + postValue(QStringLiteral("OUTPUT_BUCKET"), fEditOutputBucket).replace("\"", "\\\"") + "\"\n";
    script += "LOCAL_SCRIPT=\"" + postValue(QStringLiteral("LOCAL_SCRIPT"), fEditPpLocalScript).replace("\"", "\\\"") + "\"\n";
    script += "EXTRA_PIP_PACKAGES=\"" + postValue(QStringLiteral("EXTRA_PIP_PACKAGES"), fEditPpExtraPip).replace("\"", "\\\"") + "\"\n";
    script += "SIM_DIR=\"projects/${PROJECT_NAME}/${RUN_DATE}\"\n";
    script += "SCRIPT_BASENAME=\"$(basename \"$LOCAL_SCRIPT\")\"\n";
    script += "SCRIPT_S3_URI=\"s3://${OUTPUT_BUCKET}/${SIM_DIR}/${SCRIPT_BASENAME}\"\n";
    script += "echo \"Uploading script to ${SCRIPT_S3_URI}\"\n";
    script += "\"${AWSCLI[@]}\" s3 cp \"${LOCAL_SCRIPT}\" \"${SCRIPT_S3_URI}\" --only-show-errors\n";
    script += "JOB_ID=$(\"${AWSCLI[@]}\" batch submit-job \\\n";
    script += "  --job-name \"postprocess-${PROJECT_NAME}-${RUN_DATE}\" \\\n";
    script += "  --job-queue \"" + postValue(QStringLiteral("JOB_QUEUE"), fEditJobQueue).replace("\"", "\\\"") + "\" \\\n";
    script += "  --job-definition \"" + ppDef.replace("\"", "\\\"") + "\" \\\n";
    script += "  --container-overrides \"{\n";
    script += "    \\\"environment\\\": [\n";
    script += "      {\\\"name\\\": \\\"OUTPUT_BUCKET\\\",   \\\"value\\\": \\\"${OUTPUT_BUCKET}\\\"},\n";
    script += "      {\\\"name\\\": \\\"SIM_DIR\\\",         \\\"value\\\": \\\"${SIM_DIR}\\\"},\n";
    script += "      {\\\"name\\\": \\\"SCRIPT_S3_URI\\\",   \\\"value\\\": \\\"${SCRIPT_S3_URI}\\\"},\n";
    script += "      {\\\"name\\\": \\\"SCRIPT_FILENAME\\\", \\\"value\\\": \\\"${SCRIPT_BASENAME}\\\"},\n";
    script += "      {\\\"name\\\": \\\"EXTRA_PIP_PACKAGES\\\", \\\"value\\\": \\\"${EXTRA_PIP_PACKAGES}\\\"}\n";
    script += "    ]\n";
    script += "  }\" \\\n";
    script += "  --query jobId --output text)\n";
    script += "echo \"$JOB_ID\"\n";
    WriteTextFile(path, script);
    QFile::setPermissions(path, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner | QFile::ReadGroup | QFile::ExeGroup |
                                      QFile::ReadOther | QFile::ExeOther);
}

// ---------------------------------------------------------------------------
// Polling Batch job status (main simulation jobs)
// ---------------------------------------------------------------------------

void TsQtAWS::RefreshJobStatus()
{
    auto setCountLabel = [this](const QString& key, int count, const QString& stateWord, bool failedStyle = false) {
        QLabel* lbl = fMonitorStateLabels.value(key, nullptr);
        if (!lbl)
            return;
        if (failedStyle)
            lbl->setText(QString("%1 jobs failed").arg(count));
        else
            lbl->setText(QString("%1 jobs %2").arg(count).arg(stateWord));
    };
    auto setDotColor = [this](const QString& key, const QString& color) {
        QFrame* dot = fMonitorStateDots.value(key, nullptr);
        if (dot)
            dot->setStyleSheet(QString("background-color: %1; border-radius: 6px;").arg(color));
    };
    auto setAllGray = [&]() {
        const QStringList keys = {"SUBMITTED", "PENDING", "RUNNABLE", "STARTING", "RUNNING", "SUCCEEDED", "FAILED"};
        for (const QString& k : keys)
            setDotColor(k, "rgb(110,110,110)");
    };
    if (fMonitorLastUpdated) {
        const QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        fMonitorLastUpdated->setText("Dashboard last updated: " + ts + ". Auto-refreshes every 60 seconds.");
    }
    if (fJobIds.isEmpty()) {
        setAllGray();
        setDotColor("SUBMITTED", "rgb(52,199,89)");
        setCountLabel("SUBMITTED", 0, "submitted");
        setCountLabel("PENDING", 0, "pending");
        setCountLabel("RUNNABLE", 0, "runnable");
        setCountLabel("STARTING", 0, "starting");
        setCountLabel("RUNNING", 0, "running");
        setCountLabel("SUCCEEDED", 0, "succeeded");
        setCountLabel("FAILED", 0, "", true);
        if (fMonitorText)
            fMonitorText->setPlainText("No job IDs recorded.");
        return;
    }
    QStringList args;
    args << "batch"
         << "describe-jobs"
         << "--jobs";
    args.append(fJobIds);
    QString out;
    QString err;
    int code = 0;
    if (!RunAws(args, &out, &err, &code) || code != 0) {
        setAllGray();
        setDotColor("SUBMITTED", "rgb(52,199,89)");
        setCountLabel("SUBMITTED", fJobIds.size(), "submitted");
        setCountLabel("PENDING", 0, "pending");
        setCountLabel("RUNNABLE", 0, "runnable");
        setCountLabel("STARTING", 0, "starting");
        setCountLabel("RUNNING", 0, "running");
        setCountLabel("SUCCEEDED", 0, "succeeded");
        setCountLabel("FAILED", 0, "", true);
        if (fMonitorText)
            fMonitorText->setPlainText(QString("describe-jobs failed:\n") + err + "\n" + out);
        return;
    }
    if (fMonitorText)
        fMonitorText->setPlainText(out);

    QJsonParseError pe;
    QJsonDocument jd = QJsonDocument::fromJson(out.toUtf8(), &pe);
    if (pe.error != QJsonParseError::NoError || !jd.isObject()) {
        setAllGray();
        setDotColor("SUBMITTED", "rgb(52,199,89)");
        setCountLabel("SUBMITTED", fJobIds.size(), "submitted");
        setCountLabel("PENDING", 0, "pending");
        setCountLabel("RUNNABLE", 0, "runnable");
        setCountLabel("STARTING", 0, "starting");
        setCountLabel("RUNNING", 0, "running");
        setCountLabel("SUCCEEDED", 0, "succeeded");
        setCountLabel("FAILED", 0, "", true);
        return;
    }
    QJsonArray jobs = jd.object().value("jobs").toArray();
    setAllGray();
    setDotColor("SUBMITTED", "rgb(52,199,89)");

    auto rankFor = [](const QString& st) {
        if (st == "PENDING")
            return 1;
        if (st == "RUNNABLE")
            return 2;
        if (st == "STARTING")
            return 3;
        if (st == "RUNNING")
            return 4;
        if (st == "SUCCEEDED")
            return 5;
        return 0;
    };
    int maxRank = 0;
    bool anyFailed = false;
    int submittedCount = jobs.size();
    int pendingCount = 0;
    int runnableCount = 0;
    int startingCount = 0;
    int runningCount = 0;
    int succeededCount = 0;
    int failedCount = 0;
    for (const QJsonValue& jv : jobs) {
        const QString st = jv.toObject().value("status").toString().trimmed().toUpper();
        if (st == "PENDING")
            pendingCount++;
        else if (st == "RUNNABLE")
            runnableCount++;
        else if (st == "STARTING")
            startingCount++;
        else if (st == "RUNNING")
            runningCount++;
        else if (st == "SUCCEEDED")
            succeededCount++;
        else if (st == "FAILED") {
            anyFailed = true;
            failedCount++;
        }
        maxRank = std::max(maxRank, rankFor(st));
    }
    if (maxRank >= 1)
        setDotColor("PENDING", "rgb(52,199,89)");
    if (maxRank >= 2)
        setDotColor("RUNNABLE", "rgb(52,199,89)");
    if (maxRank >= 3)
        setDotColor("STARTING", "rgb(52,199,89)");
    if (maxRank >= 4)
        setDotColor("RUNNING", "rgb(52,199,89)");
    if (maxRank >= 5)
        setDotColor("SUCCEEDED", "rgb(52,199,89)");
    if (anyFailed)
        setDotColor("FAILED", "rgb(255,69,58)");
    setCountLabel("SUBMITTED", submittedCount, "submitted");
    setCountLabel("PENDING", pendingCount, "pending");
    setCountLabel("RUNNABLE", runnableCount, "runnable");
    setCountLabel("STARTING", startingCount, "starting");
    setCountLabel("RUNNING", runningCount, "running");
    setCountLabel("SUCCEEDED", succeededCount, "succeeded");
    setCountLabel("FAILED", failedCount, "", true);
}

void TsQtAWS::OnPollJobs()
{
    if (fCurrentStep != 8) // not monitoring step
        return;
    RefreshJobStatus();
    QString txt = fMonitorText ? fMonitorText->toPlainText() : QString();
    QJsonParseError pe;
    QJsonDocument jd = QJsonDocument::fromJson(txt.toUtf8(), &pe);
    if (pe.error != QJsonParseError::NoError || !jd.isObject()) {
        if (fBtnNext && fCurrentStep == 8)
            fBtnNext->setEnabled(false);
        return;
    }
    QJsonArray jobs = jd.object().value("jobs").toArray();
    bool anyRunning = false;
    for (const QJsonValue& jv : jobs) {
        QJsonObject jo = jv.toObject();
        QString st = jo.value("status").toString();
        if (st == "RUNNING" || st == "PENDING" || st == "RUNNABLE" || st == "STARTING")
            anyRunning = true;
    }
    if (fBtnNext && fCurrentStep == 8)
        fBtnNext->setEnabled(!anyRunning && !jobs.isEmpty());
}

void TsQtAWS::StartMonitoring()
{
    if (!fPollTimer) {
        fPollTimer = new QTimer(this);
        connect(fPollTimer, &QTimer::timeout, this, &TsQtAWS::OnPollJobs);
    }
    if (fMonitorStateDots.contains("SUBMITTED")) {
        for (auto it = fMonitorStateDots.begin(); it != fMonitorStateDots.end(); ++it)
            if (it.value())
                it.value()->setStyleSheet("background-color: rgb(110,110,110); border-radius: 6px;");
        fMonitorStateDots["SUBMITTED"]->setStyleSheet("background-color: rgb(52,199,89); border-radius: 6px;");
    }
    RefreshJobStatus();
    fPollTimer->start(60000);
    OnPollJobs();
}

void TsQtAWS::StopMonitoring()
{
    if (fPollTimer)
        fPollTimer->stop();
}


#endif
