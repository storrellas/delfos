// --

#include <wcore/utils/helper/CompilerWarningsOff++Begin.h>
// '- TODO Avoid this wrap!
# include <wcore/logging/objects/WErrorStack.h>
#include <wcore/utils/helper/CompilerWarningsOff++End.h>

#include "WizardBucketRAII.h"

namespace wc = delfos::core;

WizardBucketRAII::WizardBucketRAII(
    const wcore::SmartModel::DAIConf& dai_conf,
    uint number_of_agents) :
    _weh(new delfosErrorHandler),
    _wizard_bucket(_weh.get())
{

  _wizard_bucket.init(dai_conf, true, number_of_agents, number_of_agents);

  // NOTE: Alter default values of branchgroup
  // to set max lead alive time to 1 day
  _wizard_bucket.get_branchgroup().set_max_sched_time(1);
  _wizard_bucket.get_branchgroup().set_max_time(1);

  // Create entities
  WErrorStack es;
  bool succeeded = _wizard_bucket.request_to_dai_create_db(es);
  if (!succeeded) { // Clean up and throw
    succeeded = _wizard_bucket.request_to_dai_delete_db(es);
    if (succeeded)
      throw
        wc::Exception(
          "WizardBucketRAII constructor: 'request_to_dai_create_db' failed "
          "(the cleanup 'request_to_dai_delete' succeeded)");
    else
      throw
        wc::Exception(
          "WizardBucketRAII constructor: Both 'request_to_dai_create_db' "
          "and 'request_to_dai_delete' failed");
  }
}

WizardBucketRAII::~WizardBucketRAII() {
  WErrorStack es;
  const bool succeeded = _wizard_bucket.request_to_dai_delete_db(es);
  if (!succeeded){
    wc::Exception(
            "WizardBucketRAII destructor: 'request_to_dai_delete' failed");
  }

}

// -- eof
