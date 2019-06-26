/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_READ_MODEL_HH
#define GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_READ_MODEL_HH 1

#include "model-fwd.hh"

#include <exception>
#include <string>

class InputError : public std::exception
{
    private:
        std::string _message;

    public:
        InputError(const std::string & message) noexcept;

        virtual auto what() const noexcept -> const char *;
};

auto read_model(const std::string & filename) -> Model;

#endif
