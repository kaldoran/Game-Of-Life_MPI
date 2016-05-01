#!/bin/bash

readonly MIN_ROWS=3
readonly MIN_COLS=3
readonly PROBA_ALIVE_CELL=40

# Needed for print pourcentage value
LC_NUMERIC=C

mkdir -p ../Random_example

MAX_ROWS=150
MAX_COLS=150

# If there is a parameter in first arg
if [ ! -z "$1" ]; then
    if ! [[ "$1" =~ ^[0-9]+$ ]]; then
        echo "$1 need to be an integer";
    else
        MAX_ROWS=$1;
    fi
fi

# If there is a parameter in second arg
if [ ! -z "$2" ]; then
    if ! [[ "$2" =~ ^[0-9]+$ ]]; then
        echo "$2 need to be an integer";
    else
        MAX_COLS=$2;
    fi
fi

# If there is an arg in third arg then use the value as strict value
if [ ! -z "$3" ]; then 
    COLS=$MAX_COLS;   # Fix value in cols and rows
    ROWS=$MAX_ROWS;
else
# Else take random value between min and max
    ROWS=$(( RANDOM % $MAX_ROWS ));
    COLS=$(( RANDOM % $MAX_COLS ));
fi;

# If they are lower than minimum then set them to mini
if [ "$ROWS" -lt "$MIN_ROWS" ]; then ROWS=$MIN_ROWS; fi
if [ "$COLS" -lt "$MIN_COLS" ]; then COLS=$MIN_COLS; fi

if [ -z "$3" ]; then
    echo "$ROWS rows and $COLS column";
fi;

FILE_NAME="./Script/random.gol"
POURCENT_SLICE=$(echo "scale=2;$ROWS/100" | bc -l)

echo "Rows : $ROWS" > "$FILE_NAME";
echo "Cols : $COLS" >> "$FILE_NAME";

for (( i = 1; i <= $ROWS; i++ )); do
    for (( j = 1; j <= $COLS; j++ )); do
		PROBA_CELL_ALIVE=$(( RANDOM % 100 ));
		if [ $PROBA_CELL_ALIVE -lt $PROBA_ALIVE_CELL ]; then 
			echo -n "." >> "$FILE_NAME";
		else
			echo -n "#" >> "$FILE_NAME";
		fi
	done;
	echo "" >> "$FILE_NAME";
    printf '\r%.2f %% done' "$(echo "scale=2; $i/$POURCENT_SLICE" | bc -l )"
done;
echo ""

if [ -z "$3" ]; then
    echo "File generate under the name : $FILE_NAME";
fi
