/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_PROOF_HH
#define CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_PROOF_HH 1

#include "proof-fwd.hh"
#include "variable-fwd.hh"

#include <exception>
#include <iosfwd>
#include <list>
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
        Proof(const std::string & opb_file, const std::string & log_file, bool asserty, bool levels);
        Proof(Proof &&);
        ~Proof();
        auto operator= (Proof &&) -> Proof &;

        Proof(const Proof &) = delete;
        auto operator= (const Proof &) -> Proof & = delete;

        auto write_header() -> void;
        auto load_problem_constraints() -> void;

        auto create_anonymous_extra_variable() -> UnderlyingVariableID;
        auto create_variable_value_mapping(VariableID, VariableValue) -> UnderlyingVariableID;
        auto variable_value_mapping(VariableID, VariableValue) const -> UnderlyingVariableID;
        auto wrote_variable_takes_at_least_one_value(VariableID, ProofLineNumber) -> void;
        auto wrote_variable_takes_at_most_one_value(VariableID, ProofLineNumber) -> void;

        auto model_stream() -> std::ostream &;
        [[ nodiscard ]] auto last_model_line() const -> ProofLineNumber;
        auto next_model_line() -> void;

        auto proof_stream() -> std::ostream &;
        [[ nodiscard ]] auto last_proof_line() const -> ProofLineNumber;
        auto next_proof_line() -> void;

        auto line_for_var_takes_at_least_one_value(VariableID) -> ProofLineNumber;
        auto line_for_var_takes_at_most_one_value(VariableID) -> ProofLineNumber;

        auto enstackinate_guess(VariableID, const std::string &, VariableValue) -> void;
        auto incorrect_guess() -> void;

        auto asserty() const -> bool;
        auto levels() const -> bool;
};

#endif
