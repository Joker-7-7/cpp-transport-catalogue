#pragma once

#include <iosfwd>
#include <string_view>
#include <ostream>

#include "transport_catalogue.h"

namespace reader
{
    void ParseAndPrintStat(const catalogue::Transport& transport_catalogue, std::string_view request,
                        std::ostream& output);
}
