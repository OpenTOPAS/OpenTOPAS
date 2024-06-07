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

#ifdef G4UI_USE_QT

#include "TsQt.hh"

#include "TsParameterManager.hh"
#include "TsExtensionManager.hh"
#include "TsMaterialManager.hh"
#include "TsGeometryManager.hh"
#include "TsScoringManager.hh"
#include "TsSequenceManager.hh"
#include "TsGraphicsManager.hh"
#include "TsSourceManager.hh"

#include "TsGeometryHub.hh"
#include "TsScoringHub.hh"

#include "G4UImanager.hh"
#include "G4VViewer.hh"
#include "G4SystemOfUnits.hh"

#include <string.h>
#include <vector>

#include "G4UIQt.hh"

#include <qtoolbar.h>
#include <qlabel.h>
#include <qaction.h>
#include <qsignalmapper.h>
#include <qmainwindow.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qtablewidget.h>
#include <qheaderview.h>
#include <qapplication.h>
#include <qlist.h>

TsQt::TsQt(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsSequenceManager* sqM,
		   TsGraphicsManager* grM, TsSourceManager* soM) :
fPm(pM), fEm(eM), fMm(mM), fGm(gM), fScm(scM), fSqm(sqM), fGrm(grM), fSom(soM), fTestSucceeded(true), fParameterEditorDialog(0),
fParameterTableWidget(0),
fAddComponentDialog(0), fCurrentComponentName(""), fAddedComponentCounter(0),
fAddScorerDialog(0), fCurrentScorerName(""), fAddedScorerCounter(0),
fAddSourceDialog(0), fCurrentSourceName(""), fAddedSourceCounter(0)
{
	fParameterEditorWidget = new QWidget();
	QVBoxLayout* layoutWidget = new QVBoxLayout();
	fParameterEditorWidget->setLayout(layoutWidget);

	fParameterTableGroupBox = new QGroupBox();
	fParameterTablevbox = new QVBoxLayout;

	fUIQt = static_cast<G4UIQt*> (G4UImanager::GetUIpointer()->GetG4UIWindow());
	fUIQt->GetUserInterfaceWidget()->setWindowTitle("");

	// Add our own control widget
	fUIQt->GetUITabWidget()->addTab(fParameterEditorWidget,"Parameter Control");

	if (fPm->GetBooleanParameter("Ts/IncludeDefaultGeant4QtWidgets")) {
		fUIQt->GetUITabWidget()->setCurrentIndex(3);
	} else {
		// Get rid of the default Geant4 Qt control widgets
		fUIQt->GetUITabWidget()->removeTab(1);
		fUIQt->GetUITabWidget()->removeTab(1);
		fUIQt->GetUITabWidget()->setCurrentIndex(1);

		// Remove many of the detault Geant4 Qt menu bar actions
		QList<QToolBar *> allToolBars = fUIQt->GetMainWindow()->findChildren<QToolBar *>();
		G4int actionCounter = 0;
		foreach (QAction *action, allToolBars[0]->actions()) {
			if (actionCounter < 3 || actionCounter > 7)
				allToolBars[0]->removeAction(action);
			actionCounter++;
		}

		// Remove "Useful Tips" tab from Viewer Tab Widget
		fUIQt->GetViewerTabWidget()->removeTab(0);
	}

	QToolBar* toolbar = new QToolBar();

	QSignalMapper* saveSignalMapper = new QSignalMapper(this);
	QAction* saveAction = toolbar->addAction(QString("Save"), saveSignalMapper, SLOT(map()));
	connect(saveSignalMapper, SIGNAL(mapped(int)),this, SLOT(SaveCallback()));
	int saveIntVP = 0;
	saveSignalMapper->setMapping(saveAction, saveIntVP);

	QSignalMapper* componentSignalMapper = new QSignalMapper(this);
	toolbar->addSeparator();
	QAction* componentAction = toolbar->addAction(QString("+Geom"), componentSignalMapper, SLOT(map()));
	connect(componentSignalMapper, SIGNAL(mapped(int)),this, SLOT(AddComponentCallback()));
	int componentIntVP = 0;
	componentSignalMapper->setMapping(componentAction, componentIntVP);
	
	QSignalMapper* scorerSignalMapper = new QSignalMapper(this);
	toolbar->addSeparator();
	QAction* scorerAction = toolbar->addAction(QString("+Scorer"), scorerSignalMapper, SLOT(map()));
	connect(scorerSignalMapper, SIGNAL(mapped(int)),this, SLOT(AddScorerCallback()));
	int scorerIntVP = 0;
	scorerSignalMapper->setMapping(scorerAction, scorerIntVP);

	QSignalMapper* sourceSignalMapper = new QSignalMapper(this);
	toolbar->addSeparator();
	QAction* sourceAction = toolbar->addAction(QString("+Source"), sourceSignalMapper, SLOT(map()));
	connect(sourceSignalMapper, SIGNAL(mapped(int)),this, SLOT(AddSourceCallback()));
	int sourceIntVP = 0;
	sourceSignalMapper->setMapping(sourceAction, sourceIntVP);
	
	QSignalMapper* runSignalMapper = new QSignalMapper(this);
	toolbar->addSeparator();
	QAction* runAction = toolbar->addAction(QString("Run"), runSignalMapper, SLOT(map()));
	connect(runSignalMapper, SIGNAL(mapped(int)),this, SLOT(RunCallback()));
	int runIntVP = 0;
	runSignalMapper->setMapping(runAction, runIntVP);

	QSignalMapper* printSignalMapper = new QSignalMapper(this);
	toolbar->addSeparator();
	QAction* printAction = toolbar->addAction(QString("PDF"), printSignalMapper, SLOT(map()));
	connect(printSignalMapper, SIGNAL(mapped(int)),this, SLOT(PrintCallback()));
	int printIntVP = 0;
	printSignalMapper->setMapping(printAction, printIntVP);

	fParameterTablevbox->addWidget(toolbar);

	fPm->SetInQtSession();

	UpdateParameterEditor();
}


