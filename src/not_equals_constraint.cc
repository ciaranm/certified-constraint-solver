/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "not_equals_constraint.hh"
#include "model.hh"
#include "variable.hh"
#include "proof.hh"

#include <iomanip>
#include <ostream>
#include <set>

using std::endl;
using std::optional;
using std::set;
using std::string;

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
    auto & w = model.get_variable(_second)->values;
    for (auto & v : model.get_variable(_first)->values)
        if (w.count(v)) {
            proof.model_stream() << "-1 x" << proof.variable_value_mapping(_first, v)
                << " -1 x" << proof.variable_value_mapping(_second, v) << " >= -1 ;" << endl;
            proof.next_model_line();
            _constraint_number.emplace(v, proof.last_model_line());
        }
}


