#!/bin/bash

set -x

if ! type roundingsat && [[ -x ${HOME}/roundingsat/roundingsat ]] ; then
    export PATH="${HOME}/roundingsat:$PATH"
fi

if ! type refpy && [[ -x ${HOME}/.local/bin/refpy ]] ; then
    export PATH="${HOME}/.local/bin:$PATH"
fi

if ! grep '^status = false$' <(./certified_constraint_solver models/nosearchunsat.model ) ; then
    echo "no search unsat test failed" 1>&2
    exit 1
fi

if ! type roundingsat ; then
    echo "couldn't find roundingsat executable" 1>&2
    exit 1
fi

if ! type refpy ; then
    echo "couldn't find refpy executable" 1>&2
    exit 1
fi

if ! grep '^status = false$' <(./certified_constraint_solver models/nosearchunsat.model --prove ) ; then
    echo "no search unsat test failed" 1>&2
    exit 1
elif ! refpy models/nosearchunsat.opb models/nosearchunsat.log ; then
    echo "no search unsat refpy verification failed" 1>&2
    exit 1
fi
rm -f models/nosearchunsat.opb models/nosearchunsat.log

if ! grep '^status = false$' <(./certified_constraint_solver models/equal.model ) ; then
    echo "equal test failed" 1>&2
    exit 1
fi

if ! grep '^status = false$' <(./certified_constraint_solver models/equal.model --prove ) ; then
    echo "equal test failed" 1>&2
    exit 1
elif ! refpy models/equal.opb models/equal.log ; then
    echo "equal refpy verification failed" 1>&2
    exit 1
fi
rm -f models/equal.opb models/equal.log

if ! grep '^status = false$' <(./certified_constraint_solver models/twoequals.model ) ; then
    echo "two equals test failed" 1>&2
    exit 1
fi

if [[ -x refpy ]] ; then
    if ! grep '^status = false$' <(./certified_constraint_solver models/twoequals.model --prove ) ; then
        echo "two equals test failed" 1>&2
        exit 1
    elif ! refpy models/twoequals.opb models/twoequals.log ; then
        echo "two equals refpy verification failed" 1>&2
        exit 1
    fi
    rm -f models/twoequals.opb models/twoequals.log
fi

if ! grep '^status = false$' <(./certified_constraint_solver models/nosearchunsattable.model ) ; then
    echo "no search unsat table test failed" 1>&2
    exit 1
fi

if ! grep '^status = false$' <(./certified_constraint_solver models/nosearchunsattable.model --prove ) ; then
    echo "no search unsattable test failed" 1>&2
    exit 1
elif ! refpy models/nosearchunsattable.opb models/nosearchunsattable.log ; then
    echo "no search unsattable refpy verification failed" 1>&2
    exit 1
fi
rm -f models/nosearchunsattable.opb models/nosearchunsattable.log

if ! grep '^status = false$' <(./certified_constraint_solver models/branchonce.model ) ; then
    echo "branch once unsat test failed" 1>&2
    exit 1
fi

if ! grep '^status = false$' <(./certified_constraint_solver models/branchtwice.model ) ; then
    echo "branch twice unsat test failed" 1>&2
    exit 1
fi

if ! grep '^status = false$' <(./certified_constraint_solver models/branchonce.model --prove ) ; then
    echo "branch once unsat test failed" 1>&2
    exit 1
elif ! refpy models/branchonce.opb models/branchonce.log ; then
    echo "branch once refpy verification failed" 1>&2
    exit 1
fi
rm -f models/branchonce.opb models/branchonce.log

if ! grep '^status = false$' <(./certified_constraint_solver models/branchtwice.model --prove ) ; then
    echo "branch twice unsat test failed" 1>&2
    exit 1
elif ! refpy models/branchtwice.opb models/branchtwice.log ; then
    echo "branch twice refpy verification failed" 1>&2
    exit 1
fi
rm -f models/branchtwice.opb models/branchtwice.log

if ! grep '^status = true$' <(./certified_constraint_solver models/babysat.model ) ; then
    echo "baby sat test failed" 1>&2
    exit 1
fi

if ! grep '^status = false$' <(./certified_constraint_solver models/babyunsat.model ) ; then
    echo "baby unsat test failed" 1>&2
    exit 1
fi

if ! grep '^status = true$' <(./certified_constraint_solver models/babytable.model ) ; then
    echo "baby table test failed" 1>&2
    exit 1
fi

if ! grep '^status = true$' <(./certified_constraint_solver models/reusetable.model ) ; then
    echo "reuse table test failed" 1>&2
    exit 1
fi

if ! grep '^status = false$' <(./certified_constraint_solver models/alldiff.model --prove ) ; then
    echo "alldiff unsat test failed" 1>&2
    exit 1
elif ! refpy models/alldiff.opb models/alldiff.log ; then
    echo "alldiff refpy verification failed" 1>&2
    exit 1
fi
rm -f models/alldiff.opb models/alldiff.log

if ! grep '^status = false$' <(./certified_constraint_solver models/littlesip.model --prove ) ; then
    echo "littlesip unsat test failed" 1>&2
    exit 1
elif ! refpy models/littlesip.opb models/littlesip.log ; then
    echo "littlesip refpy verification failed" 1>&2
    exit 1
fi
rm -f models/littlesip.opb models/littlesip.log

if ! grep '^status = true$' <(./certified_constraint_solver models/array.model ) ; then
    echo "array test failed" 1>&2
    exit 1
fi

if ! grep '^status = true$' <(./certified_constraint_solver models/sudoku.model ) ; then
    echo "sudoku test failed" 1>&2
    exit 1
fi

if ! grep '^status = true$' <(./certified_constraint_solver models/hallunit.model --prove --asserty ) ; then
    echo "hallunit test failed" 1>&2
    exit 1
elif ! refpy models/hallunit.opb models/hallunit.log ; then
    echo "hallunit refpy verification failed" 1>&2
    exit 1
fi
rm -f models/hallunit.opb models/hallunit.log

if ! grep '^status = true$' <(./certified_constraint_solver models/hall.model --prove --asserty ) ; then
    echo "hall test failed" 1>&2
    exit 1
elif ! refpy models/hall.opb models/hall.log ; then
    echo "hall refpy verification failed" 1>&2
    exit 1
fi
rm -f models/hall.opb models/hall.log

if ! grep '^status = true$' <(./certified_constraint_solver models/hall2.model --prove --asserty ) ; then
    echo "hall2 test failed" 1>&2
    exit 1
elif ! refpy models/hall2.opb models/hall2.log ; then
    echo "hall2 refpy verification failed" 1>&2
    exit 1
fi
rm -f models/hall2.opb models/hall2.log

if ! grep '^status = true$' <(./certified_constraint_solver models/toobigforahall.model --prove --asserty ) ; then
    echo "toobigforahall test failed" 1>&2
    exit 1
elif ! refpy models/toobigforahall.opb models/toobigforahall.log ; then
    echo "toobigforahall refpy verification failed" 1>&2
    exit 1
fi
rm -f models/toobigforahall.opb models/toobigforahall.log

if ! grep '^status = false$' <(./certified_constraint_solver models/hardsudoku.model --prove --asserty ) ; then
    echo "hardsudoku test failed" 1>&2
    exit 1
elif ! refpy models/hardsudoku.opb models/hardsudoku.log ; then
    echo "hardsudoku refpy verification failed" 1>&2
    exit 1
fi
rm -f models/hardsudoku.opb models/hardsudoku.log

true

