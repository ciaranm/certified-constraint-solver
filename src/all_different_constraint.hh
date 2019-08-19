/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_ALL_DIFFERENT_CONSTRAINT_HH
#define CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_ALL_DIFFERENT_CONSTRAINT_HH 1

#include "constraint.hh"

#include <list>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

enum class AllDifferentStrength
{
    Matching,
    GAC
};

class AllDifferentConstraint : public Constraint
{
    private:
        std::vector<VariableID> _vars;
        std::map<VariableValue, int> _constraint_numbers;
        AllDifferentStrength _strength;

        auto _prove_deletion_using_hall_set(
                Model &,
                Proof &,
                const std::map<VariableID, std::list<VariableValue> > & edges_out_from_variable,
                const std::map<VariableValue, std::list<VariableID> > & edges_out_from_value,
                const VariableID delete_variable,
                const VariableValue delete_value
                ) const -> void;

        auto _prove_matching_is_too_small(
                Model &,
                Proof &,
                const std::set<std::pair<VariableID, VariableValue> > & edges,
                const std::set<VariableID> & lhs,
                const std::set<VariableID> & left_covered,
                const std::set<std::pair<VariableID, VariableValue> > & matching
                ) const -> void;

    public:
        AllDifferentConstraint(std::vector<VariableID> &&, AllDifferentStrength);
        virtual ~AllDifferentConstraint() override;

        virtual auto propagate(Model & model, std::optional<Proof> &, std::set<VariableID> &) const -> bool override;

        virtual auto start_proof(const Model &, Proof &) -> void override;

        virtual auto associated_variables() const -> std::set<VariableID> override;

        virtual auto priority() const -> int override;
};

#endif
