/*=========================================================================

  Library:   CTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.commontk.org/LICENSE

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/

// Qt includes
#include <QFile>
#include <QRegExp>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

// PythonQtWrapper includes
#include "ctkPythonQtWrapper.h"
#include "ctkPythonQtWrapperVersion.h"

// STD includes
#include <iostream>

//-----------------------------------------------------------------------------
ctkPythonQtWrapper::ctkPythonQtWrapper()
{
  this->Verbose = false;
  this->ProgramName = "PythonQtWrapper";
}

//-----------------------------------------------------------------------------
ctkPythonQtWrapper::~ctkPythonQtWrapper()
{
}

//-----------------------------------------------------------------------------
void ctkPythonQtWrapper::setVerbose(bool value)
{
  this->Verbose = value;
}

//-----------------------------------------------------------------------------
bool ctkPythonQtWrapper::verbose()const
{
  return this->Verbose;
}

//-----------------------------------------------------------------------------
void ctkPythonQtWrapper::displayVerboseMessage(const QString& msg)const
{
  if (!this->Verbose)
    {
    return;
    }
  QTextStream(stdout, QIODevice::WriteOnly) << msg << "\n";
}

//-----------------------------------------------------------------------------
QString ctkPythonQtWrapper::wrappingNamespace()const
{
  return this->WrappingNamespace;
}

//-----------------------------------------------------------------------------
QString ctkPythonQtWrapper::wrappingNamespaceUnderscore()const
{
  QString newWrappingNamespaceUnderscore = this->WrappingNamespace;
  return newWrappingNamespaceUnderscore.replace(".", "_");
}

//-----------------------------------------------------------------------------
void ctkPythonQtWrapper::setWrappingNamespace(const QString& newWrappingNamespace)
{
  this->WrappingNamespace = newWrappingNamespace;
}

//-----------------------------------------------------------------------------
QString ctkPythonQtWrapper::targetName()const
{
  return this->TargetName;
}

//-----------------------------------------------------------------------------
void ctkPythonQtWrapper::setTargetName(const QString& newTargetName)
{
  this->TargetName = newTargetName;
}

//-----------------------------------------------------------------------------
bool ctkPythonQtWrapper::setInput(const QStringList& pathToCppHeaders)
{
  bool valid = false;
  foreach(const QString& pathToCppHeader, pathToCppHeaders)
    {
    if (!QFile::exists(pathToCppHeader))
      {
      this->LastError = QString("warning: File %1 doesn't exist").arg(pathToCppHeader);
      std::cerr << qPrintable(this->LastError) << std::endl;
      break;
      }
    this->displayVerboseMessage(QString("setInput [%1]").arg(pathToCppHeader));
    this->PathToExistingCppHeaders << pathToCppHeader;
    valid = true;
    }
  return valid;
}

//-----------------------------------------------------------------------------
bool ctkPythonQtWrapper::setOutput(const QString& outputDir)
{
  this->displayVerboseMessage(QString("setOutput [%1]").arg(outputDir));
  this->OutputDir = outputDir;
  return true;
}

//-----------------------------------------------------------------------------
int ctkPythonQtWrapper::validateInputFiles()
{
  int rejectedCount = 0;
  foreach(const QString& pathToCppHeader, this->PathToExistingCppHeaders)
    {
    if (!this->validate(pathToCppHeader))
      {
      std::cerr << "error: " << qPrintable(this->LastError) << std::endl;
      rejectedCount++;
      }
    }
  return rejectedCount;
}

//-----------------------------------------------------------------------------
bool ctkPythonQtWrapper::validate(const QString& filePath)
{
  this->displayVerboseMessage(QString("validate [%1]").arg(filePath));
  if (!this->isRegularHeader(filePath))
    {
    this->LastError = QString("%1: skipping - Not a regular header").arg(filePath);
    return false;
    }
  if (this->isPimplHeader(filePath))
    {
    this->LastError = QString("%1: skipping - Pimpl header (*._p.h)").arg(filePath);
    return false;
    }

  if (!this->hasQObjectMacro(filePath))
    {
    this->LastError = QString("%1: skipping - No Q_OBJECT macro").arg(filePath);
    return false;
    }

  QFileInfo fileinfo(filePath);
  QString className = fileinfo.completeBaseName();
  this->displayVerboseMessage(QString("className [%1]").arg(className));

  if (!this->hasValidConstructor(filePath, className))
    {
    this->LastError = QString("%1: skipping - Missing expected constructor signature").arg(filePath);
    return false;
    }

  if (this->hasVirtualPureMethod(filePath))
    {
    this->LastError = QString("%1: skipping - Contains a virtual pure method").arg(filePath);
    return false;
    }

  QString parentClassName;
  if (!this->extractParentClassName(filePath, className, parentClassName))
    {
    this->LastError = QString("%1: skipping - Failed to extract parent className").arg(filePath);
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
bool ctkPythonQtWrapper::generateOutputs()
{
  QString target = this->targetName();
  QString wrapWrapIntDir =
      QString("generated_cpp/%1_%2").arg(this->wrappingNamespaceUnderscore(), target);

  if (!QDir().mkpath(QString("%1/%2").arg(this->OutputDir).arg(wrapWrapIntDir)))
    {
    this->LastError = QString("%1 - Failed to create directory").arg(wrapWrapIntDir);
    return false;
    }

  // Header file
  QString headerFilePath =
      wrapWrapIntDir + "/" +
      this->wrappingNamespaceUnderscore() + "_" + target + "0.h";
  QFile headerFile(headerFilePath);
  if (!headerFile.open(QIODevice::WriteOnly))
    {
    this->LastError = QString("%1 - Failed to open file").arg(headerFilePath);
    return false;
    }

  QTextStream headerStream(&headerFile);
  headerStream << "//\n"
      << "// File auto-generated by " << this->ProgramName << " " << PythonQtWrapper_VERSION << "\n"
      << "//\n"
      << "\n"
      << "#ifndef __" << this->wrappingNamespaceUnderscore() << "_" << target << "0_h\n"
      << "#define __" << this->wrappingNamespaceUnderscore() << "_" << target << "0_h\n"
      << "\n"
      << "#include <QWidget>\n";

  foreach(const QString& pathToHeader, this->PathToExistingCppHeaders)
    {
    headerStream << "#include \"" << QFileInfo(pathToHeader).baseName() << ".h\"\n";
    }

  headerStream << "\n";

  foreach(const QString& pathToHeader, this->PathToExistingCppHeaders)
    {
    QString className = QFileInfo(pathToHeader).completeBaseName();
    QString parentClassName;
    this->extractParentClassName(pathToHeader, className, parentClassName);
    headerStream << "\n";
    headerStream << generateClassWrapperCode(className, parentClassName);
    }

  headerStream << "#endif\n";

  // Init Cpp file
  QString initFilePath =
      QString("%1/%2_%3_init.cpp").arg(wrapWrapIntDir).arg(this->wrappingNamespaceUnderscore()).arg(target);
  QFile initFile(initFilePath);
  if (!initFile.open(QIODevice::WriteOnly))
    {
    this->LastError = QString("%1 - Failed to open file").arg(initFilePath);
    return false;
    }
  QTextStream initStream(&initFile);
  initStream << "//\n"
      << "// File auto-generated by " << this->ProgramName << " " << PythonQtWrapper_VERSION << "\n"
      << "//\n"
      << "\n"
      << "#include <PythonQt.h>\n"
      << "#include \"" << this->wrappingNamespaceUnderscore() << "_" << target << "0.h\"\n"
      << "\n"
      << "void PythonQt_init_" << this->wrappingNamespaceUnderscore()
                               << "_" << target << "(PyObject* module)\n"
      << "{\n"
      << "  Q_UNUSED(module);\n";

  foreach(const QString& pathToHeader, this->PathToExistingCppHeaders)
    {
    QString className = QFileInfo(pathToHeader).completeBaseName();
    initStream << "\n";
    initStream << this->generateRegisterClassCode(className, this->TargetName);
    }

  initStream << "}\n";

  return true;
}

//-----------------------------------------------------------------------------
QString ctkPythonQtWrapper::generateClassWrapperCode(const QString& className,
                                                     const QString& parentClassName)
{
  if (!parentClassName.isEmpty())
    {
    QString wrappedClassWithParent =
        "//-----------------------------------------------------------------------------\n"
        "class PythonQtWrapper_%1 : public QObject\n"
        "{\n"
        "  Q_OBJECT\n"
        "public:\n"
        "public slots:\n"
        "  %1* new_%1(%2*  parent = 0)\n"
        "    {\n"
        "    return new %1(parent);\n"
        "    }\n"
        "  void delete_%1(%1* obj) { delete obj; }\n"
        "};\n";
    return wrappedClassWithParent.arg(className).arg(parentClassName);
    }
  else
    {
    QString wrappedClassWithoutParent =
        "//-----------------------------------------------------------------------------\n"
        "class PythonQtWrapper_%1 : public QObject\n"
        "{\n"
        "  Q_OBJECT\n"
        "public:\n"
        "public slots:\n"
        "  %1* new_%1()\n"
        "    {\n"
        "    return new %1();\n"
        "    }\n"
        "  void delete_%1(%1* obj) { delete obj; }\n"
        "};\n";
    return wrappedClassWithoutParent.arg(className).arg(parentClassName);
    }
}

//-----------------------------------------------------------------------------
QString ctkPythonQtWrapper::generateRegisterClassCode(const QString& className,
                                                      const QString& targetName)
{
  QString registerClass =
      "  PythonQt::self()->registerClass(\n"
      "    &%1::staticMetaObject, \"%2\",\n"
      "    PythonQtCreateObject<PythonQtWrapper_%1>);\n";

  return registerClass.arg(className, targetName);
}

//-----------------------------------------------------------------------------
bool ctkPythonQtWrapper::isRegularHeader(const QString& filePath)
{
  QRegExp re("^.*\\.h$", Qt::CaseInsensitive);
  return re.exactMatch(filePath);
}

//-----------------------------------------------------------------------------
bool ctkPythonQtWrapper::isPimplHeader(const QString& filePath)
{
  QRegExp re("^.*_p\\.h", Qt::CaseInsensitive);
  return re.exactMatch(filePath);
}

//-----------------------------------------------------------------------------
bool ctkPythonQtWrapper::hasQObjectMacro(const QString& filePath)
{
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly))
    {
    this->LastError = QString("%1 - Failed to open file").arg(filePath);
    return false;
    }
  QTextStream stream(&file);
  while (!stream.atEnd())
    {
    QString line = stream.readLine();
    if (line.contains("Q_OBJECT"))
      {
      return true;
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
bool ctkPythonQtWrapper::hasValidConstructor(const QString& filePath,
                                             const QString& className)
{
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly))
    {
    this->LastError = QString("%1 - Failed to open file").arg(filePath);
    return false;
    }
  QTextStream stream(&file);
  QString content = stream.readAll();
  QString reStr = QString(
      "[^~]%1[\\s\\n]*\\([\\s\\n]*((QObject|QWidget)[\\s\\n]*\\*[\\s\\n]*\\w+[\\s\\n]*(\\=[\\s\\n]*(0|NULL)|,.*\\=.*\\)|\\)|\\)))").arg(className);
  QRegExp re(reStr);
  return re.indexIn(content) >=0;
}

//-----------------------------------------------------------------------------
bool ctkPythonQtWrapper::hasVirtualPureMethod(const QString& filePath)
{
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly))
    {
    this->LastError = QString("%1 - Failed to open file").arg(filePath);
    return false;
    }
  QTextStream stream(&file);
  QString content = stream.readAll();
  QRegExp re("virtual[\\w\\n\\s\\*\\(\\)]+\\=[\\s\\n]*(0|NULL)[\\s\\n]*;");
  return re.indexIn(content) >= 0;
}

//-----------------------------------------------------------------------------
bool ctkPythonQtWrapper::extractParentClassName(const QString& filePath,
                                                   const QString& className,
                                                   QString& parentClassName)
{
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly))
    {
    this->LastError = QString("%1 - Failed to open file").arg(filePath);
    return false;
    }
  QTextStream stream(&file);
  QString content = stream.readAll();

  parentClassName.clear();

  QRegExp reNoParent(QString("[^~]%1[\\s\\n]*\\([\\s\\n]*\\)").arg(className));
  if (reNoParent.indexIn(content) >= 0)
    {
    return true;
    }

  QRegExp reQObjectParent(QString("%1[\\s\\n]*\\([\\s\\n]*QObject[\\s\\n]*\\*[\\s\\n]*\\w+[\\s\\n]*(\\=[\\s\\n]*(0|NULL)|,.*\\=.*\\)|\\))").arg(className));
  if (reQObjectParent.indexIn(content) >= 0)
    {
    parentClassName = QLatin1String("QObject");
    return true;
    }

  QRegExp reQWidgetParent(QString("%1[\\s\\n]*\\([\\s\\n]*QWidget[\\s\\n]*\\*[\\s\\n]*\\w+[\\s\\n]*(\\=[\\s\\n]*(0|NULL)|,.*\\=.*\\)|\\))").arg(className));
  if (reQWidgetParent.indexIn(content) >= 0)
    {
    parentClassName = QLatin1String("QWidget");
    return true;
    }

  return false;
}




