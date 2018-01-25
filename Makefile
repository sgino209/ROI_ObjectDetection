# ROI Object-Detection makefile, for cross-platform compilation & linking
# (c) Shahar Gino, January-2018, sgino209@gmail.com

AR		:=	ar
ifeq ($(shell uname -s), Darwin)
CXX		:=	clang++
CXXFLAGS	:=	-stdlib=libc++ -fpic -O3 -Wall -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -DNO_HISTOGRAM -std=c++11 \
                        -Wno-c++11-extensions -Wno-reorder -Wno-overloaded-virtual -Wno-null-conversion -Wno-delete-non-virtual-dtor -Wno-comment -Wno-format -Wno-char-subscripts
else
CXX		:=	g++
CXXFLAGS	:=	-fpic -O3 -Wall -DLUA_ENABLED -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -DNO_HISTOGRAM -std=c++11 \
                        -Wno-c++11-extensions -Wno-reorder -Wno-overloaded-virtual -Wno-null-conversion -Wno-delete-non-virtual-dtor -Wno-comment -Wno-format -Wno-char-subscripts
endif

LDFLAGS		:= 	-L/usr/local/lib `pkg-config --libs opencv`

INC		:= 	-I/usr/local/lib \
    			-I/usr/local/include \
			-I~/opencv/** \
			-I./

MODULES		:=	main

LIB_NAME	:=	objdetector.a

EXEC_NAME	:=	objdetector

BUILD		:=	build

EXECUTABLE	:=	$(BUILD)/$(EXEC_NAME)

LIB		:=	$(BUILD)/$(LIB_NAME)

OBJ		:=	$(patsubst %,$(BUILD)/%.o,$(MODULES))


$(EXECUTABLE) : $(LIB)
	$(CXX) $(LIB) $(LDFLAGS) -o $@

$(LIB) : $(BUILD) $(OBJ)
	$(AR) rcu $(LIB) $(OBJ)

$(BUILD)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

.PHONY: clean
clean:
	rm -f $(LIB)
	rm -f $(OBJ)

.PHONY: clobber
clobber:
	rm -rf $(BUILD)


