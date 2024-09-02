#invoke make inside following directories and in this order: loader, launch, fib
#move the lib_simpleloader.so and launch binaries inside bin directory
#Provide the command for cleanup

# Variable names used for directories 
LOADER_DIR = loader
LAUNCH_DIR = launcher
FIB_DIR = test
BIN_DIR = bin

# Variable names of the files 
LIBRARY = lib_simpleloader.so
LIBRARY_NAME = _simpleloader
LAUNCH_BINARY = launch
FIB_BINARY = fib

# Dyanmic loader path for shared library linking, laoding the path from terminal
LOADER_PATH = $(shell pwd)/$(LOADER_DIR)

# Default make file  output on terminal , to run the file write make all then make run 
all: $(BIN_DIR)/$(FIB_BINARY) $(BIN_DIR)/$(LAUNCH_BINARY)

# Making fib output file and moving to bin
$(BIN_DIR)/$(FIB_BINARY): $(FIB_DIR)/fib.c
	mkdir -p $(BIN_DIR)
	cd $(FIB_DIR) && gcc -m32 -no-pie -nostdlib -o $(FIB_BINARY) fib.c && mv $(FIB_BINARY) ../$(BIN_DIR)/

# Making shared library and moving 
$(LOADER_DIR)/$(LIBRARY): $(LOADER_DIR)/loader.c
	mkdir -p $(LOADER_DIR)
	cd $(LOADER_DIR) && gcc -m32 -o loader.o -c -fpic loader.c && gcc -m32 -shared -o $(LIBRARY) loader.o

# Making launch output file  using shared library and moving to bin
$(BIN_DIR)/$(LAUNCH_BINARY): $(LAUNCH_DIR)/launch.c $(LOADER_DIR)/$(LIBRARY)
	mkdir -p $(BIN_DIR)
	cd $(LAUNCH_DIR) && gcc -m32 -c -fpic launch.c && gcc -m32 -L$(LOADER_PATH) -Wl,-rpath=$(LOADER_PATH) -o $(LAUNCH_BINARY) launch.o -l$(LIBRARY_NAME) && mv $(LAUNCH_BINARY) ../$(BIN_DIR)/

# to run first write make on temrinal of root directory and then write "make run"
run: all
	@echo "Running $(FIB_BINARY) using $(LAUNCH_BINARY)..."
	@LD_LIBRARY_PATH=$(LOADER_PATH) ./$(BIN_DIR)/$(LAUNCH_BINARY) $(BIN_DIR)/$(FIB_BINARY)

# after running write "make clean" to clean all the executable  files formed  
clean:
	rm -f $(BIN_DIR)/$(FIB_BINARY) $(BIN_DIR)/$(LAUNCH_BINARY)
	rm -f $(LOADER_DIR)/$(LIBRARY)
	cd $(LOADER_DIR) && rm -f *.o $(LIBRARY)
	cd $(LAUNCH_DIR) && rm -f *.o

.PHONY: all clean run
