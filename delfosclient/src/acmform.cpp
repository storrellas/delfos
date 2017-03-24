#include "acmform.h"
#include "ui_acmform.h"

namespace wc = delfos::core;
namespace wcm = delfos::core::model;
namespace wcc = delfos::core::constants;

const string ACM_CLIENT_IP = wcc::localhost_ip;
const string ACM_CLIENT_HTTPPORT = "8020";
const string ACM_CLIENT_COREPORT = "9920";

const string BRANCH_GUID = "D487678F-ABF5-4B38-B55A-5AEA79640420";

ACMForm::ACMForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ACMForm)
{
  ui->setupUi(this);

  // Configure Logger
  _weh.setAllLogLevel(wc::severity_level::debug);
  _weh.setAllSyslogLevel(wc::severity_level::debug);
  _weh.StartEH("acm_client", delfosLMServer, false);


  // Initilialise ACMClient
  _acm_client = new ACMTester(&_weh);
  _acm_client->set_connection_parameters(ACM_CLIENT_IP, ACM_CLIENT_HTTPPORT, ACM_CLIENT_COREPORT);
  _acm_client->init();

}

ACMForm::~ACMForm()
{
  delete _acm_client;
  delete ui;
}

// ---------------------------
// Clicked slots
// ---------------------------

void ACMForm::on_newAlphaButton_clicked()
{
  loggerMacroDebug("Requesting New Alpha ...")

  WJson json_response;

  const string cookie_guid = NewGuid();
  const string kpi_guid = NewGuid();
  bool res = _acm_client->request_landing_new_alpha(cookie_guid, BRANCH_GUID, kpi_guid, json_response);
  if(!res){
    loggerMacroDebug("Error receiving ACM")
    return;
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }

  string token, alpha;
  if(json_response.HasMember(wcc::ACM_TAG_TOKEN) and json_response[wcc::ACM_TAG_TOKEN].IsString()){
    token = json_response[wcc::ACM_TAG_TOKEN].GetString();
  }
  if(json_response.HasMember(wcc::ACM_TAG_ALPHA) and json_response[wcc::ACM_TAG_ALPHA].IsString()){
    alpha = json_response[wcc::ACM_TAG_ALPHA].GetString();
  }

  // Insert alpha-token to map
  loggerMacroDebug("Token: " + QString(token.c_str()) + " Alpha: " + QString(alpha.c_str()))
  _alpha_map.insert(std::pair<string, string>(alpha, token));

}

void ACMForm::on_alphaStatusButton_clicked()
{
  loggerMacroDebug("Requesting Alpha Status ...")

  const string alpha = _alpha_map.begin()->first;
  const string token = _alpha_map.begin()->second;
  WJson json_response;
  bool res = _acm_client->request_landing_alpha_status(alpha, token, json_response);
  if(!res){
    loggerMacroDebug("Error receiving ACM")
    return;
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}

void ACMForm::on_getAlphaButton_clicked()
{
  loggerMacroDebug("Requesting Get Alpha ...")

  const string alpha = _alpha_map.begin()->first;
  WJson json_response;
  bool res = _acm_client->request_lm_get_alpha(alpha, json_response);
  if(!res){
    loggerMacroDebug("Error receiving ACM")
    return;
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}

void ACMForm::on_goAlphaButton_clicked()
{
  loggerMacroDebug("Requesting Go Alpha ...")

  wcm::SessionMetadata metadata;
  metadata.fillTestMetadata(NewGuid());
  const string alpha = _alpha_map.begin()->first;
  WJson json_response;
  bool res = _acm_client->request_lm_go_alpha(NewGuid(), BRANCH_GUID,
                                              alpha, metadata, json_response);
  if(!res){
    loggerMacroDebug("Error receiving ACM")
    return;
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }

}

void ACMForm::on_freeAlphaTokenButton_clicked()
{
  loggerMacroDebug("Requesting Free Alpha by token ...")
  if( _alpha_map.empty() ){
    loggerMacroDebug("No alpha reserved. Aborting ...")
    return;
  }

  const string token = _alpha_map.begin()->second;
  WJson json_response;
  bool res = _acm_client->request_lm_free_alpha(token, json_response);
  if(!res){
    loggerMacroDebug("Error receiving ACM")
    return;
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }

  // Erase alpha
  if( _alpha_map.size() > 0){
    auto it = _alpha_map.begin();
    _alpha_map.erase(it);
  }
}

void ACMForm::on_freeAlphaListButton_clicked()
{
  loggerMacroDebug("Requesting Free Alpha list ...")
  if( _alpha_map.empty() ){
    loggerMacroDebug("No alpha reserved. Aborting ...")
    return;
  }

  const string alpha = _alpha_map.begin()->first;
  std::list<string> alpha_list;
  alpha_list.push_back(alpha);
  WJson json_response;
  bool res = _acm_client->request_lm_free_alpha(alpha_list, json_response);
  if(!res){
    loggerMacroDebug("Error receiving ACM")
    return;
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }

  // Erase alpha
  if( _alpha_map.size() > 0){
    auto it = _alpha_map.begin();
    _alpha_map.erase(it);
  }
  /**/

}
