BUILDDIR = build
SRC = Animation.cpp HandlingData.cpp PedStats.cpp TxdStore.cpp streaming.cpp FileLoader.cpp ModelInfo.cpp PedType.cpp Game.cpp PathFind.cpp TimeCycle.cpp base.cpp gl3main.cpp VisibilityPlugins.cpp TempColModels.cpp ModelIndices.cpp zones.cpp
SRCDIR=src
SRC := $(wildcard $(SRCDIR)/*.cpp $(SRCDIR)/*/*.cpp)
OBJ := $(patsubst %.cpp, $(BUILDDIR)/%.o, $(SRC))
DEP := $(patsubst %.cpp, $(BUILDDIR)/%.d, $(SRC))
RWDIR=$(CURDIR)/../../../librw
RWGTADIR=$(CURDIR)/../..

INC := -I/usr/local/include -I$(RWDIR) -I$(RWGTADIR)/src
LDLIBS=-lGL -lGLEW -lglfw
STATICLIBS= $(RWGTADIR)/librwgta-gl3.a $(RWDIR)/librw-gl3.a

CFLAGS=-g -DRW_GL3 -I$(RWDIR) \
	-Wall -Wextra -Wno-invalid-offsetof -Wno-parentheses \
	-Wno-unused-parameter -fno-diagnostics-show-caret

IIItest: $(OBJ) $(STATICLIBS)
	$(CXX) $^ $(LDLIBS) -o $@

$(BUILDDIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) $(INC) -c $< -o $@

$(BUILDDIR)/%.d: %.cpp
	@mkdir -p $(@D)
	$(CXX) -MM -MT '$(patsubst %.cpp,$(BUILDDIR)/%.o,$<)' $(CFLAGS) $(INC) $< > $@

clean:
	rm $(BUILDDIR)/* gl3test

-include $(DEP)
