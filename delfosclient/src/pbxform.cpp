#include "pbxform.h"
#include "ui_pbxform.h"

namespace wc = delfos::core;
namespace wcc = delfos::core::constants;
namespace wcm = delfos::core::model;

const string PBX_CLIENT_IP = wcc::localhost_ip;
//const string PBX_CLIENT_IP = "preawspbx.delfos.com";
const string PBX_CLIENT_PORT = "9889";

const string PBX_TWILIO_IP = wcc::localhost_ip;
//const string PBX_TWILIO_IP = "preawspbx.delfos.com";
const string PBX_TWILIO_PORT = "9890";

const QString USER_PHONE =  "+34695634751";
//const QString AGENT_PHONE = "+34910800161";
const QString AGENT_PHONE = "+34673736410";
const QString THIRD_PHONE = "+34672771287";
const QString INBOUND_PHONE = "+34947880101";
const QString IV_PHONE = "+34947880102";
const int SERVICE_ID = 100521;

#define AUTOMATIC_HANGUP_ALL_CHANNELS
//#define UPDATE_BUTTONS_ACCORDING_TO_STATE

const bool ENABLE_LM_MOCKUP = false;

PbxForm::PbxForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::PbxForm)
{
  ui->setupUi(this);


  // Configure Logger
  _weh.setAllLogLevel(wc::severity_level::debug);
  _weh.setAllSyslogLevel(wc::severity_level::debug);
  _weh.StartEH("pbx_client", delfosLMServer, false);

  // Initilialise PbxClient
  _pbx_tester = new PBXTester(&_weh);
  _pbx_tester->set_connection_parameters(PBX_CLIENT_IP, PBX_CLIENT_PORT);
  _pbx_tester->set_event_pbx_handler(this);
  _pbx_tester->init(ENABLE_LM_MOCKUP);

  // Initilialise TwilioFake
  _twilio_fake = new TwilioMockup(&_weh);
  _twilio_fake->set_connection_parameters(PBX_TWILIO_IP, PBX_TWILIO_PORT);
  _twilio_fake->set_event_handler(this);
  _twilio_fake->init();

  // Connect signals and slots
  // ----------------------------------------
  connect(this, SIGNAL(update_buttons_signal()),
                      this, SLOT(update_buttons_slot()));
  connect(this, SIGNAL(hangup_channels_signal(QString)),
                      this, SLOT(hangup_channels_slot(QString)));

#ifdef UPDATE_BUTTONS_ACCORDING_TO_STATE
  // Initial status of buttons
  update_buttons_slot();
#endif

  _current_conference = wc::call_selector_type::pbx_bridge_user_agent;

  // --------------------
  // Cheating
  // --------------------
  ui->userPhoneLineEdit->setText(USER_PHONE);
  ui->agentPhoneLineEdit->setText(AGENT_PHONE);
  ui->thirdPhoneLineEdit->setText(THIRD_PHONE);


  ui->inboundPhoneLineEdit->setText(INBOUND_PHONE);
  ui->ivPhoneLineEdit->setText(IV_PHONE);
  ui->hangupPhoneLineEdit->setText(USER_PHONE);

  ui->bridgeStatusLineEdit->setText("Current:NONE/Next:USER-AGENT");

  bool pin_reply_ok = false;
  ui->pinOKCheckBox->setChecked(pin_reply_ok);
  _pbx_tester->set_pin_correct(pin_reply_ok);

}

PbxForm::~PbxForm()
{
  delete _pbx_tester;
  delete _twilio_fake;
  delete ui;
}

void PbxForm::generate_call_object(){

  // Selected service
  wcm::Service service(true);
  service.set_service_id(SERVICE_ID);

  // Destination number
  wc::WPhone user_phone;
  user_phone.set_phone(ui->userPhoneLineEdit->text().toStdString());
  wc::WPhone agent_phone;
  agent_phone.set_phone(ui->agentPhoneLineEdit->text().toStdString());
  wc::WPhone third_phone;
  third_phone.set_phone(ui->thirdPhoneLineEdit->text().toStdString());


  // Create user_channel
  wcm::CallChannel user_channel;
  user_channel.set_phone(user_phone);
  wcm::CallChannel agent_channel;
  agent_channel.set_phone(agent_phone);
  wcm::CallChannel third_channel;
  third_channel.set_phone(third_phone);


  // Generate call object
  _call.set_service(service);
  _call.set_session_id(NewGuid());
  _call.set_user_channel(user_channel);
  _call.set_agent_channel(agent_channel);
  _call.set_third_party_channel(third_channel);

}

// -------------------------------
// Inner Slots
// -------------------------------

