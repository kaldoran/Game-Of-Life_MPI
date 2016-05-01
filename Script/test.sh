#!/bin/bash

START=2
END=50
MAX_ITERATION=100;
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

echo -e "--------------\n";

if [ ! -z "$1" ]; then
    if [ $1 -lt $START ]; then
        echo "'$1' need to be an integer > 1";
        exit;
    else
        START=$1;
    fi
fi

if [ ! -z "$2" ]; then
    to=$(echo "scale=0;sqrt( $2 )" | bc -l);
    if [[ $to -gt $END ]]; then
        echo "$2 need to be an integer > 0";
        exit;
    else
        echo "Be carefull, with integer (around) > 2000, the program may not end [Max 2500]";
        echo ""; 
        END=$to; 
    fi
fi

if [ $END -gt 50 ]; then
    END=50
fi


# Number of proc if the side size is X
# [X]="nbProc1,nbProc2" [Y]="nbProc3"
# 
# For example : [9]="3,9"
# => If the slice if 9 wide, then test with 3 proc then with 9
declare -A nbProcSpec=( [100]="4" );

# If you use odd number of iteration those file WON'T work [cause all of them got a period of 2 

AllSucces="";
for (( i = $START; i <= $END; i++)); do
    TOTAL_ITERATION=$(( $RANDOM % $MAX_ITERATION + 1))
    DEFAULT_OPT="-s -f ./Script/random.gol -t $TOTAL_ITERATION";

    SIZE=$(( $i * $i ));
    if ! [[ -z "${nbProcSpec[$SIZE]}" ]]; then
        NBPROC=(${nbProcSpec[$SIZE]//,/ });
    else
        NBPROC=$(($i * $i));
    fi
    
    echo "[TEST] Start creating a random board [$SIZE x $SIZE]"
    ./Script/createRandomBoard.sh $SIZE $SIZE true
    echo "[TEST] End of creation"
    echo "-----------------------"
    
    for (( j = 0; j < ${#NBPROC[@]}; j++ )); do
        echo "[TEST] ${NBPROC[$j]} Processor - $TOTAL_ITERATION Iteration"
        echo -n "[TEST] Row division test    : START .. "
        mpirun -np ${NBPROC[$j]} $PROG $DEFAULT_OPT > /dev/null
        mv output.gol compare.gol
        echo -e "END";

        echo -n "[TEST] Matrix division test : START .. "
        mpirun -np ${NBPROC[$j]} $PROG $DEFAULT_OPT -m > /dev/null
        DIFF=$(diff output.gol compare.gol 2>&1)
        echo -e "END\n";

        echo -n "[TEST] Compare output of the previous test : "
        diffOutput $DIFF;
        AllSucces+=$([ $? -eq 0 ] && echo "." || echo "#")
        
        echo ""
    done
done
rm compare.gol

echo "";
echo "[TEST] All Done : $AllSucces";

