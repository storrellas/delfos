#include "lmform.h"
#include "ui_lmform.h"

namespace wc = delfos::core;
namespace wcm = delfos::core::model;
namespace wcc = delfos::core::constants;

const string LM_CLIENT_IP = wcc::localhost_ip;
const string LM_CLIENT_PORT = "8011";

const int LM_TIMER_ALIVE = 5000;
const int LM_TIMER_PIN = 5000;

// Local constants - For MySQL

const string LM_USER_ID = "C2729F6D-8A4F-49A6-8ED5-819A617AA7E4";
const string LM_BRANCH_ID = "D487678F-ABF5-4B38-B55A-5AEA79640420";
const string LM_BRANCHGROUP_ID = "23290B6D-693B-44C3-9DD1-F4FEF0B5C9A3";
const string LM_BRANCHGROUP_WNAR_ID = "23290B6D-693B-44C3-9DD1-F4FEF0B5C9A4";
const string LM_CAMPAIGN_ID = "23290B6D-693B-44C3-9DD1-F4FEF0B5C9A3";
const string LM_VAGROUP_ID = "D47487C9-4B7F-4BC4-A5EE-6D9D1F2BC0FA";
const string LM_VACENTER_ID = "A311DC47-FC05-4856-B850-54F15EC8F40F";
const string LM_PHONE = "+34695698745";
const string LM_PHONE_COUNTRY = "ES";
const string LM_NAME = "John Matheus";
const string LM_EMAIL = "";

const string ALIVE_EMPTY_TYPE       = "Empty";
const string ALIVE_LEAD_TYPE        = "Lead";
const string ALIVE_BRANCHGROUP_TYPE = "Branchgroup";

const unsigned int LM_LEAD_RESULT = 1007;
const bool ENABLE_PBX_MOCKUP = false;
const bool ENABLE_GARI_MOCKUP = false;

LMForm::LMForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::LMForm)
{
  ui->setupUi(this);

  // Configure Logger
  _weh.setAllLogLevel(wc::severity_level::debug);
  _weh.setAllSyslogLevel(wc::severity_level::debug);
  _weh.StartEH("lm_client", delfosLMServer, false);

  // Initilialise KPIClient
  _lm_tester = new LMTester(&_weh);
  _lm_tester->set_web_connection_parameters(LM_CLIENT_IP, LM_CLIENT_PORT);
  MockupCoreModule::event_post_handler_t f_ptr = (MockupCoreModule::event_post_handler_t)(&LMForm::event_pbx);
  _lm_tester->set_pbx_event_handler(f_ptr);
  f_ptr = (MockupCoreModule::event_post_handler_t)(&LMForm::event_gari);
  _lm_tester->set_gari_event_handler(f_ptr);
  _lm_tester->init(ENABLE_PBX_MOCKUP, ENABLE_GARI_MOCKUP);

  // Timer to send alive request
  _alive_timer = new QTimer(this);
  _pin_alive_timer = new QTimer(this);
  connect(_alive_timer, SIGNAL(timeout()), this, SLOT(on_aliveButton_clicked()));
  connect(_pin_alive_timer, SIGNAL(timeout()), this, SLOT(on_ivflagButton_clicked()));

  // Generate status combobox
  for(int i = 0; i < static_cast<uint>(wc::agent_status::max); i++){
    string status = wc::as_string(static_cast<wc::agent_status>(i));
    ui->statusComboBox->addItem(QString(status.c_str()));
  }

  // Generate action combobox
  string action_str = wc::as_string(wc::lead_action_type::call_me_now);
  ui->actionComboBox->addItem(QString(action_str.c_str()));
  _action_map.insert(pair<string,int>(action_str, static_cast<int>(wc::lead_action_type::call_me_now)));
  action_str = wc::as_string(wc::lead_action_type::queued);
  ui->actionComboBox->addItem(QString(action_str.c_str()));
  _action_map.insert(pair<string,int>(action_str, static_cast<int>(wc::lead_action_type::queued)));
  action_str = wc::as_string(wc::lead_action_type::scheduled_from_landing);
  ui->actionComboBox->addItem(QString(action_str.c_str()));
  _action_map.insert(pair<string,int>(action_str, static_cast<int>(wc::lead_action_type::scheduled_from_landing)));

  // Session Type
  string session_type_str = wc::as_string(wc::session_type::lead);
  ui->sessionTypeComboBox->addItem(QString::fromStdString(session_type_str));
  session_type_str = wc::as_string(wc::session_type::delfosnar);
  ui->sessionTypeComboBox->addItem(QString::fromStdString(session_type_str));

  // Configure alive type
  _alive_type_array.push_back(ALIVE_EMPTY_TYPE);
  _alive_type_array.push_back(ALIVE_LEAD_TYPE);
  _alive_type_array.push_back(ALIVE_BRANCHGROUP_TYPE);
  for( auto item : _alive_type_array)
    ui->aliveComboBox->addItem(QString::fromStdString(item));

  // -------------------------------
  // Setting default phone
  // -------------------------------
  ui->locatorLineEdit->setText(QString(LM_PHONE.c_str()));

}

