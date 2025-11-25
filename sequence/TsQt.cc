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
#include <string>
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
#include <qtreewidget.h>
#include <qheaderview.h>
#include <qapplication.h>
#include <qlist.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include <qtoolbutton.h>
#include <qscrollbar.h>
#include <qcolor.h>
#include <qbrush.h>
#include <qtoolbutton.h>
#include <qscrollbar.h>
#include <qmenu.h>
#include <qinputdialog.h>
#include <map>
#include <set>
#include <qpixmap.h>

TsQt::TsQt(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsSequenceManager* sqM,
		   TsGraphicsManager* grM, TsSourceManager* soM) :
fPm(pM), fEm(eM), fMm(mM), fGm(gM), fScm(scM), fSqm(sqM), fGrm(grM), fSom(soM), fTestSucceeded(true), fParameterEditorDialog(0),
fParameterTableWidget(0),
fAddComponentDialog(0), fCurrentComponentName(""), fAddedComponentCounter(0),
fAddScorerDialog(0), fCurrentScorerName(""), fAddedScorerCounter(0),
fAddSourceDialog(0), fCurrentSourceName(""), fAddedSourceCounter(0),
fShowReadOnlyNoteMessage(true)
{
	fParameterEditorWidget = new QWidget();
	QVBoxLayout* layoutWidget = new QVBoxLayout();
	fParameterEditorWidget->setLayout(layoutWidget);

	fParameterTableGroupBox = new QGroupBox();
	fParameterTablevbox = new QVBoxLayout;

	fUIQt = static_cast<G4UIQt*> (G4UImanager::GetUIpointer()->GetG4UIWindow());
	fUIQt->GetUserInterfaceWidget()->setWindowTitle("");

	// Set window and tab icon
	QPixmap logoPixmap("/Applications/TOPAS/OpenTOPAS/graphics/TOPASLogo.png");
	if (!logoPixmap.isNull()) {
		QSize targetSize(286,192);
		QIcon logoIcon(logoPixmap.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		fUIQt->GetMainWindow()->setWindowIcon(logoIcon);
		int tabIndex = fUIQt->GetUITabWidget()->addTab(fParameterEditorWidget,"Parameter Control");
		fUIQt->GetUITabWidget()->setCurrentIndex(tabIndex);
	} else {
		fUIQt->GetUITabWidget()->addTab(fParameterEditorWidget,"Parameter Control");
	}

	// Add our own control widget
	// Already added above

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
	QIcon runIcon;
	QPixmap runPixmap("/Applications/TOPAS/OpenTOPAS/graphics/TOPASLogo.png");
	if (!runPixmap.isNull()) {
		runIcon = QIcon(runPixmap.scaled(QSize(32,32), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		toolbar->setIconSize(QSize(32,32));
	}
	QAction* runAction = runIcon.isNull() ? toolbar->addAction(QString("Run"), runSignalMapper, SLOT(map()))
										   : toolbar->addAction(runIcon, QString(""), runSignalMapper, SLOT(map()));
	connect(runSignalMapper, SIGNAL(mapped(int)),this, SLOT(RunCallback()));
	int runIntVP = 0;
	runSignalMapper->setMapping(runAction, runIntVP);

	QSignalMapper* printSignalMapper = new QSignalMapper(this);
	toolbar->addSeparator();
	QAction* printAction = toolbar->addAction(QString("PDF"), printSignalMapper, SLOT(map()));
	connect(printSignalMapper, SIGNAL(mapped(int)),this, SLOT(PrintCallback()));
	int printIntVP = 0;
	printSignalMapper->setMapping(printAction, printIntVP);

	toolbar->addSeparator();
	QToolButton* expandCollapseButton = new QToolButton();
	expandCollapseButton->setText("Collapse All");
	expandCollapseButton->setCheckable(true);
	connect(expandCollapseButton, &QToolButton::clicked, this, [=](bool checked){
		if (checked) {
			expandCollapseButton->setText("Expand All");
			for (int i = 0; i < fParameterTableWidget->topLevelItemCount(); ++i)
				fParameterTableWidget->collapseItem(fParameterTableWidget->topLevelItem(i));
		} else {
			expandCollapseButton->setText("Collapse All");
			for (int i = 0; i < fParameterTableWidget->topLevelItemCount(); ++i)
				fParameterTableWidget->expandItem(fParameterTableWidget->topLevelItem(i));
		}
	});
	toolbar->addWidget(expandCollapseButton);

	fParameterTablevbox->addWidget(toolbar);
	// Filter bar
	QHBoxLayout* filterLayout = new QHBoxLayout();
	QLabel* filterLabel = new QLabel(QString("Filter:"));
	fFilterLineEdit = new QLineEdit();
	fFilterLineEdit->setPlaceholderText(QString("Type to filter parameters by name or value"));
	QPushButton* clearFilterButton = new QPushButton(QString("Clear"));
	filterLayout->addWidget(filterLabel);
	filterLayout->addWidget(fFilterLineEdit);
	filterLayout->addWidget(clearFilterButton);
	QWidget* filterWidget = new QWidget();
	filterWidget->setLayout(filterLayout);
	fParameterTablevbox->addWidget(filterWidget);
	connect(fFilterLineEdit, SIGNAL(textChanged(QString)), this, SLOT(UpdateParameterEditor()));
	connect(clearFilterButton, SIGNAL(clicked()), fFilterLineEdit, SLOT(clear()));

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
	if (fParameterTableWidget) {
		fSavedScrollPosition = fParameterTableWidget->verticalScrollBar()->value();
	}

	std::map<G4String, bool> previousCategoryExpansion;
	std::map<G4String, bool> previousComponentExpansion;
	if (fParameterTableWidget) {
		for (int i = 0; i < fParameterTableWidget->topLevelItemCount(); ++i) {
			QTreeWidgetItem* categoryItem = fParameterTableWidget->topLevelItem(i);
			G4String catName = categoryItem->text(0).toStdString().c_str();
			previousCategoryExpansion[catName] = categoryItem->isExpanded();
			for (int j = 0; j < categoryItem->childCount(); ++j) {
				QTreeWidgetItem* compItem = categoryItem->child(j);
				G4String compKey = catName + "/" + compItem->text(0).toStdString().c_str();
				previousComponentExpansion[compKey] = compItem->isExpanded();
			}
		}
		delete fParameterTableWidget;
	}
	
	fParameterTableWidget = new QTreeWidget();
	QSizePolicy vPolicy = fParameterTableWidget->sizePolicy();
	vPolicy.setVerticalStretch(4);
	
	std::vector<G4String>* parameterNames = new std::vector<G4String>;
	std::vector<G4String>* parameterValues = new std::vector<G4String>;
	fPm->GetChangeableParameters(parameterNames, parameterValues);

	QString filterText = "";
	if (fFilterLineEdit)
		filterText = fFilterLineEdit->text().trimmed().toLower();

	G4int nParameters = parameterNames->size();

	fParameterTableWidget->setColumnCount(2);
	fParameterTableWidget->setColumnWidth(0,260);
	fParameterTableWidget->setColumnWidth(1,110);
	fParameterTableWidget->setMinimumWidth(380);
	fParameterTableWidget->setHeaderLabels(QStringList() << tr("Parameter") << tr("Value"));
	fParameterTableWidget->setRootIsDecorated(true);
	fParameterTableWidget->setAlternatingRowColors(true);
	fParameterTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(fParameterTableWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowParameterContextMenu(const QPoint&)));

	G4String searchName = "";
	if (fCurrentComponentName != "")
		searchName = fCurrentComponentName;
	if (fCurrentScorerName != "")
		searchName = fCurrentScorerName;
	if (fCurrentSourceName != "")
		searchName = fCurrentSourceName;

    G4StrUtil::to_lower(searchName);
	G4bool findLineToSelect = true;

	std::map<G4String, QTreeWidgetItem*> categoryItems;
	std::map<G4String, QTreeWidgetItem*> componentItems;
	std::set<G4String> componentsWithParentRow;

	auto getCategoryItem = [&](const G4String& categoryName) {
		std::map<G4String, QTreeWidgetItem*>::iterator iter = categoryItems.find(categoryName);
		if (iter != categoryItems.end())
			return iter->second;

		QTreeWidgetItem* catItem = new QTreeWidgetItem();
		catItem->setText(0, QString(categoryName));
		catItem->setFlags(catItem->flags() ^ Qt::ItemIsEditable);
		bool expanded = false;
		std::map<G4String, bool>::const_iterator expandIter = previousCategoryExpansion.find(categoryName);
		if (expandIter != previousCategoryExpansion.end())
			expanded = expandIter->second;
		catItem->setExpanded(expanded);
		categoryItems[categoryName] = catItem;
		return catItem;
	};

	auto getGroupItem = [&](const G4String& categoryName, const G4String& groupName) {
		G4String key = categoryName + "/" + groupName;
		std::map<G4String, QTreeWidgetItem*>::iterator iter = componentItems.find(key);
		if (iter != componentItems.end())
			return iter->second;

		QTreeWidgetItem* categoryItem = getCategoryItem(categoryName);
		QTreeWidgetItem* groupItem = new QTreeWidgetItem(categoryItem);
		groupItem->setText(0, QString(groupName));
		groupItem->setFlags(groupItem->flags() ^ Qt::ItemIsEditable);
		bool expanded = false;
		std::map<G4String, bool>::const_iterator expandIter = previousComponentExpansion.find(key);
		if (expandIter != previousComponentExpansion.end())
			expanded = expandIter->second;
			groupItem->setExpanded(expanded);
		componentItems[key] = groupItem;
		return groupItem;
	};

	for (G4int iParam = 0; iParam < nParameters; iParam++) {
		G4String parameterName = (*parameterNames)[iParam];
		G4String parameterNameLower = parameterName;
   		G4StrUtil::to_lower(parameterNameLower);

		QString qNameLower = QString(parameterNameLower);
		QString qValueLower = QString((*parameterValues)[iParam]).toLower();
		if (!filterText.isEmpty()) {
			if (!qNameLower.contains(filterText) && !qValueLower.contains(filterText))
				continue;
		}

		G4int colonPos = parameterName.find(":");
		G4String categoryName = "Other";
		G4String groupName = "General";
		G4String displayName = fPm->GetPartAfterLastSlash(parameterName);
		G4String fullPathLower = parameterNameLower;
		if (colonPos != G4int(std::string::npos)) {
			G4String afterColon = parameterName.substr(colonPos + 1);
			size_t firstSlash = afterColon.find("/");
			if (firstSlash != std::string::npos) {
				G4String prefix = afterColon.substr(0, firstSlash+1);
				if (prefix == "Ge/")
					categoryName = "Geometries";
				else if (prefix == "Sc/")
					categoryName = "Scorers";
				else if (prefix == "So/")
					categoryName = "Sources";
				size_t secondSlash = afterColon.find("/", firstSlash + 1);
				if (secondSlash != std::string::npos) {
					groupName = afterColon.substr(firstSlash + 1, secondSlash - firstSlash - 1);
					if (categoryName == "Geometries" && fPm->GetPartAfterLastSlash(parameterNameLower) == "parent")
						componentsWithParentRow.insert(groupName);
				}
			}
		}

		QTreeWidgetItem* groupItem = getGroupItem(categoryName, groupName);

		QTreeWidgetItem* paramItem = new QTreeWidgetItem(groupItem);
		paramItem->setText(0, QString(displayName));
		paramItem->setToolTip(0, QString(parameterName));
		paramItem->setData(0, Qt::UserRole, QString(parameterName));
		paramItem->setFlags(paramItem->flags() | Qt::ItemIsEnabled | Qt::ItemIsSelectable);

		G4String parameterValue = (*parameterValues)[iParam];
   		G4StrUtil::to_lower(parameterValue);
		if (parameterValue.length() > 1)
			parameterValue = parameterValue.substr(1,parameterValue.length()-2);

		if (parameterName.substr(0,1) == "b") {
			paramItem->setFlags(paramItem->flags() | Qt::ItemIsUserCheckable);
			if ((*parameterValues)[iParam] == "0")
				paramItem->setCheckState(1, Qt::Unchecked);
			else
				paramItem->setCheckState(1, Qt::Checked);
			paramItem->setText(1, QString(""));
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
			fParameterTableWidget->setItemWidget(paramItem, 1, combo);
			paramItem->setFlags(paramItem->flags() ^ Qt::ItemIsEditable);
		} else if (fPm->GetPartAfterLastSlash(parameterName) == "material" ||
				   fPm->GetPartAfterLastSlash(parameterName) == "activematerial") {
			QComboBox* combo = new QComboBox();
			combo->setProperty("name", QString(parameterName));
			std::vector<G4String> names = fMm->GetMaterialNames();
			std::vector<G4String>::iterator iter;
			for (iter=names.begin(); iter!=names.end(); iter++) {
				combo->addItem(QString(*iter));
				G4String iterLower = *iter;
   				G4StrUtil::to_lower(iterLower);
				if (iterLower == parameterValue)
					combo->setCurrentIndex(distance(names.begin(),iter));
			}
			connect(combo, SIGNAL(currentIndexChanged(int)),this, SLOT(ParameterComboChanged()));
			fParameterTableWidget->setItemWidget(paramItem, 1, combo);
			paramItem->setFlags(paramItem->flags() ^ Qt::ItemIsEditable);
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
			fParameterTableWidget->setItemWidget(paramItem, 1, combo);
			paramItem->setFlags(paramItem->flags() ^ Qt::ItemIsEditable);
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
			fParameterTableWidget->setItemWidget(paramItem, 1, combo);
			paramItem->setFlags(paramItem->flags() ^ Qt::ItemIsEditable);
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
			fParameterTableWidget->setItemWidget(paramItem, 1, combo);
			paramItem->setFlags(paramItem->flags() ^ Qt::ItemIsEditable);
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
			fParameterTableWidget->setItemWidget(paramItem, 1, combo);
			paramItem->setFlags(paramItem->flags() ^ Qt::ItemIsEditable);
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
			fParameterTableWidget->setItemWidget(paramItem, 1, combo);
			paramItem->setFlags(paramItem->flags() ^ Qt::ItemIsEditable);
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
			fParameterTableWidget->setItemWidget(paramItem, 1, combo);
			paramItem->setFlags(paramItem->flags() ^ Qt::ItemIsEditable);
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
			fParameterTableWidget->setItemWidget(paramItem, 1, combo);
			paramItem->setFlags(paramItem->flags() ^ Qt::ItemIsEditable);
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
			fParameterTableWidget->setItemWidget(paramItem, 1, combo);
			paramItem->setFlags(paramItem->flags() ^ Qt::ItemIsEditable);
		} else {
			paramItem->setText(1, QString((*parameterValues)[iParam]));
			paramItem->setFlags(paramItem->flags() | Qt::ItemIsEditable);
			bool locked = false;
			if ((parameterName.substr(0,4)=="s:Ge/" || parameterName.substr(0,6)=="sc:Ge/") && fPm->GetPartAfterLastSlash(parameterName)=="type") {
				paramItem->setFlags(paramItem->flags() ^ Qt::ItemIsEditable);
				locked = true;
			}
			if ((parameterName.substr(0,4)=="s:Ge/" || parameterName.substr(0,6)=="sc:Ge/") && fPm->GetPartAfterLastSlash(parameterName)=="parent") {
				paramItem->setFlags(paramItem->flags() ^ Qt::ItemIsEditable);
				locked = true;
			}
			if (parameterName.substr(0,6)=="sc:So/" && fPm->GetPartAfterLastSlash(parameterName)=="type") {
				paramItem->setFlags(paramItem->flags() ^ Qt::ItemIsEditable);
				locked = true;
			}
			G4String lastPartLower = fPm->GetPartAfterLastSlash(parameterNameLower);
			if (lastPartLower == "numberofhistoriesinrun")
				paramItem->setToolTip(1, QString("Must be a positive integer"));
			else if (lastPartLower=="xbins" || lastPartLower=="ybins" || lastPartLower=="zbins" ||
					 lastPartLower=="rbins" || lastPartLower=="phibins" || lastPartLower=="thetabins")
				paramItem->setToolTip(1, QString("Must be an integer > 0"));
			if (locked) {
				QFont boldFont = paramItem->font(0);
				boldFont.setBold(true);
				paramItem->setFont(0, boldFont);
				paramItem->setFont(1, boldFont);
			}
		}

		// Bold all non-editable items (including combos above) except user-checkable rows
		if (!(paramItem->flags() & Qt::ItemIsEditable) &&
			!(paramItem->flags() & Qt::ItemIsUserCheckable)) {
			QFont boldFont = paramItem->font(0);
			boldFont.setBold(true);
			paramItem->setFont(0, boldFont);
			paramItem->setFont(1, boldFont);
		}

		if (findLineToSelect && searchName!="") {
			size_t firstSlash = parameterNameLower.find("/", colonPos+1);
			size_t secondSlash = parameterNameLower.find("/", firstSlash+1);
			if (firstSlash != std::string::npos && secondSlash != std::string::npos) {
				G4String foundName = parameterNameLower.substr(firstSlash+1, secondSlash-firstSlash-1);
				if (foundName == searchName) {
					fParameterTableWidget->setCurrentItem(paramItem);
					if (groupItem) {
						groupItem->setExpanded(true);
						if (groupItem->parent())
							groupItem->parent()->setExpanded(true);
					}
					findLineToSelect = false;
				}
			}
		}
	}

	// Ensure geometry parent is displayed even if not changeable/editable
	for (std::map<G4String, QTreeWidgetItem*>::const_iterator iter = componentItems.begin(); iter != componentItems.end(); ++iter) {
		G4String key = iter->first;
		G4String prefix = "Geometries/";
		if (key.substr(0, prefix.length()) != prefix)
			continue;
		G4String compName = key.substr(prefix.length());
		if (componentsWithParentRow.find(compName) != componentsWithParentRow.end())
			continue;
		G4String parentParam = "Ge/" + compName + "/Parent";
		if (fPm->ParameterExists(parentParam)) {
			G4String parentValue = fPm->GetStringParameter(parentParam);
			QTreeWidgetItem* parentItem = new QTreeWidgetItem(iter->second);
			parentItem->setText(0, QString("parent"));
			parentItem->setText(1, QString("\"") + QString(parentValue) + QString("\""));
			parentItem->setData(0, Qt::UserRole, QString("s:Ge/") + QString(compName) + QString("/Parent"));
			parentItem->setFlags((parentItem->flags() | Qt::ItemIsEnabled | Qt::ItemIsSelectable) ^ Qt::ItemIsEditable);
			QFont boldFont = parentItem->font(0);
			boldFont.setBold(true);
			parentItem->setFont(0, boldFont);
			parentItem->setFont(1, boldFont);
		}
	}

	// Add read-only Quantity and Component for Scorers if missing; add read-only Type for Sources if missing
	for (std::map<G4String, QTreeWidgetItem*>::const_iterator iter = componentItems.begin(); iter != componentItems.end(); ++iter) {
		G4String key = iter->first;
		G4String prefixSc = "Scorers/";
		G4String prefixSo = "Sources/";

		if (key.substr(0, prefixSc.length()) == prefixSc) {
			G4String scName = key.substr(prefixSc.length());

			G4String qtyParam = "Sc/" + scName + "/Quantity";
			if (fPm->ParameterExists(qtyParam)) {
				QTreeWidgetItem* qtyItem = new QTreeWidgetItem(iter->second);
				qtyItem->setText(0, QString("quantity"));
				qtyItem->setText(1, QString("\"") + QString(fPm->GetStringParameter(qtyParam)) + QString("\""));
				qtyItem->setData(0, Qt::UserRole, QString("s:") + QString(qtyParam));
				qtyItem->setFlags((qtyItem->flags() | Qt::ItemIsEnabled | Qt::ItemIsSelectable) ^ Qt::ItemIsEditable);
				QFont boldFont = qtyItem->font(0);
				boldFont.setBold(true);
				qtyItem->setFont(0, boldFont);
				qtyItem->setFont(1, boldFont);
			}

			G4String compParam = "Sc/" + scName + "/Component";
			G4String surfParam = "Sc/" + scName + "/Surface";
			if (fPm->ParameterExists(compParam)) {
				QTreeWidgetItem* compItem = new QTreeWidgetItem(iter->second);
				compItem->setText(0, QString("component"));
				compItem->setText(1, QString("\"") + QString(fPm->GetStringParameter(compParam)) + QString("\""));
				compItem->setData(0, Qt::UserRole, QString("s:") + QString(compParam));
				compItem->setFlags((compItem->flags() | Qt::ItemIsEnabled | Qt::ItemIsSelectable) ^ Qt::ItemIsEditable);
				QFont boldFont = compItem->font(0);
				boldFont.setBold(true);
				compItem->setFont(0, boldFont);
				compItem->setFont(1, boldFont);
			} else if (fPm->ParameterExists(surfParam)) {
				QTreeWidgetItem* compItem = new QTreeWidgetItem(iter->second);
				compItem->setText(0, QString("surface"));
				compItem->setText(1, QString("\"") + QString(fPm->GetStringParameter(surfParam)) + QString("\""));
				compItem->setData(0, Qt::UserRole, QString("s:") + QString(surfParam));
				compItem->setFlags((compItem->flags() | Qt::ItemIsEnabled | Qt::ItemIsSelectable) ^ Qt::ItemIsEditable);
				QFont boldFont = compItem->font(0);
				boldFont.setBold(true);
				compItem->setFont(0, boldFont);
				compItem->setFont(1, boldFont);
			}
		} else if (key.substr(0, prefixSo.length()) == prefixSo) {
			G4String soName = key.substr(prefixSo.length());
			G4String typeParam = "So/" + soName + "/Type";
			if (fPm->ParameterExists(typeParam)) {
				QTreeWidgetItem* typeItem = new QTreeWidgetItem(iter->second);
				typeItem->setText(0, QString("type"));
				typeItem->setText(1, QString("\"") + QString(fPm->GetStringParameter(typeParam)) + QString("\""));
				typeItem->setData(0, Qt::UserRole, QString("s:") + QString(typeParam));
				typeItem->setFlags((typeItem->flags() | Qt::ItemIsEnabled | Qt::ItemIsSelectable) ^ Qt::ItemIsEditable);
				QFont boldFont = typeItem->font(0);
				boldFont.setBold(true);
				typeItem->setFont(0, boldFont);
				typeItem->setFont(1, boldFont);
			}
		}
	}

	fParameterTablevbox->addWidget(fParameterTableWidget);
	fParameterTableGroupBox->setLayout(fParameterTablevbox);
	
	fParameterEditorWidget->layout()->addWidget(fParameterTableGroupBox);
	connect(fParameterTableWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)),this, SLOT(ParameterTableWidgetSetItemChanged(QTreeWidgetItem*,int)));

	// Enforce category ordering Geometries -> Scorers -> Sources -> Other
	QStringList catOrder;
	catOrder << "Geometries" << "Scorers" << "Sources" << "Other";
	for (int i = 0; i < catOrder.size(); ++i) {
		QTreeWidgetItem* catItem = categoryItems[catOrder[i].toStdString().c_str()];
		if (catItem) {
			fParameterTableWidget->addTopLevelItem(catItem);
		}
	}
	// Add any remaining categories not in the fixed order
	for (std::map<G4String, QTreeWidgetItem*>::const_iterator iter = categoryItems.begin(); iter != categoryItems.end(); ++iter) {
		QString catName = QString(iter->first);
		if (!catOrder.contains(catName))
			fParameterTableWidget->addTopLevelItem(iter->second);
	}

fCurrentComponentName = "";
fCurrentScorerName = "";
fCurrentSourceName = "";

	if (fParameterTableWidget)
		fParameterTableWidget->verticalScrollBar()->setValue(fSavedScrollPosition);
}


