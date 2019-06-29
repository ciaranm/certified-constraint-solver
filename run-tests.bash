#!/bin/bash

set -x

if ! grep '^status = false$' <(./certified_constraint_solver models/nosearchunsat.model ) ; then
    echo "no search unsat test failed" 1>&2
    exit 1
fi

if [[ -x ~/.local/bin/refpy ]] ; then
    if ! grep '^status = false$' <(./certified_constraint_solver models/nosearchunsat.model --write-opb-to models/nosearchunsat.opb --write-ref-to models/nosearchunsat.log ) ; then
        echo "no search unsat test failed" 1>&2
        exit 1
    elif ! ~/.local/bin/refpy models/nosearchunsat.opb models/nosearchunsat.log ; then
        echo "no search unsat refpy verification failed" 1>&2
        exit 1
    fi
    rm -f models/nosearchunsat.opb models/nosearchunsat.log
fi

if ! grep '^status = false$' <(./certified_constraint_solver models/nosearchunsattable.model ) ; then
    echo "no search unsat table test failed" 1>&2
    exit 1
fi

if [[ -x ~/.local/bin/refpy ]] ; then
    if ! grep '^status = false$' <(./certified_constraint_solver models/nosearchunsattable.model --write-opb-to models/nosearchunsattable.opb --write-ref-to models/nosearchunsattable.log ) ; then
        echo "no search unsattable test failed" 1>&2
        exit 1
    elif ! ~/.local/bin/refpy models/nosearchunsattable.opb models/nosearchunsattable.log ; then
        echo "no search unsattable refpy verification failed" 1>&2
        exit 1
    fi
    rm -f models/nosearchunsattable.opb models/nosearchunsattable.log
fi

if ! grep '^status = false$' <(./certified_constraint_solver models/branchonce.model ) ; then
    echo "branch once unsat test failed" 1>&2
    exit 1
fi

if ! grep '^status = false$' <(./certified_constraint_solver models/branchtwice.model ) ; then
    echo "branch twice unsat test failed" 1>&2
    exit 1
fi

if [[ -x ~/.local/bin/refpy ]] ; then
    if ! grep '^status = false$' <(./certified_constraint_solver models/branchonce.model --write-opb-to models/branchonce.opb --write-ref-to models/branchonce.log ) ; then
        echo "branch once unsat test failed" 1>&2
        exit 1
    elif ! ~/.local/bin/refpy models/branchonce.opb models/branchonce.log ; then
        echo "branch once refpy verification failed" 1>&2
        exit 1
    fi
    rm -f models/branchonce.opb models/branchonce.log
fi

if [[ -x ~/.local/bin/refpy ]] ; then
    if ! grep '^status = false$' <(./certified_constraint_solver models/branchtwice.model --write-opb-to models/branchtwice.opb --write-ref-to models/branchtwice.log ) ; then
        echo "branch twice unsat test failed" 1>&2
        exit 1
    elif ! ~/.local/bin/refpy models/branchtwice.opb models/branchtwice.log ; then
        echo "branch twice refpy verification failed" 1>&2
        exit 1
    fi
    rm -f models/branchtwice.opb models/branchtwice.log
fi

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

true

