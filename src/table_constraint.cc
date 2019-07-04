/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "table_constraint.hh"
#include "model.hh"
#include "variable.hh"
#include "proof.hh"

#include <iomanip>
#include <ostream>

using std::endl;
using std::optional;
using std::shared_ptr;
using std::set;
using std::string;
using std::to_string;
using std::vector;

Table::Table(int a) :
    arity(a)
{
}

TableConstraint::TableConstraint(const shared_ptr<const Table> & t) :
    _table(t)
{
}

TableConstraint::~TableConstraint() = default;

auto TableConstraint::associate_with_variable(VariableID n) -> void
{
    _vars.push_back(n);
}

auto TableConstraint::propagate(Model & model, optional<Proof> & proof, set<VariableID> &) const -> bool
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
            return true;
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
        proof->proof_stream() << " " << controls.size() << " d 0" << endl;
        proof->next_proof_line();
    }

    return false;
}

auto TableConstraint::start_proof(const Model & model, Proof & proof) -> void
{
    proof.model_stream() << "* table" << endl;

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
        int control_idx = proof.create_anonymous_extra_variable();
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

auto TableConstraint::associated_variables() const -> set<VariableID>
{
    set<VariableID> result{ _vars.begin(), _vars.end() };
    return result;
}

