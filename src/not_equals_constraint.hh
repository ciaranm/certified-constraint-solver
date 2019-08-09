/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_NOT_EQUALS_CONSTRAINT_HH
#define CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_NOT_EQUALS_CONSTRAINT_HH 1

#include "constraint.hh"

#include <map>
#include <string>

class NotEqualConstraint : public Constraint
{
    private:
        VariableID _first, _second;
        std::map<VariableValue, int> _constraint_number;

    public:
        NotEqualConstraint(VariableID, VariableID);
        virtual ~NotEqualConstraint() override;

        virtual auto propagate(Model & model, std::optional<Proof> &, std::set<VariableID> &) const -> bool override;

        virtual auto start_proof(const Model &, Proof &) -> void override;

        virtual auto associated_variables() const -> std::set<VariableID> override;

        virtual auto priority() const -> int override;
};

#endif
