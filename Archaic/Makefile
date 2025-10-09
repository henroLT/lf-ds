CXX = clang++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2
BIN_DIR = bin

standard:
	@$(MAKE) build_all TYPE=Standard PREFIX=s

new:
	@$(MAKE) build_all TYPE=New PREFIX=n

# Build all module folders inside TYPE directory
build_all:
	@mkdir -p $(BIN_DIR)
	@for module in $$(find $(TYPE) -mindepth 1 -maxdepth 1 -type d -exec basename {} \;); do \
		echo "Building $$module..."; \
		$(MAKE) build TYPE=$(TYPE) MODULE=$$module PREFIX=$(PREFIX); \
	done

# Build single module into one executable
build:
	@mkdir -p $(BIN_DIR)
	@SRCS=$$(find $(TYPE)/$(MODULE) -name '*.cpp'); \
	OBJS=""; \
	for SRC in $$SRCS; do \
		OBJ=$$(basename $$SRC .cpp).o; \
		$(CXX) $(CXXFLAGS) -c $$SRC -o $$OBJ; \
		OBJS="$$OBJS $$OBJ"; \
	done; \
	$(CXX) $(CXXFLAGS) $$OBJS -o $(BIN_DIR)/$(PREFIX)$(MODULE); \
	rm -f *.o

clean:
	rm -rf $(BIN_DIR) *.o
