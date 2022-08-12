

participant:
	g++ source/*.cpp -o sleep_manager
	./sleep_manager

manager:
	g++ source/*.cpp -o sleep_manager
	./sleep_manager manager

clean:
	rm sleep_manager