void PbxForm::update_buttons_slot(){

  qDebug() << "update_buttons_slot";

  // Get current status
  bool user_called =
      _call.get_user_channel().get_current_status() == wcm::CallChannel::status::called_ok;
  bool agent_called =
      _call.get_agent_channel().get_current_status() == wcm::CallChannel::status::called_ok;
  bool third_called =
      _call.get_third_party_channel().get_current_status() == wcm::CallChannel::status::called_ok;
  bool user_bridged =
      _call.get_user_channel().get_current_status() == wcm::CallChannel::status::bridged_ok;
  bool agent_bridged =
      _call.get_agent_channel().get_current_status() == wcm::CallChannel::status::bridged_ok;
  bool third_bridged =
      _call.get_third_party_channel().get_current_status() == wcm::CallChannel::status::bridged_ok;

#ifdef UPDATE_BUTTONS_ACCORDING_TO_STATE
  bool user_in_channel  = user_called or user_bridged;
  bool agent_in_channel = agent_called or agent_bridged;
  bool thrid_in_channel = third_called or third_bridged;


  // Call buttons control
  bool user_to_call = not user_in_channel;
  bool agent_to_call = (not agent_in_channel) and user_called;
  bool third_to_call = (not thrid_in_channel) and user_bridged;
  ui->callUserButton->setEnabled(  user_to_call );
  ui->callAgentButton->setEnabled( agent_to_call );
  ui->callThirdButton->setEnabled( third_to_call );

  // Bridge button control
  bool user_agent_to_bridge = (user_called and agent_in_channel) and (not user_bridged);
  bool agent_third_to_bridge = (third_called and agent_in_channel) and (not third_bridged);
  ui->bridgeButton->setEnabled( user_agent_to_bridge or agent_third_to_bridge );

  // Hangup button control
  bool user_to_hangup = (user_in_channel);
  bool agent_to_hangup = (agent_in_channel);
  bool third_to_hangup = (thrid_in_channel);
  ui->hangupAllButton->setEnabled( user_to_hangup or agent_to_hangup or third_to_hangup);

  // Redirect button
  ui->redirectButton->setEnabled(thrid_in_channel);
#endif

  // Status label
  if(user_bridged and agent_bridged)
      ui->bridgeStatusLineEdit->setText("Current:USER-AGENT/Next:NONE");
  if(user_bridged and agent_bridged and third_called)
      ui->bridgeStatusLineEdit->setText("Current:USER-AGENT/Next:AGENT-THIRD");
  if(user_called and agent_bridged and third_bridged)
      ui->bridgeStatusLineEdit->setText("Current: AGENT-THIRD/Next:USER-AGENT");

}

void PbxForm::hangup_channels_slot(QString channel){
#ifdef AUTOMATIC_HANGUP_ALL_CHANNELS
  std::thread t(&PbxForm::hangup, this);
  t.detach();
#else
  QMessageBox msgBox;
  msgBox.setText("You received a hangup from '" + channel + "'");
  msgBox.setInformativeText("Do you want to proceed to hangup all channels?");
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  if( msgBox.exec() == QMessageBox::Yes ){
    std::thread t(&MainWindow::hangup, this);
    t.detach();
  }
#endif
}

// -------------------------------
// Clicked slots
// -------------------------------

void PbxForm::on_callUserButton_clicked()
{

  // Generates the call object
  generate_call_object();

  loggerMacroDebug("Generated call object");
  qDebug() << _call.to_str().c_str();

  // Launches call_user
  std::thread t(&PbxForm::call_user, this);
  t.detach();
}

void PbxForm::on_callAgentButton_clicked()
{
  std::thread t(&PbxForm::call_agent, this);
  t.detach();
}

void PbxForm::on_callThirdButton_clicked()
{
  std::thread t(&PbxForm::call_third, this);
  t.detach();
}

void PbxForm::on_bridgeButton_clicked()
{
  std::thread t(&PbxForm::bridge, this);
  t.detach();
}

void PbxForm::on_redirectButton_clicked()
{
  std::thread t(&PbxForm::redirect, this);
  t.detach();
}

void PbxForm::on_hangupAllButton_clicked()
{
  std::thread t(&PbxForm::hangup, this);
  t.detach();
}


void PbxForm::on_hangupButton_clicked()
{
  const QString hangup_phone = ui->hangupPhoneLineEdit->text();
  const string sid = _twilio_fake->get_sid(hangup_phone.toStdString());
  loggerMacroDebug("Hanging up phone " + hangup_phone + " Sid: " + sid.c_str());

  _twilio_fake->request_call_hangup(sid);
}

