#pool_cgi_main: pool_cgi_test.o 
#	g++ -g pool_cgi_test.o -o pool_cgi_main
#pool_cgi_test.o: pool_cgi_test.cpp processpool.h
#	g++ -g -c pool_cgi_test.cpp
#clean:
#	@echo "cleaning project"
#	-rm pool_cgi_test*.o
#	@echo "clean completed"
#.PHONY: clean

http_main: main.o http_conn.o
	g++ main.o http_conn.o -o http_main -lpthread

main.o: http_conn.h locker.h threadpool.h
http_conn.o: http_conn.h locker.h

main.o: main.cpp http_conn.cpp 
	g++ -g -c main.cpp http_conn.cpp 
http_conn.o: http_conn.cpp
	g++ -g -c http_conn.cpp

clean:
	@echo "cleaning project"
	-rm main*.o http_conn*.o
	@echo "clean completed"
.PHONY: clean


