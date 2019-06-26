/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_RESULT_HH
#define GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_RESULT_HH 1

#include "result-fwd.hh"
#include <string>
#include <map>

struct Result
{
    unsigned long long nodes = 0;
    std::map<std::string, std::string> solution;
};

#endif
