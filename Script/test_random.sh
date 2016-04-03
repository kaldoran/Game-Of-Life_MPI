#!/bin/bash

# Do not use 0 or a value below 0 for MAX_ITERATION, you will get an infinit loop
readonly MAX_THREAD=20
readonly MAX_ITERATION=100
readonly PROG="./BIN/GameOfLife"

# Check the diff between 2 file, is there is some it print them
function diffOutput {
    if [ "$1" != "" ]; then
        echo "FAIL"; 
        echo $DIFF;
        return 1;
    else
        echo "SUCCESS";
        return 0;
    fi;
}

# Rand between 1 and max [ fiven in parameter ]
function randMax {
    rand_max=$(( $RANDOM % $1 ));
    if [ "$rand_max" -eq 0 ]; then rand_max=1; fi

    echo $rand_max;
}

if ! [ -e $PROG ]; then
    echo "[TEST] Compilation : START"
    make rebuild > /dev/null
    
    if [ $? != 0 ]; then
        echo "Compilation error, abort test";
        return -1;
    fi;

    echo -e "[TEST] Compilation : SUCCESS\n";
fi;

# Default value for TOTAL_TEST
TOTAL_TEST=10
if [ ! -z "$1" ]; then 
    if ! [[ "$1" =~ ^[1-9]||1[0-9]+$ ]]; then
        echo "$1 need to be an integer >0";
    else
        TOTAL_TEST=$1;
    fi;
fi;

AllSuccess="";

for (( i = 0; i < $TOTAL_TEST; i++ )); do

    echo -e "\n--------------"
    echo "[TEST] Start creating a random board";
    ./Script/createRandomBoard.sh 150 150
    echo "[TEST] End of creation";

    TOTAL_ITERATION=$(randMax $MAX_ITERATION);
    DEFAULT_OPT="-s -f ./Script/random.gol -t $TOTAL_ITERATION"

    echo -e "\n--------------"
    echo "[TEST] Let's start with $TOTAL_ITERATION iteration [$(( i + 1 ))/$TOTAL_TEST]";
    echo -e "--------------"

    # --------------------------------------------
    # Create the good file to compare with
    # --------------------------------------------

    echo -e "\n[TEST] Sequential : START";
    $PROG $DEFAULT_OPT > /dev/null
    mv output.gol good.gol
    echo "[TEST] Sequential : End";

    # --------------------------------------------

    RANDOM_THREAD=$(randMax $MAX_THREAD);

    echo -e "\n[TEST] $RANDOM_THREAD thread fined grained : START";
    $PROG $DEFAULT_OPT -p $RANDOM_THREAD -g > /dev/null
    DIFF=$(diff good.gol output.gol 2>&1)
    echo -n "[TEST] $RANDOM_THREAD thread fined grained : ";

    diffOutput $DIFF;
    AllSuccess+=$([ $? -eq 0 ] && echo "." || echo "#" );

    # ------------------------------------------

    RANDOM_THREAD=$(randMax $MAX_THREAD);

    echo -e "\n[TEST] $RANDOM_THREAD thread average grained : START";
    $PROG $DEFAULT_OPT -p $RANDOM_THREAD > /dev/null
    DIFF=$(diff good.gol output.gol 2>&1)
    echo -n "[TEST] $RANDOM_THREAD thread average grained : ";

    diffOutput $DIFF;
    AllSuccess+=$([ $? -eq 0 ] && echo "." || echo "#" );

done;

rm good.gol

echo "";
echo "[TEST] All Done : $AllSuccess";
