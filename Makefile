
run:
	g++ -std=c++14 source/*.cpp -o sleep_manager -lpthread
	./sleep_manager

clean:
	rm sleep_manager
