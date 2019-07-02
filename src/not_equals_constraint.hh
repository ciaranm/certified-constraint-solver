/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_NOT_EQUALS_CONSTRAINT_HH
#define CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_NOT_EQUALS_CONSTRAINT_HH 1

#include "constraint.hh"

#include <map>
#include <string>

class NotEqualConstraint : public Constraint
{
    private:
        std::string _first, _second;
        std::map<int, int> _constraint_number;

    public:
        NotEqualConstraint(const std::string &, const std::string &);
        virtual ~NotEqualConstraint() override;

        virtual auto propagate(Model & model, std::optional<Proof> &) const -> PropagationResult override;

        virtual auto start_proof(const Model &, Proof &) -> void override;
};

#endif
