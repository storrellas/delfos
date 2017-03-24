// -- spy.cpp

#include "../utils/spy.h"

#include <wcore/utils/helper/CompilerWarningsOff++Begin.h>
# include <boost/date_time/c_local_time_adjustor.hpp>
# include <boost/date_time/posix_time/posix_time.hpp>
# include <boost/filesystem.hpp>
# include <boost/version.hpp>
#include <wcore/utils/helper/CompilerWarningsOff++End.h>

#include <wcore/utils/helper/Helper.h>

#include "../utils/preprocessor-pool.h"

namespace wc = delfos::core;

namespace {

std::string
colored(const std::string& color, const std::string& text) {
  return color + text + wc::reset;
}

} // namespace

std::string as_function(const std::string& text) { return colored(wc::cyan, text); }
std::string as_value(const std::string& text) { return colored(wc::blue, text); }
std::string as_info(const std::string& text) { return colored(wc::magenta, text); }
std::string as_warning(const std::string& text) { return colored(wc::yellow, text); }
std::string as_error(const std::string& text) { return colored(wc::red, text); }
std::string as_error_prepended(const std::string& text) {
  return as_error(wc::concat("Error: ", text)); }
std::string as_ok(const std::string& text) { return colored(wc::green, text); }

std::ostream&
spy::boost_version(std::ostream& a_os) {
	return
		a_os
			<< BOOST_VERSION / 100000 << '.'	// major version
			<< BOOST_VERSION / 100 % 1000 << '.'	// minor version
			<< BOOST_VERSION % 100	// patch level
			<< std::flush;
}

std::ostream&
spy::clang_version(std::ostream& a_os) {
	return
		a_os
#ifdef __clang__
		<< __clang_version__
		// << __clang_major__ << '.'
		// << __clang_minor__ << '.'
		// << __clang_patchlevel__
#else
		<< "[not applicable]"
#endif
		<< std::flush;
}

std::ostream&
spy::gnu_gpp_version(std::ostream& a_os) {
	return
		a_os
#ifdef ENZO__GNUGPP_VERSION
			<< ENZO__GNUGPP_VERSION / 10000 << '.'	// major version
			<< ENZO__GNUGPP_VERSION / 100 % 100 << '.'	// minor version
			<< ENZO__GNUGPP_VERSION % 100	// patch level
#else
			<< "[not applicable]"
#endif
			<< std::flush;
}

namespace {

std::string
get_hostname()
{
#ifdef _MSC_VER
	#pragma warning(disable : 4996) // This function or variable may be unsafe
#endif

#if defined(WIN32)
	//
	// This block is meant for vs10 or MinGW's g++
	//
	const char* hostname = getenv("HOSTNAME");
	if (hostname)
		return hostname; // iff hostname != nullptr, it can be converted to string

	hostname = getenv("COMPUTERNAME");
	if (hostname)
		return hostname; // iff like above

#ifdef _WINDOWS
#pragma warning(default : 4996) // This function or variable may be unsafe
#endif

	return "UNKNOWN";
#else
	//
	// This block is meant only for Linux and g++
	//
	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);
	return hostname;
#endif
}

} // namespace

std::ostream&
spy::hostname(std::ostream& a_os) {
	return a_os << get_hostname() << std::flush;
}

namespace {

std::string
get_username() {
	const char* username = getenv("USER");
	if (username)
		return username; // iff username != nullptr, it can be converted to string

	username = getenv("USERNAME");
	return username ? username : "unknown-username"; // iff like above
}

} // namespace

std::ostream&
spy::username(std::ostream& a_os) {
	return a_os << get_username() << std::flush;
}

std::ostream&
spy::local_date(std::ostream& a_os) {
	namespace pt = boost::posix_time;
	const pt::ptime now = pt::second_clock::local_time();

	std::ostringstream oss;
	const pt::ptime::date_type date = now.date();
	oss << date.day_of_week() << ", "
		<< date.month() << ' ' << date.day() << ", " << date.year();
	a_os << oss.str() << std::flush;
	return a_os;
}

std::ostream&
spy::local_time(std::ostream& a_os) {
	namespace pt = boost::posix_time;
	const pt::ptime now = pt::second_clock::local_time();

	std::ostringstream oss;
	pt::time_facet* const f = new pt::time_facet("%H:%M:%S");
	oss.imbue(std::locale(oss.getloc(), f));
	oss << now;

	a_os << oss.str() << std::flush;
	return a_os;
}

namespace {

namespace bf = boost::filesystem;

const std::string pad = "  ";

std::string
get_argv0_info(const std::string& a_argv0) {
	const bf::path full_argv0 =
		bf::system_complete(bf::canonical(bf::path(a_argv0)));

	std::ostringstream oss;
	oss << "The full argv[0]:\n"
		<< pad << "is " << full_argv0 << ",\n";
	if (bf::exists(full_argv0)) {
		oss << pad << "which is" << (bf::is_regular(full_argv0) ? "" : " not")
			<< " a regular file, created by ";
#if defined(CMAKE_MYUSERNAME)
		oss << BOOST_PP_STRINGIZE(CMAKE_MYUSERNAME);
#else
		oss << '?';
#endif
		oss << ", at ";
#if defined(CMAKE_MYHOSTNAME)
		oss << BOOST_PP_STRINGIZE(CMAKE_MYHOSTNAME);
#else
		oss << '?';
#endif
		oss << ',';

		// Day of the week, month, day, year
		//
		const std::time_t the_time_t = bf::last_write_time(full_argv0);
		namespace pt = boost::posix_time;
		const pt::ptime the_ptime =
			// pt::from_time_t(theTime_t); <-- Does not work properly for Darwin
			// Solution from http://stackoverflow.com/questions/6143569/
			//	how-to-get-a-local-date-time-from-a-time-t-with-boostdate-time
			boost::date_time::c_local_adjustor<pt::ptime>::utc_to_local(
				pt::from_time_t(the_time_t));
		const pt::ptime::date_type date = the_ptime.date();
		oss << " on " << date.day_of_week() << ", " << date.month() << ' '
			<< date.day() << ", " << date.year();

		// Hours, minutes and seconds
		//
		pt::time_facet* const f = new pt::time_facet("%H:%M:%S");
		oss.imbue(std::locale(oss.getloc(), f));
		oss << ", at " << the_ptime;
	} else
		oss << "(which does not seem to exist)";
	return oss.str();
}

std::string
get_current_dir_info()
{
	const std::string dot = ".";
	const bf::path current_dir(dot);
	const bf::path full_path_cd = bf::system_complete(bf::canonical(dot));
	std::ostringstream oss;
	oss << "The full path of the current directory is " << full_path_cd;
	return oss.str();
}

} // namespace

std::ostream&
spy::operator<<(std::ostream& a_os, const spy::RunInfo& a_run_info) {
	const std::string argv0_info = get_argv0_info(a_run_info.get_argv0());
	const std::string current_dir_info = get_current_dir_info();

	a_os << '\n' << a_run_info.get_progname()
		<< " was launched by " << username << " at " << hostname
		<< " on " << local_date << ", at " << local_time << '\n'
		<< argv0_info << '\n'
		<< current_dir_info << '\n'
		<< "Using Boost version " << boost_version << '\n'
		<< "Using Clang version " << clang_version << '\n'
    << "Using GNU g++ version " << gnu_gpp_version << '\n'
    << "List of preprocessor defines:\n" << list_of_preprocessor_defines;

	return a_os;
}

// -- eof
