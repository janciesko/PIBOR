CXX=mcxx --ompss 
FLAGS =  -k -O3 --no-copy-deps -std=c++11
DBGFLAGS = -k -g --no-copy-deps --debug 
TRCFLAGS= -k --no-copy-deps --instrument

TARGET = randomaccess
TARGET_D = randomaccess_dbg
TARGET_T = randomaccess_trc

default: $(TARGET)
debug: $(TARGET_D)
trace: $(TARGET_T)
all: $(TARGET) $(TARGET_D) $(TARGET_T)


$(TARGET).o: src/$(TARGET).cpp
	$(CXX) $(FLAGS) -c $< -o $@

$(TARGET):$(TARGET).o
	$(CXX) $^ -o $@

$(TARGET_D).o: src/$(TARGET).cpp
	$(CXX) $(DBGFLAGS) -c $< -o $@

$(TARGET_D):$(TARGET_D).o
	$(CXX) $(DBGFLAGS)  $^ -o $@

$(TARGET_T).o: src/$(TARGET).cpp
	$(CXX) $(TRCFLAGS) -c $< -o $@

$(TARGET_T):$(TARGET_T).o 
	$(CXX) $(TRCFLAGS) $^ -o $@





clean:
	rm -rf $(TARGET) $(TARGET_D) $(TARGET_T) *.o mcxx* *.out *.err

 
