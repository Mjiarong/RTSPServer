CXX = g++

IDIR =./header
OBJS_PATH = ./obj/Release/source
BIN_PATH = ./bin/Release

CFLAGS=-I$(IDIR)
LD_FLAGS  = -pthread -lpthread 

TARGET = rtsp_server
SRC  = $(notdir $(wildcard ./source/*.cpp))
OBJS = $(patsubst %.cpp,$(OBJS_PATH)/%.o,$(SRC))

$(BIN_PATH)/$(TARGET) : $(OBJS) 
	$(CXX) $^ -o $@ $(LD_FLAGS)
	
$(OBJS_PATH)/%.o : ./source/%.cpp
	$(CXX) -c  $< -o  $@  $(CFLAGS) 
	
clean:
	-rm -rf $(OBJS_PATH)/*.o $(BIN_PATH)/$(TARGET) 