LMForm::~LMForm()
{
  delete _lm_tester;
  delete ui;
}

// -----------------------------
// Desktop User slots
// -----------------------------

void LMForm::on_loginButton_clicked()
{
  loggerMacroDebug("Requesting 'login' to LM ...")

  WJson json_response;
  bool res = _lm_tester->request_login(LM_USER_ID, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))

    // Capture session_id
    if( json_response.HasMember(WAGENT_KEY_STR_SESSION_ID)
        and json_response[WAGENT_KEY_STR_SESSION_ID].IsString()){
      _login_session = json_response[WAGENT_KEY_STR_SESSION_ID].GetString();
      _alive_timer->start(LM_TIMER_ALIVE);
    }

    // Capture vateams
    if(json_response.HasMember(WAGENT_KEY_STR_VATEAMS)
        and json_response[WAGENT_KEY_STR_VATEAMS].IsArray()){

        WJson vateam_array_json = json_response[WAGENT_KEY_STR_VATEAMS];
        for (auto itr = vateam_array_json.Begin(); itr != vateam_array_json.End(); ++itr){
          WJson vateam_json = (*itr);
          const string guid = vateam_json[VAGROUP_KEY_STR_GUID.c_str()].GetString();
          const string name = vateam_json[VAGROUP_KEY_STR_NAME.c_str()].GetString();

          _vagroup_map.clear();
          vagroup_t vagroup;
          vagroup.name = name;
          vagroup.guid = guid;
          vagroup.check = false;
          _vagroup_map.insert(std::pair<string, vagroup_t>(name, vagroup));

          ui->vateamComboBox->clear();
          ui->vateamComboBox->addItem(QString(name.c_str()));
        }

    }

    loggerMacroDebug("Login session -> " + QString::fromStdString(_login_session))
  }
}

void LMForm::on_logoutButton_clicked()
{
  loggerMacroDebug("Requesting 'logout' to LM ...")

  WJson json_response;
  bool res = _lm_tester->request_logout(LM_USER_ID, _login_session, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
    _alive_timer->stop();
    _pin_alive_timer->stop();
    _vagroup_map.clear();
  }
}

void LMForm::on_changeStatusButton_clicked()
{
  loggerMacroDebug("Requesting 'change status' to LM ...")
  const int current_index = ui->statusComboBox->currentIndex();
  wc::agent_status status = static_cast<wc::agent_status>(current_index);
  WJson json_response;
  bool res = _lm_tester->request_change_status(LM_USER_ID, _login_session, status, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}

void LMForm::on_checkVateamButton_clicked()
{

  const int current_index = ui->vateamComboBox->currentIndex();
  const string name = ui->vateamComboBox->itemText(current_index).toStdString();
  auto it = _vagroup_map.find(name);
  if( it == _vagroup_map.end() ){
    loggerMacroDebug("Vagroup not found")
    return;
  }
  const string guid = it->second.guid;


  WJson json_response;
  bool res = false;
  if( it->second.check == false ){
    loggerMacroDebug("Requesting 'check vateam' to LM ...")
    res = _lm_tester->request_check_vateam(LM_USER_ID, _login_session, guid, json_response);
  }else{
    loggerMacroDebug("Requesting 'uncheck vateam' to LM ...")
    res = _lm_tester->request_uncheck_vateam(LM_USER_ID, _login_session, guid, json_response);
  }
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
    it->second.check = !it->second.check;
    if( it->second.check ){
      ui->checkVateamButton->setText("Uncheck Vateam");
    }else{
      ui->checkVateamButton->setText("Check Vateam");
    }

  }

}

