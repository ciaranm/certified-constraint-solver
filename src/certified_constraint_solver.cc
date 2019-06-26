/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "model.hh"
#include "read_model.hh"
#include "result.hh"
#include "solve.hh"

#include <boost/program_options.hpp>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <vector>

#include <unistd.h>

namespace po = boost::program_options;

using std::boolalpha;
using std::cerr;
using std::copy;
using std::cout;
using std::endl;
using std::exception;
using std::function;
using std::istreambuf_iterator;
using std::localtime;
using std::make_pair;
using std::make_shared;
using std::make_unique;
using std::ofstream;
using std::ostreambuf_iterator;
using std::put_time;
using std::string;
using std::stringstream;
using std::vector;

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::operator""s;
using std::chrono::seconds;
using std::chrono::steady_clock;
using std::chrono::system_clock;

auto main(int argc, char * argv[]) -> int
{
    try {
        po::options_description display_options{ "Program options" };
        display_options.add_options()
            ("help",                                         "Display help information")
            ("write-opb-to",    po::value<string>(),         "Write the model, re-encoded in OPB format, to this file")
            ;

        po::positional_options_description positional_options;
        positional_options
            .add("model-file", 1)
            ;

        po::options_description all_options{ "All options" };
        all_options.add_options()
            ("model-file", "Specify the model file")
            ;
        all_options.add(display_options);

        po::variables_map options_vars;
        po::store(po::command_line_parser(argc, argv)
                .options(all_options)
                .positional(positional_options)
                .run(), options_vars);
        po::notify(options_vars);

        /* --help? Show a message, and exit. */
        if (options_vars.count("help")) {
            cout << "Usage: " << argv[0] << " [options] model" << endl;
            cout << endl;
            cout << display_options << endl;
            return EXIT_SUCCESS;
        }

        /* No algorithm or no input file specified? Show a message and exit. */
        if (! options_vars.count("model-file")) {
            cout << "Usage: " << argv[0] << " [options] model" << endl;
            return EXIT_FAILURE;
        }

        char hostname_buf[255];
        if (0 == gethostname(hostname_buf, 255))
            cout << "hostname = " << string(hostname_buf) << endl;
        cout << "commandline =";
        for (int i = 0 ; i < argc ; ++i)
            cout << " " << argv[i];
        cout << endl;

        auto started_at = system_clock::to_time_t(system_clock::now());
        cout << "started_at = " << put_time(localtime(&started_at), "%F %T") << endl;

        auto model = read_model(options_vars["model-file"].as<string>());

        cout << "model_file = " << options_vars["model-file"].as<string>() << endl;

        if (options_vars.count("write-opb-to")) {
            stringstream body;
            int nb_vars = 0;
            int nb_constraints = 0;

            model.encode_as_opb(body, nb_vars, nb_constraints);

            ofstream opb{ options_vars["write-opb-to"].as<string>() };
            if (! opb) {
                cerr << "Cannot write OBP model" << endl;
                return EXIT_FAILURE;
            }

            opb << "* #variable= " << nb_vars << " #constraint= " << nb_constraints << endl;
            copy(istreambuf_iterator<char>{ body }, istreambuf_iterator<char>{ },
                    ostreambuf_iterator<char>{ opb });
        }

        /* Start the clock */
        auto start_time = steady_clock::now();

        auto result = solve(model);

        /* Stop the clock. */
        auto overall_time = duration_cast<milliseconds>(steady_clock::now() - start_time);

        cout << "status = ";
        if (! result.solution.empty())
            cout << "true";
        else
            cout << "false";
        cout << endl;

        cout << "nodes = " << result.nodes << endl;
        cout << "runtime = " << overall_time.count() << endl;

        if (! result.solution.empty()) {
            for (auto & [ k, v ] : result.solution)
                cout << k << " = " << v << endl;
        }

        return EXIT_SUCCESS;
    }
    catch (const po::error & e) {
        cerr << "Error: " << e.what() << endl;
        cerr << "Try " << argv[0] << " --help" << endl;
        return EXIT_FAILURE;
    }
    catch (const exception & e) {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }
}

