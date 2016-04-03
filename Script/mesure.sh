#!/bin/bash

PROG="./BIN/GameOfLife"

if ! [ -e $PROG ]; then
    echo "[TEST] Compilation : START"
    make rebuild > /dev/null
    
    if [ $? != 0 ]; then
        echo "Compilation error, abort test";
        return -1;
    fi;

    echo -e "[TEST] Compilation : SUCCESS\n";
fi;

grid_size=(10 50 100 500 1000 2000)
number_thread=(1 2 4 8 16 32 64 128)

mkdir -p Time

for (( i = 0; i < ${#grid_size[@]}; i++ )); do
	
    # Generate a grid of ${grid_size[$i]} size
    size=${grid_size[$i]};
	echo -e "\n[TEST] Start creating a random board [$size x $size]";
	./Script/createRandomBoard.sh $size $size strict
	echo -e "[TEST] End of creation\n";

	FILE="./Time/size_${size}"
    rm "./Time/*.dat" 2> /dev/null

    # Lets go generate the final grid for all threads number
    for (( j = 0; j < ${#number_thread[@]}; j++ )); do
		thread=${number_thread[$j]};

		echo "Completing data into file for $thread threads";
		echo -n "$thread " >> "${FILE}.dat"
        echo -e "\t- Sequential";
        $PROG -f ./Script/random.gol | awk '{printf "%f ", $3}' >> "${FILE}.dat"
        echo -e "\t- Thread avegare grained";
        $PROG -p $thread -f ./Script/random.gol |  awk '{printf "%f ", $3}' >> "${FILE}.dat"
        echo -e "\t- Thread fined grained"
        $PROG -p $thread -g -f ./Script/random.gol |  awk '{printf "%f ", $3}' >> "${FILE}.dat"

		echo "" >> "${FILE}.dat"
        echo "";
    done
    
    # First graph is the time graph
	gnuplot <<- EOF
		reset 
		set style line 1 lc rgb "blue" lt 1 lw 2 pt 7
		set style line 2 lc rgb "orange" lt 1 lw 2 pt 7
		set style line 3 lc rgb "red" lw 2 pt 7
        set style line 4 lc rgb "green" lt 1 lw 2
        set logscale x
		set title "Temps d'execution en fonction du nombre de threads pour n = $size"
		set xlabel "Nombre de threads"
		set ylabel "Temps d'execution (en sec)"
		set term png
        set yrange [*<0:0.16<*]
		set output "${FILE}_time.png"
		plot "${FILE}.dat" using 1:2:xtic(1) title "Sequential" with linespoints ls 1, \
			 "${FILE}.dat" using 1:3:xtic(1) title "Thread average" with linespoints ls 2,\
			 "${FILE}.dat" using 1:4:xtic(1) title "Thread fined" with linespoints ls 3
	EOF
 
    # And here is the absolute acceleration graph
    gnuplot <<- EOF
		reset 
		set style line 1 lc rgb "blue" lt 1 lw 2 pt 7
		set style line 2 lc rgb "orange" lt 1 lw 2 pt 7
		set style line 3 lc rgb "red" lw 2 pt 7
        set style line 4 lc rgb "green" lt 1 lw 2
        set logscale x
		set title "Acceleration absolue en fonction du nombre de threads pour n = $size"
		set xlabel "Nombre de threads"
		set ylabel "Acceleration absolue"
		set term png
        set yrange [*<0:0.16<*]
		set output "${FILE}_acceleration.png"
		plot "${FILE}.dat" using 1:(\$2/\$2):xtic(1) title "Sequential" with linespoints ls 1, \
			 "${FILE}.dat" using 1:(\$2/\$3):xtic(1) title "Thread average" with linespoints ls 2,\
			 "${FILE}.dat" using 1:(\$2/\$4):xtic(1) title "Thread fined" with linespoints ls 3
	EOF
done
