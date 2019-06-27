/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_VARIABLE_HH
#define GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_VARIABLE_HH 1

#include "refutation_log-fwd.hh"

#include <iosfwd>
#include <map>
#include <set>
#include <utility>

struct Variable
{
    Variable(int lw, int ub);
    Variable(const Variable &);
    ~Variable();

    std::set<int> original_values, values;

    auto encode_as_opb(const std::string & name, std::ostream & s, std::map<std::pair<std::string, int>, int> & vars_map, int & nb_constraints, RefutationLog & log) const -> void;
};

#endif
