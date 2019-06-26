/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "read_model.hh"
#include "model.hh"
#include "constraint.hh"
#include "variable.hh"

#include <fstream>
#include <memory>

using std::ifstream;
using std::make_shared;
using std::string;

InputError::InputError(const string & m) noexcept :
    _message("Input file error: " + m)
{
}

auto InputError::what() const noexcept -> const char *
{
    return _message.c_str();
}

auto read_model(const string & filename) -> Model
{
    ifstream infile{ filename };
    string word;

    Model model;

    while (infile >> word) {
        if (word == "intvar") {
            string name;
            int lb, ub;
            if (! (infile >> name >> lb >> ub))
                throw InputError{ "Bad arguments to '" + word + "' command" };
            if (! model.add_variable(name, make_shared<Variable>(lb, ub)))
                throw InputError{ "Duplicate variable '" + name + "'" };
        }
        else if (word == "notequal") {
            string first, second;
            if (! (infile >> first >> second))
                throw InputError{ "Bad arguments to '" + word + "' command" };
            model.constraints.push_back(make_shared<NotEqualConstraint>(first, second));
        }
        else {
            throw InputError{ "Unknown command '" + word + "'" };
        }
    }

    return model;
};

