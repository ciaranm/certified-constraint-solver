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
    int number_of_variables{ 0 };
    ProofLineNumber proof_line{ 0 };
    ProofLineNumber variable_axioms_start{ 0 };
    int anonymous_variables = 66666;

    map<VariableID, ProofLineNumber> variable_takes_at_least_one_value, variable_takes_at_most_one_value;
    map<pair<VariableID, VariableValue>, UnderlyingVariableID> vv_mapping;

    list<tuple<VariableID, string, VariableValue> > stack;

    bool asserty = false;
    bool levels = false;
    bool numbered_variables = false;
};

Proof::Proof(const string & opb, const string & log, bool asserty, bool levels, bool numbered_variables)
{
    _imp = make_unique<Proof::Imp>();
    _imp->asserty = asserty;
    _imp->levels = levels;
    _imp->numbered_variables = numbered_variables;

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
    proof_stream() << "pseudo-Boolean proof version 1.0" << endl;

    _imp->opb_file << "* #variable= " << _imp->number_of_variables << " #constraint= " << _imp->model_constraints_line << endl;
     copy(istreambuf_iterator<char>{ _imp->opb_body_file }, istreambuf_iterator<char>{}, ostreambuf_iterator<char>{ _imp->opb_file });
}

auto Proof::load_problem_constraints() -> void
{
    proof_stream() << "f " << _imp->model_constraints_line << " 0" << endl;
    _imp->proof_line = ProofLineNumber{ int{ _imp->proof_line} + int{ _imp->model_constraints_line } };
}

auto Proof::create_variable_value_mapping(const std::string & nn, VariableID n, VariableValue v) -> UnderlyingVariableID
{
    ++_imp->number_of_variables;
    UnderlyingVariableID name{ _imp->numbered_variables ? to_string(_imp->number_of_variables) : nn + "_" + to_string(int{ v }) };
    _imp->vv_mapping.emplace(pair{ n, v }, name);
    return name;
}

auto Proof::create_anonymous_extra_variable() -> UnderlyingVariableID
{
    ++_imp->number_of_variables;
    UnderlyingVariableID name{ to_string(_imp->number_of_variables) };
    _imp->vv_mapping.emplace(pair{ VariableID{ _imp->anonymous_variables++ }, 0 }, name);
    return name;
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

auto Proof::asserty() const -> bool
{
    return _imp->asserty;
}

auto Proof::levels() const -> bool
{
    return _imp->levels;
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

    proof_stream() << "u ";
    for (auto & [ var, _, val ] : _imp->stack)
        proof_stream() << " -1 x" << variable_value_mapping(var, val);
    proof_stream() << " -1 x" << variable_value_mapping(wrong_var, wrong_val);
    proof_stream() << " >= -" << _imp->stack.size() << " ;" << endl;
    next_proof_line();
}

