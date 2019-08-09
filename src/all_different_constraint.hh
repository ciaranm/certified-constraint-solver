/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_ALL_DIFFERENT_CONSTRAINT_HH
#define CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_ALL_DIFFERENT_CONSTRAINT_HH 1

#include "constraint.hh"

#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

class AllDifferentConstraint : public Constraint
{
    private:
        std::vector<VariableID> _vars;
        std::map<VariableValue, int> _constraint_numbers;

        auto _prove_matching_is_too_small(
                Model &,
                Proof &,
                const std::set<std::pair<VariableID, VariableValue> > & edges,
                const std::set<VariableID> & lhs,
                const std::set<VariableID> & left_covered,
                const std::set<std::pair<VariableID, VariableValue> > & matching
                ) const -> void;

    public:
        explicit AllDifferentConstraint(std::vector<VariableID> &&);
        virtual ~AllDifferentConstraint() override;

        virtual auto propagate(Model & model, std::optional<Proof> &, std::set<VariableID> &) const -> bool override;

        virtual auto start_proof(const Model &, Proof &) -> void override;

        virtual auto associated_variables() const -> std::set<VariableID> override;

        virtual auto priority() const -> int override;
};

#endif
