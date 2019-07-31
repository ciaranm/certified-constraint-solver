/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "proof.hh"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <list>
#include <map>
#include <sstream>
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
    list<map<pair<VariableID, VariableValue>, ProofLineNumber> > var_not_equal_value;
    map<pair<VariableID, VariableValue>, UnderlyingVariableID> vv_mapping;

    const list<pair<VariableID, VariableValue> > * active_stack;

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

auto Proof::line_for_var_not_equal_value(VariableID v, VariableValue n) -> ProofLineNumber
{
    for (auto r = _imp->var_not_equal_value.rbegin() ; r != _imp->var_not_equal_value.rend() ; ++r) {
        auto w = r->find(pair{ v, n });
        if (w != r->end())
            return w->second;
    }

    throw ProofError{ "Don't have a stored reason" };
}

auto Proof::proved_var_not_equal_value(VariableID v, VariableValue n, ProofLineNumber l) -> void
{
    _imp->var_not_equal_value.back().emplace(pair{ v, n }, l);
    if (_imp->asserty) {
        assert_we_proved_var_not_equal_value(v, n, "storing conflict line");
    }
}

auto Proof::line_for_var_takes_at_least_one_value(VariableID n) -> ProofLineNumber
{
    return _imp->variable_takes_at_least_one_value.find(n)->second;
}

auto Proof::line_for_var_takes_at_most_one_value(VariableID n) -> ProofLineNumber
{
    return _imp->variable_takes_at_most_one_value.find(n)->second;
}

auto Proof::push_context() -> void
{
    _imp->var_not_equal_value.emplace_back();
}

auto Proof::pop_context() -> void
{
    _imp->var_not_equal_value.pop_back();
}

auto Proof::line_for_var_val_is_at_most_one(VariableID n, VariableValue v) const -> ProofLineNumber
{
    return ProofLineNumber{ int{ _imp->variable_axioms_start } + 2 * int{ _imp->vv_mapping.find(pair{ n, v })->second } };
}

auto Proof::line_for_var_val_is_at_least_zero(VariableID n, VariableValue v) const -> ProofLineNumber
{
    return ProofLineNumber{ int{ _imp->variable_axioms_start } + 2 * int{ _imp->vv_mapping.find(pair{ n, v })->second } - 1 };
}

auto Proof::assert_what_we_just_did(const string & why) -> void
{
    proof_stream() << "* verifying that the conflict generated is what we expect, " << why << endl;
    auto recover_line = last_proof_line();
    proof_stream() << "p " << last_proof_line();
    for (auto & [ var, val ] : *_imp->active_stack)
        proof_stream() << " " << line_for_var_val_is_at_most_one(var, val) << " 1000 * +";
    proof_stream() << " 0" << endl;
    next_proof_line();
    proof_stream() << "p " << last_proof_line() << " 1001 d 0" << endl;
    next_proof_line();

    proof_stream() << "e " << last_proof_line() << " opb";
    for (auto & [ var, val ] : *_imp->active_stack)
        proof_stream() << " 1 ~x" << variable_value_mapping(var, val);
    proof_stream() << " >= 1 ;" << endl;

    proof_stream() << "p " << recover_line << " 0 + 0" << endl;
    next_proof_line();

    proof_stream() << "* end of verification" << endl;
}

auto Proof::assert_we_proved_var_not_equal_value(VariableID var, VariableValue val,
        const string & why) -> void
{
    proof_stream() << "* verifying that we just proved var " << int{ var } << " not equal value " << int{ val } << ", x" << variable_value_mapping(var, val) << ", " << why << endl;
    auto recover_line = last_proof_line();
    proof_stream() << "p " << last_proof_line();
    for (auto & [ var, val ] : *_imp->active_stack)
        proof_stream() << " " << line_for_var_val_is_at_most_one(var, val) << " 1000 * +";
    proof_stream() << " 0" << endl;
    next_proof_line();
    proof_stream() << "p " << last_proof_line() << " 1001 d 0" << endl;
    next_proof_line();

    proof_stream() << "e " << last_proof_line() << " opb";
    for (auto & [ var, val ] : *_imp->active_stack)
        proof_stream() << " 1 ~x" << variable_value_mapping(var, val);
    proof_stream() << " 1 ~x" << variable_value_mapping(var, val);
    proof_stream() << " >= 1 ;" << endl;

    proof_stream() << "p " << recover_line << " 0 + 0" << endl;
    next_proof_line();

    proof_stream() << "* end of verification" << endl;
}

auto Proof::set_active_stack(const list<pair<VariableID, VariableValue> > * s) -> void
{
    _imp->active_stack = s;
}

auto Proof::asserty() const -> bool
{
    return _imp->asserty;
}

