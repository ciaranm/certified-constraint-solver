/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "constraint.hh"
#include "model.hh"
#include "variable.hh"

using std::string;

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

Table::Table(int a) :
    arity(a)
{
}

TableConstraint::TableConstraint(const std::shared_ptr<const Table> & t) :
    _table(t)
{
}

TableConstraint::~TableConstraint() = default;

auto TableConstraint::associate_with_variable(const string & n) -> void
{
    _vars.push_back(n);
}

auto TableConstraint::propagate(Model & model) const -> PropagationResult
{
    if (unsigned(_table->arity) != _vars.size())
        throw ModelError{ "Wrong number of variables in table constraint" };

    for (auto & a : _table->allowed_tuples) {
        bool ok = true;

        for (int i = 0 ; i < _table->arity ; ++i) {
            auto v = model.get_variable(_vars[i]);
            if (! v->values.count(a[i])) {
                ok = false;
                break;
            }
        }

        if (ok)
            return PropagationResult::NoChange;
    }

    return PropagationResult::Inconsistent;
}

