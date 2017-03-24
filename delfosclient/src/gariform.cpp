#include "gariform.h"
#include "ui_gariform.h"

namespace wc = delfos::core;
namespace wcm = delfos::core::model;
namespace wcc = delfos::core::constants;
namespace wct = delfos::core::tags;

//const string GARI_CLIENT_IP = wcc::localhost_ip;
//const string GARI_CONTROLPORT = "9814";
//const string GARI_WEB_PORT = "8014";
const string GARI_CLIENT_IP = "preawsgari.delfos.com";
const string GARI_WEB_PORT = "80";
const string GARI_CONTROLPORT = "9880";

const uint CHATFILE_POD = 0;
const uint CAMERA_POD = 1;
const uint SHARING_POD = 2;

const bool ENABLE_LM_MOCKUP = false;
const bool ENABLE_KPI_MOCKUP = false;

GariForm::GariForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::GariForm)
{
  ui->setupUi(this);

  // Configure Logger
  _weh.setAllLogLevel(wc::severity_level::debug);
  _weh.setAllSyslogLevel(wc::severity_level::debug);
  _weh.StartEH("gari form", delfosLMServer, false);


  // Initilialise GARIClient
  _gari_tester = new GARITester(&_weh);
  _gari_tester->set_web_connection_parameters(GARI_CLIENT_IP, GARI_WEB_PORT);
  _gari_tester->set_lm_connection_parameters(GARI_CLIENT_IP, GARI_CONTROLPORT);
  _gari_tester->set_event_gari_handler(this);
  _gari_tester->set_event_kpi_handler(this);
  _gari_tester->init(ENABLE_LM_MOCKUP, ENABLE_KPI_MOCKUP);

}

GariForm::~GariForm()
{
  delete _gari_tester;
  delete ui;
}

bool GariForm::kpi_mockup_event(const WHttp http_request){
  loggerMacroDebug("Received KPI event")
}


bool GariForm::gari_event(delfos_command_parser::CommandParser& parser){

  loggerMacroDebug("Received GARI event")
  return true;
}



// ------------------------------------
// Seed Methods
// ------------------------------------

void GariForm::on_createSeedButton_clicked()
{

  loggerMacroDebug("Creating GARI seed ...")

  /// Generate room for seed
  // NOTE: This implementation could lead to a memory leak here
  wcm::Room seed;

  wcm::ChatFilePod* file_pod = new wcm::ChatFilePod(CHATFILE_POD);
  seed.test_add_pod( file_pod );

  wcm::CameraPod* camera_pod = new wcm::CameraPod(CAMERA_POD);
  camera_pod->set_camera_on(true);
  seed.test_add_pod(camera_pod);

  wcm::SharingPod* sharing_pod = new wcm::SharingPod(SHARING_POD);
  list<unsigned int> slide_selector = {0};
  sharing_pod->test_set_slide_selector(slide_selector);
  string file_tree = "{\"255e1ffd-ea00-4432-b358-3581e5af09ec\":{" \
                          "\"url\":\"//static.delfos.com/PrivateResources/255e1ffd-ea00-4432-b358-3581e5af09ec.jpg\","\
                          "\"ext\":\"jpg\","\
                          "\"name\":\"Porsche Pr\","\
                          "\"children\":{},"\
                          "\"tree_key\":[]"\
                          "}}";
  sharing_pod->test_set_file_tree(file_tree);
  seed.test_add_pod(sharing_pod);

  /// Generate seed
  _seed_id = NewGuid();
  WJson json_response;
  if( ! _gari_tester->request_create_seed(_seed_id, seed, json_response) ){
    loggerMacroDebug("Failed to contact GARI")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }

}

