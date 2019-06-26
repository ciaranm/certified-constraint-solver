A Certified Constraint Solver
=============================

This is a very simple constraint programming solver whose main feature is (going to be, at some
point) support for proof logging.  This is a research toy to try out a few new ideas, not serious
code. It is not designed to have good performance, to be powerful, or to be something you would want
to use for application purposes.

Please contact [Ciaran McCreesh](mailto:ciaran.mccreesh@glasgow.ac.uk) with any queries.

Compiling
---------

To build, type 'make'. You will need a C++17 compiler (we test with GCC 7.3, GCC 8.3, and Clang
7.0.1 on Linux, and Xcode 10.2 on Mac OS X) and Boost (we use 1.65.1 or later, built with threads
enabled).

Modelling
---------

An input file looks like this:

```
intvar x1 1 3
intvar x2 1 3
notequal x1 x2
```

Running
-------

```shell session
./certified_constraint_solver models/babysat.model
```

Funding Acknowledgements
------------------------

This work was supported by the Engineering and Physical Sciences Research Council (grant number
EP/P026842/1), although they don't know that.

<!-- vim: set tw=100 spell spelllang=en : -->

