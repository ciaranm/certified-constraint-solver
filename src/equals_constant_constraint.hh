/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_EQUALS_CONSTANT_CONSTRAINT_HH
#define CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_EQUALS_CONSTANT_CONSTRAINT_HH 1

#include "constraint.hh"

class EqualConstantConstraint : public Constraint
{
    private:
        VariableID _first;
        VariableValue _second;
        ProofLineNumber _constraint_number;

    public:
        EqualConstantConstraint(VariableID, VariableValue);
        virtual ~EqualConstantConstraint() override;

        virtual auto propagate(Model & model, std::optional<Proof> &, std::set<VariableID> &) const -> bool override;

        virtual auto start_proof(const Model &, Proof &) -> void override;

        virtual auto associated_variables() const -> std::set<VariableID> override;

        virtual auto priority() const -> int override;
};

#endif
