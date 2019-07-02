A Certified Constraint Solver
=============================

This is a very simple constraint programming solver whose main feature is (going to be, at some
point) support for proof logging.  This is a research toy to try out a few new ideas, not serious
code. It is not designed to have good performance, to be powerful, or to be something you would want
to use for application purposes.

Please contact [Ciaran McCreesh](mailto:ciaran.mccreesh@glasgow.ac.uk) with any queries.

Compiling
---------

To build, type 'make'. You will need a C++17 compiler (we test with GCC 8.3, and Clang 7.0.1 on
Linux, and Xcode 10.2 on Mac OS X) and Boost (we use 1.65.1 or later, built with threads enabled).

Modelling
---------

An input file looks like this:

```
intvar a 1 3
intvar b 1 3
notequal a b
```

The solver also supports all different constraints:

```
intvar a 1 2
intvar b 1 2
intvar c 1 3
alldifferent 3 a b c
```

And table constraints, where a table can be reused multiple times:

```
intvar x1 0 1
intvar x2 0 1
intvar x3 0 1
intvar x4 0 1
createtable oneofthree 3
addtotable oneofthree 1 0 0
addtotable oneofthree 0 1 0
addtotable oneofthree 0 0 1
table oneofthree x1 x2 x3
table oneofthree x2 x3 x4
```

Running
-------

To run the solver:

```shell session
./certified_constraint_solver models/babysat.model
```

To run the solver, and produce a proof of unsat (assuming the model actually is unsat):

```shell session
./certified_constraint_solver --prove models/babyunsat.model
```

This will write an equivalent pseudo-boolean model in OPB format to ``models/babyunsat.opb``, and a
proof log to ``models/babyunsat.log``. These can then be verified using ``refpy``:

```shell session
refpy models/babyunsat.opb models/babyunsat.log
```

You can find refpy at https://github.com/StephanGocht/refpy/ .

Funding Acknowledgements
------------------------

This work was supported by the Engineering and Physical Sciences Research Council (grant number
EP/P026842/1), although they don't know that.

<!-- vim: set tw=100 spell spelllang=en : -->

