#!/bin/bash

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

AllSucces="";

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

input=("block.gol" "blinker.gol" "beacon.gol" "empty.gol" "toad.gol")

# If you use odd number of iteration those file WON'T work [cause all of them got a period of 2 
output=("block.gol" "blinker.gol" "beacon.gol" "empty.gol" "toad.gol")

thread=(1 2 4 8 16)

for (( file = 0; file < ${#input[@]}; file++ )); do

    TOTAL_ITERATION=$(( $RANDOM % 100 )) 
    if [ $(( $TOTAL_ITERATION % 2 )) -ne 0 ]; then ((--TOTAL_ITERATION)); fi
    if [ $TOTAL_ITERATION -eq 0 ]; then TOTAL_ITERATION=2; fi # We don't want 0 [cause it's infinit] 

    DEFAULT_OPT="-s -f ./Famous_example/${input[$file]} -t $TOTAL_ITERATION"
    DIFF_FILE="./Famous_example/${output[$file]}";
    
    echo -e "--------------";
    echo "Lets start for file : ${input[$file]} - Use : $TOTAL_ITERATION interation";
    echo -e "--------------";
    
    echo -e "[TEST] Sequential : START";
    
    $PROG $DEFAULT_OPT > /dev/null
    DIFF=$(diff output.gol $DIFF_FILE 2>&1)
       
    echo -n "[TEST] Sequential : ";

    diffOutput $DIFF;
    AllSucces+=$([ $? -eq 0 ] && echo "." || echo "#")

    echo -e "\n--------------";
    echo "Multi thread fined grained : ";
    echo -e "--------------\n";

    for (( j = 0; j < ${#thread[@]}; j++ )); do
        
        echo -e "[TEST] ${thread[$j]} thread fined grained : START";

        $PROG $DEFAULT_OPT -p ${thread[$j]} -g > /dev/null
        DIFF=$(diff output.gol $DIFF_FILE 2>&1)

        echo -n "[TEST] ${thread[$j]} thread fined grained : ";

        diffOutput $DIFF;
        AllSucces+=$([ $? -eq 0 ] && echo "." || echo "#")
        
        echo "";
    done;

    echo -e "--------------";
    echo "Multi thread average grained: ";
    echo -e "--------------\n";

    for (( j = 0; j < ${#thread[@]}; j++ )); do
        echo -e "[TEST] ${thread[$j]} thread average grained : START";

        $PROG $DEFAULT_OPT -p ${thread[$j]} > /dev/null
        DIFF=$(diff output.gol $DIFF_FILE 2>&1)
        
        echo -n "[TEST] ${thread[$j]} thread average grained : ";
        
        diffOutput $DIFF;
        AllSucces+=$([ "$DIFF" == "" ] && echo "." || echo "#")

        echo "";
    done;
done

echo "";
echo "[TEST] All Done : $AllSucces";

rm output.gol
