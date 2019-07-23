/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "config.hh"
#include "model.hh"
#include "proof.hh"
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
#include <memory>
#include <optional>
#include <vector>

#include <unistd.h>

#if defined(STD_FS_IS_EXPERIMENTAL)
#  include <experimental/filesystem>
#elif defined(STD_FS_IS_STD)
#  include <filesystem>
#elif defined(STD_FS_IS_BOOST)
#  include <boost/filesystem.hpp>
#endif

#if defined(STD_FS_IS_EXPERIMENTAL)
using std::experimental::filesystem::path;
#elif defined(STD_FS_IS_STD)
using std::filesystem::path;
#elif defined(STD_FS_IS_BOOST)
using boost::filesystem::path;
#endif

namespace po = boost::program_options;

using std::boolalpha;
using std::cerr;
using std::copy;
using std::cout;
using std::endl;
using std::exception;
using std::localtime;
using std::make_optional;
using std::optional;
using std::put_time;
using std::string;
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
            ("prove",                                        "Produce an unsat proof")
            ("write-opb-to",    po::value<string>(),         "Specify the proof model file (default: input file with .obp extension)")
            ("write-ref-to",    po::value<string>(),         "Specify the proof log file (default: input file with .log extension)")
            ("asserty",                                      "Generate lots of extra assertions in the proof")
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

        /* Start the clock */
        auto start_time = steady_clock::now();

        optional<Proof> proof;
        if (options_vars.count("prove")) {
            path opb_file;
            if (options_vars.count("write-opb-to"))
                opb_file = options_vars["write-opb-to"].as<string>();
            else {
                opb_file = options_vars["model-file"].as<string>();
                opb_file = opb_file.replace_extension(".opb");
            }

            path log_file;
            if (options_vars.count("write-ref-to"))
                log_file = options_vars["write-ref-to"].as<string>();
            else {
                log_file = options_vars["model-file"].as<string>();
                log_file = log_file.replace_extension(".log");
            }

            bool asserty = options_vars.count("asserty");

#if defined(STD_FS_IS_BOOST)
            proof = make_optional<Proof>(opb_file.string(), log_file.string(), asserty);
#else
            proof = make_optional<Proof>(opb_file, log_file, asserty);
#endif
        }

        auto result = solve(model, proof);

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

