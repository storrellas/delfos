// --

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream> // std::ostringstream oss
#include <vector>

#include <wcore/utils/helper/CompilerWarningsOff++Begin.h>
# include <boost/algorithm/string.hpp>
# include <boost/program_options.hpp>
# include <boost/tokenizer.hpp>
#include <wcore/utils/helper/CompilerWarningsOff++End.h>

#include "cli-parser.h"
#include "spy.h"

namespace logging = boost::log;
namespace wc = delfos::core;

namespace {

char* argv0 = 0;
char* progname = 0;

}

std::string
cli::argv_0()
{	return argv0; }

std::string
cli::program_name()
{	return progname; }

namespace cli {

// 1) File selection
//
#ifdef WHISBI_USE_FILE_SELECTION_20161031
std::string filename_in;
#endif

// 2) Operation flags/parameters
//
std::string fake_module;
std::string fake_instance;
io_type io;
uint number_of_threads_in_log_tests = 3;
uint number_of_agents = 0;
bool clear_request_trace = true;

// 3) Informative output
//
wc::severity_level log_min_severity_syslog;
wc::severity_level log_min_severity_console;

// Helping function
//
bool check_arguments(const boost::program_options::variables_map&);

namespace { // only for this translation unit

// 1) File selection
//

// 2) Operation flags/parameters
//
fake_module_cli::Enum fake_module_cli = fake_module_cli::Enum::Enzo;
size_t fake_instance_cli = 1;
io_type_cli::Enum io_type_cli = io_type_cli::Enum::asio;

// 3) Informative output
//
log_min_severity::Enum log_min_severity_syslog_cli =
  log_min_severity::Enum::warning;
log_min_severity::Enum log_min_severity_console_cli =
  log_min_severity::Enum::warning;

const std::string usage_parameter_examples =
	"  --input-file C:\\\\tmp\\\\input.txt --input-positive-double 0.8"
		" --platonic-solid octahedron"
	"\n\n" // ie. a new example starts here
	"  --input-file C:\\\\tmp\\\\input2.txt --input-positive-double 2.5"
		" --platonic-solid hexahedron"
	"\n\n" // ie. a new example starts here
	"  --input-file C:\\\\tmp\\\\input3.txt --input-positive-double 2.5";

// Miscellany

std::string
get_set_of_defined_strings(const std::vector<std::string>& a_definedStrings)
{
	const std::string result =
		'{' + boost::algorithm::join(a_definedStrings, ", ") + '}';
	return result;
}

} // namespace

} // namespace cli