void GariForm::on_updateSeedButton_clicked()
{
  loggerMacroDebug("Update GARI seed ...")
  /// Generate seed
  wcm::Room seed;
  WJson json_response;
  if( ! _gari_tester->request_update_seed(_seed_id, seed, json_response) ){
    loggerMacroDebug("Failed to contact GARI")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}

void GariForm::on_readSeedButton_clicked()
{
  loggerMacroDebug("Read GARI seed ...")
  WJson json_response;
  if( ! _gari_tester->request_read_seed(_seed_id, json_response) ){
    loggerMacroDebug("Failed to contact GARI")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
    _seed_id.clear();
  }
}

void GariForm::on_deleteSeedButton_clicked()
{
  loggerMacroDebug("Delete GARI seed ...")
  WJson json_response;
  if( ! _gari_tester->request_delete_seed(_seed_id, json_response) ){
    loggerMacroDebug("Failed to contact GARI")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
    _seed_id.clear();
  }
}

// ------------------------------------
// Session Methods
// ------------------------------------

void GariForm::on_createSessionButton_clicked()
{
  loggerMacroDebug("Creating GARI session ...")
  wc::room_type room_type = static_cast<wc::room_type>(ui->sessionTypeComboBox->currentIndex());

  _session_id = NewGuid();
  string customer_guid;
  WJson json_response;
  if( ! _gari_tester->request_create_session(_seed_id, _session_id, room_type, 0, customer_guid, json_response) ){
    loggerMacroDebug("Failed to contact GARI")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}

void GariForm::on_deleteSessionButton_clicked()
{
  loggerMacroDebug("Delete GARI session ...")
  WJson json_response;
  if( ! _gari_tester->request_delete_session(_session_id, json_response) ){
    loggerMacroDebug("Failed to contact GARI")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
    _session_id.clear();
    ui->useridLineEdit->clear();
  }
}

void GariForm::on_updateSessionButton_clicked()
{
  loggerMacroDebug("Updating GARI session ...")

  // Generate ChatMessage update pod
  wcm::ChatMessage chat_message;
  chat_message.set_type(0);
  chat_message.set_content("mycontent");
  chat_message.set_source(0);
  chat_message.set_source_id( ui->useridLineEdit->text().toUInt() );
  wcm::ChatFilePod file_pod(CHATFILE_POD);
  file_pod.push_back_msg(chat_message);

  // Perform request
  uint user_id = ui->useridLineEdit->text().toUInt();
  WJson json_response;
  if( ! _gari_tester->request_update(_session_id,
                                             user_id,
                                             CHATFILE_POD,
                                             &file_pod,
                                             json_response) )
  {
    loggerMacroDebug("Failed to contact GARI")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}

void GariForm::on_readSessionButton_clicked()
{
  loggerMacroDebug("Reading GARI session ...")

  QString user_id_str = ui->useridLineEdit->text();
  if(!user_id_str.isEmpty()){
    uint user_id = user_id_str.toUInt();
    // Read session diff
    WJson json_response;
    if( ! _gari_tester->request_read_session_user(_session_id,
                                                          user_id,
                                                         json_response) )
    {
      loggerMacroDebug("Failed to contact GARI")
    }else{
      loggerMacroDebug(QString(json_response.to_str().c_str()))
    }

  }else{

    // Read session to create user
    WJson json_response;
    if( ! _gari_tester->request_read_session(_session_id,
                                                     wc::room_user_type::user,
                                                     json_response) )
    {
      loggerMacroDebug("Failed to contact GARI")
    }else{
      loggerMacroDebug(QString(json_response.to_str().c_str()))

      // Store user_id to Line Edit
      if(json_response.HasMember(wcm::ROOM_KEY_STR_USER_ID) and
         json_response[wcm::ROOM_KEY_STR_USER_ID].IsUint() ){
         const unsigned int user_id = json_response[wcm::ROOM_KEY_STR_USER_ID].GetUint();
         ui->useridLineEdit->setText( QString::number(user_id) );
      }
    }
  }




}

// ------------------------------------
// User Info methods
// ------------------------------------


void GariForm::on_updateUserInfoButton_clicked()
{
  loggerMacroDebug("Updating User Info ... ")

  WJson user_json;
  user_json.append_pair_to_json(wct::NAME, "myname");
  user_json.append_pair_to_json(wct::SESSION_RATE, static_cast<uint>(5));
  user_json.append_pair_to_json(wct::PRESENTER_RATE, static_cast<uint>(3));
  user_json.append_pair_to_json(wct::LIKE, true);
  user_json.append_pair_to_json("key1", "key1value");

  uint user_id = ui->useridLineEdit->text().toUInt();
  WJson json_response;
  if( ! _gari_tester->request_update_user_info(_session_id, user_id,
                                               user_json, json_response) )
  {
    loggerMacroDebug("Failed to contact GARI")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}


void GariForm::on_readUserInfoButton_clicked()
{
  loggerMacroDebug("Reading User Info ... ")
  uint user_id = ui->useridLineEdit->text().toUInt();
  WJson json_response;
  if( ! _gari_tester->request_read_user_info(_session_id, user_id, json_response) ){
    loggerMacroDebug("Failed to contact GARI")
  }else{
    loggerMacroDebug(QString(json_response.to_str().c_str()))
  }
}



