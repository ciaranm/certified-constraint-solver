/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "constraint.hh"
#include "model.hh"
#include "variable.hh"

#include <set>

using std::endl;
using std::list;
using std::map;
using std::ostream;
using std::pair;
using std::set;
using std::shared_ptr;
using std::string;

Constraint::~Constraint() = default;

NotEqualConstraint::NotEqualConstraint(const string & a, const string & b) :
    _first(a),
    _second(b)
{
    if (_first == _second)
        throw ModelError{ "Cannot have not_equals constraint with '" + _first + "' as both arguments" };
}

NotEqualConstraint::~NotEqualConstraint() = default;

auto NotEqualConstraint::propagate(Model & model, RefutationLog & log) const -> PropagationResult
{
    bool changed = false;

    auto half_propagate = [&] (Variable & m, const string & my_name, Variable & o, const string & other_name) {
        if (m.values.size() == 1) {
            auto o_cannot_be = *m.values.begin();
            if (o.values.count(o_cannot_be)) {
                o.values.erase(o_cannot_be);
                changed = true;
                if (log) {
                    // m must take a single value o_cannot_be. we know m must take
                    // at least one value...
                    set<int> conflicts;
                    // and we know it cannot take any of the other values...
                    for (auto & v : m.original_values)
                        if (v != o_cannot_be)
                            conflicts.insert(log->why_not(my_name, v));

                    // now o cannot take the value o_cannot_be
                    *log->stream << "* not_equals means " << other_name << " can't take "
                        << my_name << "'s only value " << o_cannot_be << endl;

                    *log->stream << "p " << log->var_takes_at_least_one_value(my_name);
                    for (auto & c : conflicts)
                        *log->stream << " " << c << " +";
                    *log->stream << " " << (conflicts.size() + 1) << " d";
                    *log->stream << " " << _constraint_number.find(o_cannot_be)->second << " + 0" << endl;
                    ++log->current_index;
                    log->record_why_not(other_name, o_cannot_be, log->current_index);
                }
            }
        }
    };

    auto f = model.get_variable(_first);
    auto s = model.get_variable(_second);

    half_propagate(*f, _first, *s, _second);
    half_propagate(*s, _second, *f, _first);

    if (changed && (f->values.empty() || s->values.empty())) {
        if (log) {
            if (f->values.empty()) {
                // we know f must take at least one value...
                *log->stream << "* got domain wipeout on not_equals " << _first << " " << _second << endl;
                *log->stream << "p " << log->var_takes_at_least_one_value(_first);
                // and we have ruled out all of its values...
                for (auto & v : f->original_values)
                    *log->stream << " " << log->why_not(_first, v) << " +";
                *log->stream << " 0" << endl;
                ++log->current_index;
                *log->stream << "c " << log->current_index << " 2 d 0" << endl;
                ++log->current_index;
            }
            else {
                // we know s must take at least one value...
                *log->stream << "* got domain wipeout on not_equals " << _first << " " << _second << endl;
                *log->stream << "p " << log->var_takes_at_least_one_value(_second);
                // and we have ruled out all of its values...
                for (auto & v : s->original_values)
                    *log->stream << " " << log->why_not(_second, v) << " +";
                *log->stream << " 2 d 0" << endl;
                ++log->current_index;
            }
        }
        return PropagationResult::Inconsistent;
    }
    else if (changed)
        return PropagationResult::Consistent;
    else
        return PropagationResult::NoChange;
}

auto NotEqualConstraint::encode_as_opb(const Model & model, ostream & s, map<pair<string, int>, int> & vars_map, int & nb_constraints, RefutationLog & log) -> void
{
    s << "* not equals " << _first << " " << _second << endl;
    auto w = model.get_variable(_second)->values;
    for (auto & v : model.get_variable(_first)->values)
        if (w.count(v)) {
            ++nb_constraints;
            if (log)
                ++log->current_index;
            s << "-1 x" << vars_map.find(pair{ _first, v })->second << " -1 x" << vars_map.find(pair{ _second, v })->second << " >= -1 ;" << endl;
            _constraint_number.emplace(v, nb_constraints);
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

auto TableConstraint::propagate(Model & model, RefutationLog & log) const -> PropagationResult
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

    if (log) {
        // for each tuple that has a constraint in turn, show that its disable
        // variable is set to true
        list<int> disabled;
        for (auto & a : _table->allowed_tuples) {
            // skip any infeasible tuples, because they don't have an associated constraint
            if (! _constraint_for_tuple.count(a))
                continue;

            set<int> conflicts;
            // find a contradicting variable
            int contradicting_variable = 0;
            for (int i = 0 ; i < _table->arity ; ++i) {
                auto v = model.get_variable(_vars[i]);
                if (v->values.count(a[i]))
                    continue;

                contradicting_variable = i;
                conflicts.insert(log->why_not(_vars[i], a[i]));
                break;
            }

            // now eliminate non-contradicting variables
            for (int i = 0 ; i < _table->arity ; ++i) {
                if (i == contradicting_variable)
                    continue;
                auto v = model.get_variable(_vars[i]);
                conflicts.insert(log->inverse_is_at_least_zero(_vars[i], a[i]));
            }

            *log->stream << "p " << _constraint_for_tuple.find(a)->second;
            for (auto & c : conflicts)
                *log->stream << " " << c << " +";
            *log->stream << " " << conflicts.size() << " d 0" << endl;
            disabled.push_back(++log->current_index);
        }
        *log->stream << "p " << _must_have_one_constraint;
        for (auto & d : disabled)
            *log->stream << " " << d << " +";
        *log->stream << " 0" << endl;
        ++log->current_index;
    }

    return PropagationResult::Inconsistent;
}

auto TableConstraint::encode_as_opb(const Model & model, std::ostream & s,
        map<pair<string, int>, int> & vars_map, int & nb_constraints, RefutationLog & log) -> void
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
        if (log) {
            _var_for_tuple.emplace(a, idx);
        }
        s << _table->arity << " x" << idx;
        for (int i = 0 ; i < _table->arity ; ++i)
            s << " 1 x" << vars_map.find(pair{ _vars[i], a[i] })->second;
        s << " >= " << _table->arity << " ;" << endl;
        ++nb_constraints;
        if (log) {
            _constraint_for_tuple.emplace(a, ++log->current_index);
        }
    }

    if (orig_nb_constraints == nb_constraints) {
        s << "* table is infeasible" << endl;
        s << "1 x1 >= 999 ;" << endl;
        ++nb_constraints;
        if (log)
            ++log->current_index;
    }
    else {
        for (int c = first_table_index ; c <= last_table_index ; ++c)
            s << "-1 x" << c << " ";
        s << ">= " << -(last_table_index - first_table_index) << " ;" << endl;
        ++nb_constraints;
        if (log) {
            _must_have_one_constraint = ++log->current_index;
        }
    }
}

