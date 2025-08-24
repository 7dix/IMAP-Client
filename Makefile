# Makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
LDFLAGS = -lssl -lcrypto
CPPFLAGS = -I/usr/local/opt/openssl/include

# GUI specific flags
GTK_CFLAGS = $(shell pkg-config --cflags gtkmm-3.0)
GTK_LIBS = $(shell pkg-config --libs gtkmm-3.0)

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = .

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
CLI_SOURCES = $(filter-out $(SRC_DIR)/gui_main.cpp $(SRC_DIR)/ImapClientGUI.cpp, $(SOURCES))
GUI_SOURCES = $(filter-out $(SRC_DIR)/main.cpp, $(SOURCES))

CLI_OBJECTS = $(CLI_SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
GUI_OBJECTS = $(GUI_SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CLI_EXECUTABLE = $(BIN_DIR)/imapcl
GUI_EXECUTABLE = $(BIN_DIR)/imapcl-gui
DEBUG_CLI_EXECUTABLE = $(BIN_DIR)/imapcl_debug
DEBUG_GUI_EXECUTABLE = $(BIN_DIR)/imapcl-gui_debug

all: $(CLI_EXECUTABLE) $(GUI_EXECUTABLE)

cli: $(CLI_EXECUTABLE)

gui: $(GUI_EXECUTABLE)

$(CLI_EXECUTABLE): $(CLI_OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(CLI_OBJECTS) -o $@ $(LDFLAGS)

$(GUI_EXECUTABLE): $(GUI_OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(GTK_CFLAGS) $(GUI_OBJECTS) -o $@ $(LDFLAGS) $(GTK_LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	@if echo "$<" | grep -q "GUI\|gui_main"; then \
		$(CXX) $(CXXFLAGS) $(GTK_CFLAGS) $(CPPFLAGS) -c $< -o $@; \
	else \
		$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@; \
	fi

debug: CXXFLAGS += -g
debug: $(DEBUG_CLI_EXECUTABLE) $(DEBUG_GUI_EXECUTABLE)

$(DEBUG_CLI_EXECUTABLE): $(CLI_OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(CLI_OBJECTS) -o $@ $(LDFLAGS)

$(DEBUG_GUI_EXECUTABLE): $(GUI_OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(GTK_CFLAGS) $(GUI_OBJECTS) -o $@ $(LDFLAGS) $(GTK_LIBS)

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(CLI_EXECUTABLE) $(GUI_EXECUTABLE) $(DEBUG_CLI_EXECUTABLE) $(DEBUG_GUI_EXECUTABLE)

.PHONY: all cli gui clean debug