BUILDDIR = build
SRC = main.cpp camera.cpp common.cpp
OBJ = $(patsubst %.cpp, $(BUILDDIR)/%.o, $(SRC))
DEP = $(patsubst %.cpp, $(BUILDDIR)/%.d, $(SRC))
RWDIR=$(HOME)/src/librw
RWGTADIR=$(HOME)/src/librwgta

INC := -I/usr/local/include -I$(RWDIR) -I$(RWGTADIR)/src
LDLIBS=-lGL -lGLEW -lglfw
STATICLIBS= $(RWGTADIR)/librwgta-null.a $(RWDIR)/librw-gl3.a

CFLAGS=-g -DRW_GL3 -I$(RWDIR) \
	-Wall -Wextra -Wno-invalid-offsetof -Wno-parentheses \
	-Wno-unused-parameter -fno-diagnostics-show-caret

gl3test: $(OBJ) $(STATICLIBS)
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
