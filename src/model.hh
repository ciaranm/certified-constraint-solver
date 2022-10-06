/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_MODEL_HH
#define GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_MODEL_HH 1

#include "model-fwd.hh"
#include "variable-fwd.hh"
#include "constraint-fwd.hh"
#include "result-fwd.hh"
#include "proof-fwd.hh"

#include <exception>
#include <list>
#include <map>
#include <memory>
#include <optional>
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
        struct Imp;
        std::unique_ptr<Imp> _imp;

    public:
        Model();
        Model(const Model &);
        ~Model();

        [[ nodiscard ]] auto add_variable(const std::string &, VariableID, std::shared_ptr<Variable>) -> bool;
        auto add_constraint(std::shared_ptr<Constraint>) -> void;

        auto get_variable(VariableID) const -> std::shared_ptr<Variable>;
        auto select_branch_variable() const -> std::pair<VariableID, std::shared_ptr<Variable> >;
        auto original_name(VariableID) const -> std::string;

        auto save_result(Result &) const -> void;

        auto start_proof(Proof &) const -> void;

        [[ nodiscard ]] auto propagate(std::optional<Proof> &) -> bool;
};

#endif
