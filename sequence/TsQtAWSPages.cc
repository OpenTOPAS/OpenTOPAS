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

#ifdef G4UI_USE_QT

#include "TsQtAWS.hh"
#include "TsQtAWSUtils.hh"

#include <QCoreApplication>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
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
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QStringList>
#include <QTextEdit>
#include <QTimer>
#include <QPixmap>
#include <QScrollBar>
#include <QVBoxLayout>

using namespace TsQtAWSUtils;

void TsQtAWS::CreateWizardPagesAndNavigation()
{
    QVBoxLayout* mainLay = new QVBoxLayout(fWizard);

    QHBoxLayout* stepHeader = new QHBoxLayout();
    fStepLabel = new QLabel("Welcome");
    stepHeader->addWidget(fStepLabel, 1);
    mainLay->addLayout(stepHeader);

    fStepProgress = new QProgressBar();
    fStepProgress->setRange(0, 9);
    fStepProgress->setValue(0);
    mainLay->addWidget(fStepProgress);

    fStack = new QStackedWidget();

    // Welcome page (before wizard step 1)
    QWidget* pageWelcome = new QWidget();
    pageWelcome->setFocusPolicy(Qt::ClickFocus);
    QVBoxLayout* lWelcome = new QVBoxLayout(pageWelcome);
    lWelcome->setContentsMargins(12, 18, 12, 18);
    lWelcome->setSpacing(48);
    auto loadTopasLogo = []() {
        const QString appDir = QCoreApplication::applicationDirPath();
        QStringList candidates;
        candidates << QDir::homePath() + "/Applications/TOPAS/OpenTOPAS/graphics/TOPASLogo.png"
                   << appDir + "/../../OpenTOPAS/graphics/TOPASLogo.png";
        for (const QString& path : candidates) {
            QPixmap pix(path);
            if (!pix.isNull())
                return pix;
        }
        return QPixmap();
    };
    QHBoxLayout* welcomeHeader = new QHBoxLayout();
    welcomeHeader->setSpacing(14);
    QLabel* logoLabel = new QLabel();
    QPixmap logo = loadTopasLogo();
    if (!logo.isNull())
        logoLabel->setPixmap(logo.scaledToHeight(84, Qt::SmoothTransformation));
    QLabel* welcomeTitle = new QLabel("<b>Welcome to the OpenTOPAS AWS Cloud Wizard!</b>");
    welcomeTitle->setTextFormat(Qt::RichText);
    QFont welcomeTitleFont = welcomeTitle->font();
    welcomeTitleFont.setPointSize(welcomeTitleFont.pointSize() + 4);
    welcomeTitle->setFont(welcomeTitleFont);
    welcomeHeader->addWidget(logoLabel, 0, Qt::AlignVCenter);
    welcomeHeader->addWidget(welcomeTitle, 1, Qt::AlignVCenter);
    lWelcome->addLayout(welcomeHeader);
    QLabel* welcomeText1 = new QLabel(
        "This wizard will guide you through the process of setting up and running your simulations using AWS computing resources.");
    welcomeText1->setWordWrap(true);
    welcomeText1->setAlignment(Qt::AlignJustify);
    lWelcome->addWidget(welcomeText1);
    lWelcome->addSpacing(16);
    QLabel* welcomeText2 = new QLabel(
        "Before beginning these steps, please ensure that you have fully read the cloud section of the "
        "<a href=\"https://topas-nbio.readthedocs.io/en/latest/\">documentation</a>. You will need to have a valid AWS account and have "
        "completed up to and including <a href=\"https://topas-nbio.readthedocs.io/en/latest/\">step 4</a> of the setup instructions.");
    welcomeText2->setTextFormat(Qt::RichText);
    welcomeText2->setOpenExternalLinks(true);
    welcomeText2->setWordWrap(true);
    welcomeText2->setAlignment(Qt::AlignJustify);
    lWelcome->addWidget(welcomeText2);
    QLabel* supportText = new QLabel(
        "If you encounter any problems using this wizard, please check the "
        "<a href=\"https://github.com/OpenTOPAS/OpenTOPAS/discussions\">user forum</a> for relevant posts, or create your own post. "
        "Please report any bugs by creating a GitHub "
        "<a href=\"https://github.com/OpenTOPAS/OpenTOPAS/issues\">Issue</a>.");
    supportText->setTextFormat(Qt::RichText);
    supportText->setOpenExternalLinks(true);
    supportText->setWordWrap(true);
    supportText->setAlignment(Qt::AlignJustify);
    lWelcome->addWidget(supportText);
    QLabel* liabilityText = new QLabel(
        "The TOPAS Collaboration is not responsible for any charges incurred while using AWS compute resources.");
    liabilityText->setWordWrap(true);
    lWelcome->addWidget(liabilityText);
    lWelcome->addSpacing(16);
    QPushButton* btnBegin = new QPushButton("Begin");
    btnBegin->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    ApplyPrimaryButtonStyle(btnBegin);
    lWelcome->addWidget(btnBegin);
    lWelcome->addStretch(1);
    fStack->addWidget(pageWelcome);

    // Step 2 — default (field editors) vs advanced (JSON text)
    QWidget* pageMode = new QWidget();
    pageMode->setFocusPolicy(Qt::ClickFocus);
    QVBoxLayout* lMode = new QVBoxLayout(pageMode);
    lMode->setContentsMargins(12, 18, 12, 18);
    lMode->setSpacing(16);
    AddUnderlinedTitle(lMode, "<b>Choose the type of setup instructions</b>");
    QLabel* modeDesc = new QLabel(
        "If you are a new user we recommend using the default setup steps. These steps will guide you through the "
        "process of setting up your AWS environment based on the templates provided by TOPAS-nBio in the AWS "
        "directory you downloaded.<br><br>"
        "The advanced option allows you to directly edit the raw JSON or bash scripts for more fine-tuned control.");
    modeDesc->setWordWrap(true);
    lMode->addWidget(modeDesc);
    fRadioWizardDefault = new QRadioButton("Default");
    fRadioWizardAdvanced = new QRadioButton("Advanced");
    fRadioWizardDefault->setChecked(true);
    lMode->addWidget(fRadioWizardDefault);
    lMode->addWidget(fRadioWizardAdvanced);
    QObject::connect(fRadioWizardDefault, &QRadioButton::toggled, fWizard, [this](bool) { UpdateNavButtons(); });
    QObject::connect(fRadioWizardAdvanced, &QRadioButton::toggled, fWizard, [this](bool) { UpdateNavButtons(); });
    lMode->addStretch(1);

    // Wizard step 1 of 7 — AWS scripts path, profile, and region (before workflow choice)
    QWidget* page0 = new QWidget();
    page0->setFocusPolicy(Qt::ClickFocus);
    QVBoxLayout* l0 = new QVBoxLayout(page0);
    l0->setContentsMargins(12, 18, 12, 18);
    l0->setSpacing(16);
    {
        AddUnderlinedTitle(l0, "<b>AWS scripts folder</b>");
        QLabel* folderDetail = new QLabel(
            "Provide the directory which contains the AWS Batch job definition and compute environment JSON files, "
            "along with the simulation submission and post-processing Bash scripts. These files can be found "
            "<a href=\"https://github.com/topas-nbio/TOPAS-nBio/tree/main/aws\">here</a> and should be copied to a local directory.<br><br>"
            "This GUI is aimed at new users, who may not be familiar with the range of AWS services. As such, the current implementation "
            "requires these exact files to be present. A more flexible solution may be implemented in the future.");
        folderDetail->setTextFormat(Qt::RichText);
        folderDetail->setOpenExternalLinks(true);
        folderDetail->setWordWrap(true);
        folderDetail->setAlignment(Qt::AlignJustify);
        l0->addWidget(folderDetail);
    }
    l0->addSpacing(24);
    fAwsDirEdit = new QLineEdit(fAwsDir);
    fAwsDirEdit->setFixedHeight(22);
    l0->addWidget(fAwsDirEdit);
    QPushButton* browse = new QPushButton("Browse...");
    browse->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    ApplyPrimaryButtonStyle(browse);
    l0->addWidget(browse);
    l0->addSpacing(36);
    {
        AddUnderlinedTitle(l0, "<b>Environment variables</b>");
    }
    QLabel* envDescription = new QLabel(
        "If you have correctly setup your AWS CLI profile, these environment variable fields should be pre-filled. "
        "If not, please supply them.");
    envDescription->setWordWrap(true);
    l0->addWidget(envDescription);
    l0->addSpacing(8);
    QHBoxLayout* envRow = new QHBoxLayout();
    envRow->setSpacing(16);
    QVBoxLayout* profileCol = new QVBoxLayout();
    {
        QLabel* profileLbl = new QLabel("<b>AWS profile</b>");
        profileLbl->setTextFormat(Qt::RichText);
        profileCol->addWidget(profileLbl);
    }
    fProfileEdit = new QLineEdit(fProfile);
    fProfileEdit->setFixedHeight(22);
    profileCol->addWidget(fProfileEdit);
    QVBoxLayout* regionCol = new QVBoxLayout();
    {
        QLabel* regionLbl = new QLabel("<b>AWS region</b>");
        regionLbl->setTextFormat(Qt::RichText);
        regionCol->addWidget(regionLbl);
    }
    fEditAwsRegion = new QLineEdit(fRegion);
    fEditAwsRegion->setFixedHeight(22);
    regionCol->addWidget(fEditAwsRegion);
    envRow->addLayout(profileCol, 1);
    envRow->addLayout(regionCol, 1);
    l0->addLayout(envRow);
    l0->addStretch(1);
    fStack->addWidget(page0);
    fStack->addWidget(pageMode);

    QObject::connect(browse, &QPushButton::clicked, fWizard, [this]() {
        QString d = QFileDialog::getExistingDirectory(fWizard, "AWS scripts folder", fAwsDirEdit->text());
        if (!d.isEmpty()) {
            fAwsDirEdit->setText(d);
            ReloadAwsFilesFromUi();
        }
    });
    QObject::connect(btnBegin, &QPushButton::clicked, fWizard, [this]() { SetStep(1); });

    // Wizard step 3 — S3 buckets
    QWidget* pageBuckets = new QWidget();
    pageBuckets->setFocusPolicy(Qt::ClickFocus);
    QVBoxLayout* lbk = new QVBoxLayout(pageBuckets);
    lbk->setContentsMargins(12, 18, 12, 18);
    lbk->setSpacing(12);
    AddUnderlinedTitle(lbk, "<b>S3 buckets</b>");
    QLabel* bucketsDesc = new QLabel(
        "See the <a href=\"https://topas-nbio.readthedocs.io/en/latest/\">relevant documentation</a> for more information about "
        "AWS S3 buckets. "
        "If you already have S3 buckets for TOPAS-nBio enter their names below. "
        "Otherwise, provide names for the new buckets. These buckets will be created by this setup wizard and used in all future steps.");
    bucketsDesc->setTextFormat(Qt::RichText);
    bucketsDesc->setOpenExternalLinks(true);
    bucketsDesc->setWordWrap(true);
    lbk->addWidget(bucketsDesc);
    fRadioCreateUseExistingBucketsLogs = new QRadioButton("Use existing buckets");
    fRadioCreateNewBucketsLogs = new QRadioButton("Create new buckets");
    fRadioCreateUseExistingBucketsLogs->setChecked(true);
    lbk->addWidget(fRadioCreateUseExistingBucketsLogs);
    lbk->addWidget(fRadioCreateNewBucketsLogs);
    fEditProvisionInputBucket = new QLineEdit(QStringLiteral("topas-nbio-input"));
    fEditProvisionInputBucket->setFixedHeight(22);
    fEditProvisionInputBucket->setPlaceholderText(QStringLiteral("Input bucket name"));
    lbk->addWidget(fEditProvisionInputBucket);
    fEditProvisionOutputBucket = new QLineEdit(QStringLiteral("topas-nbio-output"));
    fEditProvisionOutputBucket->setFixedHeight(22);
    fEditProvisionOutputBucket->setPlaceholderText(QStringLiteral("Output bucket name"));
    lbk->addWidget(fEditProvisionOutputBucket);
    QObject::connect(fEditProvisionInputBucket, &QLineEdit::textChanged, fWizard, [this](const QString&) { UpdateNavButtons(); });
    QObject::connect(fEditProvisionOutputBucket, &QLineEdit::textChanged, fWizard, [this](const QString&) { UpdateNavButtons(); });
    QObject::connect(fRadioCreateNewBucketsLogs, &QRadioButton::toggled, fWizard, [this](bool) { UpdateNavButtons(); });
    QObject::connect(fRadioCreateUseExistingBucketsLogs, &QRadioButton::toggled, fWizard, [this](bool) { UpdateNavButtons(); });
    lbk->addStretch(1);
    fStack->addWidget(pageBuckets);

    // Wizard step 4 — compute environment (stack index 4): nested choice + editor
    QWidget* page1 = new QWidget();
    page1->setFocusPolicy(Qt::ClickFocus);
    QVBoxLayout* l1Outer = new QVBoxLayout(page1);
    l1Outer->setContentsMargins(0, 0, 0, 0);
    fComputeStepStack = new QStackedWidget();
    QWidget* computeChoicePage = new QWidget();
    QVBoxLayout* lc = new QVBoxLayout(computeChoicePage);
    lc->setContentsMargins(12, 18, 12, 18);
    lc->setSpacing(14);
    AddUnderlinedTitle(lc, "<b>Setting up the compute environment</b>");
    QLabel* computeChoiceDesc = new QLabel(
        "See the <a href=\"https://topas-nbio.readthedocs.io/en/latest/\">relevant documentation</a> for more information about "
        "AWS Batch compute environments. "
        "If you have never launched simulations using AWS you should create a new compute "
        "environment from the templates provided. Alternatively, if you have already launched simulations using AWS "
        "you can use an existing compute environment by providing its name below.");
    computeChoiceDesc->setTextFormat(Qt::RichText);
    computeChoiceDesc->setOpenExternalLinks(true);
    computeChoiceDesc->setWordWrap(true);
    lc->addWidget(computeChoiceDesc);
    fRadioComputeExisting = new QRadioButton("I already have a compute environment");
    fRadioComputeCreate = new QRadioButton("Create a new compute environment from the template");
    fRadioComputeExisting->setChecked(true);
    lc->addWidget(fRadioComputeExisting);
    fEditExistingComputeEnvName = new QLineEdit();
    fEditExistingComputeEnvName->setFixedHeight(22);
    fEditExistingComputeEnvName->setPlaceholderText("e.g. topas-nbio-compute-env");
    lc->addWidget(fEditExistingComputeEnvName);
    lc->addWidget(fRadioComputeCreate);
    auto syncComputeNameEnabled = [this]() {
        const bool ex = fRadioComputeExisting && fRadioComputeExisting->isChecked();
        if (fEditExistingComputeEnvName)
            fEditExistingComputeEnvName->setEnabled(ex);
        if (fBtnNext)
            UpdateNavButtons();
    };
    QObject::connect(fRadioComputeExisting, &QRadioButton::toggled, fWizard, [syncComputeNameEnabled](bool) { syncComputeNameEnabled(); });
    QObject::connect(fRadioComputeCreate, &QRadioButton::toggled, fWizard, [syncComputeNameEnabled](bool) { syncComputeNameEnabled(); });
    QObject::connect(fEditExistingComputeEnvName, &QLineEdit::textChanged, fWizard, [this](const QString&) { UpdateNavButtons(); });
    lc->addStretch(1);
    fComputeStepStack->addWidget(computeChoicePage);

    QWidget* computeEditorPage = new QWidget();
    QVBoxLayout* le = new QVBoxLayout(computeEditorPage);
    le->setContentsMargins(0, 0, 0, 0);
    fStep2Scroll = new QScrollArea();
    fStep2Scroll->setWidgetResizable(true);
    fStep2Scroll->setFrameShape(QFrame::NoFrame);
    QWidget* step2Content = new QWidget();
    QVBoxLayout* l1 = new QVBoxLayout(step2Content);
    l1->setSpacing(18);
    AddUnderlinedTitle(l1, "<b>Setting up the compute environment</b>");
    QLabel* computeDesc = new QLabel(
        "Define the compute environment which determines the kind of resources AWS Batch uses. "
        "Modify the template below, then continue to create the environment. "
        "While some information is provided in the tooltip of each field, full information can be found in the "
        "<a href=\"https://docs.aws.amazon.com/AWSCloudFormation/latest/TemplateReference/aws-resource-batch-computeenvironment.html\">AWS documentation</a>.");
    computeDesc->setTextFormat(Qt::RichText);
    computeDesc->setOpenExternalLinks(true);
    computeDesc->setWordWrap(true);
    l1->addWidget(computeDesc);
    QLabel* computeDesc2 = new QLabel(
        "If the pre-filled values are already correct, save the changes locally and proceed to create the compute environment. If they need to be modified "
        "please refer to <a href=\"https://topas-nbio.readthedocs.io/en/latest/\"> section 6 </a> of the cloud documentation, which details "
        "where exactly in the AWS console the values for the different fields can be found.");
    computeDesc2->setTextFormat(Qt::RichText);
    computeDesc2->setOpenExternalLinks(true);
    computeDesc2->setWordWrap(true);
    l1->addWidget(computeDesc2);
    l1->addSpacing(14);
    fComputeModeStack = new QStackedWidget();
    fComputeFormPage = new QWidget();
    QVBoxLayout* l1Form = new QVBoxLayout(fComputeFormPage);
    l1Form->setContentsMargins(0, 0, 0, 0);
    l1Form->setSpacing(15);
    fComputeModeStack->addWidget(fComputeFormPage);
    QWidget* computeTextPage = new QWidget();
    QVBoxLayout* l1Text = new QVBoxLayout(computeTextPage);
    l1Text->setContentsMargins(0, 0, 0, 0);
    fComputeTextEditor = new QPlainTextEdit();
    fComputeTextEditor->setFont(QFont("monospace"));
    l1Text->addWidget(fComputeTextEditor);
    QObject::connect(fComputeTextEditor, &QPlainTextEdit::textChanged, fWizard, [this]() {
        if (fComputeTextMode)
            MarkComputeNeedsApply();
    });
    fComputeEdit = fComputeTextEditor; // keep existing loading/apply wiring
    fComputeModeStack->addWidget(computeTextPage);
    l1->addWidget(fComputeModeStack);
    AddUnderlinedTitle(l1, "<b>Save changes locally</b>");
    fComputeApplyDesc = new QLabel();
    fComputeApplyDesc->setWordWrap(true);
    l1->addWidget(fComputeApplyDesc);
    l1->addWidget(new QLabel("Choose a folder name"));
    fEditComputeSnapshotFolder = new QLineEdit();
    fEditComputeSnapshotFolder->setFixedHeight(22);
    l1->addWidget(fEditComputeSnapshotFolder);
    QObject::connect(fEditComputeSnapshotFolder, &QLineEdit::textChanged, fWizard, [this](const QString&) { UpdateNavButtons(); });
    l1->addStretch(1);
    fStep2Scroll->setWidget(step2Content);
    le->addWidget(fStep2Scroll);
    fComputeStepStack->addWidget(computeEditorPage);
    fComputeStepStack->setCurrentIndex(0);
    l1Outer->addWidget(fComputeStepStack);
    fComputeModeStack->setCurrentIndex(0);
    fComputeTextMode = false;
    fStack->addWidget(page1);

    // Wizard step 5 — job queue, CloudWatch, and job definitions (stack index 5): nested flow
    QWidget* page2 = new QWidget();
    page2->setFocusPolicy(Qt::ClickFocus);
    QVBoxLayout* l2OuterRoot = new QVBoxLayout(page2);
    l2OuterRoot->setContentsMargins(0, 0, 0, 0);
    fJobBatchStepStack = new QStackedWidget();

    // --- 4a: job queue ---
    QWidget* queueChoicePage = new QWidget();
    QVBoxLayout* lbc = new QVBoxLayout(queueChoicePage);
    lbc->setContentsMargins(12, 18, 12, 18);
    lbc->setSpacing(12);
    AddUnderlinedTitle(lbc, "<b>Job queue</b>");
    QLabel* batchChoiceDesc = new QLabel(
        "See the <a href=\"https://topas-nbio.readthedocs.io/en/latest/\">relevant documentation</a> for more information about "
        "AWS Batch job queues. "
        "If you already have an AWS Batch job queue, choose the first option and enter its name below. "
        "Otherwise choose create a new job queue (it can be created when you register job definitions).");
    batchChoiceDesc->setTextFormat(Qt::RichText);
    batchChoiceDesc->setOpenExternalLinks(true);
    batchChoiceDesc->setWordWrap(true);
    lbc->addWidget(batchChoiceDesc);
    fRadioBatchExisting = new QRadioButton("I already have a job queue");
    fRadioBatchCreate = new QRadioButton("Create a new job queue");
    fRadioBatchExisting->setChecked(true);
    lbc->addWidget(fRadioBatchExisting);
    lbc->addWidget(fRadioBatchCreate);
    QStackedWidget* queueNameStack = new QStackedWidget();
    QWidget* existingQueueBlock = new QWidget();
    QVBoxLayout* existingQueueLayout = new QVBoxLayout(existingQueueBlock);
    existingQueueLayout->setContentsMargins(0, 0, 0, 0);
    existingQueueLayout->setSpacing(0);
    fEditExistingJobQueue = new QLineEdit();
    fEditExistingJobQueue->setFixedHeight(22);
    fEditExistingJobQueue->setPlaceholderText("e.g. topas-nbio-queue");
    existingQueueLayout->addWidget(fEditExistingJobQueue);
    queueNameStack->addWidget(existingQueueBlock);
    QWidget* newQueueBlock = new QWidget();
    QVBoxLayout* newQueueLayout = new QVBoxLayout(newQueueBlock);
    newQueueLayout->setContentsMargins(0, 0, 0, 0);
    newQueueLayout->setSpacing(0);
    fEditWizardJobQueueName = new QLineEdit();
    fEditWizardJobQueueName->setFixedHeight(22);
    fEditWizardJobQueueName->setPlaceholderText("e.g. topas-nbio-queue");
    newQueueLayout->addWidget(fEditWizardJobQueueName);
    queueNameStack->addWidget(newQueueBlock);
    lbc->addWidget(queueNameStack);
    auto syncQueueChoiceFields = [this, queueNameStack]() {
        const bool ex = fRadioBatchExisting && fRadioBatchExisting->isChecked();
        if (queueNameStack)
            queueNameStack->setCurrentIndex(ex ? 0 : 1);
        if (fBtnNext)
            UpdateNavButtons();
    };
    QObject::connect(fRadioBatchExisting, &QRadioButton::toggled, fWizard, [syncQueueChoiceFields](bool) { syncQueueChoiceFields(); });
    QObject::connect(fRadioBatchCreate, &QRadioButton::toggled, fWizard, [syncQueueChoiceFields](bool) { syncQueueChoiceFields(); });
    QObject::connect(fEditExistingJobQueue, &QLineEdit::textChanged, fWizard, [this](const QString&) { UpdateNavButtons(); });
    QObject::connect(fEditWizardJobQueueName, &QLineEdit::textChanged, fWizard, [this](const QString&) { UpdateNavButtons(); });
    syncQueueChoiceFields();
    lbc->addStretch(1);
    fJobBatchStepStack->addWidget(queueChoicePage);

    // --- 5b: CloudWatch log group ---
    QWidget* batchLogsPage = new QWidget();
    QVBoxLayout* lblg = new QVBoxLayout(batchLogsPage);
    lblg->setContentsMargins(12, 18, 12, 18);
    lblg->setSpacing(12);
    AddUnderlinedTitle(lblg, "<b>CloudWatch log group</b>");
    QLabel* logsDesc = new QLabel(
        "See the <a href=\"https://topas-nbio.readthedocs.io/en/latest/\">relevant documentation</a> for more information about "
        "Amazon CloudWatch Logs for AWS Batch. "
        "If you already have a CloudWatch log group for Batch jobs, choose use existing and enter its name below. "
        "Otherwise choose create new log group (the wizard can create it when provisioning).");
    logsDesc->setTextFormat(Qt::RichText);
    logsDesc->setOpenExternalLinks(true);
    logsDesc->setWordWrap(true);
    lblg->addWidget(logsDesc);
    fRadioLogGroupExisting = new QRadioButton("Use an existing CloudWatch log group");
    fRadioLogGroupCreate = new QRadioButton("Create a new CloudWatch log group");
    fRadioLogGroupExisting->setChecked(true);
    lblg->addWidget(fRadioLogGroupExisting);
    lblg->addWidget(fRadioLogGroupCreate);
    fEditProvisionLogGroup = new QLineEdit(QStringLiteral("/aws/batch/topas-nbio"));
    fEditProvisionLogGroup->setFixedHeight(22);
    fEditProvisionLogGroup->setPlaceholderText(QStringLiteral("Log group name"));
    lblg->addWidget(fEditProvisionLogGroup);
    QObject::connect(fEditProvisionLogGroup, &QLineEdit::textChanged, fWizard, [this](const QString&) { UpdateNavButtons(); });
    QObject::connect(fRadioLogGroupCreate, &QRadioButton::toggled, fWizard, [this](bool) { UpdateNavButtons(); });
    QObject::connect(fRadioLogGroupExisting, &QRadioButton::toggled, fWizard, [this](bool) { UpdateNavButtons(); });
    lblg->addStretch(1);
    fJobBatchStepStack->addWidget(batchLogsPage);

    // --- 5c: simulation job definition source (existing vs create) ---
    QWidget* simJobDefChoicePage = new QWidget();
    QVBoxLayout* lsimChoice = new QVBoxLayout(simJobDefChoicePage);
    lsimChoice->setContentsMargins(12, 18, 12, 18);
    lsimChoice->setSpacing(14);
    AddUnderlinedTitle(lsimChoice, "<b>Setting up simulation job definition</b>");
    QLabel* simChoiceDesc = new QLabel(
        "See the <a href=\"https://topas-nbio.readthedocs.io/en/latest/\">relevant documentation</a> for more information about "
        "AWS Batch job definitions for simulation. "
        "If you have never launched simulations using AWS you should create a new simulation "
        "job definition from the templates provided. Alternatively, if you have already launched simulations using AWS "
        "you can use an existing simulation job definition by providing its name below.");
    simChoiceDesc->setTextFormat(Qt::RichText);
    simChoiceDesc->setOpenExternalLinks(true);
    simChoiceDesc->setWordWrap(true);
    lsimChoice->addWidget(simChoiceDesc);
    fRadioSimJobDefExisting = new QRadioButton("I already have a simulation job definition");
    fRadioSimJobDefCreate = new QRadioButton("Create a new simulation job definition from the template");
    fRadioSimJobDefExisting->setChecked(true);
    lsimChoice->addWidget(fRadioSimJobDefExisting);
    fEditExistingJobDefSim = new QLineEdit();
    fEditExistingJobDefSim->setFixedHeight(22);
    fEditExistingJobDefSim->setPlaceholderText("e.g. topas-nbio-job");
    lsimChoice->addWidget(fEditExistingJobDefSim);
    lsimChoice->addWidget(fRadioSimJobDefCreate);
    auto syncSimJobDefChoiceFields = [this]() {
        const bool useExisting = fRadioSimJobDefExisting && fRadioSimJobDefExisting->isChecked();
        fBatchUseExistingJobDefs = useExisting;
        if (fEditExistingJobDefSim)
            fEditExistingJobDefSim->setEnabled(useExisting);
        if (fBtnNext)
            UpdateNavButtons();
    };
    QObject::connect(fRadioSimJobDefExisting, &QRadioButton::toggled, fWizard, [syncSimJobDefChoiceFields](bool) { syncSimJobDefChoiceFields(); });
    QObject::connect(fRadioSimJobDefCreate, &QRadioButton::toggled, fWizard, [syncSimJobDefChoiceFields](bool) { syncSimJobDefChoiceFields(); });
    QObject::connect(fEditExistingJobDefSim, &QLineEdit::textChanged, fWizard, [this](const QString&) { UpdateNavButtons(); });
    syncSimJobDefChoiceFields();
    lsimChoice->addStretch(1);
    fJobBatchStepStack->addWidget(simJobDefChoicePage);

    // --- 5d: simulation job definition ---
    QWidget* simJobDefPage = new QWidget();
    QVBoxLayout* lsim = new QVBoxLayout(simJobDefPage);
    lsim->setContentsMargins(12, 18, 12, 18);
    lsim->setSpacing(12);
    AddUnderlinedTitle(lsim, "<b>Simulation job definition</b>");
    QLabel* simJobDefDesc = new QLabel(
        "Provide the simulation job definition name. The job definition determines how jobs on AWS should be run."
            "Modify the template below, then continue to create the job definition. "
            "While some information is provided in the tooltip of each field, full information can be found in the "
            "<a href=\"https://docs.aws.amazon.com/AWSCloudFormation/latest/TemplateReference/aws-resource-batch-jobdefinition.html\">"
            "AWS documentation</a>.");
    simJobDefDesc->setTextFormat(Qt::RichText);
    simJobDefDesc->setOpenExternalLinks(true);
    simJobDefDesc->setWordWrap(true);
    lsim->addWidget(simJobDefDesc);
    QLabel* simJobDefDesc2 = new QLabel(
        "If the pre-filled values are already correct, save the changes locally and proceed to create the job definition. If they need to be modified "
        "please refer to <a href=\"https://topas-nbio.readthedocs.io/en/latest/\"> section 6 </a> of the cloud documentation, which details "
        "where exactly in the AWS console the values for the different fields can be found.");
    simJobDefDesc2->setTextFormat(Qt::RichText);
    simJobDefDesc2->setOpenExternalLinks(true);
    simJobDefDesc2->setWordWrap(true);
    lsim->addWidget(simJobDefDesc2);
    lsim->addSpacing(14);

    QWidget* simHostWidget = new QWidget();
    fSimJobDefHostLayout = new QVBoxLayout(simHostWidget);
    fSimJobDefHostLayout->setContentsMargins(0, 0, 0, 0);
    lsim->addWidget(simHostWidget, 1);

    // --- 5e: post-processing (yes / no) ---
    QWidget* batchPostPage = new QWidget();
    QVBoxLayout* lbpp = new QVBoxLayout(batchPostPage);
    lbpp->setContentsMargins(12, 18, 12, 18);
    lbpp->setSpacing(12);
    AddUnderlinedTitle(lbpp, "<b>Post-processing</b>");
    QLabel* postChoiceDesc = new QLabel(
        "Do you want to post-process your results using this wizard as well? If so please ensure that you have a post-processing "
        "script file in the same aws directory as all the other scripts. Please refer to <a href=\"https://topas-nbio.readthedocs.io/en/latest/\"> section 7 </a> " 
        "of the cloud documentation for more information.");
    postChoiceDesc->setWordWrap(true);
    lbpp->addWidget(postChoiceDesc);
    fRadioProvisionPostYes = new QRadioButton("Yes. Please select the script file you wish to use for post-processing.");
    fRadioProvisionPostNo = new QRadioButton("No");
    fRadioProvisionPostYes->setChecked(true);
    lbpp->addWidget(fRadioProvisionPostYes);
    lbpp->addWidget(fRadioProvisionPostNo);
    QHBoxLayout* postScriptRow = new QHBoxLayout();
    fEditPostprocessScriptPath = new QLineEdit();
    fEditPostprocessScriptPath->setFixedHeight(22);
    postScriptRow->addWidget(fEditPostprocessScriptPath, 1);
    QPushButton* btnBrowsePostScript = new QPushButton("Browse...");
    btnBrowsePostScript->setObjectName("postScriptBrowseBtn");
    postScriptRow->addWidget(btnBrowsePostScript);
    lbpp->addLayout(postScriptRow);
    QObject::connect(btnBrowsePostScript, &QPushButton::clicked, fWizard, [this]() {
        const QString base = (fAwsDirEdit && !fAwsDirEdit->text().trimmed().isEmpty()) ? fAwsDirEdit->text().trimmed() : fAwsDir;
        const QString script = QFileDialog::getOpenFileName(fWizard, "Select post-processing script file", base);
        if (!script.isEmpty() && fEditPostprocessScriptPath) {
            fEditPostprocessScriptPath->setText(script);
            fPostprocessScriptSelectedByBrowse = true;
            UpdateNavButtons();
        }
    });
    QObject::connect(fEditPostprocessScriptPath, &QLineEdit::textChanged, fWizard, [this](const QString&) {
        fPostprocessScriptSelectedByBrowse = false;
        UpdateNavButtons();
    });
    QObject::connect(fRadioProvisionPostYes, &QRadioButton::toggled, fWizard, [this, btnBrowsePostScript](bool y) {
        if (y) {
            fWantsPostprocessing = true;
            if (fEditPostprocessScriptPath)
                fEditPostprocessScriptPath->setEnabled(true);
            if (btnBrowsePostScript)
                btnBrowsePostScript->setEnabled(true);
            UpdateJobDefTabVisibility();
            UpdateNavButtons();
        }
    });
    QObject::connect(fRadioProvisionPostNo, &QRadioButton::toggled, fWizard, [this, btnBrowsePostScript](bool y) {
        if (y) {
            fWantsPostprocessing = false;
            if (fEditPostprocessScriptPath)
                fEditPostprocessScriptPath->setEnabled(false);
            if (btnBrowsePostScript)
                btnBrowsePostScript->setEnabled(false);
            UpdateJobDefTabVisibility();
            UpdateNavButtons();
        }
    });
    lbpp->addStretch(1);

    // --- 5f: post-processing job definition source (existing vs create) ---
    QWidget* postJobDefChoicePage = new QWidget();
    QVBoxLayout* lpostChoice = new QVBoxLayout(postJobDefChoicePage);
    lpostChoice->setContentsMargins(12, 18, 12, 18);
    lpostChoice->setSpacing(14);
    AddUnderlinedTitle(lpostChoice, "<b>Setting up post-processing job definition</b>");
    QLabel* postChoiceDesc2 = new QLabel(
        "Choose whether to use an existing post-processing job definition or create a new one from the template.");
    postChoiceDesc2->setWordWrap(true);
    lpostChoice->addWidget(postChoiceDesc2);
    fRadioPostJobDefExisting = new QRadioButton("I already have a post-processing job definition");
    fRadioPostJobDefCreate = new QRadioButton("Create a new post-processing job definition from the template");
    fRadioPostJobDefExisting->setChecked(true);
    lpostChoice->addWidget(fRadioPostJobDefExisting);
    fEditExistingJobDefPost = new QLineEdit();
    fEditExistingJobDefPost->setFixedHeight(22);
    fEditExistingJobDefPost->setPlaceholderText("e.g. topas-nbio-postprocess-job");
    lpostChoice->addWidget(fEditExistingJobDefPost);
    lpostChoice->addWidget(fRadioPostJobDefCreate);
    auto syncPostJobDefChoiceFields = [this]() {
        const bool useExisting = fRadioPostJobDefExisting && fRadioPostJobDefExisting->isChecked();
        fBatchUseExistingPostJobDef = useExisting;
        if (fEditExistingJobDefPost)
            fEditExistingJobDefPost->setEnabled(useExisting);
        if (fBtnNext)
            UpdateNavButtons();
    };
    QObject::connect(fRadioPostJobDefExisting, &QRadioButton::toggled, fWizard, [syncPostJobDefChoiceFields](bool) { syncPostJobDefChoiceFields(); });
    QObject::connect(fRadioPostJobDefCreate, &QRadioButton::toggled, fWizard, [syncPostJobDefChoiceFields](bool) { syncPostJobDefChoiceFields(); });
    QObject::connect(fEditExistingJobDefPost, &QLineEdit::textChanged, fWizard, [this](const QString&) { UpdateNavButtons(); });
    syncPostJobDefChoiceFields();
    lpostChoice->addStretch(1);

    // --- 5g: post-processing job definition ---
    QWidget* postJobDefPage = new QWidget();
    QVBoxLayout* lpost = new QVBoxLayout(postJobDefPage);
    lpost->setContentsMargins(12, 18, 12, 18);
    lpost->setSpacing(12);
    AddUnderlinedTitle(lpost, "<b>Post-processing job definition</b>");
    QLabel* postJobDefDesc = new QLabel(
        "Edit the post-processing job definition template. Bucket and log group values from earlier steps are reflected in the JSON.");
    postJobDefDesc->setWordWrap(true);
    lpost->addWidget(postJobDefDesc);
    QWidget* postHostWidget = new QWidget();
    fPostJobDefHostLayout = new QVBoxLayout(postHostWidget);
    fPostJobDefHostLayout->setContentsMargins(0, 0, 0, 0);
    lpost->addWidget(postHostWidget, 1);

    // Shared editor (form / JSON) + snapshot folder — reparented between sim and post pages
    fJobDefEditorHost = new QWidget();
    QVBoxLayout* hlay = new QVBoxLayout(fJobDefEditorHost);
    hlay->setContentsMargins(0, 0, 0, 0);
    hlay->setSpacing(12);
    fStep3Scroll = new QScrollArea();
    fStep3Scroll->setWidgetResizable(true);
    fStep3Scroll->setFrameShape(QFrame::NoFrame);
    QWidget* step3Content = new QWidget();
    QVBoxLayout* l2 = new QVBoxLayout(step3Content);
    l2->setContentsMargins(0, 0, 0, 0);
    l2->setSpacing(12);
    fJobDefModeStack = new QStackedWidget();
    fJobDefFormPage = new QWidget();
    QVBoxLayout* l2Form = new QVBoxLayout(fJobDefFormPage);
    l2Form->setContentsMargins(0, 0, 0, 0);
    l2Form->setSpacing(15);
    fJobDefModeStack->addWidget(fJobDefFormPage);
    QWidget* jobDefTextPage = new QWidget();
    QVBoxLayout* l2Text = new QVBoxLayout(jobDefTextPage);
    l2Text->setContentsMargins(0, 0, 0, 0);
    fJobDefTextEditor = new QPlainTextEdit();
    fJobDefTextEditor->setFont(QFont("monospace"));
    l2Text->addWidget(fJobDefTextEditor);
    QObject::connect(fJobDefTextEditor, &QPlainTextEdit::textChanged, fWizard, [this]() {
        if (fJobDefTextMode)
            MarkJobDefNeedsApply();
    });
    fJobDefEdit = fJobDefTextEditor;
    fJobDefModeStack->addWidget(jobDefTextPage);
    l2->addWidget(fJobDefModeStack);
    fStep3Scroll->setWidget(step3Content);
    hlay->addWidget(fStep3Scroll, 1);
    AddUnderlinedTitle(hlay, "<b>Save changes locally</b>");
    fJobDefApplyDesc = new QLabel();
    fJobDefApplyDesc->setWordWrap(true);
    hlay->addWidget(fJobDefApplyDesc);
    hlay->addWidget(new QLabel("Choose a folder name"));
    fEditJobDefSnapshotFolder = new QLineEdit();
    fEditJobDefSnapshotFolder->setFixedHeight(22);
    hlay->addWidget(fEditJobDefSnapshotFolder);
    QObject::connect(fEditJobDefSnapshotFolder, &QLineEdit::textChanged, fWizard, [this](const QString&) { UpdateNavButtons(); });
    fSimJobDefHostLayout->addWidget(fJobDefEditorHost, 1);
    fJobDefModeStack->setCurrentIndex(0);
    fJobDefTextMode = false;

    fJobBatchStepStack->addWidget(simJobDefPage);
    fJobBatchStepStack->addWidget(batchPostPage);
    fJobBatchStepStack->addWidget(postJobDefChoicePage);
    fJobBatchStepStack->addWidget(postJobDefPage);
    fJobBatchStepStack->setCurrentIndex(0);
    l2OuterRoot->addWidget(fJobBatchStepStack);
    fStack->addWidget(page2);

    // Wizard step 6 of 8 — cloud submission mode
    QWidget* page3 = new QWidget();
    page3->setFocusPolicy(Qt::ClickFocus);
    QVBoxLayout* page3Outer = new QVBoxLayout(page3);
    page3Outer->setContentsMargins(12, 18, 12, 18);
    page3Outer->setSpacing(12);
    AddUnderlinedTitle(page3Outer, "<b>Cloud submission</b>");
    QLabel* submitModeDesc = new QLabel(
        "Do you want to submit the parameter file you opened and containing directory to AWS? "
        "The directory should contain all includeFiles.");
    submitModeDesc->setWordWrap(true);
    page3Outer->addWidget(submitModeDesc);
    fRadioCloudSubmitUseOpenParam = new QRadioButton("Yes, use the parameter file");
    fRadioCloudSubmitUseTemplateVars = new QRadioButton("No, use the variables from the template script");
    fRadioCloudSubmitUseOpenParam->setChecked(true);
    page3Outer->addWidget(fRadioCloudSubmitUseOpenParam);
    page3Outer->addWidget(fRadioCloudSubmitUseTemplateVars);
    page3Outer->addStretch(1);
    fStack->addWidget(page3);
    QObject::connect(fRadioCloudSubmitUseOpenParam, &QRadioButton::toggled, fWizard, [this](bool checked) {
        if (checked)
            fSubmitUseOpenedParameterFile = true;
        UpdateNavButtons();
    });
    QObject::connect(fRadioCloudSubmitUseTemplateVars, &QRadioButton::toggled, fWizard, [this](bool checked) {
        if (checked)
            fSubmitUseOpenedParameterFile = false;
        UpdateNavButtons();
    });

    // Wizard step 7 of 8 — Batch submit parameters
    QWidget* page4 = new QWidget();
    page4->setFocusPolicy(Qt::ClickFocus);
    QVBoxLayout* page4Outer = new QVBoxLayout(page4);
    page4Outer->setContentsMargins(12, 18, 12, 18);
    page4Outer->setSpacing(16);
    AddUnderlinedTitle(page4Outer, "<b>Cloud submission</b>");
    QLabel* submitDesc = new QLabel("The fields below are editable variables from the submission script.");
    submitDesc->setWordWrap(true);
    page4Outer->addWidget(submitDesc);
    fSubmitVarsGrid = new QGridLayout();
    page4Outer->addLayout(fSubmitVarsGrid);
    AddUnderlinedTitle(page4Outer, "<b>Save changes locally</b>");
    QLabel* submitApplyDesc = new QLabel(
        "These settings will be saved locally in the AWS directory that you specified in step 1.");
    submitApplyDesc->setWordWrap(true);
    page4Outer->addWidget(submitApplyDesc);
    page4Outer->addWidget(new QLabel("Choose a folder name"));
    fEditSubmitSnapshotFolder = new QLineEdit();
    fEditSubmitSnapshotFolder->setFixedHeight(22);
    page4Outer->addWidget(fEditSubmitSnapshotFolder);
    QObject::connect(fEditSubmitSnapshotFolder, &QLineEdit::textChanged, fWizard, [this](const QString&) { UpdateNavButtons(); });
    page4Outer->addStretch(1);
    fStack->addWidget(page4);

    // Wizard step 8 of 8 — job monitoring
    QWidget* page5 = new QWidget();
    page5->setFocusPolicy(Qt::ClickFocus);
    QVBoxLayout* l4 = new QVBoxLayout(page5);
    l4->setContentsMargins(12, 18, 12, 18);
    l4->setSpacing(14);
    AddUnderlinedTitle(l4, "<b>Job status dashboard</b>");
    QLabel* monitorDesc1 = new QLabel(
        "You can monitor the status of your submitted jobs on this page. It is refreshed periodically.");
    monitorDesc1->setWordWrap(true);
    l4->addWidget(monitorDesc1);
    QLabel* monitorDesc2 = new QLabel(
        "Once jobs are completed you will be able to click \"Next\" which will take you to the last step of this wizard "
        "which allows you to download and optionally postprocess your results.");
    monitorDesc2->setWordWrap(true);
    l4->addWidget(monitorDesc2);
    QLabel* monitorDesc3 = new QLabel(
        "Exiting this monitoring page early will exit the TOPAS AWS Cloud wizard and you will need to download/postprocess "
        "the simulation outputs yourself.");
    monitorDesc3->setWordWrap(true);
    l4->addWidget(monitorDesc3);
    fMonitorLastUpdated = new QLabel("Dashboard last updated: --. Auto-refreshes every 60 seconds.");
    l4->addWidget(fMonitorLastUpdated);

    auto addStatusRow = [this, l4](const QString& key, const QString& labelText) {
        QHBoxLayout* row = new QHBoxLayout();
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(10);
        QFrame* dot = new QFrame();
        dot->setFixedSize(12, 12);
        dot->setStyleSheet("background-color: rgb(110,110,110); border-radius: 6px;");
        QLabel* lbl = new QLabel(labelText);
        row->addWidget(dot, 0, Qt::AlignVCenter);
        row->addWidget(lbl, 0, Qt::AlignVCenter);
        row->addStretch(1);
        l4->addLayout(row);
        fMonitorStateDots[key] = dot;
        fMonitorStateLabels[key] = lbl;
    };
    addStatusRow("SUBMITTED", "0 jobs submitted");
    addStatusRow("PENDING", "0 jobs pending");
    addStatusRow("RUNNABLE", "0 jobs runnable");
    addStatusRow("STARTING", "0 jobs starting");
    addStatusRow("RUNNING", "0 jobs running");
    addStatusRow("SUCCEEDED", "0 jobs succeeded");
    addStatusRow("FAILED", "0 job/s failed");

    fMonitorText = new QTextEdit();
    fMonitorText->setReadOnly(true);
    fMonitorText->setFont(QFont("monospace"));
    fMonitorText->setVisible(false);
    l4->addWidget(fMonitorText);
    QPushButton* exitEarly = new QPushButton("Exit monitoring");
    l4->addWidget(exitEarly);
    fStack->addWidget(page5);
    QObject::connect(exitEarly, &QPushButton::clicked, fWizard, [this]() {
        QMessageBox m(fWizard);
        m.setWindowTitle("Stop monitoring");
        m.setText("If you exit now, you must monitor and post-process yourself outside this dialog.");
        m.setIcon(QMessageBox::Warning);
        m.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        if (m.exec() == QMessageBox::Ok)
            SetStep(9); // download / post-process (skip wait on monitoring)
    });

    // Wizard step 9 of 9 — download and optional post-process
    QWidget* page6 = new QWidget();
    page6->setFocusPolicy(Qt::ClickFocus);
    QVBoxLayout* l5 = new QVBoxLayout(page6);
    l5->setContentsMargins(12, 18, 12, 18);
    l5->setSpacing(8);
    QLabel* downloadTitle = new QLabel("<b>Download outputs</b>");
    downloadTitle->setTextFormat(Qt::RichText);
    downloadTitle->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QFont downloadTitleFont = downloadTitle->font();
    downloadTitleFont.setPointSize(downloadTitleFont.pointSize() + 3);
    downloadTitle->setFont(downloadTitleFont);
    l5->addWidget(downloadTitle);
    QFrame* downloadUnderline = new QFrame();
    downloadUnderline->setFrameShape(QFrame::HLine);
    downloadUnderline->setFrameShadow(QFrame::Plain);
    downloadUnderline->setLineWidth(2);
    downloadUnderline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    l5->addWidget(downloadUnderline);
    QLabel* downloadDesc = new QLabel("Choose a local folder to download your data into.");
    downloadDesc->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    l5->addWidget(downloadDesc);
    QHBoxLayout* h5 = new QHBoxLayout();
    fEditDownloadDir = new QLineEdit();
    fEditDownloadDir->setFixedHeight(22);
    h5->addWidget(fEditDownloadDir);
    QPushButton* browseDl = new QPushButton("Browse...");
    h5->addWidget(browseDl);
    l5->addLayout(h5);
    fBtnDownloadS3 = new QPushButton("Download simulation job outputs from S3");
    ApplyPrimaryButtonStyle(fBtnDownloadS3);
    l5->addWidget(fBtnDownloadS3);
    l5->addSpacing(10);
    fPostprocessSectionWidget = new QWidget();
    QVBoxLayout* postSectionLayout = new QVBoxLayout(fPostprocessSectionWidget);
    postSectionLayout->setContentsMargins(0, 0, 0, 0);
    postSectionLayout->setSpacing(6);
    QLabel* postTitle = new QLabel("<b>Post-process</b> (optional)");
    postTitle->setTextFormat(Qt::RichText);
    postTitle->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QFont postTitleFont = postTitle->font();
    postTitleFont.setPointSize(postTitleFont.pointSize() + 3);
    postTitle->setFont(postTitleFont);
    postSectionLayout->addWidget(postTitle);
    QFrame* postUnderline = new QFrame();
    postUnderline->setFrameShape(QFrame::HLine);
    postUnderline->setFrameShadow(QFrame::Plain);
    postUnderline->setLineWidth(2);
    postUnderline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    postSectionLayout->addWidget(postUnderline);
    fPostprocessVarsGrid = new QGridLayout();
    postSectionLayout->addLayout(fPostprocessVarsGrid);
    fBtnSubmitPostprocess = new QPushButton("Submit postprocess job");
    ApplyPrimaryButtonStyle(fBtnSubmitPostprocess);
    postSectionLayout->addWidget(fBtnSubmitPostprocess);
    l5->addWidget(fPostprocessSectionWidget);
    l5->addStretch(1);
    fStack->addWidget(page6);

    QObject::connect(browseDl, &QPushButton::clicked, fWizard, [this]() {
        QString d = QFileDialog::getExistingDirectory(fWizard, "Download folder", fEditDownloadDir->text());
        if (!d.isEmpty())
            fEditDownloadDir->setText(d);
    });
    QObject::connect(fBtnDownloadS3, &QPushButton::clicked, fWizard, [this]() {
        const QString projectName = !fProjectName.isEmpty() ? fProjectName : EditTextOrEmpty(fEditProjectName);
        const QString runDate = !fRunDate.isEmpty() ? fRunDate : EditTextOrEmpty(fEditRunDate);
        const QString outBucket = !fOutputBucket.isEmpty() ? fOutputBucket : EditTextOrEmpty(fEditOutputBucket);
        if (projectName.isEmpty() || runDate.isEmpty() || outBucket.isEmpty()) {
            ShowAwsMessage(fWizardParent, "s3 sync", false, "Required submission variables are missing.");
            return;
        }
        QString simDir = "projects/" + projectName + "/" + runDate;
        QString s3uri = "s3://" + outBucket + "/" + simDir + "/";
        QString localTarget = fEditDownloadDir->text();
        if (fDownloadPostprocessResults)
            s3uri += "postP_results/";
        if (fDownloadPostprocessResults)
            localTarget = QDir(localTarget).filePath("postP_results");
        QStringList args;
        args << "s3"
             << "sync"
             << s3uri
             << localTarget
             << "--only-show-errors";
        QDialog* busy = new QDialog(fWizard);
        busy->setWindowTitle("Download");
        busy->setModal(true);
        busy->setWindowFlag(Qt::WindowCloseButtonHint, false);
        QVBoxLayout* dl = new QVBoxLayout(busy);
        QLabel* status = new QLabel("Busy downloading ...");
        status->setWordWrap(true);
        dl->addWidget(status);
        QHBoxLayout* buttons = new QHBoxLayout();
        QPushButton* btnCancelDownload = new QPushButton("Cancel");
        QPushButton* btnExitDownload = new QPushButton("Exit");
        btnExitDownload->setEnabled(false);
        btnCancelDownload->setStyleSheet(
            "QPushButton { border-radius: 8px; padding: 6px 12px; border: none; }"
            "QPushButton:enabled { background-color: rgb(58,58,58); color: rgb(235,235,235); }"
            "QPushButton:disabled { background-color: rgb(58,58,58); color: rgb(136,136,136); }");
        ApplyPrimaryButtonStyle(btnExitDownload);
        buttons->addWidget(btnCancelDownload);
        buttons->addWidget(btnExitDownload);
        buttons->addStretch(1);
        dl->addLayout(buttons);

        QProcess* proc = new QProcess(busy);
        QStringList awsArgs;
        AwsCliArgs(&awsArgs);
        awsArgs.append(args);
        QString* stdOut = new QString();
        QString* stdErr = new QString();
        bool* wasCancelled = new bool(false);
        QObject::connect(busy, &QDialog::finished, busy, [stdOut, stdErr, wasCancelled](int) {
            delete stdOut;
            delete stdErr;
            delete wasCancelled;
        });
        QObject::connect(proc, &QProcess::readyReadStandardOutput, busy, [proc, stdOut]() {
            *stdOut += QString::fromUtf8(proc->readAllStandardOutput());
        });
        QObject::connect(proc, &QProcess::readyReadStandardError, busy, [proc, stdErr]() {
            *stdErr += QString::fromUtf8(proc->readAllStandardError());
        });
        QObject::connect(btnCancelDownload, &QPushButton::clicked, busy, [proc, status, wasCancelled]() {
            if (proc->state() == QProcess::NotRunning)
                return;
            *wasCancelled = true;
            status->setText("Cancelling download ...");
            proc->kill();
        });
        QObject::connect(btnExitDownload, &QPushButton::clicked, busy, [busy]() { busy->accept(); });
        QObject::connect(proc, &QProcess::errorOccurred, busy, [status, btnCancelDownload, btnExitDownload](QProcess::ProcessError) {
            status->setText("Download failed.");
            btnCancelDownload->setEnabled(false);
            btnExitDownload->setEnabled(true);
        });
        QObject::connect(proc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), busy,
                         [this, status, btnCancelDownload, btnExitDownload, stdOut, stdErr, wasCancelled]
                         (int code, QProcess::ExitStatus st) {
            btnCancelDownload->setEnabled(false);
            btnExitDownload->setEnabled(true);
            if (*wasCancelled) {
                status->setText("Download cancelled.");
                return;
            }
            if (st == QProcess::NormalExit && code == 0) {
                status->setText("Download complete!");
                return;
            }
            status->setText("Download failed.");
            ShowAwsMessage(fWizardParent, "s3 sync", false, *stdErr + "\n" + *stdOut);
        });
        proc->start("aws", awsArgs);
        if (!proc->waitForStarted(3000)) {
            status->setText("Download failed.");
            btnCancelDownload->setEnabled(false);
            btnExitDownload->setEnabled(true);
        }
        busy->exec();
        busy->deleteLater();
    });
    QObject::connect(fBtnSubmitPostprocess, &QPushButton::clicked, fWizard, [this]() {
        fSnapshotRoot = SnapshotSubdir();
        QDir().mkpath(fSnapshotRoot);
        QString scriptPath = QDir(fSnapshotRoot).filePath("postprocess_gui.sh");
        BuildPostprocessScriptToFile(scriptPath);
        QString err;
        int code = 0;
        QString out;
        if (!RunSubmitScript(scriptPath, &out, &err, &code) || code != 0) {
            ShowAwsMessage(fWizardParent, "Postprocess", false, err + "\n" + out);
            return;
        }
        QString line = out.trimmed().split('\n').last().trimmed();
        fPostprocessJobId = line;
        ShowAwsMessage(fWizardParent, "Postprocess", true, "Submitted job id: " + fPostprocessJobId);
        fJobIds.clear();
        if (!line.isEmpty())
            fJobIds.append(line);
        fMonitoringPostprocess = true;
        fDownloadPostprocessResults = true;
        SetStep(8);
    });

    mainLay->addWidget(fStack, 1);

    QHBoxLayout* nav = new QHBoxLayout();
    fBtnCancel = new QPushButton("Cancel");
    fBtnReset = new QPushButton("Reset");
    fBtnReset->setEnabled(false);
    ApplyPrimaryButtonStyle(fBtnReset);
    fBtnNext = new QPushButton("Next");
    nav->addWidget(fBtnCancel);
    nav->addWidget(fBtnReset);
    nav->addStretch(1);
    nav->addWidget(fBtnNext);
    mainLay->addLayout(nav);

    QObject::connect(fBtnCancel, &QPushButton::clicked, fWizard, &QDialog::reject);
    QObject::connect(fBtnReset, &QPushButton::clicked, fWizard, [this]() {
        QMessageBox m(fWizard);
        m.setWindowTitle("Reset values");
        m.setIcon(QMessageBox::Warning);
        m.setText("Are you sure you want to reset to default values? All changes will be lost.");
        m.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        m.setDefaultButton(QMessageBox::Cancel);
        if (m.exec() != QMessageBox::Yes)
            return;

        if (fCurrentStep == 3) {
            if (fEditProvisionInputBucket) {
                const QSignalBlocker b(fEditProvisionInputBucket);
                fEditProvisionInputBucket->setText(QStringLiteral("topas-nbio-input"));
            }
            if (fEditProvisionOutputBucket) {
                const QSignalBlocker b(fEditProvisionOutputBucket);
                fEditProvisionOutputBucket->setText(QStringLiteral("topas-nbio-output"));
            }
            if (fRadioCreateNewBucketsLogs && fRadioCreateUseExistingBucketsLogs) {
                const QSignalBlocker b1(fRadioCreateNewBucketsLogs);
                const QSignalBlocker b2(fRadioCreateUseExistingBucketsLogs);
                fRadioCreateUseExistingBucketsLogs->setChecked(true);
                fRadioCreateNewBucketsLogs->setChecked(false);
            }
        } else if (fCurrentStep == 4) {
            const int csi = ComputeStepStackIndex();
            if (csi == 0) {
                if (fRadioComputeExisting && fRadioComputeCreate) {
                    const QSignalBlocker b1(fRadioComputeExisting);
                    const QSignalBlocker b2(fRadioComputeCreate);
                    fRadioComputeExisting->setChecked(true);
                    fRadioComputeCreate->setChecked(false);
                }
                if (fEditExistingComputeEnvName) {
                    const QSignalBlocker b(fEditExistingComputeEnvName);
                    fEditExistingComputeEnvName->setText(fDefaultExistingComputeEnvName);
                    fEditExistingComputeEnvName->setEnabled(true);
                }
            } else {
                if (!fComputeDefaultJsonText.isEmpty() && fComputeTextEditor)
                    fComputeTextEditor->setPlainText(fComputeDefaultJsonText);
                fComputeJsonText = fComputeDefaultJsonText;
                fComputeApplied = true;
                fComputeNeedsApply = false;
                fComputeCreated = false;
                if (fEditComputeSnapshotFolder) {
                    const QSignalBlocker b(fEditComputeSnapshotFolder);
                    fEditComputeSnapshotFolder->clear();
                }
                RebuildComputeFormFromCurrentJson();
            }
        } else if (fCurrentStep == 5) {
            const int jsi = JobBatchStepStackIndex();
            if (jsi == 0) {
                if (fEditExistingJobQueue) {
                    const QSignalBlocker b(fEditExistingJobQueue);
                    fEditExistingJobQueue->setText(fSubmitVarDefaults.value(QStringLiteral("JOB_QUEUE")));
                }
                if (fEditWizardJobQueueName) {
                    const QSignalBlocker b(fEditWizardJobQueueName);
                    fEditWizardJobQueueName->setText(fSubmitVarDefaults.value(QStringLiteral("JOB_QUEUE")));
                }
                if (fRadioBatchCreate && fRadioBatchExisting) {
                    const QSignalBlocker b1(fRadioBatchCreate);
                    const QSignalBlocker b2(fRadioBatchExisting);
                    fRadioBatchExisting->setChecked(true);
                    fRadioBatchCreate->setChecked(false);
                }
            } else if (jsi == 1) {
                if (fEditProvisionLogGroup) {
                    const QSignalBlocker b(fEditProvisionLogGroup);
                    fEditProvisionLogGroup->setText(QStringLiteral("/aws/batch/topas-nbio"));
                }
                if (fRadioLogGroupCreate && fRadioLogGroupExisting) {
                    const QSignalBlocker b1(fRadioLogGroupCreate);
                    const QSignalBlocker b2(fRadioLogGroupExisting);
                    fRadioLogGroupExisting->setChecked(true);
                    fRadioLogGroupCreate->setChecked(false);
                }
            } else if (jsi == 2) {
                fBatchUseExistingJobDefs = true;
                if (fRadioSimJobDefCreate && fRadioSimJobDefExisting) {
                    const QSignalBlocker b1(fRadioSimJobDefCreate);
                    const QSignalBlocker b2(fRadioSimJobDefExisting);
                    fRadioSimJobDefExisting->setChecked(true);
                    fRadioSimJobDefCreate->setChecked(false);
                }
                if (fEditExistingJobDefSim) {
                    const QSignalBlocker b(fEditExistingJobDefSim);
                    QString jds = fSubmitVarDefaults.value(QStringLiteral("JOB_DEFINITION"));
                    if (jds.isEmpty())
                        jds = fSubmitVarDefaults.value(QStringLiteral("JOB_DEFINITION_NAME"));
                    fEditExistingJobDefSim->setText(jds);
                    fEditExistingJobDefSim->setEnabled(true);
                }
            } else if (jsi == 3) {
                if (!fJobDefDefaultJsonText.isEmpty()) {
                    fJobDefJsonText = fJobDefDefaultJsonText;
                    fActiveJobDefTabIndex = 0;
                    EnsureJobDefEditorHostForPage(true);
                    LoadActiveJobDefIntoEditor();
                }
            } else if (jsi == 4) {
                if (fRadioProvisionPostYes) {
                    const QSignalBlocker b(fRadioProvisionPostYes);
                    fRadioProvisionPostYes->setChecked(true);
                }
                if (fEditPostprocessScriptPath && fAwsDirEdit) {
                    const QSignalBlocker b(fEditPostprocessScriptPath);
                    fEditPostprocessScriptPath->setText(QDir::cleanPath(fAwsDirEdit->text().trimmed()));
                    fEditPostprocessScriptPath->setEnabled(true);
                }
                if (fWizard) {
                    if (QPushButton* browseBtn = fWizard->findChild<QPushButton*>("postScriptBrowseBtn"))
                        browseBtn->setEnabled(true);
                }
                fPostprocessScriptSelectedByBrowse = false;
            } else if (jsi == 5) {
                if (fRadioPostJobDefExisting && fRadioPostJobDefCreate) {
                    const QSignalBlocker b1(fRadioPostJobDefExisting);
                    const QSignalBlocker b2(fRadioPostJobDefCreate);
                    fRadioPostJobDefExisting->setChecked(true);
                    fRadioPostJobDefCreate->setChecked(false);
                }
                if (fEditExistingJobDefPost) {
                    const QSignalBlocker b(fEditExistingJobDefPost);
                    fEditExistingJobDefPost->setText(QStringLiteral("topas-nbio-postprocess-job"));
                    fEditExistingJobDefPost->setEnabled(true);
                }
            } else if (jsi == 6) {
                if (!fJobDefPostDefaultJsonText.isEmpty()) {
                    fJobDefPostJsonText = fJobDefPostDefaultJsonText;
                    fActiveJobDefTabIndex = 1;
                    EnsureJobDefEditorHostForPage(false);
                    LoadActiveJobDefIntoEditor();
                }
            }
            if (jsi >= 3) {
                fJobDefApplied = true;
                fJobDefNeedsApply = false;
                fJobDefCreated = false;
                RebuildJobDefFormFromCurrentJson();
                if (fEditWizardJobQueueName) {
                    const QSignalBlocker b(fEditWizardJobQueueName);
                    fEditWizardJobQueueName->setText(fSubmitVarDefaults.value("JOB_QUEUE"));
                }
                if (fEditJobDefSnapshotFolder) {
                    const QSignalBlocker b(fEditJobDefSnapshotFolder);
                    fEditJobDefSnapshotFolder->clear();
                }
            }
            if (fJobBatchStepStack)
                fJobBatchStepStack->setCurrentIndex(jsi);
        } else if (fCurrentStep == 7) {
            for (const QString& key : fSubmitVarOrder) {
                QLineEdit* edit = fSubmitVarEdits.value(key, nullptr);
                if (edit)
                    edit->setText(fSubmitVarDefaults.value(key));
            }
            fProjectName.clear();
            fRunDate.clear();
            fOutputBucket.clear();
            fSubmitNeedsReset = false;
            fSubmitApplied = true;
            fSubmitScriptSnapshotPath = fTopasSubmitPath;
            if (fEditSubmitSnapshotFolder) {
                const QSignalBlocker b(fEditSubmitSnapshotFolder);
                fEditSubmitSnapshotFolder->clear();
            }
        }
        UpdateNavButtons();
    });
    QObject::connect(fBtnNext, &QPushButton::clicked, fWizard, [this]() {
        if (fCurrentStep == 9) { // final step — close
            fWizard->accept();
            return;
        }
        if (fCurrentStep == 2) { // default vs advanced
            SaveAwsSessionOnly();
            fAwsDir = fAwsDirEdit ? fAwsDirEdit->text() : fAwsDir;
            if (!HasRequiredFiles(fAwsDir)) {
                ShowAwsMessage(fWizardParent, "AWS scripts folder", false,
                               "The folder must contain the TOPAS-nBio template files. Go back to step 1 to select the folder.");
                return;
            }
            SetWizardEditorModesFromStep2();
            SetStep(3);
            return;
        }
        if (fCurrentStep == 3) { // buckets
            const QString inB = fEditProvisionInputBucket ? fEditProvisionInputBucket->text().trimmed() : QString();
            const QString outB = fEditProvisionOutputBucket ? fEditProvisionOutputBucket->text().trimmed() : QString();
            if (inB.isEmpty() || outB.isEmpty()) {
                ShowAwsMessage(fWizardParent, "S3 buckets", false, "Enter both input and output bucket names.");
                return;
            }
            SetStep(4);
            if (fComputeStepStack)
                fComputeStepStack->setCurrentIndex(0);
            return;
        }
        if (fCurrentStep == 4) { // compute: nested choice or editor
            const int csi = ComputeStepStackIndex();
            if (csi == 0) {
                const bool wantExisting = fRadioComputeExisting && fRadioComputeExisting->isChecked();
                if (wantExisting) {
                    const QString n = fEditExistingComputeEnvName ? fEditExistingComputeEnvName->text().trimmed() : QString();
                    if (n.isEmpty()) {
                        ShowAwsMessage(fWizardParent, "Compute environment", false, "Enter the name of your existing compute environment.");
                        return;
                    }
                    fComputeUseExisting = true;
                    fExistingComputeEnvName = n;
                    fComputeCreated = true;
                    fComputeApplied = true;
                    SetStep(5);
                    if (fJobBatchStepStack)
                        fJobBatchStepStack->setCurrentIndex(0);
                    return;
                }
                fComputeUseExisting = false;
                fExistingComputeEnvName.clear();
                if (fComputeStepStack)
                    fComputeStepStack->setCurrentIndex(1);
                UpdateNavButtons();
                return;
            }
            if (!fComputeCreated) {
                if (!CreateComputeEnvironmentFromSnapshot())
                    return;
                fComputeCreated = true;
            }
            SetStep(5);
            if (fJobBatchStepStack)
                fJobBatchStepStack->setCurrentIndex(0);
            return;
        }
        if (fCurrentStep == 5) { // batch / job defs nested
            const int jsi = JobBatchStepStackIndex();
            if (jsi == 0) {
                const bool useExisting = fRadioBatchExisting && fRadioBatchExisting->isChecked();
                const QString jq = useExisting
                    ? (fEditExistingJobQueue ? fEditExistingJobQueue->text().trimmed() : QString())
                    : (fEditWizardJobQueueName ? fEditWizardJobQueueName->text().trimmed() : QString());
                if (jq.isEmpty()) {
                    ShowAwsMessage(fWizardParent, "Job queue", false, "Please enter a job queue name.");
                    return;
                }
                fBatchUseExisting = useExisting;
                if (fJobBatchStepStack)
                    fJobBatchStepStack->setCurrentIndex(1);
                UpdateNavButtons();
                return;
            }
            if (jsi == 1) {
                QString provErr;
                if (!RunProvisionS3AndLogs(&provErr)) {
                    ShowAwsMessage(fWizardParent, "Provision S3 / logs", false, provErr);
                    return;
                }
                fS3LogsProvisioned = true;
                const QString inB = fEditProvisionInputBucket ? fEditProvisionInputBucket->text().trimmed() : QString();
                const QString outB = fEditProvisionOutputBucket ? fEditProvisionOutputBucket->text().trimmed() : QString();
                const QString lg = fEditProvisionLogGroup ? fEditProvisionLogGroup->text().trimmed() : QString();
                const QString reg = fEditAwsRegion ? fEditAwsRegion->text().trimmed() : QString();
                fJobDefJsonText = PatchJobDefJsonBucketsAndLog(fJobDefJsonText, inB, outB, lg, reg);
                fJobDefPostJsonText = PatchJobDefJsonBucketsAndLog(fJobDefPostJsonText, inB, outB, lg, reg);
                fActiveJobDefTabIndex = 0;
                EnsureJobDefEditorHostForPage(true);
                UpdateJobDefTabVisibility();
                LoadActiveJobDefIntoEditor();
                if (!fJobDefTextMode)
                    RebuildJobDefFormFromCurrentJson();
                fJobDefApplied = true;
                fJobDefCreated = false;
                fJobDefNeedsApply = false;
                if (fJobBatchStepStack)
                    fJobBatchStepStack->setCurrentIndex(2);
                UpdateNavButtons();
                return;
            }
            if (jsi == 2) {
                if (fBatchUseExistingJobDefs) {
                    const QString existingSimJobDef = fEditExistingJobDefSim ? fEditExistingJobDefSim->text().trimmed() : QString();
                    if (existingSimJobDef.isEmpty()) {
                        ShowAwsMessage(fWizardParent, "Simulation job definition", false,
                                       "Enter the name of your existing simulation job definition.");
                        return;
                    }
                    if (fSubmitVarEdits.contains(QStringLiteral("JOB_DEFINITION")))
                        fSubmitVarEdits[QStringLiteral("JOB_DEFINITION")]->setText(existingSimJobDef);
                    if (fSubmitVarEdits.contains(QStringLiteral("JOB_DEFINITION_NAME")))
                        fSubmitVarEdits[QStringLiteral("JOB_DEFINITION_NAME")]->setText(existingSimJobDef);
                    if (fJobBatchStepStack)
                        fJobBatchStepStack->setCurrentIndex(4);
                    UpdateNavButtons();
                    return;
                }
                if (!fSimJobDefPageInitialized && !fJobDefDefaultJsonText.isEmpty()) {
                    fJobDefJsonText = fJobDefDefaultJsonText;
                    fActiveJobDefTabIndex = 0;
                    EnsureJobDefEditorHostForPage(true);
                    LoadActiveJobDefIntoEditor();
                    if (!fJobDefTextMode)
                        RebuildJobDefFormFromCurrentJson();
                    fSimJobDefPageInitialized = true;
                }
                if (fJobBatchStepStack)
                    fJobBatchStepStack->setCurrentIndex(3);
                UpdateNavButtons();
                return;
            }
            if (jsi == 3) {
                CommitJobDefEditorToActiveString();
                if (fJobBatchStepStack)
                    fJobBatchStepStack->setCurrentIndex(4);
                UpdateNavButtons();
                return;
            }
            if (jsi == 4) {
                fWantsPostprocessing = fRadioProvisionPostYes && fRadioProvisionPostYes->isChecked();
                UpdateJobDefTabVisibility();
                if (!fWantsPostprocessing) {
                    if (!fJobDefCreated) {
                        if (!CreateJobDefinitionFromSnapshot())
                            return;
                        fJobDefCreated = true;
                    }
                    SyncSubmitStepFromBatchState();
                    SetStep(6);
                    return;
                }
                const QString localPostScript = fEditPostprocessScriptPath ? fEditPostprocessScriptPath->text().trimmed() : QString();
                if (localPostScript.isEmpty()) {
                    ShowAwsMessage(fWizardParent, "Post-processing", false,
                                   "Please select the post-processing script file before continuing.");
                    return;
                }
                if (!QFileInfo(localPostScript).exists() || !QFileInfo(localPostScript).isFile()) {
                    ShowAwsMessage(fWizardParent, "Post-processing", false,
                                   "Please select a valid post-processing script file before continuing.");
                    return;
                }
                if (fEditPpLocalScript)
                    fEditPpLocalScript->setText(localPostScript);
                if (fJobBatchStepStack)
                    fJobBatchStepStack->setCurrentIndex(5);
                UpdateNavButtons();
                return;
            }
            if (jsi == 5) {
                const bool useExistingPost = fRadioPostJobDefExisting && fRadioPostJobDefExisting->isChecked();
                fBatchUseExistingPostJobDef = useExistingPost;
                if (useExistingPost) {
                    const QString existingPostJobDef = fEditExistingJobDefPost ? fEditExistingJobDefPost->text().trimmed() : QString();
                    if (existingPostJobDef.isEmpty()) {
                        ShowAwsMessage(fWizardParent, "Post-processing job definition", false,
                                       "Enter the name of your existing post-processing job definition.");
                        return;
                    }
                    if (fEditPostprocessJobDefinitionName)
                        fEditPostprocessJobDefinitionName->setText(existingPostJobDef);
                    if (!fJobDefCreated) {
                        if (!CreateJobDefinitionFromSnapshot())
                            return;
                        fJobDefCreated = true;
                    }
                    SyncSubmitStepFromBatchState();
                    SetStep(6);
                    return;
                }
                fActiveJobDefTabIndex = 1;
                if (!fPostJobDefPageInitialized && !fJobDefPostDefaultJsonText.isEmpty()) {
                    fJobDefPostJsonText = fJobDefPostDefaultJsonText;
                    fPostJobDefPageInitialized = true;
                }
                EnsureJobDefEditorHostForPage(false);
                LoadActiveJobDefIntoEditor();
                if (!fJobDefTextMode)
                    RebuildJobDefFormFromCurrentJson();
                if (fJobBatchStepStack)
                    fJobBatchStepStack->setCurrentIndex(6);
                UpdateNavButtons();
                return;
            }
            if (jsi == 6) {
                if (!fJobDefCreated) {
                    if (!CreateJobDefinitionFromSnapshot())
                        return;
                    fJobDefCreated = true;
                }
                SyncSubmitStepFromBatchState();
                SetStep(6);
                return;
            }
        }
        if (fCurrentStep == 6) { // cloud submission choice -> submit page
            SetStep(7);
            return;
        }
        if (fCurrentStep == 7) { // submit then go to monitoring
            if (fTopasSubmitPath.isEmpty() || !QFileInfo::exists(fTopasSubmitPath)) {
                ShowAwsMessage(fWizardParent, "Submit to cloud", false, "Could not find topas_submit.sh in the AWS directory.");
                return;
            }
            QMap<QString, QString> submitEnv = BuildSubmitOverridesFromUi();
            if (!ApplySubmitScriptSnapshot(false))
                return;
            const QString submitScriptPath =
                fSubmitScriptSnapshotPath.isEmpty() ? fTopasSubmitPath : fSubmitScriptSnapshotPath;
            QString out;
            QString err;
            int code = 0;
            if (!RunSubmitScript(submitScriptPath, &out, &err, &code, submitEnv) || code != 0) {
                ShowAwsMessage(fWizardParent, "Submit to cloud", false, err + "\n" + out);
                return;
            }
            fProjectName = ResolveSubmitValue(submitEnv.value("PROJECT_NAME"), submitEnv);
            fRunDate = ResolveSubmitValue(submitEnv.value("RUN_DATE"), submitEnv);
            fOutputBucket = ResolveSubmitValue(submitEnv.value("OUTPUT_BUCKET"), submitEnv);
            ParseJobIdsFromSubmitOutput(out);
            fMonitoringPostprocess = false;
            fDownloadPostprocessResults = false;
            ShowAwsMessage(fWizardParent, "Submit to cloud", true, "Submitted. Job ids:\n" + out);
            SetStep(8); // monitoring
            return;
        }
        SetStep(fCurrentStep + 1);
    });

    QObject::connect(fAwsDirEdit, &QLineEdit::editingFinished, fWizard, [this]() { ReloadAwsFilesFromUi(); });
    QObject::connect(fProfileEdit, &QLineEdit::editingFinished, fWizard, [this]() { SaveAwsSessionOnly(); });
    QObject::connect(fEditAwsRegion, &QLineEdit::editingFinished, fWizard, [this]() { SaveAwsSessionOnly(); });
}

#endif
