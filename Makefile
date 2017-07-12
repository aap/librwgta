# null, opengl
BUILD=null
TARGET=librwgta-$(BUILD).a

include Make.common

$(TARGET): $(OBJ)
	ar scr $@ $(OBJ)
