#include "rmform.h"
#include "ui_rmform.h"

namespace wc = delfos::core;
namespace wcc = delfos::core::constants;

const string RM_CLIENT_IP = wcc::localhost_ip;
const string RM_CLIENT_PORT = "8003";

const string CAMPAIGN_GUID = "23290B6D-693B-44C3-9DD1-F4FEF0B5C9A3";
const string BRANCH_GUID = "D487678F-ABF5-4B38-B55A-5AEA79640420";
const string VACENTER_GUID = "A311DC47-FC05-4856-B850-54F15EC8F40F";
const string USER_GUID = "C2729F6D-8A4F-49A6-8ED5-819A617AA7E4";
const string MODULE_TYPE_PM = "0";

RMForm::RMForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::RMForm)
{
  ui->setupUi(this);


  // Configure Logger
  _weh.setAllLogLevel(wc::severity_level::debug);
  _weh.setAllSyslogLevel(wc::severity_level::debug);
  _weh.StartEH("pbx_client", delfosLMServer, false);

  // Initilialise RMClient
  _rm_client = new RMTester(&_weh);
  _rm_client->set_connection_parameters(RM_CLIENT_IP, RM_CLIENT_PORT);
  _rm_client->init();

}

RMForm::~RMForm()
{
  delete _rm_client;
  delete ui;
}



// -----------------------
// Clicked slots button
// -----------------------
void RMForm::on_coreByCampaignButton_clicked()
{
  loggerMacroDebug("Requesting CORE by Campaign ...")
  WJson json_response;
  if(!_rm_client->request_get_core_by_campaign(CAMPAIGN_GUID, json_response)){
    loggerMacroDebug("Error receiving core")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}

void RMForm::on_coreByBranchButton_clicked()
{
  loggerMacroDebug("Requesting CORE by Branch ...")
  WJson json_response;
  if(!_rm_client->request_get_core_by_branch(BRANCH_GUID, json_response)){
    loggerMacroDebug("Error receiving core")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}

void RMForm::on_coreByVacenterButton_clicked()
{
  loggerMacroDebug("Requesting CORE by VaCenter ...")
  WJson json_response;
  if(!_rm_client->request_get_core_by_vacenter(VACENTER_GUID, json_response)){
    loggerMacroDebug("Error receiving core")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}

void RMForm::on_vacenterByUserButton_clicked()
{
   loggerMacroDebug("Requesting VACENTER by user ...")
   WJson json_response;
   if(!_rm_client->request_get_vacenter_by_user(USER_GUID, json_response)){
     loggerMacroDebug("Error receiving core")
   }else{
     loggerMacroDebug(QString(json_response.to_str().c_str()))
   }
}

void RMForm::on_modulesbytypeButton_clicked()
{
   loggerMacroDebug("Requesting MODULES by type ...")
   WJson json_response;
   if(!_rm_client->request_get_module_by_type(MODULE_TYPE_PM, json_response)){
     loggerMacroDebug("Error receiving core")
   }else{
     loggerMacroDebug(QString(json_response.to_str().c_str()))
   }
}
