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

#include "TsQtAWSUtils.hh"

#include <QComboBox>
#include <QDate>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QSizePolicy>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QSettings>
#include <QSet>
#include <QVBoxLayout>
#include <QVector>

#include <algorithm>

namespace TsQtAWSUtils {

const char* const kFileCompute = "batch-compute-env.json";
const char* const kFileJobDef = "batch-job-definition.json";
const char* const kFileJobDefPost = "batch-postP-job-definition.json";
const char* const kFileSubmit = "topas_submit.sh";
const char* const kFilePost = "postProcess_submit.sh";

QString PrimaryButtonStyleSheet()
{
    return QStringLiteral(
        "QPushButton { border-radius: 8px; padding: 6px 12px; border: none; }"
        "QPushButton:enabled { background-color: rgb(10,132,255); color: white; }"
        "QPushButton:disabled { background-color: rgb(58,58,58); color: rgb(136,136,136); }");
}

void ApplyPrimaryButtonStyle(QPushButton* button)
{
    if (button)
        button->setStyleSheet(PrimaryButtonStyleSheet());
}

void AddUnderlinedTitle(QVBoxLayout* layout, const QString& richText)
{
    if (!layout)
        return;
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
}

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
        ApplyPrimaryButtonStyle(add);
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
        ApplyPrimaryButtonStyle(remove);
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


} // namespace TsQtAWSUtils

#endif
