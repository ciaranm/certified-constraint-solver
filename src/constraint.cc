/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "constraint.hh"
#include "model.hh"
#include "variable.hh"

using std::endl;
using std::map;
using std::ostream;
using std::pair;
using std::shared_ptr;
using std::string;

Constraint::~Constraint() = default;

NotEqualConstraint::NotEqualConstraint(const string & a, const string & b) :
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

auto NotEqualConstraint::encode_as_opb(const Model & model, ostream & s, map<pair<string, int>, int> & vars_map, int & nb_constraints) const -> void
{
    s << "* not equals " << _first << " " << _second << endl;
    auto w = model.get_variable(_second)->values;
    for (auto & v : model.get_variable(_first)->values)
        if (w.count(v)) {
            ++nb_constraints;
            s << "-1 x" << vars_map.find(pair{ _first, v })->second << " -1 x" << vars_map.find(pair{ _second, v })->second << " >= -1 ;" << endl;
        }
}

Table::Table(int a) :
    arity(a)
{
}

TableConstraint::TableConstraint(const shared_ptr<const Table> & t) :
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

auto TableConstraint::encode_as_opb(const Model & model, std::ostream & s,
        map<pair<string, int>, int> & vars_map, int & nb_constraints) const -> void
{
    s << "* table" << endl;
    int orig_nb_constraints = nb_constraints;
    int first_table_index = vars_map.size() + 1;
    int last_table_index = first_table_index;

    for (auto & a : _table->allowed_tuples) {
        bool is_feasible = true;

        for (int i = 0 ; i < _table->arity ; ++i)
            if (! model.get_variable(_vars[i])->values.count(a[i])) {
                is_feasible = false;
                continue;
            }

        if (! is_feasible)
            continue;

        int idx = vars_map.size() + 1;
        last_table_index = idx;
        vars_map.emplace(pair{ "_table_", nb_constraints }, idx);
        s << _table->arity << " x" << idx;
        for (int i = 0 ; i < _table->arity ; ++i)
            s << " 1 x" << vars_map.find(pair{ _vars[i], a[i] })->second;
        s << " >= " << _table->arity << " ;" << endl;
        ++nb_constraints;
    }

    if (orig_nb_constraints == nb_constraints) {
        s << "* table is infeasible" << endl;
        s << "1 force_infeasible <= -1 ;" << endl;
        ++nb_constraints;
    }
    else {
        for (int c = first_table_index ; c <= last_table_index ; ++c)
            s << "-1 x" << c << " ";
        s << ">= " << -(last_table_index - first_table_index) << " ;" << endl;
        ++nb_constraints;
    }
}

