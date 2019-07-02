/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_CONSTRAINT_HH
#define GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_CONSTRAINT_HH 1

#include "constraint-fwd.hh"
#include "model-fwd.hh"
#include "proof-fwd.hh"

#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

struct Constraint
{
    Constraint() = default;
    virtual ~Constraint() = 0;

    Constraint(const Constraint &) = delete;
    Constraint & operator= (const Constraint &) = delete;

    [[ nodiscard ]] virtual auto propagate(Model & model, std::optional<Proof> &) const -> PropagationResult = 0;

    virtual auto start_proof(const Model &, Proof &) -> void = 0;
};

class NotEqualConstraint : public Constraint
{
    private:
        std::string _first, _second;
        std::map<int, int> _constraint_number;

    public:
        NotEqualConstraint(const std::string &, const std::string &);
        virtual ~NotEqualConstraint() override;

        virtual auto propagate(Model & model, std::optional<Proof> &) const -> PropagationResult override;

        virtual auto start_proof(const Model &, Proof &) -> void override;
};

struct Table
{
    explicit Table(int a);

    int arity;
    std::vector<std::vector<int> > allowed_tuples;
};

class TableConstraint : public Constraint
{
    private:
        int _arity;
        std::vector<std::string> _vars;
        std::shared_ptr<const Table> _table;
        std::map<std::vector<int>, int> _var_for_tuple;
        std::map<int, int> _constraint_for_tuple;
        int _must_have_one_constraint;

    public:
        explicit TableConstraint(const std::shared_ptr<const Table> &);
        virtual ~TableConstraint() override;

        auto associate_with_variable(const std::string &) -> void;

        virtual auto propagate(Model & model, std::optional<Proof> &) const -> PropagationResult override;

        virtual auto start_proof(const Model &, Proof &) -> void override;
};

#endif
