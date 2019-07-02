/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "constraint.hh"
#include "model.hh"
#include "proof.hh"
#include "variable.hh"

#include <cstdint>
#include <set>
#include <vector>

using std::endl;
using std::map;
using std::optional;
using std::ostream;
using std::pair;
using std::set;
using std::shared_ptr;
using std::string;
using std::to_string;
using std::uintptr_t;
using std::vector;

Constraint::~Constraint() = default;

NotEqualConstraint::NotEqualConstraint(const string & a, const string & b) :
    _first(a),
    _second(b)
{
    if (_first == _second)
        throw ModelError{ "Cannot have not_equals constraint with '" + _first + "' as both arguments" };
}

NotEqualConstraint::~NotEqualConstraint() = default;

auto NotEqualConstraint::propagate(Model & model, optional<Proof> & proof) const -> PropagationResult
{
    bool changed = false;

    auto half_propagate = [&] (Variable & m, const string & my_name, Variable & o, const string & other_name) {
        if (m.values.size() == 1) {
            auto o_cannot_be = *m.values.begin();
            if (o.values.count(o_cannot_be)) {
                o.values.erase(o_cannot_be);
                changed = true;
                if (proof) {
                    // m must take a single value o_cannot_be. we know m must take
                    // at least one value...
                    set<int> conflicts;
                    // and we know it cannot take any of the other values...
                    for (auto & v : *m.original_values)
                        if (v != o_cannot_be)
                            conflicts.insert(proof->line_for_var_not_equal_value(my_name, v));

                    // now o cannot take the value o_cannot_be
                    proof->proof_stream() << "* not_equals means " << other_name << " can't take "
                        << my_name << "'s only value " << o_cannot_be << endl;

                    proof->proof_stream() << "p " << proof->line_for_var_takes_at_least_one_value(my_name);
                    for (auto & c : conflicts)
                        proof->proof_stream() << " " << c << " +";
                    proof->proof_stream() << " " << (conflicts.size() + 1) << " d";
                    proof->proof_stream() << " " << _constraint_number.find(o_cannot_be)->second << " + 0" << endl;
                    proof->next_proof_line();
                    proof->proved_var_not_equal_value(other_name, o_cannot_be, proof->last_proof_line());
                }
            }
        }
    };

    auto f = model.get_variable(_first);
    auto s = model.get_variable(_second);

    half_propagate(*f, _first, *s, _second);
    half_propagate(*s, _second, *f, _first);

    if (changed && (f->values.empty() || s->values.empty())) {
        if (proof) {
            auto half_prove_wipeout = [&] (
                    Variable & empty, const string & empty_name, Variable &, const string & other_name) {
                proof->proof_stream() << "* got domain wipeout on not_equals " << empty_name << " " << other_name << endl;
                proof->proof_stream() << "p " << proof->line_for_var_takes_at_least_one_value(empty_name);
                for (auto & v : *empty.original_values)
                    proof->proof_stream() << " " << proof->line_for_var_not_equal_value(empty_name, v) << " +";
                proof->proof_stream() << " " << (empty.original_values->size() + 1) << " d 0" << endl;
                proof->next_proof_line();
            };

            if (f->values.empty())
                half_prove_wipeout(*f, _first, *s, _second);
            else
                half_prove_wipeout(*s, _second, *f, _first);
        }
        return PropagationResult::Inconsistent;
    }
    else if (changed)
        return PropagationResult::Consistent;
    else
        return PropagationResult::NoChange;
}

auto NotEqualConstraint::start_proof(const Model & model, Proof & proof) -> void
{
    proof.model_stream() << "* not equals " << _first << " " << _second << endl;
    auto w = model.get_variable(_second)->values;
    for (auto & v : model.get_variable(_first)->values)
        if (w.count(v)) {
            proof.model_stream() << "-1 x" << proof.variable_value_mapping(_first, v)
                << " -1 x" << proof.variable_value_mapping(_second, v) << " >= -1 ;" << endl;
            proof.next_model_line();
            _constraint_number.emplace(v, proof.last_model_line());
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

auto TableConstraint::propagate(Model & model, optional<Proof> & proof) const -> PropagationResult
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

    if (proof) {
        // show that every control tuple has to be selected
        vector<int> controls;
        for (unsigned t = 0 ; t < _table->allowed_tuples.size() ; ++t) {
            // check this isn't an infeasible tuple not listed in the model
            auto c =  _constraint_for_tuple.find(t);
            if (c == _constraint_for_tuple.end())
                continue;

            proof->proof_stream() << "* table constraint infeasible tuple" << endl;
            proof->proof_stream() << "p " << c->second;
            for (int i = 0 ; i < _table->arity ; ++i) {
                if (! model.get_variable(_vars[i])->values.count(_table->allowed_tuples[t][i])) {
                    // this can contribute at most zero
                    proof->proof_stream() << " " << proof->line_for_var_not_equal_value(_vars[i], _table->allowed_tuples[t][i]) << " +";
                } else {
                    // this can contribute at most one
                    proof->proof_stream() << " " << proof->line_for_var_val_is_at_most_one(_vars[i], _table->allowed_tuples[t][i]) << " +";
                }
            }
            // and all of this sums up to too little
            proof->proof_stream() << " " << _table->arity << " d 0" << endl;
            proof->next_proof_line();
            controls.push_back(proof->last_proof_line());
        }

        // we can't select every control tuple
        proof->proof_stream() << "* table constraint is infeasible overall" << endl;
        proof->proof_stream() << "p " << _must_have_one_constraint;
        for (auto & c : controls) {
            proof->proof_stream() << " " << c << " +";
        }
        proof->proof_stream() << " 0" << endl;
        proof->next_proof_line();
    }

    return PropagationResult::Inconsistent;
}

auto TableConstraint::start_proof(const Model & model, Proof & proof) -> void
{
    proof.model_stream() << "* table";
    for (auto & v : _vars)
        proof.model_stream() << " " << v;
    proof.model_stream() << endl;

    // only write out feasible tuples. for each tuple, we have a control
    // variable, and either it is selected, or its control variable is
    // selected.
    vector<int> controls;
    for (unsigned t = 0 ; t < _table->allowed_tuples.size() ; ++t) {
        bool is_feasible = true;
        for (int i = 0 ; i < _table->arity ; ++i)
            if (! model.get_variable(_vars[i])->original_values->count(_table->allowed_tuples[t][i])) {
                is_feasible = false;
                break;
            }

        if (! is_feasible)
            continue;

        // either we pick this tuple, or we pick its control variable
        int control_idx = proof.create_variable_value_mapping("_table_" + to_string(reinterpret_cast<uintptr_t>(this)), t);
        controls.push_back(control_idx);

        proof.model_stream() << _table->arity << " x" << control_idx;
        for (int i = 0 ; i < _table->arity ; ++i)
            proof.model_stream() << " 1 x" << proof.variable_value_mapping(_vars[i], _table->allowed_tuples[t][i]);
        proof.model_stream() << " >= " << _table->arity << " ;" << endl;
        proof.next_model_line();
        _constraint_for_tuple.emplace(t, proof.last_model_line());
    }

    // this could just imply unsat, rather than an exception...
    if (controls.empty())
        throw ModelError{ "Table constraint is infeasible" };

    // we can't pick every control variable
    for (auto & c : controls)
        proof.model_stream() << "-1 x" << c << " ";
    proof.model_stream() << ">= -" << (controls.size() - 1) << " ;" << endl;
    proof.next_model_line();
    _must_have_one_constraint = proof.last_model_line();
}

