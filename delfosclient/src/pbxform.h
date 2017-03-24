#ifndef PBXFORM_H
#define PBXFORM_H

// Qt includes
#include <QWidget>
#include <wcore/utils/unit_test/tester/PBXTester.h>
#include <wcore/utils/unit_test/mockups/TwilioMockup.h>

// Project includes
#include <src/commonparameters.h>

namespace Ui {
class PbxForm;
}

class PbxForm : public QWidget, public PBXEventHandler,
                                public TwilioEventHandler
{
  Q_OBJECT

private:
  /**
   * @brief _pbx_client client for pbx
   */
  PBXTester* _pbx_tester;

  /**
   * @brief _pbx_client client for pbx
   */
  TwilioMockup* _twilio_fake;

  /**
   * @brief ui widget object
   */
  Ui::PbxForm *ui;

  /**
   * @brief call Stores parameters of the call
   */
  delfos::core::model::Call _call;
  delfos::core::call_selector_type _current_conference;

  /**
   * @brief log_iface
   */
  delfosErrorHandler _weh;


public:

  /**
   * @brief PbxForm Public constructor
   * @param parent
   */
  explicit PbxForm(QWidget *parent = 0);
  ~PbxForm();

private:

  // ---------------------
  // actions to PBX
  // ---------------------

  /**
   * @brief call_user Starts a call to the user
   */
  void call_user();

  /**
   * @brief call_agent Starts a call to the agent
   */
  void call_agent();

  /**
   * @brief call_third Starts a call to the third party
   */
  void call_third();

  /**
   * @brief bridge Starts a bridge between user-agent
   */
  void bridge();

  /**
   * @brief hangup Starts a hangup of all channels
   */
  void hangup();

  /**
   * @brief redirect Starts a redirect to third
   */
  void redirect();

  // ---------------------
  // event handlers
  // ---------------------

  /**
   * Signaled when Twilio Fake received
   * @param http_request
   * @param http_response
   * @return
   */
  bool twilio_event(const WHttp http_request) override;

  /**
   * @brief twilio_request A request was performed to PBX
   * @param parser
   * @return
   */
  bool twilio_request(const WHttp http_request, const WHttp http_response) override;

  /**
   * @brief pbx_event_hangup handles the hangup event from pbx
   * @param parser
   * @return
   */
  bool pbx_event(delfos_command_parser::CommandParser& parser) override;

  /**
   * @brief pbx_event_hangup handles the hangup event from pbx
   * @param parser
   * @return
   */
  bool pbx_event_hangup(delfos_command_parser::CommandParser& parser);

  /**
   * @brief pbx_event_checkivpin handles the checkivpin from pbx
   * @param parser
   * @return
   */
  bool pbx_event_checkivpin(delfos_command_parser::CommandParser& parser);

  /**
   * @brief pbx_event_checkivpin handles the checkivpin from pbx
   * @param parser
   * @return
   */
  bool pbx_event_inbound(delfos_command_parser::CommandParser& parser);

  /*!
   * \brief generate_call_object generates the call object with the constants
   */
  void generate_call_object();


signals:
  /*!
   * \brief update_buttons_signal updates the buttons state
   */
  void update_buttons_signal();

  /*!
   * \brief hangup_channels_signal used to send hangup to channels
   */
  void hangup_channels_signal(QString channel);

private slots:

  // -------------------------------
  // Inner Slots
  // -------------------------------

  /*!
   * \brief update_buttons_slot updates the buttons state
   */
  void update_buttons_slot();

  /*!
   * \brief hangup_channels_signal used to send hangup to channels if confirmation
   */
  void hangup_channels_slot(QString channel);

  /**
   * Clicked slots
   */
  void on_callUserButton_clicked();
  void on_callAgentButton_clicked();
  void on_bridgeButton_clicked();
  void on_callThirdButton_clicked();
  void on_redirectButton_clicked();
  void on_hangupAllButton_clicked();
  void on_hangupButton_clicked();
  void on_inboundButton_clicked();
  void on_ivButton_clicked();
  void on_pinOKCheckBox_clicked();

};

#endif // PBXFORM_H
