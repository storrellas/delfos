#ifndef GARIFORM_H
#define GARIFORM_H

#include <QWidget>
#include <wcore/utils/unit_test/tester/GARITester.h>

// Project includes
#include "commonparameters.h"

namespace Ui {
class GariForm;
}

class GariForm : public QWidget,
                 public KPIEventHandler,
                 public GARIEventHandler
{
  Q_OBJECT

public:
  explicit GariForm(QWidget *parent = 0);
  ~GariForm();

private slots:

  /**
   * Clicked slots
   */

  void on_createSeedButton_clicked();
  void on_createSessionButton_clicked();
  void on_deleteSessionButton_clicked();
  void on_deleteSeedButton_clicked();

  void on_readSessionButton_clicked();
  void on_updateSessionButton_clicked();

  void on_updateUserInfoButton_clicked();
  void on_readUserInfoButton_clicked();

  void on_updateSeedButton_clicked();

  void on_readSeedButton_clicked();

private:
  Ui::GariForm *ui;

  /**
   * @brief log_iface
   */
  delfosErrorHandler _weh;

  /**
   * @brief _rm_client client for rm
   */
  GARITester* _gari_tester;

  /**
   * @brief _kpi_mockup mockup of KPI for Gari
   */
  KPIMockup* _kpi_mockup;

  /**
   * @brief _seed_id stores the seed id generated
   */
  std::string _seed_id;

  /**
   * @brief _seed_id stores the session id generated
   */
  std::string _session_id;

  /**
   * @brief kpi_mockup_event handler for the mockup event
   * @param http_request
   * @return
   */
  bool kpi_mockup_event(const WHttp http_request) override;

  /**
   * @brief gari_event event from GARI
   * @param parser
   * @return
   */
  bool gari_event(delfos_command_parser::CommandParser& parser) override;

};

#endif // GARIFORM_H
