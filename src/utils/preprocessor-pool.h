// --

#pragma once

#include <ostream>

#ifdef __GNUG__
#define ENZO__GNUGPP_VERSION (__GNUC__ * 10000 \
	+ __GNUC_MINOR__ * 100 \
	+ __GNUC_PATCHLEVEL__)
// #if ADA_BYRON__GNUGPP_VERSION > 30200 // Test for GCC > 3.2.0
#endif

namespace spy {

std::ostream& list_of_preprocessor_defines(std::ostream&);

} // namespace spy

// -- eof
