/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "constraint.hh"
#include "model.hh"
#include "variable.hh"

Constraint::~Constraint() = default;

NotEqualConstraint::NotEqualConstraint(const std::string & a, const std::string & b) :
    _first(a),
    _second(b)
{
}

NotEqualConstraint::~NotEqualConstraint() = default;

auto NotEqualConstraint::propagate(Model & model) const -> PropagationResult
{
    auto f = model.get_variable(_first);
    auto s = model.get_variable(_second);
    auto f_initial_size = f->values.size();
    auto s_initial_size = s->values.size();

    if (f->values.size() == 1)
        s->values.erase(*f->values.begin());
    if (s->values.size() == 1)
        f->values.erase(*s->values.begin());

    if (f->values.empty() || s->values.empty() || f == s)
        return PropagationResult::Inconsistent;
    else if (f_initial_size != f->values.size() || s_initial_size != s->values.size())
        return PropagationResult::Consistent;
    else
        return PropagationResult::NoChange;
}

