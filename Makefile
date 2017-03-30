CXX = g++
#CP = cp
CXXFLAGS = -lpthread -levent
TARGET = http_main
OBJS = main.o job.o threadpool.o threadmanage.o tcpeventserver.o http_conn.o
CFLAGS = -g -c
#LKFLAGS =
#INSTALLDIR = /mnt/
#install:
#	$(CP) $(TARGET) $(INSTALLDIR)

all:$(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(CXXFLAGS)

main.o: main.cpp tcpeventserver.cpp
	$(CXX) $(CFLAGS) $^
tcpeventserver.o: tcpeventserver.cpp threadmanage.cpp job.cpp threadpool.cpp
	$(CXX) $(CFLAGS) $^
job.o: job.cpp threadpool.cpp
	$(CXX) $(CFLAGS) $^
threadpool.o: threadpool.cpp job.cpp tcpeventserver.cpp http_conn.cpp
	$(CXX) $(CFLAGS) $^
threadmanage.o: threadmanage.cpp job.cpp threadpool.cpp
	$(CXX) $(CFLAGS) $^
http_conn.o: http_conn.cpp job.cpp tcpeventserver.cpp
	$(CXX) $(CFLAGS) $^

clean:
	@echo "cleaning project"
	-rm main*.o job*.o threadpool*.o threadmanage*.o tcpeventserver*.o http_conn*.o
	@echo "clean completed"
.PHONY: clean
