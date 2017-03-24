#pragma once

#include <boost/core/noncopyable.hpp>

#include <wcore/model/config/elements/WizardBucket.h>
#include <wcore/utils/helper/CompilerWarningsOff++Begin.h>
// '- TODO Avoid this wrap! Caution: Avoid double #include's!
# include <wcore/utils/unit_test/delfosAssert.h>
#include <wcore/utils/helper/CompilerWarningsOff++End.h>

#include <wcore/utils/helper/CompilerWarningsOff++Begin.h>
class WizardBucketRAII : private boost::noncopyable {
#include <wcore/utils/helper/CompilerWarningsOff++End.h>
public:
  WizardBucketRAII(
      const wcore::SmartModel::DAIConf& dai_conf,
      uint number_of_agents);
  ~WizardBucketRAII();

  const delfos::core::model::WizardBucket& get() const { return _wizard_bucket; }
private:
  std::unique_ptr<delfosErrorHandler> _weh;
  delfos::core::model::WizardBucket _wizard_bucket;
};

// -- eof
