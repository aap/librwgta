CXX=ee-g++
BUILD := ps2
TARGET := ps2test
CFLAGS := -Wall -g
INC := -I$(PS2SDK)/ee/include -I$(PS2SDK)/common/include

include ../../Make.common

LINK := -T$(PS2SDK)/ee/startup/linkfile
LIBPATH := -L$(PS2SDK)/ee/lib
LDFLAGS := -mno-crt0 $(LIBPATH)
LIBS := $(RWDIR)/librw-$(BUILD).a $(RWGTADIR)/librwgta-$(BUILD).a -lc -lc -lkernel -lmf

$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) $(LINK) $(PS2SDK)/ee/startup/crt0.o $(OBJ) $(LIBS) -o $(TARGET).elf
