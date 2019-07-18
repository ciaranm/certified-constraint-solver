/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "equals_constant_constraint.hh"
#include "model.hh"
#include "variable.hh"
#include "proof.hh"

using std::endl;
using std::optional;
using std::set;

EqualConstantConstraint::EqualConstantConstraint(VariableID a, VariableValue b) :
    _first(a),
    _second(b)
{
}

EqualConstantConstraint::~EqualConstantConstraint() = default;

auto EqualConstantConstraint::propagate(Model & model, optional<Proof> & proof, set<VariableID> & changed_vars) const -> bool
{
    auto f = model.get_variable(_first);

    if (proof) {
        proof->proof_stream() << "* equals" << endl;
        for (auto & v : f->values)
            if (v != _second) {
                proof->proof_stream() << "p " << proof->line_for_var_takes_at_most_one_value(_first) << " "
                    << _constraint_number << " +";
                for (auto & w : *f->original_values)
                    if (w != v && w != _second)
                        proof->proof_stream() << " " << proof->line_for_var_val_is_at_least_zero(_first, w) << " +";
                proof->proof_stream() << " 0" << endl;
                proof->next_proof_line();
                proof->proved_var_not_equal_value(_first, v, proof->last_proof_line());
            }
    }

    // either the variable doesn't contain the value at all...
    if (! f->values.count(_second)) {
        if (proof) {
            proof->proof_stream() << "* got domain wipeout on equals" << endl;
            proof->proof_stream() << "p " << proof->line_for_var_takes_at_least_one_value(_first);
            for (auto & v : *f->original_values)
                proof->proof_stream() << " " << proof->line_for_var_not_equal_value(_first, v) << " +";
            proof->proof_stream() << " " << (f->original_values->size() + 1) << " d 0" << endl;
            proof->next_proof_line();
        }
        f->values.clear();
        return false;
    }
    else {
        // or it does, and if it contains other things too...
        if (f->values.size() != 1) {
            // then we nuke them
            changed_vars.insert(_first);
            f->values.clear();
            f->values.insert(_second);
        }
        return true;
    }
}

auto EqualConstantConstraint::start_proof(const Model &, Proof & proof) -> void
{
    proof.model_stream() << "* equals" << endl;
    proof.model_stream() << "1 x" << proof.variable_value_mapping(_first, _second) << " >= 1 ;" << endl;
    proof.next_model_line();
    _constraint_number = proof.last_model_line();
}

auto EqualConstantConstraint::associated_variables() const -> set<VariableID>
{
    set<VariableID> result;
    result.insert(_first);
    return result;
}


