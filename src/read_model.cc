/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "read_model.hh"
#include "model.hh"
#include "constraint.hh"
#include "table_constraint.hh"
#include "not_equals_constraint.hh"
#include "variable.hh"

#include <fstream>
#include <map>
#include <memory>
#include <utility>
#include <vector>

using std::ifstream;
using std::make_shared;
using std::map;
using std::move;
using std::string;
using std::vector;

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
    if (! infile)
        throw InputError{ "Error reading from '" + filename + "'" };

    string word;

    Model model;
    map<string, std::shared_ptr<Table> > tables;

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
        else if (word == "createtable") {
            string name;
            int arity;
            if (! (infile >> name >> arity))
                throw InputError{ "Bad arguments to '" + word + "' command" };
            if (! tables.emplace(name, make_shared<Table>(arity)).second)
                throw InputError{ "Duplicate table '" + name + "'" };
        }
        else if (word == "addtotable") {
            string name;
            if (! (infile >> name))
                throw InputError{ "Bad arguments to '" + word + "' command" };
            auto table = tables.find(name);
            if (table == tables.end())
                throw InputError{ "No table named '" + name + "'" };

            vector<int> tuple;
            for (int i = 0 ; i < table->second->arity ; ++i) {
                int value;
                if (! (infile >> value))
                    throw InputError{ "Bad arguments to '" + word + "' command" };
                tuple.push_back(value);
            }
            table->second->allowed_tuples.push_back(move(tuple));
        }
        else if (word == "table") {
            string name;
            if (! (infile >> name))
                throw InputError{ "Bad arguments to '" + word + "' command" };

            auto table = tables.find(name);
            if (table == tables.end())
                throw InputError{ "No table named '" + name + "'" };

            auto constraint = make_shared<TableConstraint>(table->second);
            for (int i = 0 ; i < table->second->arity ; ++i) {
                string name;
                if (! (infile >> name))
                    throw InputError{ "Bad arguments to '" + word + "' command" };
                constraint->associate_with_variable(name);
            }
            model.constraints.push_back(constraint);
        }
        else {
            throw InputError{ "Unknown command '" + word + "'" };
        }
    }

    return model;
};