void TsQt::AbortSession() {
	QApplication::processEvents();
	fTestSucceeded = false;
}


void TsQt::RunCallback() {
	if (fGm->GeometryHasOverlaps() && fPm->GetBooleanParameter("Ge/QuitIfOverlapDetected")) {
		G4cerr << "TOPAS Run Sequence can not begin until Geometry Overlaps are fixed." << G4endl;
	} else {
		fSqm->ClearGenerators();
		fSqm->Sequence();

		if (fPm->ParameterExists("Ts/ExtraSequenceFiles")) {
			G4String* extraSequenceFileSpecs = fPm->GetStringVector("Ts/ExtraSequenceFiles");
			for (G4int iFile=0; iFile < fPm->GetVectorLength("Ts/ExtraSequenceFiles"); ++iFile)
				fSqm->ExtraSequence(extraSequenceFileSpecs[iFile]);
		}
	}
}


void TsQt::PrintCallback() {
	G4UImanager::GetUIpointer()->ApplyCommand("/vis/ogl/set/exportFormat pdf");
	G4UImanager::GetUIpointer()->ApplyCommand("/vis/ogl/export");
}


void TsQt::UpdateParameterEditor() {
	if (fParameterTableWidget)
		delete fParameterTableWidget;
	
	fParameterTableWidget = new QTableWidget();
	QSizePolicy vPolicy = fParameterTableWidget->sizePolicy();
	vPolicy.setVerticalStretch(4);
	
	std::vector<G4String>* parameterNames = new std::vector<G4String>;
	std::vector<G4String>* parameterValues = new std::vector<G4String>;
	fPm->GetChangeableParameters(parameterNames, parameterValues);

	G4int nParameters = parameterNames->size();
	
	fParameterTableWidget->setColumnCount(2);
	fParameterTableWidget->setColumnWidth(0,260);
	fParameterTableWidget->setColumnWidth(1,110);
	fParameterTableWidget->setMinimumWidth(380);
	fParameterTableWidget->setRowCount(nParameters);
	fParameterTableWidget->setHorizontalHeaderLabels(QStringList() << tr("Parameter") << tr("Value"));
	fParameterTableWidget->verticalHeader()->setVisible(false);
	fParameterTableWidget->setAlternatingRowColors (true);

	G4String searchName = "";
	if (fCurrentComponentName != "")
		searchName = fCurrentComponentName;
	if (fCurrentScorerName != "")
		searchName = fCurrentScorerName;
	if (fCurrentSourceName != "")
		searchName = fCurrentSourceName;

#if GEANT4_VERSION_MAJOR >= 11
    G4StrUtil::to_lower(searchName);
#else
    searchName.toLower();
#endif
	G4String foundParameterName;
	G4int colonPos;
	G4int slashPos;
	G4String foundName;
	G4bool findLineToSelect = true;

	for (G4int iParam = 0; iParam < nParameters; iParam++) {
		G4String parameterName = (*parameterNames)[iParam];
		G4String parameterNameLower = parameterName;
#if GEANT4_VERSION_MAJOR >= 11
   		G4StrUtil::to_lower(parameterNameLower);
#else
    	parameterNameLower.toLower();
#endif

		QTableWidgetItem *paramName = new QTableWidgetItem();
		paramName->setText((QString(parameterName)));
		paramName->setFlags(paramName->flags() ^ Qt::ItemIsEditable);
		fParameterTableWidget->setItem(iParam, 0, paramName);
		G4String parameterValue = (*parameterValues)[iParam];
#if GEANT4_VERSION_MAJOR >= 11
   		G4StrUtil::to_lower(parameterValue);
#else
    	parameterValue.toLower();
#endif
		if (parameterValue.length() > 1)
			parameterValue = parameterValue.substr(1,parameterValue.length()-2);

		if (parameterName.substr(0,1) == "b") {
			QTableWidgetItem *paramValue = new QTableWidgetItem();
			paramValue->setText(QString(""));
			paramValue->setFlags(paramValue->flags() | Qt::ItemIsUserCheckable);
			if ((*parameterValues)[iParam] == "0")
				paramValue->setCheckState(Qt::Unchecked);
			else
				paramValue->setCheckState(Qt::Checked);
			fParameterTableWidget->setItem(iParam, 1, paramValue);
		} else if (fPm->GetPartAfterLastSlash(parameterName) == "color") {
			QComboBox* combo = new QComboBox();
			combo->setProperty("name", QString(parameterName));
			std::vector<G4String> names = fPm->GetColorNames();
			std::vector<G4String>::iterator iter;
			for (iter=names.begin(); iter!=names.end(); iter++) {
				combo->addItem(QString(*iter));
				if (*iter == parameterValue)
					combo->setCurrentIndex(distance(names.begin(),iter));
			}
			connect(combo, SIGNAL(currentIndexChanged(int)),this, SLOT(ParameterComboChanged()));
			fParameterTableWidget->setCellWidget(iParam, 1, combo);
		} else if (fPm->GetPartAfterLastSlash(parameterName) == "material" ||
				   fPm->GetPartAfterLastSlash(parameterName) == "activematerial") {
			QComboBox* combo = new QComboBox();
			combo->setProperty("name", QString(parameterName));
			std::vector<G4String> names = fMm->GetMaterialNames();
			std::vector<G4String>::iterator iter;
			for (iter=names.begin(); iter!=names.end(); iter++) {
				combo->addItem(QString(*iter));
				G4String iterLower = *iter;
#if GEANT4_VERSION_MAJOR >= 11
   				G4StrUtil::to_lower(iterLower);
#else
    			iterLower.toLower();
#endif
				if (iterLower == parameterValue)
					combo->setCurrentIndex(distance(names.begin(),iter));
			}
			connect(combo, SIGNAL(currentIndexChanged(int)),this, SLOT(ParameterComboChanged()));
			fParameterTableWidget->setCellWidget(iParam, 1, combo);
		} else if (fPm->GetPartAfterLastSlash(parameterName) == "component") {
			QComboBox* combo = new QComboBox();
			combo->setProperty("name", QString(parameterName));
			std::vector<G4String> names = fGm->GetComponentNames();
			std::vector<G4String>::iterator iter;
			for (iter=names.begin(); iter!=names.end(); iter++) {
				combo->addItem(QString(*iter));
				if (*iter == parameterValue)
					combo->setCurrentIndex(distance(names.begin(),iter));
			}
			connect(combo, SIGNAL(currentIndexChanged(int)),this, SLOT(ParameterComboChanged()));
			fParameterTableWidget->setCellWidget(iParam, 1, combo);
		} else if (fPm->GetPartAfterLastSlash(parameterName) == "drawingstyle") {
			QComboBox* combo = new QComboBox();
			combo->setProperty("name", QString(parameterName));
			combo->addItem(QString("WireFrame"));
			combo->addItem(QString("FullWireFrame"));
			combo->addItem(QString("Solid"));
			combo->addItem(QString("Cloud"));
			if (parameterValue == "wireframe")
				combo->setCurrentIndex(0);
			else if (parameterValue == "fullwireframe")
				combo->setCurrentIndex(1);
			else if (parameterValue == "cloud")
				combo->setCurrentIndex(3);
			else
				combo->setCurrentIndex(2);
			connect(combo, SIGNAL(currentIndexChanged(int)),this, SLOT(ParameterComboChanged()));
			fParameterTableWidget->setCellWidget(iParam, 1, combo);
		} else if (fPm->GetPartAfterLastSlash(parameterName) == "beamangulardistribution") {
			QComboBox* combo = new QComboBox();
			combo->setProperty("name", QString(parameterName));
			combo->addItem(QString("Gaussian"));
			combo->addItem(QString("Flat"));
			if (parameterValue == "gaussian")
				combo->setCurrentIndex(0);
			else
				combo->setCurrentIndex(1);
			connect(combo, SIGNAL(currentIndexChanged(int)),this, SLOT(ParameterComboChanged()));
			fParameterTableWidget->setCellWidget(iParam, 1, combo);
		} else if (fPm->GetPartAfterLastSlash(parameterName) == "beampositiondistribution") {
			QComboBox* combo = new QComboBox();
			combo->setProperty("name", QString(parameterName));
			combo->addItem(QString("Gaussian"));
			combo->addItem(QString("Flat"));
			if (parameterValue == "gaussian")
				combo->setCurrentIndex(0);
			else
				combo->setCurrentIndex(1);
			connect(combo, SIGNAL(currentIndexChanged(int)),this, SLOT(ParameterComboChanged()));
			fParameterTableWidget->setCellWidget(iParam, 1, combo);
		} else if (fPm->GetPartAfterLastSlash(parameterName) == "beampositioncutoffshape") {
			QComboBox* combo = new QComboBox();
			combo->setProperty("name", QString(parameterName));
			combo->addItem(QString("Ellipse"));
			combo->addItem(QString("Rectangle"));
			if (parameterValue == "ellipse")
				combo->setCurrentIndex(0);
			else
				combo->setCurrentIndex(1);
			connect(combo, SIGNAL(currentIndexChanged(int)),this, SLOT(ParameterComboChanged()));
			fParameterTableWidget->setCellWidget(iParam, 1, combo);
		} else if (fPm->GetPartAfterLastSlash(parameterName) == "beamparticle") {
			QComboBox* combo = new QComboBox();
			combo->setProperty("name", QString(parameterName));
			combo->addItem(QString("proton"));
			combo->addItem(QString("e-"));
			combo->addItem(QString("e+"));
			combo->addItem(QString("gamma"));
			combo->addItem(QString("geantino"));
			combo->addItem(QString("GenericIon(6,12)"));
			if (parameterValue == "proton")
				combo->setCurrentIndex(0);
			else if (parameterValue == "e-")
				combo->setCurrentIndex(1);
			else if (parameterValue == "e+")
				combo->setCurrentIndex(2);
			else if (parameterValue == "gamma")
				combo->setCurrentIndex(3);
			else if (parameterValue == "geantino")
				combo->setCurrentIndex(4);
			else if (parameterValue == "GenericIon(6,12)")
				combo->setCurrentIndex(5);
			connect(combo, SIGNAL(currentIndexChanged(int)),this, SLOT(ParameterComboChanged()));
			fParameterTableWidget->setCellWidget(iParam, 1, combo);
		} else if (fPm->GetPartAfterLastSlash(parameterName) == "projection") {
			QComboBox* combo = new QComboBox();
			combo->setProperty("name", QString(parameterName));
			combo->addItem(QString("Perspective"));
			combo->addItem(QString("Orthogonal"));
			if (parameterValue == "perspective")
				combo->setCurrentIndex(0);
			else
				combo->setCurrentIndex(1);
			connect(combo, SIGNAL(currentIndexChanged(int)),this, SLOT(ParameterComboChanged()));
			fParameterTableWidget->setCellWidget(iParam, 1, combo);
		} else if (fPm->GetPartAfterLastSlash(parameterName) == "geometrymethod") {
			QComboBox* combo = new QComboBox();
			combo->setProperty("name", QString(parameterName));
			combo->addItem(QString("AddBeamlets"));
			combo->addItem(QString("SubtractBeamlets"));
			combo->addItem(QString("OnlyBeamlets"));
			if (parameterValue == "addbeamlets")
				combo->setCurrentIndex(0);
			else if (parameterValue == "subtractbeamlets")
				combo->setCurrentIndex(1);
			else
				combo->setCurrentIndex(2);
			connect(combo, SIGNAL(currentIndexChanged(int)),this, SLOT(ParameterComboChanged()));
			fParameterTableWidget->setCellWidget(iParam, 1, combo);
		} else {
			QTableWidgetItem *paramValue = new QTableWidgetItem();
			paramValue->setText((QString((*parameterValues)[iParam])));
			fParameterTableWidget->setItem(iParam, 1, paramValue);
			if (parameterName.substr(0,6)=="sc:Ge/" && fPm->GetPartAfterLastSlash(parameterName)=="type")
				paramValue->setFlags(paramValue->flags() ^ Qt::ItemIsEditable);
			if (parameterName.substr(0,6)=="sc:Ge/" && fPm->GetPartAfterLastSlash(parameterName)=="parent")
				paramValue->setFlags(paramValue->flags() ^ Qt::ItemIsEditable);
			if (parameterName.substr(0,6)=="sc:So/" && fPm->GetPartAfterLastSlash(parameterName)=="type")
				paramValue->setFlags(paramValue->flags() ^ Qt::ItemIsEditable);
		}

		if (findLineToSelect && searchName!="") {
			colonPos = foundParameterName.find( ":" );
			slashPos = foundParameterName.find_last_of("/");
			foundName = parameterNameLower.substr(colonPos+4,slashPos-colonPos-4);
			if (foundName == searchName) {
				fParameterTableWidget->setCurrentItem(paramName);
				findLineToSelect = false;
			}
		}
	}

	fParameterTablevbox->addWidget(fParameterTableWidget);
	fParameterTableGroupBox->setLayout(fParameterTablevbox);
	
	fParameterEditorWidget->layout()->addWidget(fParameterTableGroupBox);
	connect(fParameterTableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),this, SLOT(ParameterTableWidgetSetItemChanged(QTableWidgetItem *)));

	fCurrentComponentName = "";
	fCurrentScorerName = "";
	fCurrentSourceName = "";
}


