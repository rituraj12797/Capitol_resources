
# we will sue this shell command to run any of our source code in 4 levels 

# non optimized g++ -o some some.cpp 
# O1 Optimized  g++ O1 -o som some.cpp
# O2 Optimized  g++ O2 -o som some.cpp
# O3 Optimized  g++ O3 -o som some.cpp

# $1 name of cpp file 
# $2 optimization level (0 1 2 3)

FILE_NAME=$1
OPT_LEVEL=$2

if [ "$OPT_LEVEL" -eq 0 ]; then 
    # non optimised compilation
    g++ -o "$FILE_NAME" "$FILE_NAME.cpp"
else
    g++ -O$OPT_LEVEL -o "$FILE_NAME" "$FILE_NAME.cpp"

fi

if [ $? -eq 0 ]; then 
#  run the executible formed 
    ./"$FILE_NAME"

# delete the executable after runnning 
    rm -f "$FILE_NAME"
else 
    echo "Compilation failed"
fi