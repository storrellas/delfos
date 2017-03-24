// --

#pragma once

#include <array>
#include <string>
#include <vector>

#include <wcore/utils/helper/CompilerWarningsOff++Begin.h>
# include <boost/log/trivial.hpp>
# include <boost/preprocessor.hpp>
#include <wcore/utils/helper/CompilerWarningsOff++End.h>

#include <wcore/logging/Logger.h>

#define WHISBI_TO_STR(unused, data, elem) BOOST_PP_STRINGIZE(elem),

// Code originarily based on
// http://www.gamedev.net/topic/437852-c-enum-names-as-strings/
//
#define WHISBI_DEFINE_NAMESPACE_WITH_ENUM_TOOLS(namespace_name, sequence) \
namespace namespace_name { \
\
enum class Enum : uint8_t { undefined, BOOST_PP_SEQ_ENUM(sequence) }; \
const std::array<const char*, 1 + BOOST_PP_SEQ_SIZE(sequence)> enum_text{{ \
	"undefined", BOOST_PP_SEQ_FOR_EACH(WHISBI_TO_STR, ~, sequence) }}; \
\
const size_t beyond = enum_text.size(); \
\
inline \
Enum \
get_enum(const std::string& a_text) { \
	size_t i = 1; \
	for (; i < beyond && a_text != enum_text[i]; ++i); \
	return i < beyond ? static_cast<Enum>(i) : Enum::undefined; \
} \
\
inline \
std::string \
get_string(const Enum a_enum) {	\
  return enum_text[static_cast<size_t>(a_enum)]; \
} \
\
inline \
std::vector<std::string> \
get_defined_strings() { \
	std::vector<std::string> result; \
	for (size_t i = 1; i < beyond; ++i) \
		result.push_back(enum_text[i]); \
	return result; \
} \
\
} // namespace namespace_name

WHISBI_DEFINE_NAMESPACE_WITH_ENUM_TOOLS(\
	fake_module_cli,
  (ACM)(DAI)(Enzo)(GARI)(KPI)(LM)(PBX)(RM)(RTV)
)

WHISBI_DEFINE_NAMESPACE_WITH_ENUM_TOOLS(\
	io_type_cli,
	(asio)(wsm)
)

WHISBI_DEFINE_NAMESPACE_WITH_ENUM_TOOLS(\
	log_min_severity,
	(trace)(debug)(info)(warning)(error)(fatal)
)

enum class io_type : uint8_t {
  asio,
  wsm
};

namespace cli {

// 1) File selection
//
#undef WHISBI_USE_FILE_SELECTION_20161031
#ifdef WHISBI_USE_FILE_SELECTION_20161031
extern std::string filename_in;
#endif

// 2) Operation flags/parameters
//
extern std::string fake_module;
extern std::string fake_instance;
extern io_type io;
extern uint number_of_threads_in_log_tests;
extern uint number_of_agents;
extern bool clear_request_trace;

// 3) Informative output
//
extern delfos::core::severity_level log_min_severity_syslog;
extern delfos::core::severity_level log_min_severity_console;

std::string argv_0();
std::string program_name();

bool
parse_command_line(int argc, char** argv);

std::ostream&
parsed_command_line(std::ostream&);

} // namespace cli

// -- eof
