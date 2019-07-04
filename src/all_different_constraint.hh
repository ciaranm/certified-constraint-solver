/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_ALL_DIFFERENT_CONSTRAINT_HH
#define CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_ALL_DIFFERENT_CONSTRAINT_HH 1

#include "constraint.hh"

#include <map>
#include <string>
#include <tuple>
#include <vector>

class AllDifferentConstraint : public Constraint
{
    private:
        std::vector<VariableID> _vars;
        std::map<std::tuple<VariableID, VariableID, VariableValue>, int> _constraint_numbers;

    public:
        explicit AllDifferentConstraint(std::vector<VariableID> &&);
        virtual ~AllDifferentConstraint() override;

        virtual auto propagate(Model & model, std::optional<Proof> &, std::set<VariableID> &) const -> bool override;

        virtual auto start_proof(const Model &, Proof &) -> void override;

        virtual auto associated_variables() const -> std::set<VariableID> override;
};

#endif
