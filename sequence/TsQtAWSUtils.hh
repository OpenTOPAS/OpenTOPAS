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

#ifndef TsQtAWSUtils_hh
#define TsQtAWSUtils_hh

#if defined(G4UI_BUILD_QT_SESSION) || defined(G4UI_USE_QT)

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <functional>

class QLineEdit;
class QPushButton;
class QVBoxLayout;

namespace TsQtAWSUtils {

extern const char* const kFileCompute;
extern const char* const kFileJobDef;
extern const char* const kFileJobDefPost;
extern const char* const kFileSubmit;
extern const char* const kFilePost;

QString AwsFieldTooltipForField(const QString& fieldId);
void ApplyAwsSharedConfig(const QString& configPath, QString* profile, QString* region);
QString PrimaryButtonStyleSheet();
void ApplyPrimaryButtonStyle(QPushButton* button);
void AddUnderlinedTitle(QVBoxLayout* layout, const QString& richText);

enum class JsonArrayShape
{
    Primitive,
    FlatObject,
    Complex
};

JsonArrayShape ClassifyArrayShape(const QJsonArray& arr);
void AddFlatObjectArrayRows(const QString& path, const QJsonArray& arr, QVBoxLayout* parentLayout, QWidget* wizard,
                            QMap<QString, QLineEdit*>* scalarEdits, QMap<QString, QJsonValue::Type>* scalarTypes,
                            QMap<QString, QVBoxLayout*>* flatRowsLayouts, QMap<QString, QStringList>* flatKeys,
                            QMap<QString, QList<QWidget*>>* flatRowWidgets, const std::function<void()>& markNeedsApply);
QStringList ExtractTopLevelJobDefKeys(const QString& rawJson, const QString& path);
QString SerializeJobDefRootPreservingTopOrder(const QJsonDocument& doc, const QString& rawText);
void CollectFormPathsInJsonOrder(const QJsonValue& value, const QString& basePath,
                                 QStringList* scalarPaths, QStringList* arrayPaths);
QString EditTextOrEmpty(QLineEdit* edit);
void ApplyScalarEditsInOrder(const QStringList& orderedScalarPaths,
                             const QMap<QString, QLineEdit*>& scalarEdits,
                             const QMap<QString, QJsonValue::Type>& scalarTypes,
                             const std::function<void(const QString&, const QJsonValue&)>& applyValue);
void ApplyPrimitiveArrayEditsInOrder(const QStringList& orderedArrayPaths,
                                     const QMap<QString, QList<QLineEdit*>>& arrayEdits,
                                     const QMap<QString, QJsonValue::Type>& arrayItemTypes,
                                     const std::function<void(const QString&, const QJsonArray&)>& applyArray);
bool ParseSubmitVariablesBlock(const QString& scriptText, QStringList* orderOut, QMap<QString, QString>* defaultsOut);
bool BuildPatchedSubmitScript(const QString& scriptText, const QMap<QString, QString>& overrides, QString* patchedOut);
QString ResolveSubmitValue(QString raw, const QMap<QString, QString>& values);

} // namespace TsQtAWSUtils

#endif

#endif