void LMForm::on_aliveButton_clicked()
{
  loggerMacroDebug("Requesting 'alive' to LM ...")
  WJson json_response;

  const int index = ui->aliveComboBox->currentIndex();
  bool res;
  if( _alive_type_array[index] == ALIVE_EMPTY_TYPE ){
    res = _lm_tester->request_alive(LM_USER_ID, _login_session, json_response);
  }else if( _alive_type_array[index] == ALIVE_LEAD_TYPE ){
    res = _lm_tester->request_alive(LM_USER_ID, _login_session, _lead_id, json_response);
  }else if( _alive_type_array[index] == ALIVE_BRANCHGROUP_TYPE ){
    set<string> branchgroup_array;
    branchgroup_array.insert(LM_BRANCHGROUP_WNAR_ID);
    res = _lm_tester->request_alive(LM_USER_ID, _login_session, branchgroup_array, json_response);
  }else{
    return;
  }

  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}

void LMForm::on_getVateamsButton_clicked()
{
  loggerMacroDebug("Requesting 'getvateams' to LM ...")
  WJson json_response;
  bool res = _lm_tester->request_getvateams(LM_USER_ID, LM_VACENTER_ID, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }

}


// -----------------------------
// Landing Session slots
// -----------------------------

void LMForm::on_requestLeadButton_clicked()
{
  loggerMacroDebug("Requesting 'request lead' to LM ...")
  const int current_index = ui->actionComboBox->currentIndex();
  const string action_str = ui->actionComboBox->itemText(current_index).toStdString();
  auto it = _action_map.find(action_str);
  if(it == _action_map.end()){
    loggerMacroDebug("Action not found")
    return;
  }

  wc::lead_action_type action = static_cast<wc::lead_action_type>(it->second);
  wcm::SessionMetadata metadata;
  metadata.fillTestMetadata(NewGuid());

  WJson json_response;
  const string phone = ui->locatorLineEdit->text().toStdString();
  const string seed_id;
  std::chrono::system_clock::time_point scheduled_datetime;
  if( action == wc::lead_action_type::scheduled_from_landing)
    scheduled_datetime = std::chrono::system_clock::now();
  bool res = _lm_tester->request_lead(action, LM_BRANCH_ID, phone, LM_PHONE_COUNTRY,
                                      LM_NAME, LM_EMAIL, seed_id, scheduled_datetime, metadata, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))

    if(json_response.HasMember(helper_capitalize(XML_TAG_DATA).c_str())
        and json_response[helper_capitalize(XML_TAG_DATA).c_str()].IsObject()){

       WJson lead_json = json_response[helper_capitalize(XML_TAG_DATA).c_str()];
       if(lead_json.HasMember(LEAD_KEY_STR_LEAD.c_str())
           and lead_json[LEAD_KEY_STR_LEAD.c_str()].IsString()){
          _lead_id = lead_json[LEAD_KEY_STR_LEAD.c_str()].GetString();
       }
    }

  }

}

void LMForm::on_randomLocatorButton_clicked()
{
  loggerMacroDebug("Generating new random locator to LM ...")
  qsrand(qrand());
  QString random = QString::number(qrand() % ((1000 + 1)));
  QString phone = ui->locatorLineEdit->text();
  loggerMacroDebug( phone )
  QString random_phone = phone.mid(0, phone.count() - 3 ) + random;
  ui->locatorLineEdit->setText(random_phone);
}

