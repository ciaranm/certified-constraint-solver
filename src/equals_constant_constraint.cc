/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "equals_constant_constraint.hh"
#include "model.hh"
#include "variable.hh"

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

    // either the variable doesn't contain the value at all...
    if (! f->values.count(_second)) {
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

auto EqualConstantConstraint::start_proof(const Model & model, Proof & proof) -> void
{
}

auto EqualConstantConstraint::associated_variables() const -> set<VariableID>
{
    set<VariableID> result;
    result.insert(_first);
    return result;
}


