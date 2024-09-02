
# Dyanmic loader path for shared library linking, laoding the path from terminal
LOADER_PATH = $(shell pwd)/loader


# Making fib output file and moving to bin
run1 : 
	mkdir -p bin
	cd test && make 

# Making shared library and moving 
run2:
	mkdir -p loader
	cd loader && make

# Making launch output file  using shared library and moving to bin
run3: 
	mkdir -p bin
	cd launcher && make


# to run first write make on temrinal of root directory and then write "make run"
run: run1 run2 run3 
	@echo "Running fib using launch..."
	@LD_LIBRARY_PATH=$(LOADER_PATH) ./bin/launch bin/fib

# after running write "make clean" to clean all the executable  files formed  
clean:
	rm -f bin/fib bin/launch
	rm -f loader/lib_simpleloader.so
	cd loader && rm -f *.o lib_simpleloader.so
	cd launcher && rm -f *.o

