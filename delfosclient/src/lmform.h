#ifndef LMFORM_H
#define LMFORM_H

#include <QWidget>
#include <QTimer>

// Project includes
#include <wcore/utils/unit_test/tester/LMTester.h>
#include "commonparameters.h"

namespace Ui {
class LMForm;
}

class LMForm : public QWidget
{
  Q_OBJECT

private:
  string _login_session;
  string _lead_id;
  string _seed_id;
  string _pin;

  typedef struct vagroup_t{
    string name;
    string guid;
    bool check;
  }vagroup_t;

  std::vector<string> _alive_type_array;


public:
  explicit LMForm(QWidget *parent = 0);
  ~LMForm();


  /**
   * @brief log_iface
   */
  delfosErrorHandler _weh;

  /**
   * @brief _rm_client client for rm
   */
  LMTester* _lm_tester;

  /**
   * @brief _alive_timer timer to send alive
   */
  QTimer* _alive_timer;

  /**
   * @brief _alive_timer timer to send alive
   */
  QTimer* _pin_alive_timer;

  /**
   * @brief _vagroup_map stores pair <name, guid>
   */
  std::map<string, vagroup_t> _vagroup_map;

  /**
   * @brief _vagroup_map stores pair <name, guid>
   */
  std::map<string, int> _action_map;

private:

  void create_lead();
  void create_delfosnar();

private slots:

  // Desktop User
  void on_loginButton_clicked();
  void on_logoutButton_clicked();
  void on_changeStatusButton_clicked();
  void on_checkVateamButton_clicked();
  void on_aliveButton_clicked();
  void on_getVateamsButton_clicked();
  void on_getResultsButton_clicked();

  // Landing session
  void on_requestLeadButton_clicked();
  void on_randomLocatorButton_clicked();
  void on_closeButton_clicked();
  void on_sendQuizButton_clicked();
  void on_getAvailabilityButton_clicked();

  // Desktop session
  void on_openSessionButton_clicked();
  void on_freeSessionResourcesButton_clicked();
  void on_setResultButton_clicked();
  void on_createLeadButton_clicked();
  void on_scheduleSessionButton_clicked();
  void on_loadDBLeadButton_clicked();
  void on_searchDBLeadutton_clicked();

  // Inbound
  void on_ivnewsessionButton_clicked();
  void on_ivflagButton_clicked();


private:
  Ui::LMForm *ui;

  // -----------------------------
  // Mockup Callback
  // -----------------------------

  /**
   * pbx_event event generated when Mockup PBX receives a request
   */
  void event_pbx(WJson* json_request, delfosXML* xml_request, WHttp* http_request);

  /**
   * event generated when Mockup Gari receives a request
   */
  void event_gari(WJson* json_request, delfosXML* xml_request, WHttp* http_request);
};

#endif // LMFORM_H
