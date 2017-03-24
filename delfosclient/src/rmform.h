#ifndef RMFORM_H
#define RMFORM_H

// Qt includes
#include <QWidget>
#include <wcore/utils/unit_test/tester/RMTester.h>

// Project includes
#include <src/commonparameters.h>

namespace Ui {
class RMForm;
}

class RMForm : public QWidget
{
  Q_OBJECT

public:
  explicit RMForm(QWidget *parent = 0);
  ~RMForm();

private:
  Ui::RMForm *ui;

  /**
   * @brief log_iface
   */
  delfosErrorHandler _weh;

  /**
   * @brief _rm_client client for rm
   */
  RMTester* _rm_client;


private slots:

  /**
   * Clicked slot buttons
   */
  void on_coreByCampaignButton_clicked();
  void on_coreByBranchButton_clicked();
  void on_coreByVacenterButton_clicked();
  void on_vacenterByUserButton_clicked();
  void on_modulesbytypeButton_clicked();
};

#endif // RMFORM_H
