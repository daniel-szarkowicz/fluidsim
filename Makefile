
# CXX = g++
CXX = clang++

BUILD_DIR = build
BINARY = $(BUILD_DIR)/fluidsim
IMGUI_DIR = src/imgui
SOURCES =
SOURCES += src/main.cpp src/camera.cpp src/shader.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_draw.cpp
SOURCES += $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
OBJS = $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)

CXXFLAGS =
CXXFLAGS += -std=c++17 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS += -g -Wall -Wformat -Wextra -fopenmp
LIBS =
LIBS += `pkg-config --static --libs glfw3`
LIBS += `pkg-config --static --libs glew`

CXXFLAGS += `pkg-config --cflags glfw3`
CFLAGS = $(CXXFLAGS)

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

$(BUILD_DIR)/%.o:%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: build

build: $(BINARY)
	@echo Build complete

run: build
	./$(BINARY)

$(BINARY): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -rf $(BUILD_DIR)
