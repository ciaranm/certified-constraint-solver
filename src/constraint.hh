/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_CONSTRAINT_HH
#define GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_CONSTRAINT_HH 1

#include "constraint-fwd.hh"
#include "model-fwd.hh"
#include "proof-fwd.hh"
#include "variable-fwd.hh"

#include <optional>
#include <set>
#include <string>

struct Constraint
{
    Constraint() = default;
    virtual ~Constraint() = 0;

    Constraint(const Constraint &) = delete;
    Constraint & operator= (const Constraint &) = delete;

    [[ nodiscard ]] virtual auto propagate(
            Model & model,
            std::optional<Proof> &,
            std::set<VariableID> & changed_variables) const -> bool = 0;

    virtual auto start_proof(const Model &, Proof &) -> void = 0;

    virtual auto associated_variables() const -> std::set<VariableID> = 0;
};

#endif
