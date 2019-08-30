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

NotEqualConstraint::NotEqualConstraint(VariableID a, VariableID b) :
    _first(a),
    _second(b)
{
    if (_first == _second)
        throw ModelError{ "Cannot have not_equals constraint with duplicate arguments" };
}

NotEqualConstraint::~NotEqualConstraint() = default;

auto NotEqualConstraint::propagate(Model & model, optional<Proof> & proof, set<VariableID> & changed_vars) const -> bool
{
    bool changed = false;

    auto half_propagate = [&] (Variable & m, const VariableID &, Variable & o, const VariableID & other_name) {
        if (m.values.size() == 1) {
            auto o_cannot_be = *m.values.begin();
            if (o.values.count(o_cannot_be)) {
                o.values.erase(o_cannot_be);
                changed_vars.insert(other_name);
                changed = true;
            }
        }
    };

    auto f = model.get_variable(_first);
    auto s = model.get_variable(_second);

    half_propagate(*f, _first, *s, _second);
    half_propagate(*s, _second, *f, _first);

    if (changed && (f->values.empty() || s->values.empty())) {
        if (proof)
            proof->proof_stream() << "* got domain wipeout on not_equals" << endl;
        return false;
    }

    return true;
}

auto NotEqualConstraint::start_proof(const Model & model, Proof & proof) -> void
{
    proof.model_stream() << "* not equals" << endl;
    auto & w = model.get_variable(_second)->values;
    for (auto & v : model.get_variable(_first)->values)
        if (w.count(v)) {
            proof.model_stream() << "-1 x" << proof.variable_value_mapping(_first, v)
                << " -1 x" << proof.variable_value_mapping(_second, v) << " >= -1 ;" << endl;
            proof.next_model_line();
            _constraint_number.emplace(v, proof.last_model_line());
        }
}

auto NotEqualConstraint::associated_variables() const -> set<VariableID>
{
    set<VariableID> result;
    result.insert(_first);
    result.insert(_second);
    return result;
}

auto NotEqualConstraint::priority() const -> int
{
    return 1;
}

