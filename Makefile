CXX = g++
CXXFLAGS = -lpthread

http_main: main.o job.o threadpool.o threadmanage.o http_conn.o
	$(CXX) -o http_main main.o job.o threadpool.o threadmanage.o http_conn.o $(CXXFLAGS) 

main.o: http_conn.h threadmanage.h
job.o: job.h threadpool.h
threadpool.o: threadpool.h job.h threadlock.h
threadmanage.o: threadmanage.h job.h threadpool.h threadlock.h
http_conn.o: http_conn.h job.h

main.o: main.cpp threadmanage.cpp http_conn.cpp
	$(CXX) -g -c main.cpp threadmanage.cpp http_conn.cpp
job.o: job.cpp threadpool.cpp
	$(CXX) -g -c job.cpp threadpool.cpp
threadpool.o: threadpool.cpp job.cpp
	$(CXX) -g -c threadpool.cpp job.cpp 
threadmanage.o: threadmanage.cpp job.cpp threadpool.cpp
	$(CXX) -g -c threadmanage.cpp job.cpp threadpool.cpp
http_conn.o: http_conn.cpp job.cpp
	$(CXX) -g -c http_conn.cpp job.cpp

clean:
	@echo "cleaning project"
	-rm main*.o job*.o threadpool*.o threadmanage*.o http_conn*.o
	@echo "clean completed"
.PHONY: clean