void PbxForm::on_inboundButton_clicked()
{
  const QString inbound_phone = ui->inboundPhoneLineEdit->text();
  const QString user_phone = ui->userPhoneLineEdit->text();
  loggerMacroDebug("Generate call inbound phone " + user_phone + " -> " + inbound_phone);


  _twilio_fake->request_call_inbound(user_phone.toStdString(), inbound_phone.toStdString());
}

void PbxForm::on_ivButton_clicked()
{
  const QString iv_phone = ui->ivPhoneLineEdit->text();
  const QString user_phone = ui->userPhoneLineEdit->text();

  loggerMacroDebug("Generate call inbound video phone " + user_phone + " -> " + iv_phone);

  const string pin = "12345";
  _twilio_fake->request_call_iv(user_phone.toStdString(), iv_phone.toStdString(), pin);
}

void PbxForm::on_pinOKCheckBox_clicked()
{
  loggerMacroDebug("Changing reply to pin OK checkbox");
  bool pin_reply_ok = ui->pinOKCheckBox->isChecked();
  _pbx_tester->set_pin_correct(pin_reply_ok);
}

// ---------------------
// actions to PBX
// ---------------------

void PbxForm::call_user(){

  // Check whether call is possible
  bool call_to_be_done = _call.get_user_channel().call();
  if( !call_to_be_done ){
    loggerMacroDebug("Call cannot be completed");
    return;
  }

  // Perform call request
  loggerMacroDebug("Perform call to user ...");
  WJson json_response;
  bool res = _pbx_tester->request_call(_pbx_tester->get_lm_ip(), _pbx_tester->get_lm_port(), false,
                                      wc::call_selector_type::pbx_call_user, _call, json_response);
  if( res ){
    loggerMacroDebug("Call performed ok!");
    _call.get_user_channel().pbx_ok();
  }else{
    loggerMacroDebug("Call Failed!");
    _call.get_user_channel().pbx_ko();
  }

  // Signal to update buttons
  emit update_buttons_signal();
}

void PbxForm::call_agent(){

  // Check whether call is possible
  bool call_to_be_done = _call.get_agent_channel().call();
  if( !call_to_be_done ){
    loggerMacroDebug("Call cannot be completed");
    return;
  }

  // Perform call request
  WJson json_response;
  loggerMacroDebug("Perform call to agent ...");
  bool res = _pbx_tester->request_call(_pbx_tester->get_lm_ip(), _pbx_tester->get_lm_port(), false,
                                      wc::call_selector_type::pbx_call_agent, _call, json_response);
  if( res ){
    loggerMacroDebug("Call performed ok!");
    _call.get_agent_channel().pbx_ok();
  }else{
    loggerMacroDebug("Call Failed!");
    _call.get_agent_channel().pbx_ko();
  }

  // Signal to update buttons
  emit update_buttons_signal();
}

void PbxForm::call_third(){

  // Check whether call is possible
  bool call_to_be_done = _call.get_third_party_channel().call();
  if( !call_to_be_done ){
    loggerMacroDebug("Call cannot be completed");
    return;
  }

  // Perform call request
  WJson json_response;
  loggerMacroDebug("Perform call to third ...");
  bool res = _pbx_tester->request_call(_pbx_tester->get_lm_ip(), _pbx_tester->get_lm_port(), false,
                                      wc::call_selector_type::pbx_call_third_party, _call, json_response);
  if( res ){
    loggerMacroDebug("Call performed ok!");
    _call.get_third_party_channel().pbx_ok();
  }else{
    loggerMacroDebug("Call Failed!");
    _call.get_third_party_channel().pbx_ko();
  }

  // Signal to update buttons
  emit update_buttons_signal();

}

void PbxForm::bridge(){

  // Perform call request
  loggerMacroDebug("Perform bridge ...");
  _call.hold();
  bool bridge_to_be_done = _call.bridge(_current_conference);
  if( !bridge_to_be_done ){
    loggerMacroDebug("Bridge cannot be completed");
    return;
  }

  WJson json_response;
  bool res = _pbx_tester->request_bridge(_current_conference, _call, json_response);
  if( res ){
    loggerMacroDebug("Call bridged ok!");
    _call.get_user_channel().pbx_ok();
    _call.get_agent_channel().pbx_ok();
    _call.get_third_party_channel().pbx_ok();

    // Check current conference
    if( _current_conference == wc::call_selector_type::pbx_bridge_user_agent ){
      _current_conference = wc::call_selector_type::pbx_bridge_agent_third;
    }else{
      _current_conference = wc::call_selector_type::pbx_bridge_user_agent;
    }

  }else{
    loggerMacroDebug("Call bridge Failed!");
    _call.get_user_channel().pbx_ko();
    _call.get_agent_channel().pbx_ko();
    _call.get_third_party_channel().pbx_ko();
  }

  // Signal to update buttons
  emit update_buttons_signal();
}

