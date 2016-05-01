#!/bin/bash

START=2
END=50
MAX_ITERATION=30
readonly PROG="./BIN/GameOfLife"

function diffOutput {
    if [ "$1" != "" ]; then
        echo "FAIL";
        echo "$1";
        return 1;
    else 
        echo "SUCCESS";
        return 0;
    fi
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

if [ ! -z "$1" ]; then
    if [ $1 -le $START ]; then
        echo "$1 need to be > 2";
    else
        START=$1
    fi
fi;

if [ ! -z "$2" ]; then
    TO_DO=$( echo "scale=0; sqrt($2)" | bc -l );
    if [ $TO_DO -gt $END ]; then
        echo "The max is 2500";
    else
        END=$TO_DO;
    fi
fi

echo -e "--------------\n";

# This array enable to specify the number of processos to use for a specific board size
# [9]="1,9" means that is the number of row equal 9, then test with 1 processus and then with 9
NB_PROC_SPEC=( [9]="1, 9" [196]="1,4,49" [256]="4,16,32,64" [289]="144" [400]="25,100")

AllSucces="";

for (( i = $START; i <= $END; i++ )); do
    SIZE=$(( $i * $i ));
    TOTAL_ITERATION=$(( $RANDOM % $MAX_ITERATION + 1 )) 

    DEFAULT_OPT="-s -f ./Script/random.gol -t $TOTAL_ITERATION"  

    if [ -z "${NB_PROC_SPEC[$SIZE]}" ]; then
        NB_PROC=($i);
    else
        NB_PROC=(${NB_PROC_SPEC[$SIZE]//,/ })
    fi

    echo "[TEST] Generate random board : [$SIZE x $SIZE]";
    ./Script/createRandomBoard.sh $SIZE $SIZE true
    echo -e "[TEST] Generation done\n";


    for (( j = 0; j < ${#NB_PROC[@]}; j++ )); do
        PROC=${NB_PROC[$j]};
        echo "[TEST] ${PROC} processus - ${TOTAL_ITERATION} iteration";

        echo -n "[TEST] Row division    : START .. ";
        $PROG $DEFAULT_OPT > /dev/null
        mv output.gol compare.gol
        echo -e "END";

        echo -n "[TEST] Matrix division : START .. ";
        $PROG $DEFAULT_OPT -m > /dev/null
        DIFF=$(diff output.gol compare.gol 2>&1)
        echo -e "END";

        echo -en "\n[TEST] Compare the two (2) version : ";
        diffOutput $DIFF;

        AllSucces+=$([ $? -eq 0 ] && echo "." || echo "#")
    done;
    echo -e "--------------\n";
done

echo "";
echo "[TEST] All Done : $AllSucces";