void LMForm::on_getAvailabilityButton_clicked()
{
  loggerMacroDebug("Requesting 'get_availability' to LM ...")
  WJson json_response;
  bool res = _lm_tester->request_get_availability(LM_BRANCH_ID, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}


void LMForm::on_closeButton_clicked()
{
  loggerMacroDebug("Requesting 'close' to LM ...")

  // As event is sent from landing agent and login_session is sent empty
  const string agent_id = "";
  const string login_session = "";
  WJson json_response;
  bool res = _lm_tester->request_close(_lead_id, agent_id, login_session, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}

void LMForm::on_sendQuizButton_clicked()
{
  loggerMacroDebug("Requesting 'send quiz' to LM ...")

  std::vector<string> quiz_result;
  for(int i = 0; i < wcc::QUIZ_SIZE; ++i)
    quiz_result.push_back(std::to_string(i));
/*
  delfosXML xml_response;
  bool res = _lm_tester->request_send_quiz(_lead_id, quiz_result, xml_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(xml_response.toString().c_str()))
  }
/**/
  WJson json_response;
  bool res = _lm_tester->request_send_quiz(_lead_id, quiz_result, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }

}

// -----------------------------
// Desktop Session slots
// -----------------------------

void LMForm::on_openSessionButton_clicked()
{
  loggerMacroDebug("Requesting 'open session' to LM ...")
  WJson json_response;
  const string phone = ui->locatorLineEdit->text().toStdString();
  bool res = _lm_tester->request_open_session(phone, LM_USER_ID, _login_session, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))

    // Capture lead guid
    if(json_response.HasMember(LEAD_KEY_STR_LEAD.c_str())
        and json_response[LEAD_KEY_STR_LEAD.c_str()].IsObject()){

       WJson lead_json = json_response[LEAD_KEY_STR_LEAD.c_str()];
       if(lead_json.HasMember(LEAD_KEY_STR_GUID.c_str())
           and lead_json[LEAD_KEY_STR_GUID.c_str()].IsString()){
          _lead_id = lead_json[LEAD_KEY_STR_GUID.c_str()].GetString();
       }
    }
  }
}

void LMForm::on_freeSessionResourcesButton_clicked()
{
  loggerMacroDebug("Requesting 'free session resources' to LM ...")
  WJson json_response;
  bool res = _lm_tester->request_free_sesion_resources(_lead_id, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}

void LMForm::on_setResultButton_clicked()
{
  loggerMacroDebug("Requesting set result to LM ...")
  WJson json_response;
  bool res = _lm_tester->request_set_result(_lead_id, LM_LEAD_RESULT, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }

}

void LMForm::create_lead(){

  loggerMacroDebug("Requesting 'create lead' resources to LM ...")
  _seed_id = NewGuid();
  const string phone = ui->locatorLineEdit->text().toStdString();
  const string name = "";
  WJson json_response;
  bool res = _lm_tester->request_create_lead(_seed_id, phone, LM_PHONE_COUNTRY, name,
                                             LM_CAMPAIGN_ID, LM_VAGROUP_ID, LM_USER_ID, _login_session, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))

    // Capture lead guid
    if(json_response.HasMember(LEAD_KEY_STR_LEAD.c_str())
        and json_response[LEAD_KEY_STR_LEAD.c_str()].IsObject()){

       WJson lead_json = json_response[LEAD_KEY_STR_LEAD.c_str()];
       if(lead_json.HasMember(LEAD_KEY_STR_GUID.c_str())
           and lead_json[LEAD_KEY_STR_GUID.c_str()].IsString()){
          _lead_id = lead_json[LEAD_KEY_STR_GUID.c_str()].GetString();
       }
    }
  }
}