void TsQt::ShowParameterContextMenu(const QPoint& pos) {
	QTreeWidgetItem* item = fParameterTableWidget->itemAt(pos);
	if (!item)
		return;

	QTreeWidgetItem* compItem = item;
	QTreeWidgetItem* catItem = item->parent();
	if (!catItem)
		return;
	if (compItem->parent()) {
		compItem = compItem->parent();
		catItem = compItem->parent();
	}
	if (!catItem)
		return;

	G4String category = catItem->text(0).toStdString();
	G4String compName = compItem->text(0).toStdString();

	QMenu menu(fParameterEditorWidget);
	if (category == "Geometries") {
		menu.addAction("Duplicate Geometry", [=]() { DoDuplicateGeometry(compName); });
	} else if (category == "Scorers") {
		menu.addAction("Duplicate Scorer", [=]() { DoDuplicateScorer(compName); });
	} else if (category == "Sources") {
		menu.addAction("Duplicate Source", [=]() { DoDuplicateSource(compName); });
	}
	if (!menu.isEmpty())
		menu.exec(fParameterTableWidget->viewport()->mapToGlobal(pos));
}

void TsQt::DuplicateParameters(const G4String& categoryCode, const G4String& oldName, const G4String& newName) {
	std::vector<G4String>* parameterNames = new std::vector<G4String>;
	std::vector<G4String>* parameterValues = new std::vector<G4String>;
	fPm->GetChangeableParameters(parameterNames, parameterValues);

	G4String needle = "/" + oldName + "/";
	for (size_t i = 0; i < parameterNames->size(); ++i) {
		G4String pname = (*parameterNames)[i];
		size_t colonPos = pname.find(":");
		size_t firstSlash = pname.find("/", colonPos+1);
		if (firstSlash == std::string::npos)
			continue;
		G4String prefix = pname.substr(colonPos+1, firstSlash-colonPos-1);
		if (prefix != categoryCode)
			continue;
		size_t pos = pname.find(needle);
		if (pos == std::string::npos)
			continue;
		G4String newPname = pname;
		newPname.replace(pos+1, oldName.length(), newName);
		G4String newVal = (*parameterValues)[i];
		// Normalize any boolean (type starts with 'b') to "True"/"False"
		if (!newPname.empty() && newPname[0] == 'b') {
			if (newVal == "0")
				newVal = "\"False\"";
			else if (newVal == "1")
				newVal = "\"True\"";
		}
		fPm->AddParameter(newPname, newVal);
	}
	delete parameterNames;
	delete parameterValues;
}


