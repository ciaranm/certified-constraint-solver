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
    explicit Variable(const std::set<int> & values);
    Variable(const Variable &);
    ~Variable();

    std::shared_ptr<const std::set<VariableValue> > original_values;
    std::set<VariableValue> values;

    auto start_proof(VariableID, Proof &) const -> void;
};

#endif
