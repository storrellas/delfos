// -- spy.h

#pragma once

#include <string>

#include <boost/noncopyable.hpp>

#include <wcore/utils/unit_test/delfosAssert.h>

#define TRACE_LINE \
  std::cerr << wc::yellow \
    << "@@@@ " << __FILE__ << ':' << __func__ << ":L" << __LINE__ \
    << delfos::core::reset << '\n';

std::string as_function(const std::string&);
std::string as_value(const std::string&);
std::string as_info(const std::string&);
std::string as_warning(const std::string&);
std::string as_error(const std::string&);
std::string as_error_prepended(const std::string&);
std::string as_ok(const std::string&);

namespace spy {

std::ostream& boost_version(std::ostream&);
std::ostream& clang_version(std::ostream&);
std::ostream& gnu_gpp_version(std::ostream&);

std::ostream& hostname(std::ostream&);
std::ostream& username(std::ostream&);
std::ostream& local_date(std::ostream&);
std::ostream& local_time(std::ostream&);

class RunInfo; // To be used like std::cout << RunInfo("my-progname") << ...
std::ostream& operator<<(std::ostream&, const RunInfo&);

#include <wcore/utils/helper/CompilerWarningsOff++Begin.h>
class RunInfo : private boost::noncopyable {
#include <wcore/utils/helper/CompilerWarningsOff++End.h>
public:
	RunInfo(
		const std::string& a_argv0,
		const std::string& a_progname)
	: d_argv0(a_argv0), d_progname(a_progname)
	{}

	const std::string& get_argv0() const { return d_argv0; }
	const std::string& get_progname() const { return d_progname; }
private:
	const std::string d_argv0;
	const std::string d_progname;
};

} // spy

// -- eof
