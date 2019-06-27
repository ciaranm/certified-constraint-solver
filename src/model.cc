/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "model.hh"
#include "variable.hh"
#include "constraint.hh"
#include "result.hh"

#include <map>
#include <memory>
#include <utility>

using std::endl;
using std::make_shared;
using std::map;
using std::pair;
using std::shared_ptr;
using std::string;
using std::to_string;

ModelError::ModelError(const string & m) noexcept :
    _message("Model error: " + m)
{
}

auto ModelError::what() const noexcept -> const char *
{
    return _message.c_str();
}

Model::Model() = default;

Model::Model(const Model & other) :
    constraints(other.constraints)
{
    for (auto & [ n, v ] : other._vars)
        _vars.emplace(n, make_shared<Variable>(*v));
}

Model::~Model() = default;

auto Model::add_variable(const string & n, shared_ptr<Variable> v) -> bool
{
    return _vars.emplace(n, v).second;
}

auto Model::get_variable(const string & n) const -> shared_ptr<Variable>
{
    auto r = _vars.find(n);
    if (r == _vars.end())
        throw ModelError{ "No variable named '" + n + "'" };
    else
        return r->second;
}

auto Model::select_branch_variable(std::string & result_name) const -> std::shared_ptr<Variable>
{
    std::shared_ptr<Variable> result;
    for (auto & [ name, v ] : _vars) {
        if (v->values.size() != 1) {
            if ((! result) || v->values.size() < result->values.size()) {
                result_name = name;
                result = v;
            }
        }
    }

    return result;
}

auto Model::save_result(Result & result) const -> void
{
    for (auto & [ name, v ] : _vars) {
        if (1 != v->values.size())
            throw ModelError{ "Variable '" + name + "' contains " + to_string(v->values.size()) + " values, but was expecting one" };
        result.solution.emplace(name, to_string(*v->values.begin()));
    }
}

auto Model::encode_as_opb(std::ostream & s, int & nb_vars, int & nb_constraints, RefutationLog & log) const -> void
{
    map<pair<string, int>, int> vars_map;
    for (auto & [ name, v ] : _vars)
        v->encode_as_opb(name, s, vars_map, nb_constraints, log);

    for (auto & c : constraints)
        c->encode_as_opb(*this, s, vars_map, nb_constraints, log);

    nb_vars = vars_map.size();

    log->current_index += (2 * nb_vars);
    log->vars_start_at(nb_constraints + 1);
}

auto Model::write_ref_header(RefutationLog & log, int nb_opb_vars, int nb_opb_constraints) const -> void
{
    *log->stream << "f " << nb_opb_constraints << " 0" << endl;
    *log->stream << "l " << nb_opb_vars << " 0" << endl;
}

