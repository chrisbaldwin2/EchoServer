
.PHONY: all
all: run_server run_client

run_server: run_server.cpp
	g++ -pthread -o run_server run_server.cpp 
	
run_client: run_client.cpp
	g++ -o run_client run_client.cpp

.PHONY: install
install:
	mkdir -p binaries
	cp -p run_client binaries
	cp -p run_server binaries

.PHONY: clean
clean:
	rm -f run_server
	rm -f run_client