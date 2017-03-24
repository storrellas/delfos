#ifndef ACMFORM_H
#define ACMFORM_H

// Qt includes
#include <QWidget>
#include <wcore/utils/unit_test/tester/ACMTester.h>

// Project includes
#include "commonparameters.h"

namespace Ui {
class ACMForm;
}

class ACMForm : public QWidget
{
  Q_OBJECT

private:
  Ui::ACMForm *ui;

  /**
   * @brief log_iface
   */
  delfosErrorHandler _weh;

  /**
   * @brief _rm_client client for rm
   */
  ACMTester* _acm_client;

  /**
   * @brief _token_map Stores token maps
   */
  std::map<std::string, std::string> _alpha_map;

public:
  explicit ACMForm(QWidget *parent = 0);
  ~ACMForm();

private slots:

  /**
   * Clicked slots
   */
  void on_newAlphaButton_clicked();
  void on_alphaStatusButton_clicked();
  void on_getAlphaButton_clicked();
  void on_goAlphaButton_clicked();
  void on_freeAlphaTokenButton_clicked();
  void on_freeAlphaListButton_clicked();
};

#endif // ACMFORM_H
