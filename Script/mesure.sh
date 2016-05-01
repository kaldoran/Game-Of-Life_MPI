#!/bin/bash

MAX_ITERATION=30;
readonly PROG="./BIN/GameOfLife"
readonly PATH_SEQ="../GameOfLife_Seq"
readonly SEQUENTIAL="$PATH_SEQ/BIN/GameOfLife"

if [ ! -d $PATH_SEQ ]; then
    read -r -n 1 -p "You allow me to download the sequential version ? [y|N] ";
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        git clone https://github.com/kaldoran/Conway-s_Game-Of-Life $PATH_SEQ
    fi
fi

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

echo -e "--------------\n";


# Number of proc if the side size is X
# [X]="nbProc1,nbProc2" [Y]="nbProc3"
# 
# For example : [9]="3,9"
# => If the slice if 9 wide, then test with 3 proc then with 9

# If you use odd number of iteration those file WON'T work [cause all of them got a period of 2 

AllSucces="";
mkdir -p Time
rm ./Time/* 2> /dev/null

# Valeurs tiré de la table de 1, 4, 16, 36 et 64
GRID_SIZE=(64 192 320 520 1206)

# Valeur tiré de la table Y x Y 
# Car il faut Y Processus sur l'horizontal & Y sur la verticale
TOTAL_PROC=( 1 4 16 64)

for (( i = 0; i <= ${#GRID_SIZE[@]}; i++)); do
    TOTAL_ITERATION=$(( $(( $RANDOM % $MAX_ITERATION )) + 1))
    DEFAULT_OPT="-f ./Script/random.gol";

    SIZE=${GRID_SIZE[$i]}
    echo "[TEST] Start creating a random board [$SIZE x $SIZE]"
    ./Script/createRandomBoard.sh $SIZE $SIZE true
    echo "[TEST] End of creation"
    echo "-----------------------"
   
    FILE="./Time/size_${SIZE}"

    for (( j = 0; j < ${#TOTAL_PROC[@]}; j++ )); do
        NB_PROC=${TOTAL_PROC[$j]}
        echo -n "${NB_PROC} " >> "${FILE}.dat"

        echo -n "[TEST] Sequential timer      : START .. ";
        cp ./Script/random.gol "$PATH_SEQ/Script/random.gol"
        $SEQUENTIAL $DEFAULT_OPT -t $(( $TOTAL_ITERATION + 1 )) | awk '{printf "%f ", $3}' >> "${FILE}.dat"
        echo -e "END";

        echo "[TEST] ${NB_PROC} Processor - $TOTAL_ITERATION Iteration"
        echo -ne "\t - Row division timer    : START .. "
        mpirun -np ${NB_PROC} $PROG $DEFAULT_OPT -t $TOTAL_ITERATION | awk '{printf "%f ", $3}' >> "${FILE}.dat"
        echo -e "END";

        echo -ne "\t - Matrix division timer : START .. "
        mpirun -np ${NB_PROC} $PROG $DEFAULT_OPT -m -t $TOTAL_ITERATION | awk '{printf "%f ", $3}' >> "${FILE}.dat"
        echo -e "END\n";

        echo "" >> "${FILE}.dat"
    done

    # First graph is the time graph
	gnuplot <<- EOF
		reset 
		set style line 1 lc rgb "blue" lt 1 lw 2 pt 7
		set style line 2 lc rgb "orange" lt 1 lw 2 pt 7
		set style line 3 lc rgb "red" lw 2 pt 7
        set style line 4 lc rgb "green" lt 1 lw 2
        set logscale x
		set title "Temps d'execution en fonction du nombre de processus pour n = $SIZE"
		set xlabel "Nombre de threads"
		set ylabel "Temps d'execution (en sec)"
		set term png
        set yrange [*<0:0.16<*]
		set output "${FILE}_time.png"
		plot "${FILE}.dat" using 1:2:xtic(1) title "Sequential" with linespoints ls 1, \
			 "${FILE}.dat" using 1:3:xtic(1) title "Proc Rows" with linespoints ls 2,\
			 "${FILE}.dat" using 1:4:xtic(1) title "Proc SubMatrix" with linespoints ls 3
	EOF
 
    # And here is the absolute acceleration graph
    gnuplot <<- EOF
		reset 
		set style line 1 lc rgb "blue" lt 1 lw 2 pt 7
		set style line 2 lc rgb "orange" lt 1 lw 2 pt 7
		set style line 3 lc rgb "red" lw 2 pt 7
        set style line 4 lc rgb "green" lt 1 lw 2
        set logscale x
		set title "Acceleration absolue en fonction du nombre de processus pour n = $SIZE"
		set xlabel "Nombre de threads"
		set ylabel "Acceleration absolue"
		set term png
        set yrange [*<0:0.16<*]
		set output "${FILE}_acceleration.png"
		plot "${FILE}.dat" using 1:(\$2/\$2):xtic(1) title "Sequential" with linespoints ls 1, \
			 "${FILE}.dat" using 1:(\$2/\$3):xtic(1) title "Proc Rows" with linespoints ls 2,\
			 "${FILE}.dat" using 1:(\$2/\$4):xtic(1) title "Proc SubMatrix" with linespoints ls 3
	EOF
done

