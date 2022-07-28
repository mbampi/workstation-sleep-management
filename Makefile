

participant:
	g++ source/main.cpp -o sleep_manager
	./sleep_manager

manager:
	g++ source/main.cpp -o sleep_manager -lpthread
	./sleep_manager manager

clean:
	rm sleep_manager