void LMForm::create_delfosnar(){
  loggerMacroDebug("Requesting 'create delfosnar' resources to LM ...")

  const string seed_id = NewGuid();
  WJson json_response;
  bool res = _lm_tester->request_create_delfosnar(LM_BRANCHGROUP_WNAR_ID, "title", "description", seed_id,
                                             LM_USER_ID, _login_session, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}

void LMForm::on_createLeadButton_clicked()
{
  wc::session_type session_type =
      static_cast<wc::session_type>(ui->sessionTypeComboBox->currentIndex());

  if( session_type == wc::session_type::lead ){
    create_lead();
  }else if(session_type == wc::session_type::delfosnar){
    create_delfosnar();
  }

}

void LMForm::on_scheduleSessionButton_clicked()
{
  loggerMacroDebug("Requesting 'schedule session' resources to LM ...")

  auto schedule_datetime = std::chrono::system_clock::now();
  schedule_datetime += std::chrono::hours(2);
  const string phone = ui->locatorLineEdit->text().toStdString();
  WJson json_response;
  bool res = _lm_tester->request_schedule_session(_lead_id, schedule_datetime, phone,
                                                  LM_PHONE_COUNTRY, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}

void LMForm::on_loadDBLeadButton_clicked()
{
  const int current_index = ui->vateamComboBox->currentIndex();
  loggerMacroDebug("Requesting 'load DB Lead' to LM ...")
  WJson json_response;
  bool res = _lm_tester->request_load_db_lead(_lead_id, LM_USER_ID, _login_session, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}

void LMForm::on_searchDBLeadutton_clicked()
{
  loggerMacroDebug("Requesting 'search_db_lead' to LM ...")
  string locator = ui->locatorLineEdit->text().toStdString();
  WJson json_response;
  bool res = _lm_tester->request_search_db_lead(_lead_id, LM_USER_ID, _login_session,
                                                locator, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}


void LMForm::on_getResultsButton_clicked()
{
  loggerMacroDebug("Requesting 'get_results' to LM ...")
  WJson json_response;
  bool res = _lm_tester->request_get_results(LM_CAMPAIGN_ID, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}

// -----------------------------
// Inbound slots
// -----------------------------

void LMForm::on_ivnewsessionButton_clicked()
{
  loggerMacroDebug("Requesting 'ivnewsession' to LM ...")
  WJson json_response;
  bool res = _lm_tester->request_ivnewsession(LM_BRANCH_ID, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))

    // Capture pin
    if(json_response.HasMember(wcc::LM_PBX_OUTPUT_TAG_RETURN.c_str())
        and json_response[wcc::LM_PBX_OUTPUT_TAG_RETURN.c_str()].IsObject()){

       WJson return_json = json_response[wcc::LM_PBX_OUTPUT_TAG_RETURN.c_str()];
       if(return_json.HasMember(wcc::LM_PBX_INPUT_TAG_DATA_KEY.c_str())
           and return_json[wcc::LM_PBX_INPUT_TAG_DATA_KEY.c_str()].IsString()){
           _pin = return_json[wcc::LM_PBX_INPUT_TAG_DATA_KEY.c_str()].GetString();
           _pin_alive_timer->start(LM_TIMER_PIN);
       }
    }
  }
}

void LMForm::on_ivflagButton_clicked()
{
  loggerMacroDebug("Requesting 'ivflag' to LM ...")
  WJson json_response;
  bool res = _lm_tester->request_ivflag(_pin, json_response);
  if( !res ){
    loggerMacroDebug("Error contacting LM Server")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }

}

// -----------------------------
// Mockup Callback
// -----------------------------

void LMForm::event_pbx(WJson* json_request, delfosXML* xml_request, WHttp* http_request)
{
  loggerMacroDebug("PBX Event was received")
  if( json_request )
      cout << json_request->to_str() << endl;
  if( xml_request )
      cout << xml_request->toString() << endl;
  if( http_request )
      cout << http_request->to_str() << endl;

}

void LMForm::event_gari(WJson* json_request, delfosXML* xml_request, WHttp* http_request)
{
  loggerMacroDebug("GARI Event was received")
  if( json_request )
      cout << json_request->to_str() << endl;
  if( xml_request )
      cout << xml_request->toString() << endl;
  if( http_request )
      cout << http_request->to_str() << endl;

}




