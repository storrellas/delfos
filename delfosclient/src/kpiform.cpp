#include "kpiform.h"
#include "ui_kpiform.h"

namespace wc = delfos::core;
namespace wcm = delfos::core::model;
namespace wcc = delfos::core::constants;

const string KPI_CLIENT_IP = wcc::localhost_ip;
const string KPI_CLIENT_PORT = "8025";

KpiForm::KpiForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::KpiForm)
{
  ui->setupUi(this);

  // Configure Logger
  _weh.setAllLogLevel(wc::severity_level::debug);
  _weh.setAllSyslogLevel(wc::severity_level::debug);
  _weh.StartEH("kpi_client", delfosLMServer, false);


  // Initilialise KPIClient
  _kpi_client = new KPITester(&_weh);
  _kpi_client->set_connection_parameters(KPI_CLIENT_IP, KPI_CLIENT_PORT);
  _kpi_client->init();

}

KpiForm::~KpiForm()
{
  delete _kpi_client;
  delete ui;
}


void KpiForm::on_clientKpiButton_clicked()
{
  loggerMacroDebug("Sending Client KPI ...")

  pid_t PID = syscall(SYS_gettid);
  std::string clientbrowser = "foo";
  std::string clientdev = "bar";
  std::string clientos = "os";

  // Generate dummy ClientKPI
  wcm::ClientKpi kpi;
  kpi.set_kpi( NumberToString(PID) );
  kpi.set_cookieguid( NewGuid() );
  kpi.set_kpiguid( NewGuid() );
  kpi.set_clientbrowser( clientbrowser );
  kpi.set_clientdevice( clientdev );
  kpi.set_clientos( clientos );
  kpi.set_branch( NewGuid() );
  kpi.set_ts_creation();

  WHttp http_response;
  bool res = _kpi_client->request_tracking(kpi, http_response);
  if( !res ){
    loggerMacroDebug("Error contacting KPI Server")
  }else{
    loggerMacroDebug(QString(http_response.to_str().c_str()))
  }

}

void KpiForm::on_interactionKpiButton_clicked()
{

  loggerMacroDebug("Sending Interaction KPI ...")

  // Generate InteractionKpi
  pid_t PID = syscall(SYS_gettid);
  wcm::InteractionKpi kpi(
        delfos::core::interaction_type::user_enters,
        NewGuid(),
        PID,
        NumberToString(PID),
        delfos::core::room_user_type::agent,
        delfos::core::room_type::delfosnar);
  kpi.set_ts_creation();

  WHttp http_response;
  bool res = _kpi_client->request_interaction(kpi, http_response);
  if( !res ){
    loggerMacroDebug("Error contacting KPI Server")
  }else{
    loggerMacroDebug(QString(http_response.to_str().c_str()))
  }
}
