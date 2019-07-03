/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "model.hh"
#include "variable.hh"
#include "constraint.hh"
#include "result.hh"
#include "proof.hh"
#include "queue_set.hh"

#include <iomanip>
#include <map>
#include <memory>
#include <utility>

using std::endl;
using std::list;
using std::make_shared;
using std::make_unique;
using std::map;
using std::multimap;
using std::optional;
using std::pair;
using std::shared_ptr;
using std::set;
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

struct Model::Imp
{
    map<string, shared_ptr<Variable> > vars;
    shared_ptr<list<shared_ptr<Constraint> > > constraints;
    shared_ptr<multimap<string, shared_ptr<Constraint> > > constraints_associated_with;
};

Model::Model() :
    _imp(make_unique<Model::Imp>())
{
    _imp->constraints = make_shared<list<shared_ptr<Constraint> > >();
    _imp->constraints_associated_with = make_shared<multimap<string, shared_ptr<Constraint> > >();
}

Model::Model(const Model & other) :
    _imp(make_unique<Model::Imp>())
{
    _imp->constraints = other._imp->constraints;
    for (auto & [ n, v ] : other._imp->vars)
        _imp->vars.emplace(n, make_shared<Variable>(*v));
    _imp->constraints_associated_with = other._imp->constraints_associated_with;
}

Model::~Model() = default;

auto Model::add_variable(const string & n, shared_ptr<Variable> v) -> bool
{
    return _imp->vars.emplace(n, v).second;
}

auto Model::get_variable(const string & n) const -> shared_ptr<Variable>
{
    auto r = _imp->vars.find(n);
    if (r == _imp->vars.end())
        throw ModelError{ "No variable named '" + n + "'" };
    else
        return r->second;
}

auto Model::select_branch_variable(string & result_name) const -> shared_ptr<Variable>
{
    shared_ptr<Variable> result;
    for (auto & [ name, v ] : _imp->vars) {
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
    for (auto & [ name, v ] : _imp->vars) {
        if (1 != v->values.size())
            throw ModelError{ "Variable '" + name + "' contains " + to_string(v->values.size()) + " values, but was expecting one" };
        result.solution.emplace(name, to_string(*v->values.begin()));
    }
}

auto Model::start_proof(Proof & proof) const -> void
{
    for (auto & [ name, v ] : _imp->vars)
        v->start_proof(name, proof);

    for (auto & c : *_imp->constraints)
        c->start_proof(*this, proof);

    proof.write_header();
    proof.load_problem_constraints();
    proof.load_variable_axioms();
}

auto Model::add_constraint(shared_ptr<Constraint> c) -> void
{
    _imp->constraints->push_back(c);
    for (auto & v : c->associated_variables())
        _imp->constraints_associated_with->emplace(v, c);
}

auto Model::propagate(optional<Proof> & proof) -> bool
{
    QueueSet<shared_ptr<Constraint> > q;

    // initially we have to revise every constraint
    for (auto & c : *_imp->constraints)
        q.enqueue(c);

    // until we reach a fixed point...
    while (! q.empty()) {
        // get us a constraint to revise
        auto c = q.dequeue();

        set<string> changed_variables;
        if (! c->propagate(*this, proof, changed_variables))
            return false;

        // look at every changed variable, and requeue constraints other than c
        // involving those variables. note that requeueing a constraint that is
        // already on the queue does nothing.
        for (auto & v : changed_variables) {
            auto to_requeue = _imp->constraints_associated_with->equal_range(v);
            for (auto d = to_requeue.first ; d != to_requeue.second ; ++d)
                q.enqueue(d->second);
        }
    }

    return true;
}

