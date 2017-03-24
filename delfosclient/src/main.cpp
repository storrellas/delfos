#include "mainwindow.h"


// Qt Includes
#include <QApplication>
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QSettings>
#include <QDir>
#include <QTime>
#include <QPlainTextEdit>

// Project includes
#include <src/commonparameters.h>

MainWindow* pMainWindow = NULL;
#define ENABLE_MESSAGE_HANDLER

void consoleMessageHandler(QtMsgType, const QMessageLogContext&, const QString &msg)
{

  // Output to GUI
  QString lineGUI = QTime::currentTime().toString("HH:mm:ss:zzz | ") + msg;
  if(pMainWindow != NULL){
      pMainWindow->consoleWrite( lineGUI );
  }

  // Print also to console
  QString line = QTime::currentTime().toString("HH:mm:ss:zzz | ") + msg;
  fprintf(stderr, "%s\n", line.toLocal8Bit().data());
  fflush(stderr);

}


#include <regex>
#include <string>
#include <iostream>


int main(int argc, char *argv[])
{

/*
  string account_url = "https://127.0.0.1:9912/2010-04-01/Accounts/ACcf47f772bb0172a7bddbc4c9da9b0c84.json";
  std::regex account_regex ("(.*Accounts.*)(.*json)");
  if (std::regex_match (account_url,account_regex))
    std::cout << "1-string object matched\n";
  else
    std::cout << "1-string NOT object matched\n";


  std::regex call_regex ("(.*Accounts.*)(.*\/Calls\.json)");
  string call_url = "https://127.0.0.1:9912/2010-04-01/Accounts/ACcf47f772bb0172a7bddbc4c9da9b0c84/Calls.json";
  if (std::regex_match (call_url,call_regex))
    std::cout << "2-string object matched\n";
  else
    std::cout << "2-string NOT object matched\n";

  std::regex bridge_regex ("(.*Accounts.*)(.*Calls.*)(.*json)");
  string bridge_url = "https://127.0.0.1:9912/2010-04-01/Accounts/ACcf47f772bb0172a7bddbc4c9da9b0c84/Calls/ce0a22f5-2617-4e8f-a591-47793f9e70a0.json";
  if (std::regex_match (bridge_url,bridge_regex))
    std::cout << "3-string object matched\n";
  else
    std::cout << "3-string NOT object matched\n";


  std::regex account_regex2 ("(.*Accounts.*)(.*json)");
  if (std::regex_match (call_url,account_regex2))
    std::cout << "4-string object matched\n";
  else
    std::cout << "4-string NOT object matched\n";

  std::string s ("subject");
  std::regex e2 ("(sub)(.*)");
  if (std::regex_match (s,e2))
    std::cout << "3-string object matched\n";

  return 0;
/**/

  wc::Logger
    logger("coreclient", "..", wc::severity_level::info, wc::severity_level::info);

  QApplication a(argc, argv);
  MainWindow main_window;
  main_window.show();
  pMainWindow = &main_window;

#ifdef ENABLE_MESSAGE_HANDLER
    qInstallMessageHandler(consoleMessageHandler);
#endif

  loggerMacroDebug("First log")
  main_window.initialize();

  // Connect signal&slot
  QObject::connect(&main_window, SIGNAL(quit()), &a, SLOT(quit()));

  return a.exec();
}
