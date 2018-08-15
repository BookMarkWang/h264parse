##############################################
# Makefile
##############################################
PATH_TO_SDK := /home/ywang75/study/3rd_party

BIN := test
  
CC := g++
CFLAGS := -O0 -g -std=c++11
  
SOURCE := $(wildcard ../*.cpp) $(wildcard *.cpp)
OBJS := $(patsubst %.cpp, %.o, $(SOURCE))

CC_INC := -I/usr/include -I. -I../ -I$(PATH_TO_SDK)/include
LD_INC := -L/usr/lib/
LD_LIB :=
  
all : $(BIN)
$(BIN) : $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(BIN) $(CC_INC) $(LD_INC) $(LD_LIB)

%.o : %.cpp
	@$(CC) $(CFLAGS) -c $< -o $@ $(CC_INC) 
.PHONY : clean
  
clean :
	@rm -f $(BIN) $(OBJS) *.o

