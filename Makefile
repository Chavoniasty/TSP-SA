build-run-seq:
	@echo "Running sequential version"
	@g++ -o seq main.cpp -std=c++11
	@./seq
	@python3 display.py
