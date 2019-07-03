/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_TABLE_CONSTRAINT_HH
#define CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_TABLE_CONSTRAINT_HH 1

#include "constraint.hh"

#include <map>
#include <memory>
#include <vector>

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

        virtual auto propagate(Model & model, std::optional<Proof> &, std::set<std::string> &) const -> bool override;

        virtual auto start_proof(const Model &, Proof &) -> void override;

        virtual auto associated_variables() const -> std::set<std::string> override;
};

#endif
