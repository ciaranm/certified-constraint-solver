/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_VARIABLE_HH
#define GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_VARIABLE_HH 1

#include "variable-fwd.hh"
#include "proof-fwd.hh"

#include <memory>
#include <set>
#include <utility>

struct Variable
{
    Variable(int lw, int ub);
    Variable(const Variable &);
    ~Variable();

    std::shared_ptr<const std::set<int> > original_values;
    std::set<int> values;

    auto start_proof(const std::string & name, Proof &) const -> void;
};

#endif
