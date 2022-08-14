

participant:
	g++ -std=c++17 source/*.cpp -o sleep_manager -lpthread
	./sleep_manager

manager:
	g++ -std=c++17 source/*.cpp -o sleep_manager -lpthread
	./sleep_manager manager

clean:
	rm sleep_manager
