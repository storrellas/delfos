#include "rtvform.h"
#include "ui_rtvform.h"

namespace wc = delfos::core;
namespace wcc = delfos::core::constants;

const string RTV_CLIENT_IP = wcc::localhost_ip;
const string RTV_CLIENT_PORT = "8002";

RTVForm::RTVForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::RTVForm)
{
  ui->setupUi(this);

  // Configure Logger
  _weh.setAllLogLevel(wc::severity_level::debug);
  _weh.setAllSyslogLevel(wc::severity_level::debug);
  _weh.StartEH("pbx_client", delfosLMServer, false);

  // Initilialise RMClient
  _rtv_client = new RTVTester(&_weh);
  _rtv_client->set_connection_parameters(RTV_CLIENT_IP, RTV_CLIENT_PORT);
  _rtv_client->init();

}

RTVForm::~RTVForm()
{
  delete _rtv_client;
  delete ui;
}

void RTVForm::init_test_image() {

  _test_image_size = SCHAR_MAX - SCHAR_MIN + 1;
  _test_image = new char[_test_image_size];
  char c = SCHAR_MIN;
  for (size_t i = 0; i < (SCHAR_MAX - SCHAR_MIN + 1); i++)
  {
      _test_image[i] = c;
      c++;
  }
}


void RTVForm::on_setImageButton_clicked()
{
  loggerMacroDebug("Requesting SET image ... ")
  init_test_image();

  _session_guid = NewGuid();
  WHttp http_response;
  //bool res = _rtv_client->request_set_image(image, image_size, _session_guid, http_response);
  bool res = _rtv_client->request_set_image(_test_image, _test_image_size, _session_guid, http_response);
  if( !res ){
    loggerMacroDebug("Error contacting KPI Server")
  }else{
    loggerMacroDebug(QString(http_response.to_str().c_str()))
  }
}

void RTVForm::on_getImageButton_clicked()
{
  string compression_factor = "5";
  string is_mobile_enabled = "1";

  string * p_compression_factor = nullptr;
  string * p_is_mobile_enabled = nullptr;
  if( ui->compressionCheckBox->isChecked())
    p_compression_factor = &compression_factor;
  if( ui->mobileUsedCheckBox->isChecked())
    p_is_mobile_enabled = &is_mobile_enabled;

  loggerMacroDebug("Requesting GET Image ... ")
  WHttp http_response;
  bool res = _rtv_client->request_get_image(_session_guid, p_compression_factor, p_is_mobile_enabled, http_response);
  if( !res ){
    loggerMacroDebug("Error contacting KPI Server")
  }else{
    loggerMacroDebug(QString(http_response.to_str().c_str()))
  }
}
