

participant:
	g++ source/main.cpp -o sleep_manager -lpthread
	./sleep_manager

manager:
	g++ source/main.cpp -o sleep_manager -lpthread
	./sleep_manager manager