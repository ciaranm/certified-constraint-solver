/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_MODEL_HH
#define GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_MODEL_HH 1

#include "model-fwd.hh"
#include "variable-fwd.hh"
#include "constraint-fwd.hh"
#include "result-fwd.hh"
#include "refutation_log.hh"

#include <exception>
#include <list>
#include <map>
#include <memory>
#include <string>

class ModelError : public std::exception
{
    private:
        std::string _message;

    public:
        ModelError(const std::string & message) noexcept;

        virtual auto what() const noexcept -> const char *;
};

class Model
{
    private:
        std::map<std::string, std::shared_ptr<Variable> > _vars;

    public:
        Model();
        Model(const Model &);
        ~Model();

        [[ nodiscard ]] auto add_variable(const std::string &, std::shared_ptr<Variable>) -> bool;

        auto get_variable(const std::string &) const -> std::shared_ptr<Variable>;
        auto select_branch_variable(std::string &) const -> std::shared_ptr<Variable>;

        auto save_result(Result &) const -> void;

        std::list<std::shared_ptr<Constraint> > constraints;

        auto encode_as_opb(std::ostream &, int & nb_vars, int & nb_constraints, RefutationLog & log) const -> void;
        auto write_ref_header(RefutationLog & log, int nb_vars, int nb_constraints) const -> void;
};

#endif
