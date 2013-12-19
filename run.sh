#! bin/bash

#launch ./pragma_exctractor file.cpp -> produce file_profile.cpp
#									   file_transformed.cpp
#									   file_pragmas.xml

#compile file_profile.cpp with profile_tracker.cpp -> produce file_profile.o
#compile file_transformed.cpp with thread_pool.cpp and tinyxml2.cpp -> final.o

#launch python graphCreator.py .... -> schedule.xml

#launch final.o (will read schedule.xml)

SRC_FILE = $1
PATH = dirname $SRC_FILE
TMP_FILE = ${SRC_FILE:-4}
PROFILE_FILE = $TMP_FILE"_profile.cpp"
TRANSFORMED_FILE = $TMP_FILE"_transformed.cpp"
XML_FILE = TMP_FILE"_pragmas.xml"

PROFILE_RTS = "source_exctractor/src/profile_tracker/profile_tracker.cpp"
EXECUTABLE_PROFILE = "executable_profile.o"

FINAL_RTS = "source_exctractor/src/thread_pool/thread_pool.cpp source_exctractor/src/xml_creator/tinyxml2.cpp"
EXECUTABLE_FINAL = "executable_final.o"

OPENCV_SET = $2
if $OPENCV_SET == "true"; then
	OPENCV_CONFIG = (pkg-config --cflag --libs opencv)
else
	OPENCV_CONFIG = ""
fi

PROFILE_ITER = $3
VERBOSE = $4
SCHEDULE_TIME = $5
PROGRAM_DEADLINE = $6
PARALLEL = $7

echo "Launching the source manipulation program."
g++ source_exctractor/pragma_exctractor -fopenmp $SRC_FILE

echo "Compiling the profiler."
g++ -std=c++11 -I./source_exctractor/src/ $PROFILE_FILE $PROFILE_RTS $OPENCV_CONFIG  -o $EXECUTABLE_PROFILE

echo "Compiling the final program."
g++ -std=c++1 -I./source_exctractor/src/ $TRANSFORMED_FILE $FINAL_RTS OPENCV_CONFIG -o $EXECUTABLE_FINAL

echo "Launching the profiler and scheduler."
python graph_sched/graphCreator.py $XML_FILE $EXECUTABLE_PROFILE $PROFILE_ITER $VERBOSE $SCHEDULE_TIME $PROGRAM_DEADLINE $PARALLEL

cp graph_sched/schedule.xml $PATH

echo "Launching the final program."
$PATH"/"$EXECUTABLE_FINAL