bool
cli::parse_command_line(const int argc, char** argv)
{
	argv0 = argv[0];
	if ((progname = (char *)strrchr(argv[0], '/')) == nullptr)
		progname = argv[0];
	else
		++progname;

	namespace po = boost::program_options;
	const std::string usage_title =
		std::string("Usage: ") + progname + " [OPTIONS]...";
	po::options_description odFull(usage_title);

	{	// 1) File selection
		//
#ifdef WHISBI_USE_FILE_SELECTION_20161031
		po::options_description od("File selection");
		po::options_description odReq("  Mandatory", 160, 80);
		po::options_description odOpt("  Optional", 160, 80);
		odReq.add_options()
			("input-file",
			po::value<std::string>(&filename_in)->required(),
			"<filename>");
		od.add(odReq);
		od.add(odOpt);
		odFull.add(od);
#endif
	}

	{	// 2) Operation flags/parameters
		//
		po::options_description od("Operation flags/parameters", 160, 80);
		od.add_options()
      ("fake-module",
       po::value<std::string>(),
       get_set_of_defined_strings(
         fake_module_cli::get_defined_strings()).c_str())
			("fake-instance",
       po::value<size_t>(&fake_instance_cli),
       "<size_t> # \t(fake) number of instance, from 1 to 99")
      ("io-type",
       po::value<std::string>(),
       get_set_of_defined_strings(io_type_cli::get_defined_strings()).c_str())
			("number-of-threads-in-log-tests",
       po::value<uint>(&number_of_threads_in_log_tests),
       "<uint> # \tconvenient only for log tests")
      ("number-of-agents",
       po::value<uint>(&number_of_agents),
       "<uint> # \tnumber of virtual agents who will be launched by Enzo (must be >=0)")
       ("clear-request-trace",
        po::value<bool>(&clear_request_trace),
        "<bool> # \tClear Request trace table before launching Enzo")
      ;
		odFull.add(od);
	}

	{	// 3) Informative output
		//
		po::options_description od("Informative output", 160, 80);
		od.add_options()
      ("log-min-severity-syslog",
       po::value<std::string>(),
       get_set_of_defined_strings(
         log_min_severity::get_defined_strings()).c_str())
      ("log-min-severity-console",
       po::value<std::string>(),
       get_set_of_defined_strings(
         log_min_severity::get_defined_strings()).c_str())
			("help", "# \tOutput help on the usage, then exit")
			("usage-examples",
       "# \tOutput example parameters, help on the usage, then exit");
		odFull.add(od);
	}

	{	// 4) Response file
		//
		po::options_description od("Response file", 160, 80);
		od.add_options()
			("response-file",
			po::value<std::string>(),
			"# \tConfiguration file which uses the same syntax as the command line");
		odFull.add(od);
	}

	po::variables_map vm;
	try {
		po::store(
			po::command_line_parser(argc, argv).
			options(odFull).
			style(
        po::command_line_style::allow_long |
        po::command_line_style::long_allow_next).
			run(),
			//po::parse_command_line(argc, argv, odFull),
			vm);
		if (vm.count("help")) {
			std::cout << odFull << '\n';
			return false;
		} else if (vm.count("usage-examples")) {
			std::cout << "Usage parameter example(s):\n\n"
				<< usage_parameter_examples << "\n\n" << odFull << '\n';
			return false;
		}
	} catch (const std::exception& e) {
		std::cerr << progname << ": " << as_error_prepended(e.what()) << "\n\n"
      << odFull << '\n';
		return false;
	}

	// 4) Response file
	//
	if (vm.count("response-file")) {
		// Load the file and tokenize it
		const std::string filename = vm["response-file"].as<std::string>();
		std::ifstream ifs(filename.c_str());
		if (!ifs) {
			std::cerr << progname << ": "
        << as_error("Error: Could not open the response file" + filename)
        << "\n\n" << odFull << '\n';
			return false;
		}
		// Read the whole file into a string
		std::ostringstream oss;
		oss << ifs.rdbuf();
		// Split the file content
		boost::char_separator<char> sep(" \n\r");
		std::string ResponsefileContents(oss.str());
		boost::tokenizer<boost::char_separator<char> >
			tokenizerObject(ResponsefileContents, sep);
		std::vector<std::string> args;
		copy(tokenizerObject.begin(), tokenizerObject.end(), back_inserter(args));
		// Parse the file and store the options
		namespace po = boost::program_options;
		try {
			po::store(
				po::command_line_parser(args).
				options(odFull).
				style(
          po::command_line_style::allow_long |
          po::command_line_style::long_allow_next).
				run(),
				vm);
		} catch (const std::exception& e) {
			std::cerr << progname << ": Error: " << e.what() << "\n\n" << odFull
				<< '\n';
			return false;
		}
	}

	try {
		po::notify(vm);
	} catch (const std::exception& e) {
		std::cerr << progname << ": Error: " << e.what() << "\n\n" << odFull
			<< '\n';
		return false;
	}

	const bool succeeded = check_arguments(vm);
	if (!succeeded) {
		std::cerr << odFull;
		return false;
	}

	return true;
}

namespace {

bool
output_error_and_return_false(const std::string& a_message)
{
	std::cerr << progname << ": " << as_error_prepended(a_message) << "\n\n";
	return false;
}

wc::severity_level
as_severity(const std::string& text) {
  if (text == "trace")
    return wc::severity_level::trace;
  else if (text == "debug")
    return wc::severity_level::debug;
  else if (text == "info")
    return wc::severity_level::info;
  else if (text == "warning")
    return wc::severity_level::warning;
  else if (text == "error")
    return wc::severity_level::error;
  else if (text == "fatal")
    return wc::severity_level::fatal;
  return wc::severity_level::fatal; // this line shouldn't be reached
}


} // namespace