G4bool TsQt::NameExistsInList(const std::vector<G4String>& list, const G4String& name) {
	for (size_t i=0; i<list.size(); ++i)
		if (list[i] == name)
			return true;
	return false;
}


void TsQt::DuplicateGeometryCallback() {
	std::vector<G4String> componentNames = fGm->GetComponentNames();
	if (componentNames.empty())
		return;

	bool ok = false;
	QStringList nameList;
	for (size_t i=0;i<componentNames.size();++i) nameList << QString(componentNames[i]);
	QString selected = QInputDialog::getItem(fParameterEditorWidget, "Duplicate Geometry", "Select geometry to duplicate:", nameList, 0, false, &ok);
	if (!ok || selected.isEmpty())
		return;
	DoDuplicateGeometry(selected.toStdString());
}


void TsQt::DoDuplicateGeometry(const G4String& oldName) {
	std::vector<G4String> componentNames = fGm->GetComponentNames();
	bool ok = false;
	QString newName = QInputDialog::getText(fParameterEditorWidget, "New Geometry Name", "Enter name for duplicate:", QLineEdit::Normal, QString(oldName.c_str()) + "_copy", &ok);
	if (!ok || newName.isEmpty())
		return;
	if (NameExistsInList(componentNames, newName.toStdString())) {
		QMessageBox::warning(fParameterEditorWidget, "Duplicate Geometry", "A geometry with that name already exists.");
		return;
	}

	G4String newNameStr = newName.toStdString();
	G4String typeName = fPm->GetStringParameter("Ge/" + oldName + "/Type");
	G4String parentName = fPm->GetStringParameter("Ge/" + oldName + "/Parent");
	G4String noField("no field");
	fGm->GetGeometryHub()->AddComponentFromGUI(fPm, fGm, newNameStr, parentName, typeName, noField);
	DuplicateParameters("Ge", oldName, newNameStr);
	UpdateParameterEditor();
}


