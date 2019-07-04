/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "all_different_constraint.hh"
#include "model.hh"
#include "proof.hh"
#include "variable.hh"

#include <iomanip>
#include <ostream>
#include <set>

using std::endl;
using std::map;
using std::move;
using std::optional;
using std::ostream;
using std::set;
using std::string;
using std::tuple;
using std::vector;

AllDifferentConstraint::AllDifferentConstraint(vector<VariableID> && v) :
    _vars(move(v))
{
}

AllDifferentConstraint::~AllDifferentConstraint() = default;

auto AllDifferentConstraint::propagate(Model & model, optional<Proof> & proof, set<VariableID> & changed_vars) const -> bool
{
    for (bool changed = true ; changed ; ) {
        changed = false;
        for (unsigned i = 0 ; i < _vars.size() ; ++i) {
            for (unsigned j = 0 ; j < _vars.size() ; ++j) {
                if (i == j)
                    continue;

                auto i_var = model.get_variable(_vars[i]);
                auto & i_values = i_var->values;
                auto j_var = model.get_variable(_vars[j]);
                auto & j_values = j_var->values;

                if (1 != i_values.size())
                   continue;

                auto j_cannot_be = *i_values.begin();
                if (! j_values.count(j_cannot_be))
                    continue;

                j_values.erase(j_cannot_be);
                changed_vars.insert(_vars[j]);

                changed = true;
                if (proof) {
                    // i_var must take a single value j_cannot_be. we know i_var must take
                    // at least one value...
                    set<int> conflicts;
                    // and we know it cannot take any of the other values...
                    for (auto & v : *i_var->original_values)
                        if (v != j_cannot_be)
                            conflicts.insert(proof->line_for_var_not_equal_value(_vars[i], v));

                    // now j cannot take the value j_cannot_be
                    proof->proof_stream() << "* all different" << endl;

                    proof->proof_stream() << "p " << proof->line_for_var_takes_at_least_one_value(_vars[i]);
                    for (auto & c : conflicts)
                        proof->proof_stream() << " " << c << " +";
                    proof->proof_stream() << " " << _constraint_numbers.find(tuple{ _vars[i], _vars[j], j_cannot_be })->second << " +";
                    proof->proof_stream() << " " << (conflicts.size() + 1) << " d 0" << endl;
                    proof->next_proof_line();
                    proof->proved_var_not_equal_value(_vars[j], j_cannot_be, proof->last_proof_line());
                }

                if (j_values.empty()) {
                    if (proof) {
                        proof->proof_stream() << "* got domain wipeout on all_different" << endl;
                        proof->proof_stream() << "p " << proof->line_for_var_takes_at_least_one_value(_vars[j]);
                        for (auto & v : *model.get_variable(_vars[j])->original_values)
                            proof->proof_stream() << " " << proof->line_for_var_not_equal_value(_vars[j], v) << " +";
                        proof->proof_stream() << " " << (model.get_variable(_vars[j])->original_values->size() + 1) << " d 0" << endl;
                        proof->next_proof_line();
                    }
                    return false;
                }
            }
        }
    }

    return true;
}

auto AllDifferentConstraint::start_proof(const Model & model, Proof & proof) -> void
{
    proof.model_stream() << "* all different" << endl;

    for (unsigned i = 0 ; i < _vars.size() ; ++i)
        for (unsigned j = i + 1 ; j < _vars.size() ; ++j) {
            auto w = model.get_variable(_vars[j])->values;
            for (auto & v : model.get_variable(_vars[i])->values)
                if (w.count(v)) {
                    proof.model_stream() << "-1 x" << proof.variable_value_mapping(_vars[i], v)
                        << " -1 x" << proof.variable_value_mapping(_vars[j], v) << " >= -1 ;" << endl;
                    proof.next_model_line();
                    _constraint_numbers.emplace(tuple{ _vars[i], _vars[j], v }, proof.last_model_line());
                    _constraint_numbers.emplace(tuple{ _vars[j], _vars[i], v }, proof.last_model_line());
                }
        }
}

auto AllDifferentConstraint::associated_variables() const -> set<VariableID>
{
    set<VariableID> result{ _vars.begin(), _vars.end() };
    return result;
}