void TsQt::ParameterTableWidgetSetItemChanged(QTableWidgetItem * item) {
	QTableWidgetItem* previous = fParameterTableWidget->item(fParameterTableWidget->row(item),0);
	if (previous) {
		G4String paramName = (previous->text().toStdString()).c_str();
		G4String paramValue = (item->text().toStdString()).c_str();
		G4int colonPos = paramName.find( ":" );

		if (paramName.substr(0,1) == "b") {
			if (item->checkState())
				paramValue = "\"True\"";
			else
				paramValue = "\"False\"";
        } else {
			G4String lastPart = fPm->GetPartAfterLastSlash(paramName);
			if (lastPart == "numberofhistoriesinrun") {
				if (fPm->IsInteger(paramValue, 32)) {
					if (G4UIcommand::ConvertToInt(paramValue) < 0) {
						G4cerr << "Value must be a positive integer" << G4endl;
						G4String restoreValue = fPm->GetParameterValueAsString(paramName.substr(0,colonPos-1), paramName.substr(colonPos+1));
						fParameterTableWidget->item(fParameterTableWidget->row(item),1)->setText(QString(restoreValue));
						return;
					}
				} else {
					G4cerr << "Value must be a positive integer" << G4endl;
					G4String restoreValue = fPm->GetParameterValueAsString(paramName.substr(0,colonPos-1), paramName.substr(colonPos+1));
					fParameterTableWidget->item(fParameterTableWidget->row(item),1)->setText(QString(restoreValue));
					return;
				}
			}

			if (lastPart == "xbins" ||
				lastPart == "ybins" ||
				lastPart == "zbins" ||
				lastPart == "rbins" ||
				lastPart == "phibins" ||
				lastPart == "thetabins") {
				if (fPm->IsInteger(paramValue, 32)) {
					if (G4UIcommand::ConvertToInt(paramValue) < 1) {
						G4cerr << "Value must be an integer larger than 0" << G4endl;
						G4String restoreValue = fPm->GetParameterValueAsString(paramName.substr(0,colonPos-1), paramName.substr(colonPos+1));
						fParameterTableWidget->item(fParameterTableWidget->row(item),1)->setText(QString(restoreValue));
						return;
					}
				} else {
					G4cerr << "Value must be an integer larger than 0" << G4endl;
					G4String restoreValue = fPm->GetParameterValueAsString(paramName.substr(0,colonPos-1), paramName.substr(colonPos+1));
					fParameterTableWidget->item(fParameterTableWidget->row(item),1)->setText(QString(restoreValue));
					return;
				}
			}
		}

		// First call to AddParameter has the Test flag set true, so tests but
		// does not actually add a parameter
		fTestSucceeded = true;
		fPm->AddParameter(paramName, paramValue, true);
		if (fTestSucceeded) {
			fParameterTableWidget->blockSignals(true);
			fPm->AddParameter(paramName, paramValue);
			fParameterTableWidget->blockSignals(false);

#if GEANT4_VERSION_MAJOR >= 11
   			G4StrUtil::to_lower(paramName);
#else
    		paramName.toLower();
#endif
			fSqm->UpdateForSpecificParameterChange(paramName.substr(colonPos+1));
			fSqm->UpdateForNewRunOrQtChange();
		} else {
			G4String restoreValue = fPm->GetParameterValueAsString(paramName.substr(0,colonPos-1), paramName.substr(colonPos+1));
			fParameterTableWidget->item(fParameterTableWidget->row(item),1)->setText(QString(restoreValue));
		}
	}
}


