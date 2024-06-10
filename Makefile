build-run-seq:
	@echo "Running sequential version"
	@g++ -o seq main.cpp -std=c++20
	@./seq
