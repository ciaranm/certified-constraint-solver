/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "variable.hh"
#include "proof.hh"
#include "model.hh"

#include <list>
#include <ostream>
#include <string>

using std::endl;
using std::list;
using std::make_shared;
using std::ostream;
using std::pair;
using std::set;
using std::string;

Variable::Variable(int lw, int ub)
{
    auto v = make_shared<set<VariableValue> >();
    for ( ; lw <= ub ; ++lw)
        v->insert(VariableValue{ lw });
    original_values = v;
    values = *v;
}

Variable::Variable(const set<int> & r)
{
    auto v = make_shared<set<VariableValue> >();
    for (auto & w : r)
        v->insert(VariableValue{ w });
    original_values = v;
    values = *v;
}

Variable::~Variable() = default;

Variable::Variable(const Variable &) = default;

auto Variable::start_proof(const Model & model, VariableID name, Proof & proof) const -> void
{
    list<UnderlyingVariableID> indices;

    proof.model_stream() << "* variable " << model.original_name(name) << ":";

    // record the variables in the opb file
    for (auto & v : values) {
        UnderlyingVariableID idx = proof.create_variable_value_mapping(model.original_name(name), name, v);
        indices.push_back(idx);
        proof.model_stream() << " (" << int{ v } << ", x" << idx << ")";
    }
    proof.model_stream() << endl;

    // a variable must take exactly one value

    for (auto & i : indices)
        proof.model_stream() << "1 " << "x" << i << " ";
    proof.model_stream() << ">= 1 ;" << endl;
    proof.next_model_line();
    proof.wrote_variable_takes_at_least_one_value(name, proof.last_model_line());

    for (auto & i : indices)
        proof.model_stream() << "-1 " << "x" << i << " ";
    proof.model_stream() << ">= -1 ;" << endl;
    proof.next_model_line();
    proof.wrote_variable_takes_at_most_one_value(name, proof.last_model_line());
}

