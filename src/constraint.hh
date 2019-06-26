/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_CONSTRAINT_HH
#define GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_CONSTRAINT_HH 1

#include "constraint-fwd.hh"
#include "model-fwd.hh"

#include <string>

struct Constraint
{
    Constraint() = default;
    virtual ~Constraint() = 0;

    Constraint(const Constraint &) = delete;
    Constraint & operator= (const Constraint &) = delete;

    [[ nodiscard ]] virtual auto propagate(Model & model) const -> PropagationResult = 0;
};

class NotEqualConstraint : public Constraint
{
    private:
        std::string _first, _second;

    public:
        NotEqualConstraint(const std::string &, const std::string &);
        virtual ~NotEqualConstraint() override;

        virtual auto propagate(Model & model) const -> PropagationResult override;
};

#endif
