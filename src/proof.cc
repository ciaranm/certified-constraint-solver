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

    int model_constraints_line = 0;
    int number_of_variables = 0;
    int proof_line = 0;
    int variable_axioms_start = 0;

    map<string, int> variable_takes_at_least_one_value, variable_takes_at_most_one_value;
    list<map<pair<string, int>, int> > var_not_equal_value;
    map<pair<string, int>, int> vv_mapping;
};

Proof::Proof(const string & opb, const string & log)
{
    _imp = make_unique<Proof::Imp>();

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

    _imp->opb_file << "* #variable = " << _imp->number_of_variables << " #constraint = " << _imp->model_constraints_line << endl;
     copy(istreambuf_iterator<char>{ _imp->opb_body_file }, istreambuf_iterator<char>{}, ostreambuf_iterator<char>{ _imp->opb_file });
}

auto Proof::load_problem_constraints() -> void
{
    proof_stream() << "f " << _imp->model_constraints_line << " 0" << endl;
    _imp->proof_line += _imp->model_constraints_line;
}

auto Proof::load_variable_axioms() -> void
{
    proof_stream() << "l " << _imp->number_of_variables << " 0" << endl;
    _imp->variable_axioms_start = _imp->proof_line;
    _imp->proof_line += (_imp->number_of_variables * 2);
}

auto Proof::create_variable_value_mapping(const string & n, int v) -> int
{
    _imp->vv_mapping.emplace(pair{ n, v }, ++_imp->number_of_variables);
    return _imp->number_of_variables;
}

auto Proof::variable_value_mapping(const std::string & n, int v) const -> int
{
    return _imp->vv_mapping.find(pair{ n, v })->second;
}

auto Proof::wrote_variable_takes_at_least_one_value(const string & n, int v) -> void
{
    _imp->variable_takes_at_least_one_value.emplace(n, v);
}

auto Proof::wrote_variable_takes_at_most_one_value(const string & n, int v) -> void
{
    _imp->variable_takes_at_most_one_value.emplace(n, v);
}

auto Proof::model_stream() -> std::ostream &
{
    return _imp->opb_body_file;
}

auto Proof::last_model_line() const -> int
{
    return _imp->model_constraints_line;
}

auto Proof::next_model_line() -> void
{
    ++_imp->model_constraints_line;
}

auto Proof::proof_stream() -> std::ostream &
{
    return _imp->log_file;
}

auto Proof::last_proof_line() const -> int
{
    return _imp->proof_line;
}

auto Proof::next_proof_line() -> void
{
    ++_imp->proof_line;
}

auto Proof::line_for_var_not_equal_value(const std::string & v, int n) -> int
{
    for (auto r = _imp->var_not_equal_value.rbegin() ; r != _imp->var_not_equal_value.rend() ; ++r) {
        auto w = r->find(pair{ v, n });
        if (w != r->end())
            return w->second;
    }

    throw ProofError{ "Don't have a stored reason for " + v + " != " + to_string(n) };
}

auto Proof::proved_var_not_equal_value(const std::string & v, int n, int l) -> void
{
    _imp->var_not_equal_value.back().emplace(pair{ v, n }, l);
}

auto Proof::guessing_var_not_equal_value(const std::string & v, int n) -> void
{
    _imp->var_not_equal_value.back().emplace(pair{ v, n }, line_for_var_takes_at_most_one_value(v));
}

auto Proof::line_for_var_takes_at_least_one_value(const std::string & n) -> int
{
    return _imp->variable_takes_at_least_one_value.find(n)->second;
}

auto Proof::line_for_var_takes_at_most_one_value(const std::string & n) -> int
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

auto Proof::line_for_var_val_is_at_most_one(const std::string & n, int v) const -> int
{
    return _imp->variable_axioms_start + 2 * _imp->vv_mapping.find(pair{ n, v })->second;
}