void PbxForm::hangup(){
  loggerMacroDebug("Performing hangup");


  // Perform call request
  WJson json_response;
  loggerMacroDebug("Perform hangup ...");
  _call.hangup(wc::call_member_type::all);
  bool res = _pbx_tester->request_hangup(wc::call_member_type::all, _call, json_response);
  if( res ){
    loggerMacroDebug("Hangup channels ok!");
    _call.get_user_channel().set_current_status(wcm::CallChannel::status::no_call);
    _call.get_agent_channel().set_current_status(wcm::CallChannel::status::no_call);
    _call.get_third_party_channel().set_current_status(wcm::CallChannel::status::no_call);
  }else{
    loggerMacroDebug("Hangup channels Failed!");
  }

  // Signal to update buttons
  emit update_buttons_signal();

}

void PbxForm::redirect(){

  // Bridge user-thrid
  _current_conference = wc::call_selector_type::pbx_bridge_user_third;
  bridge();

  // Perform call request
  WJson json_response;
  loggerMacroDebug("Perform hangup ...");
  _call.hangup(wc::call_member_type::all);
  bool res = _pbx_tester->request_hangup(wc::call_member_type::agent, _call, json_response);
  if( res ){
    loggerMacroDebug("Hangup channels ok!");
  }else{
    loggerMacroDebug("Hangup channels Failed!");
  }

  // Signal to update buttons
  emit update_buttons_signal();

}

// -------------------------------
// Event Handlers
// -------------------------------

bool PbxForm::twilio_event(const WHttp http_request){
  loggerMacroDebug("received twilio event ...")
  loggerMacroDebug( QString(http_request.to_str().c_str()) )
  return true;
}

bool PbxForm::twilio_request(const WHttp http_request, const WHttp http_response){
  loggerMacroDebug("Generated twilio event ...")
  loggerMacroDebug( QString(http_request.to_str().c_str()) )
  return true;
}

bool PbxForm::pbx_event(delfos_command_parser::CommandParser& parser){

  loggerMacroDebug("received pbx_event ..." + QString(parser.get_command_name().c_str()))

  if( parser.get_command_name() == wcc::EVENT_KEY_STR_PBX_HANGUP ){
    return pbx_event_hangup(parser);
  }else if( parser.get_command_name() == wc::as_string(wc::lm_event_type::pbx_check_iv_pin) ){
    return pbx_event_checkivpin(parser);
  }else if( parser.get_command_name() == wc::as_string(wc::lm_event_type::pbx_inbound) ){
    return pbx_event_inbound(parser);
  }
  return false;
}

bool PbxForm::pbx_event_hangup(delfos_command_parser::CommandParser& parser){
  // Ask for confirmation to proceed to hangup all channels
  string channel = parser.get_value_to_str("channel");
  emit hangup_channels_signal( QString(channel.c_str()) );

  return true;
}

bool PbxForm::pbx_event_checkivpin(delfos_command_parser::CommandParser& parser){

  const std::string pin = parser.get_value_to_str(wcc::LM_PBX_INPUT_TAG_DATA_PIN);
  loggerMacroDebug(QString("Received checkivpin -> ") + QString(pin.c_str()))

  return true;
}

bool PbxForm::pbx_event_inbound(delfos_command_parser::CommandParser& parser){

  const std::string pin          = parser.get_value_to_str(wcc::LM_PBX_INPUT_TAG_DATA_PIN);
  const std::string origin       = parser.get_value_to_str(wcc::LM_PBX_INPUT_TAG_DATA_ORIGIN);
  const std::string session_guid = parser.get_value_to_str(wcc::LM_PBX_INPUT_TAG_DATA_GUID);

  // Define action for the lead
  string params = "origin:" + origin + " pin:" + pin + " session_guid:" + session_guid;
  if( pin.empty() ){ // INBOUND
    loggerMacroDebug("Received request INBOUND " + QString(params.c_str()));
  } else { // INBOUND VIDEO
    loggerMacroDebug("Received request INBOUND VIDEO " + QString(params.c_str()));
  }

  // Update call object with received parameters
  ui->userPhoneLineEdit->setText(QString(origin.c_str()));
  _call.set_session_id(session_guid);
  generate_call_object();


  // User call has been completed
  _call.get_user_channel().call();
  _call.get_user_channel().pbx_ok();

  loggerMacroDebug("Generated call object with inbound call");
  qDebug() << _call.to_str().c_str();

  // Signal to update buttons
  emit update_buttons_signal();

  return true;
}
