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

#include "TsParameterManager.hh"

#include <QCoreApplication>
#include <QCheckBox>
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

namespace {

const char* kFileCompute = "batch-compute-env.json";
const char* kFileJobDef = "batch-job-definition.json";
const char* kFileJobDefPost = "batch-postP-job-definition.json";
const char* kFileSubmit = "topas_submit.sh";
const char* kFilePost = "postProcess_submit.sh";

QString AwsFieldTooltipForField(const QString& fieldId)
{
    static const QMap<QString, QString> kFieldTooltips = {
        { QStringLiteral("computeEnvironmentName"), QStringLiteral("The name for your compute environment. It can be up to 128 characters long. It can contain uppercase \nand lowercase letters, numbers, hyphens (-), and underscores (_).") },
        { QStringLiteral("type"), QStringLiteral("The type of the compute environment: MANAGED or UNMANAGED.") },
        { QStringLiteral("state"), QStringLiteral("The state of the compute environment. If the state is ENABLED, then the compute environment accepts jobs from a queue and \ncan scale out automatically based on queues.") },
        { QStringLiteral("serviceRole"), QStringLiteral("The full Amazon Resource Name (ARN) of the IAM role that allows AWS Batch to make calls to other AWS services \non your behalf.") },
        { QStringLiteral("computeResources"), QStringLiteral("Details about the compute resources managed by the compute environment. This parameter is required for \nmanaged compute environments.") },
        { QStringLiteral("computeResources.type"), QStringLiteral("The type of compute environment: EC2, SPOT, FARGATE, or FARGATE_SPOT.") },
        { QStringLiteral("computeResources.minvCpus"), QStringLiteral("The minimum number of vCPUs that an environment should maintain (even if the compute \nenvironment is DISABLED).") },
        { QStringLiteral("computeResources.maxvCpus"), QStringLiteral("The maximum number of Amazon EC2 vCPUs that an environment can reach.") },
        { QStringLiteral("computeResources.desiredvCpus"), QStringLiteral("The desired number of vCPUS in the compute environment. AWS Batch modifies this value between the \nminimum and maximum values based on job queue demand.") },
        { QStringLiteral("computeResources.instanceTypes"), QStringLiteral("The instances types that can be launched. By choosing 'optimal', AWS Batch will automatically selects \nan x86_65 based instance that best matches the resource demands of the job queue.") },
        { QStringLiteral("computeResources.subnets"), QStringLiteral("A VPC (Virtual Private Cloud) is a private network inside AWS attached to your AWS account. A subnet is a \nrange of IP addresses inside your VPC. Batch needs at least one subnet so it has a place to start the EC2 instances that run your jobs") },
        { QStringLiteral("computeResources.securityGroupIds"), QStringLiteral("A security group controls what network traffic is allowed in and out of the instances.") },
        { QStringLiteral("computeResources.instanceRole"), QStringLiteral("The Amazon ECS instance profile applied to Amazon EC2 instances in a compute environment.") },
        { QStringLiteral("jobDefinitionName"), QStringLiteral("The name of the job definition.") },
        { QStringLiteral("platformCapabilities"), QStringLiteral("The platform capabilities required by the job definition. If no value is specified, \nit defaults to using the EC2 compute environment.") },
        { QStringLiteral("containerProperties"), QStringLiteral("These properties to describe the container that's launched as part of a job.") },
        { QStringLiteral("containerProperties.image"), QStringLiteral("The image used to start a container. This should be your own/the official \nTOPAS/TOPAS-nBio images on AWS ECR or Docker Hub.") },
        { QStringLiteral("containerProperties.environment"), QStringLiteral("The environment variables to pass to a container.") },
        { QStringLiteral("containerProperties.resourceRequirements"), QStringLiteral("The type and amount of resources to assign to a container. \nThe supported resources include GPU, MEMORY, and VCPU.") },
        { QStringLiteral("containerProperties.logConfiguration"), QStringLiteral("The logging system to use.") },
        { QStringLiteral("containerProperties.logConfiguration.logDriver"), QStringLiteral("Since CloudWatch logs are used, the awslogs driver is needed \nto send stdout/stderr logs directly to CloudWatch") },
        { QStringLiteral("containerProperties.logConfiguration.options"), QStringLiteral("The configuration options to send to the log driver.") },
        { QStringLiteral("containerProperties.logConfiguration.options.awslogs-group"), QStringLiteral("The CloudWatch log group to send logs to. This was specified in the previous step.") },
        { QStringLiteral("containerProperties.logConfiguration.options.awslogs-region"), QStringLiteral("The AWS region of the account.") },
        { QStringLiteral("retryStrategy"), QStringLiteral("The retry strategy to use for failed jobs that are submitted with this job definition.") },
        { QStringLiteral("retryStrategy.attempts"), QStringLiteral("The number of times to move a job to the RUNNABLE status. You can specify between 1 and 10 attempts.") },
    };

    if (kFieldTooltips.contains(fieldId))
        return kFieldTooltips.value(fieldId);
    return QStringLiteral("Please refer to the AWS documentation for more information.");
}

QString AwsConfigGroupForProfile(const QString& profileName)
{
    if (profileName == QLatin1String("default"))
        return QStringLiteral("default");
    return QStringLiteral("profile ") + profileName;
}

void ApplyAwsSharedConfig(const QString& configPath, QString* profile, QString* region)
{
    if (!profile || !region || !QFileInfo::exists(configPath))
        return;

    QSettings cfg(configPath, QSettings::IniFormat);
    const QStringList groups = cfg.childGroups();

    if (profile->isEmpty()) {
        QStringList named;
        const QString pref = QStringLiteral("profile ");
        for (const QString& g : groups) {
            if (g.startsWith(pref))
                named.append(g.mid(pref.size()));
        }
        if (named.size() == 1)
            *profile = named[0];
        else if (named.isEmpty() && groups.contains(QStringLiteral("default")))
            *profile = QStringLiteral("default");
    }

    if (region->isEmpty() && !profile->isEmpty()) {
        cfg.beginGroup(AwsConfigGroupForProfile(*profile));
        const QString reg = cfg.value(QStringLiteral("region")).toString().trimmed();
        cfg.endGroup();
        if (!reg.isEmpty())
            *region = reg;
    }
}

enum class JsonArrayShape
{
    Primitive,
    FlatObject,
    Complex
};

JsonArrayShape ClassifyArrayShape(const QJsonArray& arr)
{
    if (arr.isEmpty())
        return JsonArrayShape::Primitive;

    bool hasObject = false;
    for (const QJsonValue& v : arr) {
        if (v.isArray())
            return JsonArrayShape::Complex;
        if (v.isObject()) {
            hasObject = true;
            const QJsonObject obj = v.toObject();
            for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
                if (it.value().isObject() || it.value().isArray())
                    return JsonArrayShape::Complex;
            }
        }
    }
    return hasObject ? JsonArrayShape::FlatObject : JsonArrayShape::Primitive;
}

