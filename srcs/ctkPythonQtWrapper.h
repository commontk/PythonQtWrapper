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

#ifndef __ctkPythonQtWrapper_h
#define __ctkPythonQtWrapper_h

// Qt includes
#include <QStringList>

class ctkPythonQtWrapper
{
public:
  ctkPythonQtWrapper();
  ~ctkPythonQtWrapper();

  void setVerbose(bool value);
  bool verbose()const;
  void displayVerboseMessage(const QString& msg)const;

  QString wrappingNamespace()const;
  QString wrappingNamespaceUnderscore()const;
  void setWrappingNamespace(const QString& newWrappingNamespace);

  QString targetName()const;
  void setTargetName(const QString& newTargetName);

  bool setInput(const QStringList& pathToCppHeaders);
  bool setOutput(const QString& outputFile);

  int validateInputFiles();
  bool validate(const QString& filePath);

  bool generateOutputs();

  QString generateClassWrapperCode(const QString& className, const QString& parentClassName);
  QString generateRegisterClassCode(const QString& className, const QString& targetName);

  bool isRegularHeader(const QString& filePath);
  bool isPimplHeader(const QString& filePath);

  bool hasQObjectMacro(const QString& filePath);
  bool hasValidConstructor(const QString& filePath, const QString& className);
  bool hasVirtualPureMethod(const QString& filePath);

  bool extractParentClassName(const QString& filePath, const QString& className,
                              QString& parentClassName);

private:
  QString     ProgramName;

  QStringList PathToExistingCppHeaders;
  QString     OutputDir;

  bool        Verbose;
  QString     LastError;

  QString     WrappingNamespace;
  QString     TargetName;
};

#endif


