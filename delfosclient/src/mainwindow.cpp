#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  // Set window title
  this->setWindowTitle(APP_NAME);
  ui->mainToolBar->hide();

  // Connect signals and slots
  // ----------------------------------------

  // Signals&slot from application
  connect(this, SIGNAL(consoleWriteSignal(QString)),
                      this, SLOT(consoleWriteSlot(QString)));


  // Insert specific tabs
  // ----------------------------------------
  ui->tabWidget->removeTab(0);
  ui->tabWidget->insertTab(0, &pbxform,  "PBXClient");
  ui->tabWidget->insertTab(1, &rmform,   "RMClient");
  ui->tabWidget->insertTab(2, &kpiform,  "KPIClient");
  ui->tabWidget->insertTab(3, &acmform,  "ACMClient");
  ui->tabWidget->insertTab(4, &rtvform,  "RTVClient");
  ui->tabWidget->insertTab(5, &gariform, "GARIClient");
  ui->tabWidget->insertTab(6, &lmform,   "LMClient");

  ui->tabWidget->setCurrentIndex(6);

  // Change stretch if index LMClient
  on_tabWidget_currentChanged(6);

}

void MainWindow::on_tabWidget_currentChanged(int index)
{
  QLayout* layout = ui->centralWidget->layout();
  QVBoxLayout* vbox_layout = static_cast<QVBoxLayout*>(layout);
  const int stretch = (index == 6)?4:1;
  vbox_layout->setStretch(0, stretch);
  vbox_layout->setStretch(1, 1);
}


MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::consoleWrite(const QString &line){
    emit consoleWriteSignal(line);
}

void MainWindow::consoleWriteSlot(const QString &line){
    QCoreApplication::processEvents();
    ui->consoleTextEdit->append(line);
}

void MainWindow::initialize(){
    loggerMacroDebug("Log from Initialize")

    loggerMacroDebug("Current thread->");
    qDebug() << QThread::currentThreadId();
}


