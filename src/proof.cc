/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "proof.hh"
#include "variable.hh"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <list>
#include <map>
#include <sstream>
#include <tuple>
#include <utility>

using std::copy;
using std::endl;
using std::istreambuf_iterator;
using std::list;
using std::make_unique;
using std::map;
using std::ofstream;
using std::ostreambuf_iterator;
using std::pair;
using std::string;
using std::stringstream;
using std::to_string;
using std::tuple;

ProofError::ProofError(const string & m) noexcept :
    _message("Proof error: " + m)
{
}

auto ProofError::what() const noexcept -> const char *
{
    return _message.c_str();
}

struct Proof::Imp
{
    ofstream opb_file;
    ofstream log_file;
    stringstream opb_body_file;

    ProofLineNumber model_constraints_line{ 0 };
    UnderlyingVariableID number_of_variables{ 0 };
    ProofLineNumber proof_line{ 0 };
    ProofLineNumber variable_axioms_start{ 0 };
    int anonymous_variables = 66666;

    map<VariableID, ProofLineNumber> variable_takes_at_least_one_value, variable_takes_at_most_one_value;
    map<pair<VariableID, VariableValue>, UnderlyingVariableID> vv_mapping;

    list<tuple<VariableID, string, VariableValue> > stack;

    bool asserty = false;
};

Proof::Proof(const string & opb, const string & log, bool asserty)
{
    _imp = make_unique<Proof::Imp>();
    _imp->asserty = asserty;

    _imp->opb_file.open(opb);
    if (! _imp->opb_file)
        throw ProofError{ "Cannot write proof model to '" + opb + "'" };

    _imp->log_file.open(log);
    if (! _imp->log_file)
        throw ProofError{ "Cannot write proof model to '" + log + "'" };
}

Proof::Proof(Proof && other) = default;

Proof::~Proof() = default;

auto Proof::operator= (Proof &&) -> Proof & = default;

auto Proof::write_header() -> void
{
    proof_stream() << "refutation using f l p c 0" << endl;

    _imp->opb_file << "* #variable= " << _imp->number_of_variables << " #constraint= " << _imp->model_constraints_line << endl;
     copy(istreambuf_iterator<char>{ _imp->opb_body_file }, istreambuf_iterator<char>{}, ostreambuf_iterator<char>{ _imp->opb_file });
}

auto Proof::load_problem_constraints() -> void
{
    proof_stream() << "f " << _imp->model_constraints_line << " 0" << endl;
    _imp->proof_line = ProofLineNumber{ int{ _imp->proof_line} + int{ _imp->model_constraints_line } };
}

auto Proof::load_variable_axioms() -> void
{
    proof_stream() << "l " << _imp->number_of_variables << " 0" << endl;
    _imp->variable_axioms_start = _imp->proof_line;
    _imp->proof_line = ProofLineNumber{ int{ _imp->proof_line } + (int{ _imp->number_of_variables } * 2) };
}

auto Proof::create_variable_value_mapping(VariableID n, VariableValue v) -> UnderlyingVariableID
{
    _imp->number_of_variables = UnderlyingVariableID{ int{ _imp->number_of_variables } + 1 };
    _imp->vv_mapping.emplace(pair{ n, v }, _imp->number_of_variables);
    return _imp->number_of_variables;
}

auto Proof::create_anonymous_extra_variable() -> UnderlyingVariableID
{
    _imp->number_of_variables = UnderlyingVariableID{ int{ _imp->number_of_variables } + 1 };
    _imp->vv_mapping.emplace(pair{ VariableID{ _imp->anonymous_variables++ }, 0 }, _imp->number_of_variables);
    return _imp->number_of_variables;
}

auto Proof::variable_value_mapping(VariableID n, VariableValue v) const -> UnderlyingVariableID
{
    return _imp->vv_mapping.find(pair{ n, v })->second;
}

auto Proof::wrote_variable_takes_at_least_one_value(VariableID n, ProofLineNumber v) -> void
{
    _imp->variable_takes_at_least_one_value.emplace(n, v);
}

auto Proof::wrote_variable_takes_at_most_one_value(VariableID n, ProofLineNumber v) -> void
{
    _imp->variable_takes_at_most_one_value.emplace(n, v);
}

auto Proof::model_stream() -> std::ostream &
{
    return _imp->opb_body_file;
}

auto Proof::last_model_line() const -> ProofLineNumber
{
    return _imp->model_constraints_line;
}

auto Proof::next_model_line() -> void
{
    _imp->model_constraints_line = ProofLineNumber{ int{ _imp->model_constraints_line } + 1 };
}

auto Proof::proof_stream() -> std::ostream &
{
    return _imp->log_file;
}

auto Proof::last_proof_line() const -> ProofLineNumber
{
    return _imp->proof_line;
}

auto Proof::next_proof_line() -> void
{
    _imp->proof_line = ProofLineNumber{ int{ _imp->proof_line} + 1 };
}

auto Proof::line_for_var_takes_at_least_one_value(VariableID n) -> ProofLineNumber
{
    return _imp->variable_takes_at_least_one_value.find(n)->second;
}

auto Proof::line_for_var_takes_at_most_one_value(VariableID n) -> ProofLineNumber
{
    return _imp->variable_takes_at_most_one_value.find(n)->second;
}

auto Proof::line_for_var_val_is_at_most_one(VariableID n, VariableValue v) const -> ProofLineNumber
{
    return ProofLineNumber{ int{ _imp->variable_axioms_start } + 2 * int{ _imp->vv_mapping.find(pair{ n, v })->second } };
}

auto Proof::line_for_var_val_is_at_least_zero(VariableID n, VariableValue v) const -> ProofLineNumber
{
    return ProofLineNumber{ int{ _imp->variable_axioms_start } + 2 * int{ _imp->vv_mapping.find(pair{ n, v })->second } - 1 };
}

auto Proof::asserty() const -> bool
{
    return _imp->asserty;
}

auto Proof::domain_wipeout(VariableID empty_var_name, const Variable & empty_var) -> void
{
    proof_stream() << "u opb";
    for (auto & [ var, _, val ] : _imp->stack)
        proof_stream() << " -1 x" << variable_value_mapping(var, val);
    for (auto & v : *empty_var.original_values)
        proof_stream() << " -1 x" << variable_value_mapping(empty_var_name, v);
    proof_stream() << " >= -" << (_imp->stack.size() + empty_var.original_values->size() - 1) << " ;" << endl;
    next_proof_line();
}

auto Proof::enstackinate_guess(VariableID var, const string & name, VariableValue val) -> void
{
    _imp->stack.push_back(tuple{ var, name, val });
    proof_stream() << "* guessing";
    for (auto & [ s, n, t ] : _imp->stack)
        proof_stream() << " " << n << "=" << int{ t } << " (" << "x" << variable_value_mapping(s, t) << ")";
    proof_stream() << endl;
}

auto Proof::incorrect_guess() -> void
{
    proof_stream() << "* incorrect guess";
    for (auto & [ s, n, t ] : _imp->stack)
        proof_stream() << " " << n << "=" << int{ t } << " (" << "x" << variable_value_mapping(s, t) << ")";
    proof_stream() << endl;

    auto [ wrong_var, _, wrong_val ] = _imp->stack.back();
    _imp->stack.pop_back();

    proof_stream() << "u opb";
    for (auto & [ var, _, val ] : _imp->stack)
        proof_stream() << " -1 x" << variable_value_mapping(var, val);
    proof_stream() << " -1 x" << variable_value_mapping(wrong_var, wrong_val);
    proof_stream() << " >= -" << _imp->stack.size() << " ;" << endl;
    next_proof_line();
}