void TsQt::DuplicateScorerCallback() {
	if (fSqm->GetRunID() != -1) {
		QMessageBox::warning(fParameterEditorWidget, "Duplicate Scorer", "Scorers may only be duplicated before the first run.");
		return;
	}

	std::vector<G4String> scorerNames = fScm->GetScoringHub()->GetScorerNames();
	if (scorerNames.empty())
		return;

	bool ok = false;
	QStringList nameList;
	for (size_t i=0;i<scorerNames.size();++i) nameList << QString(scorerNames[i]);
	QString selected = QInputDialog::getItem(fParameterEditorWidget, "Duplicate Scorer", "Select scorer to duplicate:", nameList, 0, false, &ok);
	if (!ok || selected.isEmpty())
		return;
	DoDuplicateScorer(selected.toStdString());
}

void TsQt::DoDuplicateScorer(const G4String& oldName) {
	std::vector<G4String> scorerNames = fScm->GetScoringHub()->GetScorerNames();
	bool ok = false;
	QString newName = QInputDialog::getText(fParameterEditorWidget, "New Scorer Name", "Enter name for duplicate:", QLineEdit::Normal, QString(oldName.c_str()) + "_copy", &ok);
	if (!ok || newName.isEmpty())
		return;
	if (NameExistsInList(scorerNames, newName.toStdString())) {
		QMessageBox::warning(fParameterEditorWidget, "Duplicate Scorer", "A scorer with that name already exists.");
		return;
	}

	G4String newNameStr = newName.toStdString();
	G4String qty = fPm->GetStringParameter("Sc/" + oldName + "/Quantity");
	G4String comp = fPm->GetStringParameter("Sc/" + oldName + "/Component");
	if (comp == "")
		comp = fPm->GetStringParameter("Sc/" + oldName + "/Surface");
	fScm->GetScoringHub()->AddScorerFromGUI(newNameStr, comp, qty);
	DuplicateParameters("Sc", oldName, newNameStr);
	UpdateParameterEditor();
}

