#!/bin/bash

START=2
END=20
MAX_ITERATION=30
readonly PROG="./BIN/GameOfLife"
readonly PATH_SEQ="../GameOfLife_Seq"
readonly SEQUENTIAL="$PATH_SEQ/BIN/GameOfLife"

# This array enable to specify the number of processos to use for a specific board size
# [9]="1,9" means that is the number of row equal 9, then test with 1 processus and then with 9
NB_PROC_SPEC=( [9]="1, 9" [196]="1,4,196" [256]="4,16,64" [289]="1" [324]="4" [400]="100")

if [ ! -d $PATH_SEQ ]; then
    read -r -n 1 -p "You allow me to download the sequential version ? [y|N] ";
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        git clone https://github.com/kaldoran/Conway-s_Game-Of-Life $PATH_SEQ
    else
        echo "This version is needed for test"
        exit 0;
    fi
    echo "The version had been download here : $PATH_SEQ";
fi

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

echo -n "[TEST] Lets make sequential version : "
( cd $PATH_SEQ && make rebuild > /dev/null )
echo -e "DONE";


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
    TO_DO=$( echo "scale=0; sqrt($1)" | bc -l );
    if [ $TO_DO -lt $START ]; then
        echo "$1 need to be > 2";
    else
        START=$TO_DO
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


AllSucces="";
for (( i = $START; i <= $END; i++ )); do
    SIZE=$(( $i * $i ));
    TOTAL_ITERATION=$(( $RANDOM % $MAX_ITERATION + 1 )) 

    DEFAULT_OPT="-s -f ./Script/random.gol -t $TOTAL_ITERATION"  

    if [ -z "${NB_PROC_SPEC[$SIZE]}" ]; then
        NB_PROC=($SIZE);
    else
        NB_PROC=(${NB_PROC_SPEC[$SIZE]//,/ })
    fi

    echo "[TEST] Generate random board : [$SIZE x $SIZE]";
    ./Script/createRandomBoard.sh $SIZE $SIZE true
    echo -e "[TEST] Generation done\n";

    echo -n "[TEST] Lets compute the output of sequential version : "
    $SEQUENTIAL $DEFAULT_OPT > /dev/null
    mv output.gol good.gol
    echo -e "DONE";

    for (( j = 0; j < ${#NB_PROC[@]}; j++ )); do
        PROC=${NB_PROC[$j]};
        echo -e "\n[TEST] ${PROC} processus - ${TOTAL_ITERATION} iteration";

        echo -n "[TEST] Compute Row division    : START .. ";
        mpirun -np $PROC $PROG $DEFAULT_OPT > /dev/null
        DIFF=$(diff output.gol good.gol 2>&1)
        echo -e "END";
        
        echo -n "[TEST] Compare row division output with sequential version       : "
        diffOutput $DIFF;
        AllSucces+=$([ $? -eq 0 ] && echo "." || echo "#")

        echo ""
        echo -n "[TEST] Compute Matrix division : START .. ";
        mpirun -np $PROC $PROG $DEFAULT_OPT -m > /dev/null 
        DIFF=$(diff output.gol good.gol 2>&1)
        echo -e "END";

        echo -n "[TEST] Compare subMatrix division output with sequential version : ";
        diffOutput $DIFF;

        AllSucces+=$([ $? -eq 0 ] && echo "." || echo "#")
    done;
    echo -e "--------------\n";
    sleep 0.4
done

echo "";
echo "[TEST] All Done : $AllSucces";

rm output.gol 2> /dev/null 
rm compare.gol 2> /dev/null
rm ./Script/random.gol 2> /dev/null
