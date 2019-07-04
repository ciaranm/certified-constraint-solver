/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_PROOF_HH
#define CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_PROOF_HH 1

#include "variable-fwd.hh"

#include <exception>
#include <iosfwd>
#include <memory>
#include <string>

class ProofError : public std::exception
{
    private:
        std::string _message;

    public:
        ProofError(const std::string & message) noexcept;

        virtual auto what() const noexcept -> const char *;
};

class Proof
{
    private:
        struct Imp;
        std::unique_ptr<Imp> _imp;

    public:
        Proof(const std::string & opb_file, const std::string & log_file);
        Proof(Proof &&);
        ~Proof();
        auto operator= (Proof &&) -> Proof &;

        Proof(const Proof &) = delete;
        auto operator= (const Proof &) -> Proof & = delete;

        auto write_header() -> void;
        auto load_problem_constraints() -> void;
        auto load_variable_axioms() -> void;

        auto create_anonymous_extra_variable() -> int;
        auto create_variable_value_mapping(VariableID, VariableValue) -> int;
        auto variable_value_mapping(VariableID, VariableValue) const -> int;
        auto wrote_variable_takes_at_least_one_value(VariableID, int) -> void;
        auto wrote_variable_takes_at_most_one_value(VariableID, int) -> void;

        auto model_stream() -> std::ostream &;
        auto last_model_line() const -> int;
        auto next_model_line() -> void;

        auto push_context() -> void;
        auto pop_context() -> void;

        auto proof_stream() -> std::ostream &;
        auto last_proof_line() const -> int;
        auto next_proof_line() -> void;

        auto line_for_var_not_equal_value(VariableID, VariableValue) -> int;
        auto proved_var_not_equal_value(VariableID, VariableValue, int) -> void;

        auto line_for_var_takes_at_least_one_value(VariableID) -> int;
        auto line_for_var_takes_at_most_one_value(VariableID) -> int;

        auto line_for_var_val_is_at_most_one(VariableID, VariableValue) const -> int;
        auto line_for_var_val_is_at_least_zero(VariableID, VariableValue) const -> int;
};

#endif