void TsQt::DuplicateSourceCallback() {
	if (fSqm->GetRunID() != -1) {
		QMessageBox::warning(fParameterEditorWidget, "Duplicate Source", "Sources may only be duplicated before the first run.");
		return;
	}

	std::vector<G4String> sourceNames = fSom->GetSourceNames();
	if (sourceNames.empty())
		return;

	bool ok = false;
	QStringList nameList;
	for (size_t i=0;i<sourceNames.size();++i) nameList << QString(sourceNames[i]);
	QString selected = QInputDialog::getItem(fParameterEditorWidget, "Duplicate Source", "Select source to duplicate:", nameList, 0, false, &ok);
	if (!ok || selected.isEmpty())
		return;
	DoDuplicateSource(selected.toStdString());
}

void TsQt::DoDuplicateSource(const G4String& oldName) {
	std::vector<G4String> sourceNames = fSom->GetSourceNames();
	bool ok = false;
	QString newName = QInputDialog::getText(fParameterEditorWidget, "New Source Name", "Enter name for duplicate:", QLineEdit::Normal, QString(oldName.c_str()) + "_copy", &ok);
	if (!ok || newName.isEmpty())
		return;
	if (NameExistsInList(sourceNames, newName.toStdString())) {
		QMessageBox::warning(fParameterEditorWidget, "Duplicate Source", "A source with that name already exists.");
		return;
	}

	G4String newNameStr = newName.toStdString();
	G4String type = fPm->GetStringParameter("So/" + oldName + "/Type");
	G4String comp = fPm->GetStringParameter("So/" + oldName + "/Component");
	fSom->AddSourceFromGUI(newNameStr, comp, type);
	DuplicateParameters("So", oldName, newNameStr);
	UpdateParameterEditor();
}


