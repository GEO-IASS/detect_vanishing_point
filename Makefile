#USER_HEADER = ./include
#SAMPLE_PATH = ./ExampleOCR
#BASIC_PATH = ./CV_Core
#SOURCECODE = ./SourceCode

#OpenCV_INC_PATH = /usr/include
#OpenCV_LIB_PATH = /usr/lib

OpenCV_INC_PATH = /opt/newwayy/tupian/opencv-2.4.9/include/
OpenCV_LIB_PATH = /opt/newwayy/tupian/opencv-2.4.9/lib/

VPATH = $(OpenCV_INC_PATH)

#C_ARGS = -fopenmp -Wno-unused-result -O2
C_ARGS = -Wno-unused-result -O2
TARGET = _detectVanishingP.so

CXX = g++
CC = gcc

objects = main.o detectVP.o lsd.o MSAC.o errorNIETO.o lmcurve.o lmmin.o detectVanishingP_wrap.o

#$(TARGET): $(objects)
#	$(CXX) -shared -fPIC $^ -o $@ -I$(OpenCV_INC_PATH) -L$(OpenCV_LIB_PATH) `pkg-config --cflags --libs opencv` 

$(TARGET): $(objects)
	$(CXX) $(C_ARGS) -shared $^ -o $@ -I$(OpenCV_INC_PATH) -L$(OpenCV_LIB_PATH) /opt/newwayy/tupian/opencv-2.4.9/lib/cv2.so

%.o: %.cpp
	$(CXX) -fPIC $(C_ARGS) -c $^ -o $@ -I$(OpenCV_INC_PATH)

%.o: %.c
	$(CXX) -fPIC $(C_ARGS) -c $^ -o $@ -I$(OpenCV_INC_PATH)

detectVanishingP_wrap.o: detectVanishingP_wrap.cxx
	$(CXX) -fPIC $(C_ARGS) -c $^ -o $@ -I$(OpenCV_INC_PATH) -I/usr/include/python2.6

clean:
	rm -f *.o $(TARGET)
