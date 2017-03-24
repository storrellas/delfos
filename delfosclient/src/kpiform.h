#ifndef KPIFORM_H
#define KPIFORM_H

// Qt includes
#include <QWidget>
#include <wcore/utils/unit_test/tester/KPITester.h>

// Project includes
#include "commonparameters.h"

namespace Ui {
class KpiForm;
}

class KpiForm : public QWidget
{
  Q_OBJECT

public:
  explicit KpiForm(QWidget *parent = 0);
  ~KpiForm();

private:
  Ui::KpiForm *ui;

  /**
   * @brief log_iface
   */
  delfosErrorHandler _weh;

  /**
   * @brief _rm_client client for rm
   */
  KPITester* _kpi_client;

private slots:

  /**
   * Clicked slots
   */
  void on_clientKpiButton_clicked();
  void on_interactionKpiButton_clicked();

};

#endif // KPIFORM_H