void TsQt::ParameterTableWidgetSetItemChanged(QTreeWidgetItem* item, int column) {
	if (!item || column != 1)
		return;

	QVariant paramData = item->data(0, Qt::UserRole);
	if (!paramData.isValid())
		return;

	G4String paramName = (paramData.toString().toStdString()).c_str();
	G4String paramValue = (item->text(1).toStdString()).c_str();
	G4int colonPos = paramName.find( ":" );

	// Clear any previous inline error
	item->setBackground(1, QBrush());
	item->setToolTip(1, "");

	if (paramName.substr(0,1) == "b") {
		if (item->checkState(1) == Qt::Checked)
			paramValue = "\"True\"";
		else
			paramValue = "\"False\"";
    } else {
		G4String lastPart = fPm->GetPartAfterLastSlash(paramName);
			if (lastPart == "numberofhistoriesinrun") {
				if (fPm->IsInteger(paramValue, 32)) {
					if (G4UIcommand::ConvertToInt(paramValue) < 0) {
						item->setBackground(1, QBrush(QColor(255,230,230)));
						item->setToolTip(1, QString("Value must be a positive integer"));
						G4String restoreValue = fPm->GetParameterValueAsString(paramName.substr(0,colonPos-1), paramName.substr(colonPos+1));
						item->setText(1, QString(restoreValue));
						return;
					}
				} else {
					item->setBackground(1, QBrush(QColor(255,230,230)));
					item->setToolTip(1, QString("Value must be a positive integer"));
					G4String restoreValue = fPm->GetParameterValueAsString(paramName.substr(0,colonPos-1), paramName.substr(colonPos+1));
					item->setText(1, QString(restoreValue));
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
						item->setBackground(1, QBrush(QColor(255,230,230)));
						item->setToolTip(1, QString("Value must be an integer larger than 0"));
						G4String restoreValue = fPm->GetParameterValueAsString(paramName.substr(0,colonPos-1), paramName.substr(colonPos+1));
						item->setText(1, QString(restoreValue));
						return;
					}
				} else {
					item->setBackground(1, QBrush(QColor(255,230,230)));
					item->setToolTip(1, QString("Value must be an integer larger than 0"));
					G4String restoreValue = fPm->GetParameterValueAsString(paramName.substr(0,colonPos-1), paramName.substr(colonPos+1));
					item->setText(1, QString(restoreValue));
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

   		G4StrUtil::to_lower(paramName);
		fSqm->UpdateForSpecificParameterChange(paramName.substr(colonPos+1));
		fSqm->UpdateForNewRunOrQtChange();
	} else {
		G4String restoreValue = fPm->GetParameterValueAsString(paramName.substr(0,colonPos-1), paramName.substr(colonPos+1));
		if (paramName.substr(0,1) == "b") {
			if (restoreValue == "\"True\"")
				item->setCheckState(1, Qt::Checked);
			else
				item->setCheckState(1, Qt::Unchecked);
		} else {
			item->setText(1, QString(restoreValue));
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

   	G4StrUtil::to_lower(paramName);
	G4int colonPos = paramName.find( ":" );
	fSqm->UpdateForSpecificParameterChange(paramName.substr(colonPos+1));
	fSqm->UpdateForNewRunOrQtChange();
}


void TsQt::SaveCallback() {
	G4VViewer* viewer = fGrm->GetCurrentViewer();
	G4String viewName = fGrm->GetCurrentViewName();
	if (viewName == "")
		viewName = fGrm->GetAnyViewName();
	if (viewName == "" && viewer)
		viewName = viewer->GetName();
	G4String parameterStart = "Gr/" + viewName + "/";
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
	QPushButton* createButton = new QPushButton(QString("Create"));
	connect(createButton, SIGNAL(clicked()), this, SLOT(AddComponentWidgetSetItemChanged()));
	vbox->addWidget(createButton);

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

	if (fShowReadOnlyNoteMessage) {
		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Read-Only Parameters"));
		msgBox.setText(QString("Note: Parameters in bold cannot be changed!"));
		msgBox.addButton(QString("OK"), QMessageBox::AcceptRole);
		QPushButton* dontShowButton = msgBox.addButton(QString("Don't show again"), QMessageBox::RejectRole);
		msgBox.exec();
		if (msgBox.clickedButton() == dontShowButton)
			fShowReadOnlyNoteMessage = false;
		// Proceed regardless of which was clicked.
	}

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
	QPushButton* createButton = new QPushButton(QString("Create"));
	connect(createButton, SIGNAL(clicked()), this, SLOT(AddScorerWidgetSetItemChanged()));
	vbox->addWidget(createButton);

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

	if (fShowReadOnlyNoteMessage) {
		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Read-Only Parameters"));
		msgBox.setText(QString("Note: Parameters in bold cannot be changed!"));
		msgBox.addButton(QString("OK"), QMessageBox::AcceptRole);
		QPushButton* dontShowButton = msgBox.addButton(QString("Don't show again"), QMessageBox::RejectRole);
		msgBox.exec();
		if (msgBox.clickedButton() == dontShowButton)
			fShowReadOnlyNoteMessage = false;
	}

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
	fAddSourceTypeWidget->addItem(QString("Volumetric"));
	fAddSourceTypeWidget->addItem(QString("Emittance"));
	fAddSourceTypeWidget->addItem(QString("Environment"));
	fAddSourceTypeWidget->addItem(QString("PhaseSpace"));
	fAddSourceTypeWidget->setCurrentIndex(0);

	vbox->addWidget(fAddSourceTypeWidget);
	groupBox->setLayout(vbox);
	fAddSourceWidget->layout()->addWidget(groupBox);

	// Action
	QPushButton* createButton = new QPushButton(QString("Create"));
	connect(createButton, SIGNAL(clicked()), this, SLOT(AddSourceWidgetSetItemChanged()));
	vbox->addWidget(createButton);

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

	if (fShowReadOnlyNoteMessage) {
		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Read-Only Parameters"));
		msgBox.setText(QString("Note: Parameters in bold cannot be changed!"));
		msgBox.addButton(QString("OK"), QMessageBox::AcceptRole);
		QPushButton* dontShowButton = msgBox.addButton(QString("Don't show again"), QMessageBox::RejectRole);
		msgBox.exec();
		if (msgBox.clickedButton() == dontShowButton)
			fShowReadOnlyNoteMessage = false;
	}

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
