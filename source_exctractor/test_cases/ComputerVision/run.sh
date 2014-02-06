CXX=g++

SRC_FILE=$1
FILE_PATH=$(dirname $SRC_FILE)

TMP_FILE=${SRC_FILE:0:-4}
TRANSFORMED_FILE=$TMP_FILE"_transformed.cpp"
SEQ_FILE=$TMP_FILE"_seq.cpp"
OMP_FILE=$TMP_FILE"_omp.cpp"

FINAL_LIB="-I./source_exctractor/src/ -I./source_exctractor/include/"
FINAL_RTS="source_exctractor/src/thread_pool/threads_pool.cpp source_exctractor/src/xml_creator/tinyxml2.cpp -lpthread"
EXECUTABLE_FINAL="executable_final.o"

OPENCV_CONFIG=$(/usr/bin/pkg-config --cflags --libs opencv)
OPENCV_CONFIG=$OPENCV_CONFIG" -lX11"

echo "Compiling sequential"
$CXX -std=c++11 $SEQ_FILE $OPENCV_CONFIG -o $FILE_PATH/sequential.o

echo "Compiling OpenMP"
$CXX -std=c++11 $OMP_FILE $OPENCV_CONFIG -fopenmp -o $FILE_PATH/parallel_omp.o

echo "Compiling the final program."
$CXX -std=c++11 $FINAL_LIB $TRANSFORMED_FILE $FINAL_RTS $OPENCV_CONFIG -o $FILE_PATH/$EXECUTABLE_FINAL

PAR_FILE=$FILE_PATH/parameters.txt
PARAMETERSLOW=$(sed -n '1p' < $PAR_FILE)
PARAMETERSMEDIUM=$(sed -n '3p' < $PAR_FILE)
PARAMETERSHIGH=$(sed -n '5p' < $PAR_FILE)

echo "Launching the final program"
cd $FILE_PATH

echo "Low resolution video (480)"
echo "Launching the sequential"
rm images/*.jpg
time ./sequential.o $PARAMETERSLOW > ./sequential_time_480.txt
echo "Launching the OpenMP"
rm images/*.jpg
time ./parallel_omp.o $PARAMETERSLOW > ./omp_time_480.txt
echo "Launching soma"
rm images/*.jpg
time ./$EXECUTABLE_FINAL $PARAMETERSLOW > ./soma_time_480.txt

echo "Medium resolution video (720)"
echo "Launching the sequential"
rm images/*.jpg
time ./sequential.o $PARAMETERSMEDIUM > ./sequential_time_720.txt
echo "Launching the OpenMP"
rm images/*.jpg
time ./parallel_omp.o $PARAMETERSMEDIUM > ./omp_time_720.txt
echo "Launching soma"
rm images/*.jpg
time ./$EXECUTABLE_FINAL $PARAMETERSMEDIUM > ./soma_time_720.txt

echo "Medium resolution video (1080)"
echo "Launching the sequential"
rm images/*.jpg
time ./sequential.o $PARAMETERSHIGH > ./sequential_time_1080.txt
echo "Launching the OpenMP"
rm images/*.jpg
time ./parallel_omp.o $PARAMETERSHIGH > ./omp_time_1080.txt
echo "Launching soma"
rm images/*.jpg
time ./$EXECUTABLE_FINAL $PARAMETERSHIGH > ./soma_time_1080.txt

