
CXX=g++

SRC_FILE=$1
FILE_PATH=$(dirname $SRC_FILE)

TMP_FILE=${SRC_FILE:0:-4}
PROFILE_FILE=$TMP_FILE"_profile.cpp"
TRANSFORMED_FILE=$TMP_FILE"_transformed.cpp"
XML_FILE=$TMP_FILE"_pragmas.xml"
SEQ_FILE=$TMP_FILE"_seq.cpp"
OMP_FILE=$TMP_FILE"_omp.cpp"

PROFILE_LIB="-I./source_exctractor/src/"
PROFILE_RTS="source_exctractor/src/profile_tracker/profile_tracker.cpp"
EXECUTABLE_PROFILE="executable_profile.o"

FINAL_LIB="-I./source_exctractor/src/ -I./source_exctractor/include/"
FINAL_RTS="source_exctractor/src/thread_pool/threads_pool.cpp source_exctractor/src/xml_creator/tinyxml2.cpp -lpthread"
EXECUTABLE_FINAL="executable_final.o"
OPENCV_SET=$2
if $OPENCV_SET == "true"; then
	OPENCV_CONFIG=$(/usr/bin/pkg-config --cflags --libs opencv)
	OPENCV_CONFIG=$OPENCV_CONFIG" -lX11"
else
	OPENCV_CONFIG=""
fi


PROFILE_ITER=$3
VERBOSE=$4
SCHEDULE_TIME=$5
PROGRAM_DEADLINE=$6
PARALLEL=$7

PAR_FILE=$FILE_PATH/parameters.txt
PARAMETERS=$(sed -n '1p' < $PAR_FILE)

echo "Launching the source manipulation program."
#./source_exctractor/pragma_exctractor.exec -fopenmp $SRC_FILE > a.out

echo "Compiling sequential"
$CXX -std=c++11 $SEQ_FILE $OPENCV_CONFIG -o $FILE_PATH/sequential.o

echo "Compiling OpenMP"
$CXX -std=c++11 $OMP_FILE $OPENCV_CONFIG -fopenmp -o $FILE_PATH/parallel_omp.o

echo "Compiling the profiler."
$CXX -std=c++11 $PROFILE_LIB $PROFILE_FILE $PROFILE_RTS $OPENCV_CONFIG  -o $FILE_PATH/$EXECUTABLE_PROFILE

echo "Compiling the final program."
$CXX -std=c++11 $FINAL_LIB $TRANSFORMED_FILE $FINAL_RTS $OPENCV_CONFIG -o $FILE_PATH/$EXECUTABLE_FINAL

echo "Launching the profiler and scheduler."
cp $FILE_PATH/parameters.txt graph_sched/
cd graph_sched/
#python graphCreator.py ../$XML_FILE ../$FILE_PATH/$EXECUTABLE_PROFILE $PROFILE_ITER $VERBOSE $SCHEDULE_TIME $PROGRAM_DEADLINE $PARALLEL
cd ..
#cp graph_sched/schedule.xml $FILE_PATH



echo "Launching the final program."
cd $FILE_PATH
#rm images/*.jpg
#time ./sequential.o $PARAMETERS > ./sequential_time.txt
#rm images/*.jpg
#time ./parallel_omp.o $PARAMETERS > ./omp_time.txt
rm images/*.jpg
time ./$EXECUTABLE_FINAL $PARAMETERS > ./bomber_time.txt
