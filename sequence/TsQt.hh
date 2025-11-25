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

#ifndef TsQt_hh
#define TsQt_hh

#if defined(G4UI_BUILD_QT_SESSION) || defined(G4UI_USE_QT)

#include "G4String.hh"
#include "G4Types.hh"

#include <qobject.h>

class TsParameterManager;
class TsExtensionManager;
class TsMaterialManager;
class TsGeometryManager;
class TsScoringManager;
class TsSequenceManager;
class TsGraphicsManager;
class TsSourceManager;
class TsVGeometryComponent;

class G4UIQt;
class QDialog;
class QWidget;
class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QComboBox;
class QGroupBox;
class QVBoxLayout;

class TsQt : public QObject{
	Q_OBJECT
	
public:
	TsQt(TsParameterManager* pM, TsExtensionManager* eM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsSequenceManager* sqM,
		 TsGraphicsManager* grM, TsSourceManager* soM);
	void CreateAddComponentDialog();
	void CreateAddScorerDialog();
	void CreateAddSourceDialog();

	void AbortSession();

	public slots:
	void UpdateParameterEditor();
	void ParameterTableWidgetSetItemChanged(QTreeWidgetItem* item, int column);
	void ParameterComboChanged();

	void SaveCallback();

	void AddComponentCallback();
	void AddComponentWidgetSetItemChanged();

	void AddScorerCallback();
	void AddScorerWidgetSetItemChanged();

	void AddSourceCallback();
	void AddSourceWidgetSetItemChanged();

	void RunCallback();

	void PrintCallback();

	void DuplicateGeometryCallback();
	void DuplicateGeometryTreeCallback();
	void DuplicateScorerCallback();
	void DuplicateSourceCallback();
	void ShowParameterContextMenu(const QPoint& pos);

private:
	void DuplicateParameters(const G4String& categoryCode, const G4String& oldName, const G4String& newName, std::vector<G4String>* newParameterNames = nullptr);
	G4bool NameExistsInList(const std::vector<G4String>& list, const G4String& name);
	void DoDuplicateGeometry(const G4String& oldName);
	void DoDuplicateGeometryTree(const G4String& rootName);
	void CollectGeometryDescendants(const G4String& rootName, const std::map<G4String,G4String>& parentMap, std::vector<G4String>& ordered);
	void DoDuplicateSource(const G4String& oldName);

	TsParameterManager* fPm;
	TsExtensionManager* fEm;
	TsMaterialManager*  fMm;
	TsGeometryManager* fGm;
	TsScoringManager* fScm;
	TsSequenceManager* fSqm;
	TsGraphicsManager* fGrm;
	TsSourceManager* fSom;

	G4UIQt* fUIQt;
	G4bool fTestSucceeded;

	QDialog* fParameterEditorDialog;
	QWidget* fParameterEditorWidget;
	QGroupBox* fParameterTableGroupBox;
	QVBoxLayout *fParameterTablevbox;
	QTreeWidget* fParameterTableWidget;
	QLineEdit* fFilterLineEdit;
	int fSavedScrollPosition = 0;

	QDialog* fAddComponentDialog;
	QWidget* fAddComponentWidget;
	QLineEdit* fAddComponentNameWidget;
	QComboBox* fAddComponentParentWidget;
	QComboBox* fAddComponentFieldWidget;
	QComboBox* fAddComponentTableWidget;
	QLineEdit* fAddComponentTransformEdits[6];
	G4String fCurrentComponentName;
	G4int fAddedComponentCounter;

	QDialog* fAddScorerDialog;
	QWidget* fAddScorerWidget;
	QLineEdit* fAddScorerNameWidget;
	QComboBox* fAddScorerQuantityWidget;
	QComboBox* fAddScorerComponentNameWidget;
	G4String fCurrentScorerName;
	G4int fAddedScorerCounter;

	QDialog* fAddSourceDialog;
	QWidget* fAddSourceWidget;
	QLineEdit* fAddSourceNameWidget;
	QComboBox* fAddSourceTypeWidget;
	QComboBox* fAddSourceComponentNameWidget;
	G4String fCurrentSourceName;
	G4int fAddedSourceCounter;

	G4bool fShowReadOnlyNoteMessage = true;
};

#endif
#endif