void TsQt::ParameterComboChanged() {
	G4String paramName = sender()->property("name").toString().toStdString();
	QComboBox *combo = qobject_cast<QComboBox *>(sender());
	G4String paramValue = combo->currentText().toStdString();
	G4String quotedValue = "\"" + paramValue + "\"";

	fParameterTableWidget->blockSignals(true);
	fPm->AddParameter(paramName, quotedValue);
	fParameterTableWidget->blockSignals(false);

#if GEANT4_VERSION_MAJOR >= 11
   	G4StrUtil::to_lower(paramName);
#else
    paramName.toLower();
#endif
	G4int colonPos = paramName.find( ":" );
	fSqm->UpdateForSpecificParameterChange(paramName.substr(colonPos+1));
	fSqm->UpdateForNewRunOrQtChange();
}


void TsQt::SaveCallback() {
	G4VViewer* viewer = fGrm->GetCurrentViewer();
	G4String parameterStart = "Gr/" + viewer->GetName() + "/";
	fPm->AddParameter("u:" + parameterStart + "Zoom", G4UIcommand::ConvertToString(viewer->GetViewParameters().GetZoomFactor()), false, true);
	fPm->AddParameter("d:" + parameterStart + "Theta", G4UIcommand::ConvertToString(viewer->GetViewParameters().GetViewpointDirection().getTheta() / deg) + " deg", false, true);
	fPm->AddParameter("d:" + parameterStart + "Phi", G4UIcommand::ConvertToString(viewer->GetViewParameters().GetViewpointDirection().getPhi() / deg) + " deg", false, true);
	fPm->AddParameter("d:" + parameterStart + "TargetPointX", G4UIcommand::ConvertToString(viewer->GetViewParameters().GetCurrentTargetPoint().x() / mm) + " mm", false, true);
	fPm->AddParameter("d:" + parameterStart + "TargetPointY", G4UIcommand::ConvertToString(viewer->GetViewParameters().GetCurrentTargetPoint().y() / mm) + " mm", false, true);
	fPm->AddParameter("d:" + parameterStart + "TargetPointZ", G4UIcommand::ConvertToString(viewer->GetViewParameters().GetCurrentTargetPoint().z() / mm) + " mm", false, true);
	fPm->DumpAddedParameters();
}


