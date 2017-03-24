#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt includes
#include <QMainWindow>
#include <QDebug>
#include <QThread>
#include <QMessageBox>

// Project includes
#include <src/rmform.h>
#include <src/pbxform.h>
#include <src/kpiform.h>
#include <src/acmform.h>
#include <src/rtvform.h>
#include <src/gariform.h>
#include <src/lmform.h>
#include <src/commonparameters.h>

#define APP_NAME "delfos Core Client"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

private:
  /**
   * @brief ui widget object
   */
  Ui::MainWindow *ui;

  /**
   * @brief rmform Form including RM controls
   */
  RMForm rmform;

  /**
   * @brief pbxform Form including PBX controls
   */
  PbxForm pbxform;

  /**
   * @brief kpiform Form including KPI controls
   */
  KpiForm kpiform;

  /**
   * @brief acmform Form including ACM controls
   */
  ACMForm acmform;

  /**
   * @brief acmform Form including RTV controls
   */
  RTVForm rtvform;

  /**
   * @brief acmform Form including GARI controls
   */
  GariForm gariform;

  /**
   * @brief acmform Form including LM controls
   */
  LMForm lmform;


public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  /*!
   * Writes a line to the visual console
   */
  void consoleWrite(const QString &line);

  /*!
   * \brief initialize Initialises the MainWindow properties after constructor
   */
  void initialize();

signals:

  /*!
   * \brief consoleWriteSignal Used for writing into the GUI console. There is a threading issue here
   * \param line
   */
  void consoleWriteSignal(QString line);

  /*!
   * \brief quit terminates the current application
   */
  void quit();


private slots:

  /*!
   * \brief consoleWriteSlot slot used to write into console
   * This enables to be in the thread where the GUI was created and launch QApplication::processEvents()
   * \param msg
   */
  void consoleWriteSlot(const QString& msg);

  void on_tabWidget_currentChanged(int index);
};

#endif // MAINWINDOW_H
