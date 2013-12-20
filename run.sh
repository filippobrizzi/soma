#!/bin/bash
CXX=/usr/bin/g++

SRC_FILE=$1
PATH=$(dirname $SRC_FILE)

TMP_FILE=${SRC_FILE:0:-4}
PROFILE_FILE=$TMP_FILE"_profile.cpp"
TRANSFORMED_FILE=$TMP_FILE"_transformed.cpp"
XML_FILE=$TMP_FILE"_pragmas.xml"

PROFILE_LIB="-I./source_exctractor/src/"
PROFILE_RTS="source_exctractor/src/profile_tracker/profile_tracker.cpp"
EXECUTABLE_PROFILE="executable_profile.o"

FINAL_LIB="-I./source_exctractor/src/ -I./source_exctractor/include/"
FINAL_RTS="source_exctractor/src/thread_pool/threads_pool.cpp source_exctractor/src/xml_creator/tinyxml2.cpp"
EXECUTABLE_FINAL="executable_final.o"
echo $EXECUTABLE_FINAL

OPENCV_SET=$2
if $OPENCV_SET == "true"; then
	OPENCV_CONFIG=$(/usr/bin/pkg-config --cflags --libs opencv)
else
	OPENCV_CONFIG=""
fi

echo $OPENCV_CONFIG

PROFILE_ITER=$3
VERBOSE=$4
SCHEDULE_TIME=$5
PROGRAM_DEADLINE=$6
PARALLEL=$7

echo "Launching the source manipulation program."
#./source_exctractor/pragma_exctractor.exec -fopenmp $SRC_FILE

echo "Compiling the profiler."
$CXX -std=c++11 $PROFILE_LIB $PROFILE_FILE $PROFILE_RTS $OPENCV_CONFIG  -o $EXECUTABLE_PROFILE

echo "Compiling the final program."
$CXX -std=c++11 $FINAL_LIB/ $TRANSFORMED_FILE $FINAL_RTS $OPENCV_CONFIG -o $EXECUTABLE_FINAL

echo "Launching the profiler and scheduler."
#/usr/bin/python graph_sched/graphCreator.py $XML_FILE $EXECUTABLE_PROFILE $PROFILE_ITER $VERBOSE $SCHEDULE_TIME $PROGRAM_DEADLINE $PARALLEL

#cp graph_sched/schedule.xml $PATH

echo "Launching the final program."
#$PATH"/"$EXECUTABLE_FINAL