QStringList KeysFromObjectIteration(const QJsonObject& obj)
{
    QStringList keys;
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it)
        keys.append(it.key());
    return keys;
}

void AddFlatObjectArrayRows(const QString& path, const QJsonArray& arr, QVBoxLayout* parentLayout, QWidget* wizard,
                            QMap<QString, QLineEdit*>* scalarEdits, QMap<QString, QJsonValue::Type>* scalarTypes,
                            QMap<QString, QVBoxLayout*>* flatRowsLayouts, QMap<QString, QStringList>* flatKeys,
                            QMap<QString, QList<QWidget*>>* flatRowWidgets, const std::function<void()>& markNeedsApply)
{
    auto appendRow = [path, wizard, scalarEdits, scalarTypes, markNeedsApply](
                         const QStringList& orderedKeys, const QJsonObject& itemObj,
                         QVBoxLayout* targetRowsLayout, QList<QWidget*>* rowsStore) {
        if (!targetRowsLayout || !rowsStore)
            return;
        const int rowIndex = rowsStore->size();
        QWidget* entryWidget = new QWidget();
        QGridLayout* grid = new QGridLayout(entryWidget);
        grid->setContentsMargins(24, 0, 0, 0);
        grid->setHorizontalSpacing(12);
        grid->setVerticalSpacing(6);
        for (int col = 0; col < orderedKeys.size(); ++col) {
            const QString& key = orderedKeys[col];
            QLabel* fieldLabel = new QLabel(QString("<b>%1</b>").arg(key));
            fieldLabel->setTextFormat(Qt::RichText);
            fieldLabel->setToolTip(AwsFieldTooltipForField(QStringLiteral("%1.<row>.%2").arg(path).arg(key)));
            grid->addWidget(fieldLabel, 0, col);

            const QJsonValue v = itemObj.value(key);
            QLineEdit* edit = new QLineEdit(v.toVariant().toString());
            edit->setFixedHeight(22);
            edit->setCursorPosition(0);
            QObject::connect(edit, &QLineEdit::textEdited, wizard, [markNeedsApply](const QString&) { markNeedsApply(); });
            grid->addWidget(edit, 1, col);
            const QString fieldPath = QString("%1.%2.%3").arg(path).arg(rowIndex).arg(key);
            (*scalarEdits)[fieldPath] = edit;
            QJsonValue::Type t = v.type();
            if (t == QJsonValue::Undefined || t == QJsonValue::Null)
                t = QJsonValue::String;
            (*scalarTypes)[fieldPath] = t;
        }
        rowsStore->append(entryWidget);
        const int insertAt = std::max(0, targetRowsLayout->count() - 1); // keep +/- at bottom
        targetRowsLayout->insertWidget(insertAt, entryWidget);
    };

    auto remapScalarPathsForArray = [path, scalarEdits, scalarTypes](const QStringList& orderedKeys, QList<QWidget*>* rowsStore) {
        if (!rowsStore)
            return;
        const QString prefix = path + ".";
        QStringList toRemove;
        for (auto it = scalarEdits->constBegin(); it != scalarEdits->constEnd(); ++it) {
            if (it.key().startsWith(prefix))
                toRemove.append(it.key());
        }
        QMap<QString, QJsonValue::Type> keyTypes;
        for (const QString& key : orderedKeys) {
            keyTypes[key] = QJsonValue::String;
            for (auto it = scalarTypes->constBegin(); it != scalarTypes->constEnd(); ++it) {
                if (it.key().startsWith(prefix) && it.key().endsWith("." + key)) {
                    keyTypes[key] = it.value();
                    break;
                }
            }
        }
        for (const QString& k : toRemove) {
            scalarEdits->remove(k);
            scalarTypes->remove(k);
        }
        for (int row = 0; row < rowsStore->size(); ++row) {
            QWidget* rowWidget = (*rowsStore)[row];
            QGridLayout* grid = qobject_cast<QGridLayout*>(rowWidget ? rowWidget->layout() : nullptr);
            if (!grid)
                continue;
            for (int col = 0; col < orderedKeys.size(); ++col) {
                const QString& key = orderedKeys[col];
                QLayoutItem* item = grid->itemAtPosition(1, col);
                QLineEdit* edit = item ? qobject_cast<QLineEdit*>(item->widget()) : nullptr;
                if (!edit)
                    continue;
                const QString fieldPath = QString("%1.%2.%3").arg(path).arg(row).arg(key);
                (*scalarEdits)[fieldPath] = edit;
                (*scalarTypes)[fieldPath] = keyTypes.value(key, QJsonValue::String);
            }
        }
    };

    QWidget* rowsWidget = new QWidget();
    QVBoxLayout* rowsLayout = new QVBoxLayout(rowsWidget);
    rowsLayout->setContentsMargins(24, 0, 0, 0);
    rowsLayout->setSpacing(9);
    QStringList orderedKeys;
    if (!arr.isEmpty() && arr[0].isObject())
        orderedKeys = KeysFromObjectIteration(arr[0].toObject());
    if (orderedKeys.isEmpty())
        orderedKeys.append("value");
    (*flatRowsLayouts)[path] = rowsLayout;
    (*flatKeys)[path] = orderedKeys;
    (*flatRowWidgets)[path].clear();

    for (const QJsonValue& v : arr)
        appendRow(orderedKeys, v.toObject(), rowsLayout, &((*flatRowWidgets)[path]));

    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->setSpacing(8);
    QPushButton* addBtn = new QPushButton("+");
    addBtn->setFixedWidth(30);
    QPushButton* removeBtn = new QPushButton("-");
    removeBtn->setFixedWidth(30);
    btnRow->addWidget(addBtn);
    btnRow->addWidget(removeBtn);
    btnRow->addStretch(1);
    QObject::connect(addBtn, &QPushButton::clicked, wizard, [=]() {
        const QStringList keys = flatKeys->value(path);
        QVBoxLayout* targetRowsLayout = flatRowsLayouts->value(path, nullptr);
        if (!targetRowsLayout)
            return;
        QDialog dialog(wizard);
        dialog.setWindowTitle("Add value");
        QVBoxLayout* v = new QVBoxLayout(&dialog);
        QGridLayout* g = new QGridLayout();
        QList<QLineEdit*> inputs;
        for (int i = 0; i < keys.size(); ++i) {
            QLabel* lbl = new QLabel(QString("%1:").arg(keys[i]));
            QLineEdit* in = new QLineEdit();
            in->setFixedHeight(22);
            g->addWidget(lbl, i, 0);
            g->addWidget(in, i, 1);
            inputs.append(in);
        }
        v->addLayout(g);
        QDialogButtonBox* box = new QDialogButtonBox();
        QPushButton* add = box->addButton("Add", QDialogButtonBox::AcceptRole);
        add->setStyleSheet(
            "QPushButton { border-radius: 8px; padding: 6px 12px; border: none; }"
            "QPushButton:enabled { background-color: rgb(10,132,255); color: white; }"
            "QPushButton:disabled { background-color: rgb(58,58,58); color: rgb(136,136,136); }");
        box->addButton("Cancel", QDialogButtonBox::RejectRole);
        v->addWidget(box);
        QObject::connect(box, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        QObject::connect(box, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if (dialog.exec() != QDialog::Accepted)
            return;
        QJsonObject obj;
        for (int i = 0; i < keys.size(); ++i)
            obj.insert(keys[i], inputs[i]->text().trimmed());
        appendRow(keys, obj, targetRowsLayout, &((*flatRowWidgets)[path]));
        markNeedsApply();
    });
    QObject::connect(removeBtn, &QPushButton::clicked, wizard, [=]() {
        QList<QWidget*>& rows = (*flatRowWidgets)[path];
        if (rows.isEmpty())
            return;
        const QStringList keys = flatKeys->value(path);
        QDialog dialog(wizard);
        dialog.setWindowTitle("Remove value");
        QVBoxLayout* v = new QVBoxLayout(&dialog);
        v->addWidget(new QLabel("Select entry to remove:"));
        QComboBox* combo = new QComboBox();
        QVector<int> indices;
        for (int row = 0; row < rows.size(); ++row) {
            QWidget* rowWidget = rows[row];
            QGridLayout* grid = qobject_cast<QGridLayout*>(rowWidget ? rowWidget->layout() : nullptr);
            if (!grid)
                continue;
            QString summary;
            for (int col = 0; col < keys.size(); ++col) {
                QLayoutItem* item = grid->itemAtPosition(1, col);
                QLineEdit* edit = item ? qobject_cast<QLineEdit*>(item->widget()) : nullptr;
                if (!edit)
                    continue;
                if (!summary.isEmpty())
                    summary += ", ";
                summary += QString("%1=%2").arg(keys[col], edit->text());
            }
            combo->addItem(summary.isEmpty() ? QString("Entry %1").arg(row + 1) : summary);
            indices.append(row);
        }
        v->addWidget(combo);
        QDialogButtonBox* box = new QDialogButtonBox();
        QPushButton* remove = box->addButton("Remove", QDialogButtonBox::AcceptRole);
        remove->setEnabled(combo->count() > 0);
        remove->setStyleSheet(
            "QPushButton { border-radius: 8px; padding: 6px 12px; border: none; }"
            "QPushButton:enabled { background-color: rgb(10,132,255); color: white; }"
            "QPushButton:disabled { background-color: rgb(58,58,58); color: rgb(136,136,136); }");
        box->addButton("Cancel", QDialogButtonBox::RejectRole);
        v->addWidget(box);
        QObject::connect(box, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        QObject::connect(box, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if (dialog.exec() != QDialog::Accepted || combo->currentIndex() < 0)
            return;
        const int idx = indices[combo->currentIndex()];
        if (idx < 0 || idx >= rows.size())
            return;
        QWidget* victim = rows.takeAt(idx);
        for (const QString& key : keys) {
            const QString fieldPath = QString("%1.%2.%3").arg(path).arg(idx).arg(key);
            (*scalarEdits).remove(fieldPath);
            (*scalarTypes).remove(fieldPath);
        }
        delete victim;
        remapScalarPathsForArray(keys, &rows);
        markNeedsApply();
    });
    rowsLayout->addLayout(btnRow);
    parentLayout->addWidget(rowsWidget);
}

int SkipWs(const QString& s, int pos)
{
    while (pos < s.size() && s[pos].isSpace())
        ++pos;
    return pos;
}

bool ParseJsonStringAt(const QString& s, int quotePos, QString* out, int* endPos)
{
    if (!out || !endPos || quotePos < 0 || quotePos >= s.size() || s[quotePos] != QChar('"'))
        return false;
    QString result;
    bool esc = false;
    int i = quotePos + 1;
    for (; i < s.size(); ++i) {
        const QChar c = s[i];
        if (esc) {
            result.append(c);
            esc = false;
            continue;
        }
        if (c == QChar('\\')) {
            esc = true;
            continue;
        }
        if (c == QChar('"')) {
            *out = result;
            *endPos = i + 1;
            return true;
        }
        result.append(c);
    }
    return false;
}

int FindRootObjectStartByIndex(const QString& jsonText, int targetIndex)
{
    int pos = SkipWs(jsonText, 0);
    if (pos >= jsonText.size() || jsonText[pos] != QChar('['))
        return -1;
    bool inString = false;
    bool esc = false;
    int depthObj = 0;
    int depthArr = 0;
    int currentObjectIdx = -1;
    for (int i = pos; i < jsonText.size(); ++i) {
        const QChar c = jsonText[i];
        if (inString) {
            if (esc)
                esc = false;
            else if (c == QChar('\\'))
                esc = true;
            else if (c == QChar('"'))
                inString = false;
            continue;
        }
        if (c == QChar('"')) {
            inString = true;
            continue;
        }
        if (c == QChar('[')) {
            ++depthArr;
            continue;
        }
        if (c == QChar(']')) {
            --depthArr;
            if (depthArr <= 0)
                break;
            continue;
        }
        if (c == QChar('{')) {
            if (depthArr == 1 && depthObj == 0) {
                ++currentObjectIdx;
                if (currentObjectIdx == targetIndex)
                    return i;
            }
            ++depthObj;
            continue;
        }
        if (c == QChar('}')) {
            --depthObj;
            continue;
        }
    }
    return -1;
}


QStringList ExtractFirstLevelObjectKeys(const QString& jsonText, int objectStartPos)
{
    QStringList keys;
    if (objectStartPos < 0 || objectStartPos >= jsonText.size() || jsonText[objectStartPos] != QChar('{'))
        return keys;
    bool inString = false;
    bool esc = false;
    int depthObj = 0;
    int depthArr = 0;
    for (int i = objectStartPos; i < jsonText.size(); ++i) {
        const QChar c = jsonText[i];
        if (inString) {
            if (esc)
                esc = false;
            else if (c == QChar('\\'))
                esc = true;
            else if (c == QChar('"'))
                inString = false;
            continue;
        }
        if (c == QChar('"')) {
            if (depthObj == 1 && depthArr == 0) {
                QString key;
                int after = i;
                if (ParseJsonStringAt(jsonText, i, &key, &after)) {
                    const int colonPos = SkipWs(jsonText, after);
                    if (colonPos < jsonText.size() && jsonText[colonPos] == QChar(':')) {
                        keys.append(key);
                        i = after - 1;
                        continue;
                    }
                }
            }
            inString = true;
            continue;
        }
        if (c == QChar('{')) {
            ++depthObj;
            continue;
        }
        if (c == QChar('}')) {
            --depthObj;
            if (depthObj == 0)
                break;
            continue;
        }
        if (c == QChar('[')) {
            ++depthArr;
            continue;
        }
        if (c == QChar(']')) {
            --depthArr;
            continue;
        }
    }
    return keys;
}

QStringList ExtractTopLevelJobDefKeys(const QString& rawJson, const QString& path)
{
    const bool isRootObject = path.isEmpty();
    bool isRootArrayIndex = false;
    const int rootIndex = path.toInt(&isRootArrayIndex);
    if (!isRootObject && !isRootArrayIndex)
        return QStringList();
    if (isRootObject) {
        const int start = rawJson.indexOf('{');
        return ExtractFirstLevelObjectKeys(rawJson, start);
    }
    const int start = FindRootObjectStartByIndex(rawJson, rootIndex);
    return ExtractFirstLevelObjectKeys(rawJson, start);
}

QString JsonEscaped(const QString& s)
{
    QString out;
    out.reserve(s.size() + 8);
    for (QChar c : s) {
        switch (c.unicode()) {
        case '\"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default: out += c; break;
        }
    }
    return out;
}

QString Indent(int n)
{
    return QString(n, QChar(' '));
}

QString SerializeJsonValueOrdered(const QJsonValue& value, int indent, const QStringList* preferredKeys = nullptr);

QString SerializeJsonObjectOrdered(const QJsonObject& obj, int indent, const QStringList* preferredKeys = nullptr)
{
    QStringList keys;
    if (preferredKeys) {
        for (const QString& k : *preferredKeys) {
            if (obj.contains(k))
                keys.append(k);
        }
    }
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        if (!keys.contains(it.key()))
            keys.append(it.key());
    }
    if (keys.isEmpty())
        return "{}";

    QString out = "{\n";
    for (int i = 0; i < keys.size(); ++i) {
        const QString& k = keys[i];
        out += Indent(indent + 4) + "\"" + JsonEscaped(k) + "\": "
               + SerializeJsonValueOrdered(obj.value(k), indent + 4, nullptr);
        if (i + 1 < keys.size())
            out += ",";
        out += "\n";
    }
    out += Indent(indent) + "}";
    return out;
}

QString SerializeJsonArrayOrdered(const QJsonArray& arr, int indent)
{
    if (arr.isEmpty())
        return "[]";
    QString out = "[\n";
    for (int i = 0; i < arr.size(); ++i) {
        out += Indent(indent + 4) + SerializeJsonValueOrdered(arr[i], indent + 4, nullptr);
        if (i + 1 < arr.size())
            out += ",";
        out += "\n";
    }
    out += Indent(indent) + "]";
    return out;
}

QString SerializeJsonValueOrdered(const QJsonValue& value, int indent, const QStringList* preferredKeys)
{
    if (value.isObject())
        return SerializeJsonObjectOrdered(value.toObject(), indent, preferredKeys);
    if (value.isArray())
        return SerializeJsonArrayOrdered(value.toArray(), indent);
    if (value.isString())
        return QString("\"%1\"").arg(JsonEscaped(value.toString()));
    if (value.isDouble()) {
        const double d = value.toDouble();
        return QString::number(d, 'g', 16);
    }
    if (value.isBool())
        return value.toBool() ? "true" : "false";
    if (value.isNull() || value.isUndefined())
        return "null";
    return "null";
}

QString SerializeJobDefRootPreservingTopOrder(const QJsonDocument& doc, const QString& rawText)
{
    if (doc.isArray()) {
        const QJsonArray arr = doc.array();
        QString out = "[\n";
        for (int i = 0; i < arr.size(); ++i) {
            const QStringList ordered = ExtractTopLevelJobDefKeys(rawText, QString::number(i));
            out += Indent(4) + SerializeJsonValueOrdered(arr[i], 4, &ordered);
            if (i + 1 < arr.size())
                out += ",";
            out += "\n";
        }
        out += "]";
        return out;
    }
    if (doc.isObject()) {
        const QStringList ordered = ExtractTopLevelJobDefKeys(rawText, "");
        return SerializeJsonObjectOrdered(doc.object(), 0, &ordered);
    }
    return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
}

void CollectFormPathsInJsonOrder(const QJsonValue& value, const QString& basePath,
                                 QStringList* scalarPaths, QStringList* arrayPaths)
{
    if (!scalarPaths || !arrayPaths)
        return;
    if (value.isObject()) {
        const QJsonObject obj = value.toObject();
        for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
            const QString childPath = basePath.isEmpty() ? it.key() : (basePath + "." + it.key());
            CollectFormPathsInJsonOrder(it.value(), childPath, scalarPaths, arrayPaths);
        }
        return;
    }
    if (value.isArray()) {
        const QJsonArray arr = value.toArray();
        const JsonArrayShape shape = ClassifyArrayShape(arr);
        if (shape == JsonArrayShape::Primitive) {
            arrayPaths->append(basePath);
            return;
        }
        if (shape == JsonArrayShape::FlatObject) {
            for (int i = 0; i < arr.size(); ++i) {
                const QJsonObject itemObj = arr[i].toObject();
                for (auto it = itemObj.constBegin(); it != itemObj.constEnd(); ++it) {
                    const QString p = QString("%1.%2.%3").arg(basePath).arg(i).arg(it.key());
                    scalarPaths->append(p);
                }
            }
        }
        return;
    }
    scalarPaths->append(basePath);
}

QString EditTextOrEmpty(QLineEdit* edit)
{
    return edit ? edit->text() : QString();
}

QJsonValue ParseTextByType(const QString& txt, QJsonValue::Type type)
{
    if (type == QJsonValue::Bool)
        return QJsonValue(txt.trimmed().toLower() == "true" || txt.trimmed() == "1");
    if (type == QJsonValue::Double) {
        bool ok = false;
        const double d = txt.toDouble(&ok);
        return ok ? QJsonValue(d) : QJsonValue(txt);
    }
    if (type == QJsonValue::Null)
        return QJsonValue(QJsonValue::Null);
    return QJsonValue(txt);
}

void ApplyScalarEditsInOrder(const QStringList& orderedScalarPaths,
                             const QMap<QString, QLineEdit*>& scalarEdits,
                             const QMap<QString, QJsonValue::Type>& scalarTypes,
                             const std::function<void(const QString&, const QJsonValue&)>& applyValue)
{
    QSet<QString> applied;
    for (const QString& path : orderedScalarPaths) {
        if (!scalarEdits.contains(path))
            continue;
        QLineEdit* edit = scalarEdits.value(path, nullptr);
        const QString text = edit ? edit->text() : QString();
        const QJsonValue::Type type = scalarTypes.value(path, QJsonValue::String);
        applyValue(path, ParseTextByType(text, type));
        applied.insert(path);
    }
    for (auto it = scalarEdits.constBegin(); it != scalarEdits.constEnd(); ++it) {
        const QString path = it.key();
        if (applied.contains(path))
            continue;
        const QString text = it.value() ? it.value()->text() : QString();
        const QJsonValue::Type type = scalarTypes.value(path, QJsonValue::String);
        applyValue(path, ParseTextByType(text, type));
    }
}

void ApplyPrimitiveArrayEditsInOrder(const QStringList& orderedArrayPaths,
                                     const QMap<QString, QList<QLineEdit*>>& arrayEdits,
                                     const QMap<QString, QJsonValue::Type>& arrayItemTypes,
                                     const std::function<void(const QString&, const QJsonArray&)>& applyArray)
{
    QSet<QString> applied;
    auto applyArrayPath = [&](const QString& path) {
        if (!arrayEdits.contains(path))
            return;
        const QJsonValue::Type itemType = arrayItemTypes.value(path, QJsonValue::String);
        QJsonArray arr;
        for (QLineEdit* edit : arrayEdits.value(path)) {
            if (!edit)
                continue;
            arr.append(ParseTextByType(edit->text(), itemType));
        }
        applyArray(path, arr);
        applied.insert(path);
    };
    for (const QString& path : orderedArrayPaths)
        applyArrayPath(path);
    for (auto it = arrayEdits.constBegin(); it != arrayEdits.constEnd(); ++it) {
        if (applied.contains(it.key()))
            continue;
        applyArrayPath(it.key());
    }
}

bool ParseSubmitVariablesBlock(const QString& scriptText, QStringList* orderOut, QMap<QString, QString>* defaultsOut)
{
    if (!orderOut || !defaultsOut)
        return false;
    orderOut->clear();
    defaultsOut->clear();

    const QString marker = "########################################################################################";
    const QString startTitle = "# Edit these variables for each run";
    const QStringList lines = scriptText.split('\n');
    int blockStart = -1;
    for (int i = 0; i + 2 < lines.size(); ++i) {
        if (lines[i].trimmed() == marker && lines[i + 1].trimmed() == startTitle && lines[i + 2].trimmed() == marker) {
            blockStart = i + 3;
            break;
        }
    }
    if (blockStart < 0)
        return false;
    int blockEnd = lines.size();
    for (int i = blockStart; i < lines.size(); ++i) {
        if (lines[i].trimmed() == marker) {
            blockEnd = i;
            break;
        }
    }

    QRegularExpression re("^\\s*([A-Za-z_][A-Za-z0-9_]*)\\s*=\\s*(.*)$");
    QRegularExpression envDefaultRe("^\\$\\{[A-Za-z_][A-Za-z0-9_]*:-([^}]*)\\}$");
    for (int i = blockStart; i < blockEnd; ++i) {
        QString line = lines[i];
        const int commentPos = line.indexOf('#');
        if (commentPos >= 0)
            line = line.left(commentPos);
        line = line.trimmed();
        if (line.isEmpty())
            continue;
        const QRegularExpressionMatch m = re.match(line);
        if (!m.hasMatch())
            continue;
        const QString key = m.captured(1).trimmed();
        QString value = m.captured(2).trimmed();
        if (value.startsWith('"') && value.endsWith('"') && value.size() >= 2)
            value = value.mid(1, value.size() - 2);
        else if (value.startsWith('\'') && value.endsWith('\'') && value.size() >= 2)
            value = value.mid(1, value.size() - 2);
        const QRegularExpressionMatch envDefault = envDefaultRe.match(value);
        if (envDefault.hasMatch()) {
            value = envDefault.captured(1).trimmed();
            if (value.startsWith('"') && value.endsWith('"') && value.size() >= 2)
                value = value.mid(1, value.size() - 2);
            else if (value.startsWith('\'') && value.endsWith('\'') && value.size() >= 2)
                value = value.mid(1, value.size() - 2);
        }
        orderOut->append(key);
        (*defaultsOut)[key] = value;
    }
    return !orderOut->isEmpty();
}

QString ShellDoubleQuote(const QString& value)
{
    QString out = value;
    out.replace("\\", "\\\\");
    out.replace("\"", "\\\"");
    return "\"" + out + "\"";
}

bool BuildPatchedSubmitScript(const QString& scriptText, const QMap<QString, QString>& overrides, QString* patchedOut)
{
    if (!patchedOut)
        return false;
    const QString marker = "########################################################################################";
    const QString startTitle = "# Edit these variables for each run";
    QStringList lines = scriptText.split('\n');
    int blockStart = -1;
    for (int i = 0; i + 2 < lines.size(); ++i) {
        if (lines[i].trimmed() == marker && lines[i + 1].trimmed() == startTitle && lines[i + 2].trimmed() == marker) {
            blockStart = i + 3;
            break;
        }
    }
    if (blockStart < 0)
        return false;
    int blockEnd = lines.size();
    for (int i = blockStart; i < lines.size(); ++i) {
        if (lines[i].trimmed() == marker) {
            blockEnd = i;
            break;
        }
    }

    QRegularExpression assignRe("^(\\s*)([A-Za-z_][A-Za-z0-9_]*)\\s*=\\s*([^#]*)(\\s*(?:#.*)?)$");
    for (int i = blockStart; i < blockEnd; ++i) {
        const QRegularExpressionMatch m = assignRe.match(lines[i]);
        if (!m.hasMatch())
            continue;
        const QString key = m.captured(2).trimmed();
        if (!overrides.contains(key))
            continue;
        const QString leading = m.captured(1);
        QString suffix = m.captured(4);
        if (suffix.startsWith('#'))
            suffix.prepend(' ');
        lines[i] = leading + key + "=" + ShellDoubleQuote(overrides.value(key)) + suffix;
    }
    const QString queueName = overrides.value("JOB_QUEUE").trimmed();
    QString jobDefName = overrides.value("JOB_DEFINITION").trimmed();
    if (jobDefName.isEmpty())
        jobDefName = overrides.value("JOB_DEFINITION_NAME").trimmed();
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        if (line.trimmed().startsWith('#'))
            continue;
        if (!queueName.isEmpty()) {
            QRegularExpression queueRe("^(\\s*--job-queue\\s+)([^\\s\\\\]+|\"[^\"]*\"|'[^']*')(\\s*\\\\?.*)$");
            const QRegularExpressionMatch qm = queueRe.match(line);
            if (qm.hasMatch()) {
                lines[i] = qm.captured(1) + ShellDoubleQuote(queueName) + qm.captured(3);
                continue;
            }
        }
        if (!jobDefName.isEmpty()) {
            QRegularExpression jdRe("^(\\s*--job-definition\\s+)([^\\s\\\\]+|\"[^\"]*\"|'[^']*')(\\s*\\\\?.*)$");
            const QRegularExpressionMatch jm = jdRe.match(line);
            if (jm.hasMatch()) {
                lines[i] = jm.captured(1) + ShellDoubleQuote(jobDefName) + jm.captured(3);
                continue;
            }
        }
    }
    *patchedOut = lines.join('\n');
    return true;
}

QString ResolveSubmitValue(QString raw, const QMap<QString, QString>& values)
{
    if (raw.isEmpty())
        return raw;
    raw = raw.trimmed();
    const QString today = QDate::currentDate().toString("yyyy-MM-dd");
    raw.replace("$(date +%Y-%m-%d)", today);
    raw.replace("$(date +%F)", today);

    QRegularExpression bracedVarRe("\\$\\{([A-Za-z_][A-Za-z0-9_]*)\\}");
    QRegularExpression plainVarRe("\\$([A-Za-z_][A-Za-z0-9_]*)");
    for (int pass = 0; pass < 4; ++pass) {
        bool changed = false;
        QString next = raw;
        QRegularExpressionMatchIterator it = bracedVarRe.globalMatch(next);
        while (it.hasNext()) {
            const QRegularExpressionMatch m = it.next();
            const QString key = m.captured(1);
            if (values.contains(key)) {
                next.replace(m.captured(0), values.value(key));
                changed = true;
            }
        }
        it = plainVarRe.globalMatch(next);
        while (it.hasNext()) {
            const QRegularExpressionMatch m = it.next();
            const QString key = m.captured(1);
            if (values.contains(key)) {
                next.replace(m.captured(0), values.value(key));
                changed = true;
            }
        }
        raw = next;
        if (!changed)
            break;
    }
    return raw;
}

} // namespace

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
      fBtnSubmitPostprocess(nullptr), fMonitorText(nullptr), fMonitorLastUpdated(nullptr), fPostprocessMonitorText(nullptr),
      fPollTimer(nullptr), fPostTimer(nullptr), fEditProjectName(nullptr), fEditRunDate(nullptr),
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
      fEditExistingJobDefPost(nullptr), fChkExistingNoPostprocess(nullptr), fEditProvisionInputBucket(nullptr),
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
    btnBegin->setStyleSheet(
        "QPushButton { border-radius: 8px; padding: 6px 12px; border: none; }"
        "QPushButton:enabled { background-color: rgb(10,132,255); color: white; }"
        "QPushButton:disabled { background-color: rgb(58,58,58); color: rgb(136,136,136); }");
    lWelcome->addWidget(btnBegin);
    lWelcome->addStretch(1);
    fStack->addWidget(pageWelcome);

    auto addUnderlinedTitle = [&](QVBoxLayout* layout, const QString& richText) {
        layout->addSpacing(8);
        QWidget* titleBlock = new QWidget();
        QVBoxLayout* titleLayout = new QVBoxLayout(titleBlock);
        titleLayout->setContentsMargins(0, 0, 0, 0);
        titleLayout->setSpacing(4);
        QLabel* title = new QLabel(richText);
        title->setTextFormat(Qt::RichText);
        QFont titleFont = title->font();
        titleFont.setPointSize(titleFont.pointSize() + 3);
        title->setFont(titleFont);
        titleLayout->addWidget(title);
        QFrame* underline = new QFrame();
        underline->setFrameShape(QFrame::HLine);
        underline->setFrameShadow(QFrame::Plain);
        underline->setLineWidth(2);
        underline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        titleLayout->addWidget(underline);
        layout->addWidget(titleBlock);
        layout->addSpacing(8);
    };

    // Step 2 — default (field editors) vs advanced (JSON text)
    QWidget* pageMode = new QWidget();
    pageMode->setFocusPolicy(Qt::ClickFocus);
    QVBoxLayout* lMode = new QVBoxLayout(pageMode);
    lMode->setContentsMargins(12, 18, 12, 18);
    lMode->setSpacing(16);
    addUnderlinedTitle(lMode, "<b>Choose the type of setup instructions</b>");
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
        addUnderlinedTitle(l0, "<b>AWS scripts folder</b>");
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
    browse->setStyleSheet(
        "QPushButton { border-radius: 8px; padding: 6px 12px; border: none; }"
        "QPushButton:enabled { background-color: rgb(10,132,255); color: white; }"
        "QPushButton:disabled { background-color: rgb(58,58,58); color: rgb(136,136,136); }");
    l0->addWidget(browse);
    l0->addSpacing(36);
    {
        addUnderlinedTitle(l0, "<b>Environment variables</b>");
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
    addUnderlinedTitle(lbk, "<b>S3 buckets</b>");
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
    addUnderlinedTitle(lc, "<b>Setting up the compute environment</b>");
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
    addUnderlinedTitle(l1, "<b>Setting up the compute environment</b>");
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
    addUnderlinedTitle(l1, "<b>Save changes locally</b>");
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
    addUnderlinedTitle(lbc, "<b>Job queue</b>");
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
    addUnderlinedTitle(lblg, "<b>CloudWatch log group</b>");
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
    addUnderlinedTitle(lsimChoice, "<b>Setting up simulation job definition</b>");
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
    addUnderlinedTitle(lsim, "<b>Simulation job definition</b>");
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
    addUnderlinedTitle(lbpp, "<b>Post-processing</b>");
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
    addUnderlinedTitle(lpostChoice, "<b>Setting up post-processing job definition</b>");
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
    addUnderlinedTitle(lpost, "<b>Post-processing job definition</b>");
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
    addUnderlinedTitle(hlay, "<b>Save changes locally</b>");
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
    addUnderlinedTitle(page3Outer, "<b>Cloud submission</b>");
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
    addUnderlinedTitle(page4Outer, "<b>Cloud submission</b>");
    QLabel* submitDesc = new QLabel("The fields below are editable variables from the submission script.");
    submitDesc->setWordWrap(true);
    page4Outer->addWidget(submitDesc);
    fSubmitVarsGrid = new QGridLayout();
    page4Outer->addLayout(fSubmitVarsGrid);
    addUnderlinedTitle(page4Outer, "<b>Save changes locally</b>");
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
    addUnderlinedTitle(l4, "<b>Job status dashboard</b>");
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
    fBtnDownloadS3->setStyleSheet(
        "QPushButton { border-radius: 8px; padding: 6px 12px; border: none; }"
        "QPushButton:enabled { background-color: rgb(10,132,255); color: white; }"
        "QPushButton:disabled { background-color: rgb(58,58,58); color: rgb(136,136,136); }");
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
    fBtnSubmitPostprocess->setStyleSheet(
        "QPushButton { border-radius: 8px; padding: 6px 12px; border: none; }"
        "QPushButton:enabled { background-color: rgb(10,132,255); color: white; }"
        "QPushButton:disabled { background-color: rgb(58,58,58); color: rgb(136,136,136); }");
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
        btnExitDownload->setStyleSheet(
            "QPushButton { border-radius: 8px; padding: 6px 12px; border: none; }"
            "QPushButton:enabled { background-color: rgb(10,132,255); color: white; }"
            "QPushButton:disabled { background-color: rgb(58,58,58); color: rgb(136,136,136); }");
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
    fBtnReset->setStyleSheet(
        "QPushButton { border-radius: 8px; padding: 6px 12px; border: none; }"
        "QPushButton:enabled { background-color: rgb(10,132,255); color: white; }"
        "QPushButton:disabled { background-color: rgb(58,58,58); color: rgb(136,136,136); }");
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

    fAwsDirEdit->setText(fAwsDir);
    fProfileEdit->setText(fProfile);
    fEditAwsRegion->setText(fRegion);
    ReloadAwsFilesFromUi();

    SetStep(0); // welcome page
    fWizard->exec();

    StopMonitoring();
    StopPostprocessMonitor();

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
    fChkExistingNoPostprocess = nullptr;
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
    fPostprocessMonitorText = nullptr;
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
        fBtnNext->setStyleSheet(
            "QPushButton { border-radius: 8px; padding: 6px 12px; border: none; }"
            "QPushButton:enabled { background-color: rgb(10,132,255); color: white; }"
            "QPushButton:disabled { background-color: rgb(58,58,58); color: rgb(136,136,136); }");
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
    addBtn->setStyleSheet(
        "QPushButton { border-radius: 8px; padding: 6px 12px; border: none; }"
        "QPushButton:enabled { background-color: rgb(10,132,255); color: white; }"
        "QPushButton:disabled { background-color: rgb(58,58,58); color: rgb(136,136,136); }");
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
    removeBtn->setStyleSheet(
        "QPushButton { border-radius: 8px; padding: 6px 12px; border: none; }"
        "QPushButton:enabled { background-color: rgb(10,132,255); color: white; }"
        "QPushButton:disabled { background-color: rgb(58,58,58); color: rgb(136,136,136); }");
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
    addBtn->setStyleSheet(
        "QPushButton { border-radius: 8px; padding: 6px 12px; border: none; }"
        "QPushButton:enabled { background-color: rgb(10,132,255); color: white; }"
        "QPushButton:disabled { background-color: rgb(58,58,58); color: rgb(136,136,136); }");
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
    removeBtn->setStyleSheet(
        "QPushButton { border-radius: 8px; padding: 6px 12px; border: none; }"
        "QPushButton:enabled { background-color: rgb(10,132,255); color: white; }"
        "QPushButton:disabled { background-color: rgb(58,58,58); color: rgb(136,136,136); }");
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

// ---------------------------------------------------------------------------
// Postprocess job polling
// ---------------------------------------------------------------------------

void TsQtAWS::RefreshPostprocessStatus()
{
    if (fPostprocessJobId.isEmpty()) {
        fPostprocessMonitorText->setPlainText("No postprocess job id.");
        return;
    }
    QStringList args;
    args << "batch"
         << "describe-jobs"
         << "--jobs" << fPostprocessJobId;
    QString out;
    QString err;
    int code = 0;
    if (!RunAws(args, &out, &err, &code) || code != 0) {
        fPostprocessMonitorText->setPlainText(QString("describe-jobs failed:\n") + err);
        return;
    }
    fPostprocessMonitorText->setPlainText(out);
}

void TsQtAWS::OnPollPostprocessJob()
{
    RefreshPostprocessStatus();
    QString t = fPostprocessMonitorText->toPlainText();
    QJsonParseError pe;
    QJsonDocument jd = QJsonDocument::fromJson(t.toUtf8(), &pe);
    if (pe.error != QJsonParseError::NoError || !jd.isObject())
        return;
    QJsonArray jobs = jd.object().value("jobs").toArray();
    bool anyRunning = false;
    for (const QJsonValue& jv : jobs) {
        QString st = jv.toObject().value("status").toString();
        if (st == "RUNNING" || st == "PENDING" || st == "RUNNABLE" || st == "STARTING")
            anyRunning = true;
    }
    if (!anyRunning && !jobs.isEmpty() && fPostTimer)
        fPostTimer->stop();
}

void TsQtAWS::StartPostprocessMonitor()
{
    if (!fPostTimer) {
        fPostTimer = new QTimer(this);
        connect(fPostTimer, &QTimer::timeout, this, &TsQtAWS::OnPollPostprocessJob);
    }
    RefreshPostprocessStatus();
    fPostTimer->start(8000);
}

void TsQtAWS::StopPostprocessMonitor()
{
    if (fPostTimer)
        fPostTimer->stop();
}

#endif