void TsQt::AddComponentCallback() {
	if (fAddComponentDialog != NULL)
		delete fAddComponentDialog;

	CreateAddComponentDialog();

	fAddComponentDialog->show();
	fAddComponentDialog->raise();
	fAddComponentDialog->activateWindow();
}


void TsQt::CreateAddComponentDialog() {
	fAddComponentDialog = new QDialog();
	fAddComponentDialog->setWindowTitle("TOPAS Add Component");
	fAddComponentDialog->setSizePolicy (QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
	fAddComponentWidget = new QWidget();
	QVBoxLayout* layoutWidget = new QVBoxLayout();
	fAddComponentWidget->setLayout(layoutWidget);

	// Component Name
	QGroupBox* groupBox1 = new QGroupBox();
	groupBox1->setTitle(QString("Set Component Name"));
	fCurrentComponentName = "AddedComp_" + G4UIcommand::ConvertToString(fAddedComponentCounter);
	fAddComponentNameWidget = new QLineEdit(QString(fCurrentComponentName));
	QVBoxLayout *vbox1 = new QVBoxLayout;
	vbox1->addWidget(fAddComponentNameWidget);
	groupBox1->setLayout(vbox1);
	fAddComponentWidget->layout()->addWidget(groupBox1);

	// Parent Component
	QGroupBox* groupBox2 = new QGroupBox();
	groupBox2->setTitle(QString("Select Parent Component"));
	QVBoxLayout *vbox2 = new QVBoxLayout;
	fAddComponentParentWidget = new QComboBox();
	QSizePolicy vPolicy2 = fAddComponentParentWidget->sizePolicy();
	vPolicy2.setVerticalStretch(4);

	std::vector<G4String> componentNames = fGm->GetComponentNames();
	std::vector<G4String>::iterator iter;
	for (iter=componentNames.begin(); iter!=componentNames.end(); iter++) {
		fAddComponentParentWidget->addItem(QString(*iter));
		if (*iter=="world")
			fAddComponentParentWidget->setCurrentIndex(distance(componentNames.begin(),iter));
	}

	vbox2->addWidget(fAddComponentParentWidget);
	groupBox2->setLayout(vbox2);
	fAddComponentWidget->layout()->addWidget(groupBox2);

	// Field
	QGroupBox* groupBox3 = new QGroupBox();
	QVBoxLayout *vbox3 = new QVBoxLayout;
	fAddComponentFieldWidget = new QComboBox();
	QSizePolicy vPolicy3 = fAddComponentFieldWidget->sizePolicy();
	vPolicy3.setVerticalStretch(4);
	fAddComponentFieldWidget->addItem(QString("no field"));

	if (fSqm->GetRunID() == -1) {
		groupBox3->setTitle(QString("Select Field Type"));
		fAddComponentFieldWidget->addItem(QString("DipoleMagnet"));
		fAddComponentFieldWidget->addItem(QString("QuadrupoleMagnet"));
	} else {
		groupBox3->setTitle(QString("Select Field Type not permitted after first Run"));
	}

	fAddComponentFieldWidget->setCurrentIndex(0);
	vbox3->addWidget(fAddComponentFieldWidget);
	groupBox3->setLayout(vbox3);
	fAddComponentWidget->layout()->addWidget(groupBox3);

	// Component Type
	QGroupBox* groupBox = new QGroupBox();
	groupBox->setTitle(QString("Select Component Type"));
	QVBoxLayout *vbox = new QVBoxLayout;
	fAddComponentTableWidget = new QComboBox();
	QSizePolicy vPolicy = fAddComponentTableWidget->sizePolicy();
	vPolicy.setVerticalStretch(4);

	std::vector<G4String> componentTypeNames = fPm->GetComponentTypeNames();
	for (iter=componentTypeNames.begin(); iter!=componentTypeNames.end(); iter++)
		fAddComponentTableWidget->addItem(QString(*iter));
	fAddComponentTableWidget->setCurrentIndex(0);

	vbox->addWidget(fAddComponentTableWidget);
	groupBox->setLayout(vbox);
	fAddComponentWidget->layout()->addWidget(groupBox);

	// Action
	connect(fAddComponentTableWidget, SIGNAL(activated(int)),this, SLOT(AddComponentWidgetSetItemChanged()));

	// Layout details
	QDialog* dial = static_cast<QDialog*> (fAddComponentWidget->parent());
	if (dial) dial->setWindowTitle(QString("TOPAS Add Component"));
	QVBoxLayout* layoutDialog = new QVBoxLayout();
	layoutDialog->addWidget(fAddComponentWidget);
	layoutDialog->setContentsMargins(0,0,0,0);
	fAddComponentDialog->setLayout(layoutDialog);
}


void TsQt::AddComponentWidgetSetItemChanged() {
	fAddComponentWidget->blockSignals(true);
	fAddComponentDialog->hide();
	fAddComponentWidget->hide();

	fCurrentComponentName = fAddComponentNameWidget->text().toStdString();
	fAddedComponentCounter++;
	G4String parentName = fAddComponentParentWidget->currentText().toStdString();
	G4String typeName = fAddComponentTableWidget->currentText().toStdString();
	G4String fieldName = fAddComponentFieldWidget->currentText().toStdString();

	fGm->GetGeometryHub()->AddComponentFromGUI(fPm, fGm, fCurrentComponentName, parentName, typeName, fieldName);

	fAddComponentWidget->blockSignals(false);
	fSqm->UpdateForNewRunOrQtChange();
	UpdateParameterEditor();
}


void TsQt::AddScorerCallback() {
	if (fSqm->GetRunID() != -1) {
		G4cerr << "Scorers may only be added before the first run." << G4endl;
	} else {
		if (fAddScorerDialog != NULL)
			delete fAddScorerDialog;

		CreateAddScorerDialog();

		fAddScorerDialog->show();
		fAddScorerDialog->raise();
		fAddScorerDialog->activateWindow();
	}
}


void TsQt::CreateAddScorerDialog() {
	fAddScorerDialog = new QDialog();
	fAddScorerDialog->setWindowTitle("TOPAS Add Scorer");
	fAddScorerDialog->setSizePolicy (QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
	fAddScorerWidget = new QWidget();
	QVBoxLayout* layoutWidget = new QVBoxLayout();
	fAddScorerWidget->setLayout(layoutWidget);

	// Scorer Name
	QGroupBox* groupBox1 = new QGroupBox();
	groupBox1->setTitle(QString("Set Scorer Name"));
	fCurrentScorerName = "AddedScorer_" + G4UIcommand::ConvertToString(fAddedScorerCounter);
	fAddScorerNameWidget = new QLineEdit(QString(fCurrentScorerName));
	QVBoxLayout *vbox1 = new QVBoxLayout;
	vbox1->addWidget(fAddScorerNameWidget);
	groupBox1->setLayout(vbox1);
	fAddScorerWidget->layout()->addWidget(groupBox1);

	// Component Name
	QGroupBox* groupBox2 = new QGroupBox();
	groupBox2->setTitle(QString("Select Component to Score On"));
	QVBoxLayout *vbox2 = new QVBoxLayout;
	fAddScorerComponentNameWidget = new QComboBox();
	QSizePolicy vPolicy2 = fAddScorerComponentNameWidget->sizePolicy();
	vPolicy2.setVerticalStretch(4);

	std::vector<G4String> componentNames = fGm->GetComponentNames();
	std::vector<G4String>::iterator iter;
	for (iter=componentNames.begin(); iter!=componentNames.end(); iter++)
		fAddScorerComponentNameWidget->addItem(QString(*iter));
	fAddScorerComponentNameWidget->setCurrentIndex(0);

	vbox2->addWidget(fAddScorerComponentNameWidget);
	groupBox2->setLayout(vbox2);
	fAddScorerWidget->layout()->addWidget(groupBox2);

	// Scorer Type
	QGroupBox* groupBox = new QGroupBox();
	groupBox->setTitle(QString("Select Scorer Quantity"));
	QVBoxLayout *vbox = new QVBoxLayout;
	fAddScorerQuantityWidget = new QComboBox();
	QSizePolicy vPolicy = fAddScorerQuantityWidget->sizePolicy();
	vPolicy.setVerticalStretch(4);

	std::vector<G4String> scorerQuantityNames = fPm->GetScorerQuantityNames();
	for (iter=scorerQuantityNames.begin(); iter!=scorerQuantityNames.end(); iter++)
		if ((*iter != "ambientdoseequivalent") &&
			(*iter != "protonlet") &&
			(*iter != "protonlet_denominator") &&
			(*iter != "pet"))
			fAddScorerQuantityWidget->addItem(QString(*iter));
	fAddScorerQuantityWidget->setCurrentIndex(0);

	vbox->addWidget(fAddScorerQuantityWidget);
	groupBox->setLayout(vbox);
	fAddScorerWidget->layout()->addWidget(groupBox);

	// Action
	connect(fAddScorerQuantityWidget, SIGNAL(activated(int)),this, SLOT(AddScorerWidgetSetItemChanged()));

	// Layout details
	QDialog* dial = static_cast<QDialog*> (fAddScorerWidget->parent());
	if (dial) dial->setWindowTitle(QString("TOPAS Add Scorer"));
	QVBoxLayout* layoutDialog = new QVBoxLayout();
	layoutDialog->addWidget(fAddScorerWidget);
	layoutDialog->setContentsMargins(0,0,0,0);
	fAddScorerDialog->setLayout(layoutDialog);
}


void TsQt::AddScorerWidgetSetItemChanged() {
	fAddScorerWidget->blockSignals(true);
	fAddScorerDialog->hide();
	fAddScorerWidget->hide();

	fCurrentScorerName = fAddScorerNameWidget->text().toStdString();
	fAddedScorerCounter++;
	G4String componentName = fAddScorerComponentNameWidget->currentText().toStdString();
	G4String quantityName = fAddScorerQuantityWidget->currentText().toStdString();

	fScm->GetScoringHub()->AddScorerFromGUI(fCurrentScorerName, componentName, quantityName);

	fAddScorerWidget->blockSignals(false);
	fSqm->UpdateForNewRunOrQtChange();
	UpdateParameterEditor();
}


void TsQt::AddSourceCallback() {
	if (fSqm->GetRunID() != -1) {
		G4cerr << "Particle Sources may only be added before the first run." << G4endl;
	} else {
		if (fAddSourceDialog != NULL)
			delete fAddSourceDialog;

		CreateAddSourceDialog();

		fAddSourceDialog->show();
		fAddSourceDialog->raise();
		fAddSourceDialog->activateWindow();
	}
}


void TsQt::CreateAddSourceDialog() {
	fAddSourceDialog = new QDialog();
	fAddSourceDialog->setWindowTitle("TOPAS Add Source");
	fAddSourceDialog->setSizePolicy (QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
	fAddSourceWidget = new QWidget();
	QVBoxLayout* layoutWidget = new QVBoxLayout();
	fAddSourceWidget->setLayout(layoutWidget);

	// Source Name
	QGroupBox* groupBox1 = new QGroupBox();
	groupBox1->setTitle(QString("Set Source Name"));
	fCurrentSourceName = "AddedSource_" + G4UIcommand::ConvertToString(fAddedSourceCounter);
	fAddSourceNameWidget = new QLineEdit(QString(fCurrentSourceName));
	QVBoxLayout *vbox1 = new QVBoxLayout;
	vbox1->addWidget(fAddSourceNameWidget);
	groupBox1->setLayout(vbox1);
	fAddSourceWidget->layout()->addWidget(groupBox1);

	// Component Name
	QGroupBox* groupBox2 = new QGroupBox();
	groupBox2->setTitle(QString("Select Component to Hold Source"));
	QVBoxLayout *vbox2 = new QVBoxLayout;
	fAddSourceComponentNameWidget = new QComboBox();
	QSizePolicy vPolicy2 = fAddSourceComponentNameWidget->sizePolicy();
	vPolicy2.setVerticalStretch(4);

	std::vector<G4String> componentNames = fGm->GetComponentNames();
	std::vector<G4String>::iterator iter;
	for (iter=componentNames.begin(); iter!=componentNames.end(); iter++) {
		fAddSourceComponentNameWidget->addItem(QString(*iter));
		if (*iter=="world")
			fAddSourceComponentNameWidget->setCurrentIndex(distance(componentNames.begin(),iter));
	}

	vbox2->addWidget(fAddSourceComponentNameWidget);
	groupBox2->setLayout(vbox2);
	fAddSourceWidget->layout()->addWidget(groupBox2);

	// Source Type
	QGroupBox* groupBox = new QGroupBox();
	groupBox->setTitle(QString("Select Source Type"));
	QVBoxLayout *vbox = new QVBoxLayout;
	fAddSourceTypeWidget = new QComboBox();
	QSizePolicy vPolicy = fAddSourceTypeWidget->sizePolicy();
	vPolicy.setVerticalStretch(4);

	//std::vector<G4String> scorerTypeNames = fPm->GetSourceTypeNames();
	//for (iter=scorerTypeNames.begin(); iter!=scorerTypeNames.end(); iter++)
	//	fAddSourceTypeWidget->addItem(QString(*iter));

	fAddSourceTypeWidget->addItem(QString("Beam"));
	fAddSourceTypeWidget->addItem(QString("Isotropic"));
	fAddSourceTypeWidget->addItem(QString("Emittance"));
	fAddSourceTypeWidget->addItem(QString("PhaseSpace"));
	fAddSourceTypeWidget->setCurrentIndex(0);

	vbox->addWidget(fAddSourceTypeWidget);
	groupBox->setLayout(vbox);
	fAddSourceWidget->layout()->addWidget(groupBox);

	// Action
	connect(fAddSourceTypeWidget, SIGNAL(activated(int)),this, SLOT(AddSourceWidgetSetItemChanged()));

	// Layout details
	QDialog* dial = static_cast<QDialog*> (fAddSourceWidget->parent());
	if (dial) dial->setWindowTitle(QString("TOPAS Add Source"));
	QVBoxLayout* layoutDialog = new QVBoxLayout();
	layoutDialog->addWidget(fAddSourceWidget);
	layoutDialog->setContentsMargins(0,0,0,0);
	fAddSourceDialog->setLayout(layoutDialog);
}


void TsQt::AddSourceWidgetSetItemChanged() {
	fAddSourceWidget->blockSignals(true);
	fAddSourceDialog->hide();
	fAddSourceWidget->hide();

	fCurrentSourceName = fAddSourceNameWidget->text().toStdString();
	fAddedSourceCounter++;
	G4String componentName = fAddSourceComponentNameWidget->currentText().toStdString();
	G4String typeName = fAddSourceTypeWidget->currentText().toStdString();

	fSom->AddSourceFromGUI(fCurrentSourceName, componentName, typeName);

	fAddSourceWidget->blockSignals(false);
	fSqm->UpdateForNewRunOrQtChange();
	UpdateParameterEditor();
}

#endif
