/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "model.hh"
#include "variable.hh"
#include "constraint.hh"
#include "result.hh"

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

Model::Model(const Model & other) = default;

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

