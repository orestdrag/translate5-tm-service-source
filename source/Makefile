SRC_DIR := .
OBJ_DIR := obj
BIN_DIR := bin

EXE := $(BIN_DIR)/OtmMemoryService
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

ifndef RESTBED_PATH
export RESTBED_PATH=../../../Packages/restbed
endif

INCLUDE := -I"$(RESTBED_PATH)/distribution/include"
LIBS := -L"$(RESTBED_PATH)/distribution/library" -lrestbed

CPPFLAGS := -Iinclude -MMD -MP
CXXFLAGS := -Wall -Werror
LDFLAGS := -Llib $(INCLUDE)
LDLIBS := -lm $(LIBS)

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(LDFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)
