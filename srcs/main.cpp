
// Qt includes
#include <QCoreApplication>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

// PythonQtWrapper includes
#include "ctkCommandLineParser.h"
#include "ctkPythonQtWrapper.h"
#include "ctkPythonQtWrapperVersion.h"

// STD includes
#include <iostream>
#include <cstdlib>

namespace
{
//-----------------------------------------------------------------------------
void printHelp(const ctkCommandLineParser& parser)
{
  std::cout << "PythonQtWrapper version 0.0.0\n"
      << "Usage\n\n"
      << "  PythonQtWrapper [options] -o <output-file> <path-to-cpp-header-file> [<path-to-cpp-header-file> ...]\n\n"
      << "Options\n"
      << qPrintable(parser.helpText()) << std::endl;
}
//-----------------------------------------------------------------------------
void printHelpUsage()
{
  std::cerr << "Specify --help for usage." << std::endl;
}
}

//-----------------------------------------------------------------------------
int main(int argc, char * argv[])
{
  QCoreApplication app(argc, argv);

  ctkCommandLineParser parser;
  // Use Unix-style argument names
  parser.setArgumentPrefix("--", "-");
  // Add command line argument names
  parser.addArgument("help", "h", QVariant::Bool, "Print usage information and exit.");
  parser.addArgument("verbose", "v", QVariant::Bool, "Enable verbose output.");
  parser.addArgument("wrapping-namespace", "wns", QVariant::String, "Wrapping namespace.", QVariant("org.commontk.foo"));
  parser.addArgument("target-name", "p", QVariant::String, "Target name.");
  parser.addArgument("check-only", "c", QVariant::Bool, "Return 1 (or 0) indicating if the file"
                     "could be successfully wrapped.");
  parser.addArgument("output-dir", "o", QVariant::String, "Output directory");
  
  // Parse the command line arguments
  bool ok = false;
  QHash<QString, QVariant> parsedArgs = parser.parseArguments(QCoreApplication::arguments(), &ok);
  if (!ok)
    {
    std::cerr << "Error parsing arguments: " << qPrintable(parser.errorString()) << std::endl;
    printHelpUsage();
    return EXIT_FAILURE;
    }
  // Show help message
  if (parsedArgs.contains("help"))
    {
    printHelp(parser);
    return EXIT_SUCCESS;
    }

  QString wrappingNamespace = parsedArgs.value("wrapping-namespace").toString();
  if (wrappingNamespace.isEmpty())
    {
    std::cerr << "error: Wrapping namespace not specified" << std::endl;
    printHelpUsage();
    return EXIT_FAILURE;
    }
  if (!QRegExp("[a-zA-Z0-9]+(\\.[a-zA-Z0-9]+)*").exactMatch(wrappingNamespace))
    {
    std::cerr << "error: Invalid wrapping namespace. Should match: [a-zA-Z0-9]+(.[a-zA-Z0-9]+)*" << std::endl;
    printHelpUsage();
    return EXIT_FAILURE;
    }

  QString outputDir = parsedArgs.value("output-dir").toString();
  if (outputDir.isEmpty())
    {
    std::cerr << "error: Output directory not specified" << std::endl;
    printHelpUsage();
    return EXIT_FAILURE;
    }
  QDir dir = QDir(outputDir);
  if (!dir.exists() || !dir.isReadable())
    {
    std::cerr << "error: Output directory non existent or non readable ["
        << qPrintable(outputDir) << "]" << std::endl;
    return EXIT_FAILURE;
    }

  if (parser.unparsedArguments().count() == 0)
    {
    std::cerr << "error: <path-to-cpp-header-file> not specified" << std::endl;
    printHelpUsage();
    return EXIT_FAILURE;
    }

  ctkPythonQtWrapper wrapper;
  wrapper.setVerbose(parsedArgs.contains("verbose"));
  wrapper.setWrappingNamespace(wrappingNamespace);

  if (!wrapper.setOutput(outputDir))
    {
    std::cerr << "error: Failed to set output" << std::endl;
    return EXIT_FAILURE;
    }

  if (!wrapper.setInput(parser.unparsedArguments()))
    {
    std::cerr << "error: Failed to set input" << std::endl;
    return EXIT_FAILURE;
    }

  int rejectedHeaders = wrapper.validateInputFiles();

  if (parsedArgs.contains("check-only"))
    {
    return rejectedHeaders;
    }

  if (rejectedHeaders == parser.unparsedArguments().count())
    {
    std::cerr << "error: All specified headers have been rejected" << std::endl;
    return EXIT_FAILURE;
    }

  QString targetName = parsedArgs.value("target-name").toString();
  if (parser.unparsedArguments().count() == 1)
    {
    if (targetName.isEmpty())
      {
      QFileInfo fileInfo(parser.unparsedArguments().value(0));
      targetName = fileInfo.baseName();
      }
    }
  else
    {
    if (targetName.isEmpty())
      {
      std::cerr << "error: Target name hasn't been specified" << std::endl;
      printHelpUsage();
      return EXIT_FAILURE;
      }
    }
  wrapper.setTargetName(targetName);

  wrapper.generateOutputs();
  
  return EXIT_SUCCESS;
}
