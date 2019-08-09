/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

using std::cin;
using std::cerr;
using std::cout;
using std::endl;
using std::getline;
using std::optional;
using std::sqrt;
using std::string;
using std::stringstream;
using std::vector;

auto main(int, char *[]) -> int
{
    string line;
    vector<vector<unsigned> > predefined;
    optional<unsigned> size;

    while (getline(cin, line)) {
        if (line.empty())
            break;

        stringstream line_stream{ line };
        unsigned token;
        vector<unsigned> tokens;
        while (line_stream >> token)
            tokens.push_back(token);

        if (! size) {
            size = optional<unsigned>{ sqrt(tokens.size()) };
            if (*size * *size != tokens.size()) {
                cerr << "Bad column count" << endl;
                return EXIT_FAILURE;
            }
        }
        else if (tokens.size() != *size * *size) {
            cerr << "Bad row length" << endl;
            return EXIT_FAILURE;
        }

        predefined.push_back(move(tokens));
    }

    if (predefined.size() != *size * *size) {
        cerr << "Wrong number of rows" << endl;
        return EXIT_FAILURE;
    }

    cout << "intvararray g 2 1 " << (*size * *size) << " 1 " << (*size * *size) << " 1 " << (*size * *size) << endl;
    cout << endl;

    for (unsigned r = 1 ; r <= *size * *size ; ++r) {
        cout << "alldifferent " << (*size * *size);
        for (unsigned c = 1 ; c <= *size * *size ; ++c)
            cout << " g[" << r << "," << c << "]";
        cout << endl;
    }
    cout << endl;

    for (unsigned c = 1 ; c <= *size * *size ; ++c) {
        cout << "alldifferent " << (*size * *size);
        for (unsigned r = 1 ; r <= *size * *size ; ++r)
            cout << " g[" << r << "," << c << "]";
        cout << endl;
    }
    cout << endl;

    for (unsigned c = 1 ; c <= *size ; ++c) {
        for (unsigned r = 1 ; r <= *size ; ++r) {
            cout << "alldifferent " << (*size * *size);
            for (unsigned rr = 1 ; rr <= *size ; ++rr)
                for (unsigned cc = 1 ; cc <= *size ; ++cc)
                    cout << " g[" << ((r - 1) * *size + rr) << "," << ((c - 1) * *size + cc) << "]";
            cout << endl;
        }
    }
    cout << endl;

    for (unsigned r = 1 ; r <= *size * *size ; ++r)
        for (unsigned c = 1 ; c <= *size * *size ; ++c)
            if (0 != predefined.at(r - 1).at(c - 1))
                cout << "equal g[" << r << "," << c << "] " << predefined.at(r - 1).at(c - 1) << endl;

    return EXIT_SUCCESS;
}