bool
cli::check_arguments(const boost::program_options::variables_map& a_vm)
{
	if (a_vm.count("help"))
		return false; // force a error so the usage is shown automatically

	if (a_vm.count("fake-module")) { // Check
		const std::string& text = a_vm["fake-module"].as<std::string>();
		fake_module_cli = fake_module_cli::get_enum(text);
		if (fake_module_cli == fake_module_cli::Enum::undefined) {
			const std::string message =
				"Unknown fake-module parameter '" + text + "'";
			return output_error_and_return_false(message);
		}
  }
  { // Set
    fake_module = fake_module_cli::get_string(fake_module_cli);
    if (fake_module.size() > 4)
      fake_module = std::string(fake_module, 0, 4);
    else if (fake_module.size() < 4)
      fake_module += std::string(4 - fake_module.size(), ' ');
  }

	// "instance" - Check
  if (fake_instance_cli == 0 or fake_instance_cli > 99)
    return
      output_error_and_return_false(
        "The fake-instance parameter must be between 1 and 99");
  else { // Set
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << fake_instance_cli;
    fake_instance = oss.str();
  }

	if (a_vm.count("io-type")) { // Check
		const std::string& text = a_vm["io-type"].as<std::string>();
		io_type_cli = io_type_cli::get_enum(text);
		if (io_type_cli == io_type_cli::Enum::undefined) {
			const std::string message =
				"Unknown io-type parameter '" + text + "'";
			return output_error_and_return_false(message);
    }
  }
  { // Set
    const std::string text = io_type_cli::get_string(io_type_cli);
    if (text == "asio")
      io = io_type::asio;
    else if (text == "wsm")
      io = io_type::wsm;
	}

	if (a_vm.count("number-of-threads-in-log-tests")) {
		if (number_of_threads_in_log_tests == 0)
			return
				output_error_and_return_false(
					"The number-of-threads-in-log-tests parameter must be positive");
  }

	if (a_vm.count("log-min-severity-syslog")) { // Check
		const std::string& text = a_vm["log-min-severity-syslog"].as<std::string>();
		log_min_severity_syslog_cli = log_min_severity::get_enum(text);
		if (log_min_severity_syslog_cli == log_min_severity::Enum::undefined) {
			const std::string message =
				"Unknown log-min-severity-syslog parameter '" + text + "'";
			return output_error_and_return_false(message);
		}
  }
  { // Set
    const std::string text =
      log_min_severity::get_string(log_min_severity_syslog_cli);
    log_min_severity_syslog = as_severity(text);
  }

	if (a_vm.count("log-min-severity-console")) { // Check
		const std::string& text = a_vm["log-min-severity-console"].as<std::string>();
		log_min_severity_console_cli = log_min_severity::get_enum(text);
		if (log_min_severity_console_cli == log_min_severity::Enum::undefined) {
			const std::string message =
				"Unknown log-min-severity-console parameter '" + text + "'";
			return output_error_and_return_false(message);
		}
  }
  { // Set
    const std::string text =
      log_min_severity::get_string(log_min_severity_console_cli);
    log_min_severity_console = as_severity(text);
  }

	return true;
}

#ifdef WHISBI_USE_FILE_SELECTION_20161031
#define OutputWithCare(a_string) (a_string.empty() ? "[empty]" : a_string)
#endif

std::ostream&
cli::parsed_command_line(std::ostream& a_os)
{
	a_os << progname
		<< " was called with the following options:\n\n";

	// 1) File selection
	//
#ifdef WHISBI_USE_FILE_SELECTION_20161031
	a_os << "File selection:\n"
    << "  --input-file " << OutputWithCare(as_value(filename_in)) << '\n';
	a_os << '\n';
#endif

	// 2) Operation flags/parameters
	//
	a_os << "Operation flags/parameters:\n";
	a_os
		<< "  --fake-module " << as_value(fake_module) << '\n'
		<< "  --fake-instance "
      << as_value(std::to_string(fake_instance_cli)) << '\n'
		<< "  --io-type " << as_value(get_string(io_type_cli)) << '\n'
		<< "  --number-of-threads-in-log-tests "
      << as_value(std::to_string(number_of_threads_in_log_tests)) << '\n'
    << "  --number-of-agents "
      << as_value(std::to_string(number_of_agents)) << '\n'
  << "  --clear-request-trace "
    << as_value(std::to_string(clear_request_trace)) << '\n';
	a_os << '\n';

	// 3) Informative output
	//
	a_os << "Informative output:\n"
    << "  --log-min-severity-syslog "
      << as_value(get_string(log_min_severity_syslog_cli)) << '\n'
    << "  --log-min-severity-console "
      << as_value(get_string(log_min_severity_console_cli)) << '\n';

	a_os << std::flush;
	return a_os;
}

// -- eof
