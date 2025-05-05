#compiler
CC=g++
#directories
INCLUDE=./include
SRC=./src
# libs
LIBS=-lpthread
# standard
STD=-std=c++17
#warnings
WARN=-Wall -Wextra -Wnull-dereference -Wpedantic -Wformat=2
#extra warnings (for later)
WARNEXTRA=-Wshadow -Wcast-align -Wconversion -Wsign-conversion
# dependency flags
DEPFLAGS=-MP -MD
#debug flag
DEBUGFLAG=-g
#all flags exept debug flag
CFLAGS=$(foreach D, $(INCLUDE), -I$(D)) $(STD) $(WARN) $(DEPFLAGS)
CPPFILES=$(foreach D, $(SRC), $(wildcard $(D)/*.cpp))

# object files
#relase
OBJECTSFILESR=$(patsubst $(SRC)%.cpp,$(RELASE)%.o, $(CPPFILES))
DEPFILESR=$(patsubst $(SRC)%.cpp,$(RELASE)%.d, $(CPPFILES))

#debug
OBJECTSFILESD=$(patsubst $(SRC)%.cpp,$(DEBUG)%.o, $(CPPFILES))
DEPFILESD=$(patsubst $(SRC)%.cpp,$(DEBUG)%.d, $(CPPFILES))

#output locations
OUT=/SimpleCheatEngine
DEBUG=./bin/debug
RELASE=./bin/relase

all: $(RELASE)

$(RELASE): $(OBJECTSFILESR)
	$(CC) -o $@$(OUT) $^ $(LIBS)
	
$(RELASE)%.o:$(SRC)%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

debug: $(DEBUG)

$(DEBUG): $(OBJECTSFILESD)
	$(CC) $(DEBUGFLAG) -o $@$(OUT) $^ $(LIBS)

$(DEBUG)%.o:$(SRC)%.cpp
	$(CC) $(CFLAGS) $(DEBUGFLAG) -c -o $@ $<

clean:
	rm -rf $(RELASE)$(OUT) $(DEBUG)$(OUT) $(OBJECTSFILESR) $(DEPFILESR) $(OBJECTSFILESD) $(DEPFILESD)

cleanRelase:
	rm -rf $(RELASE)$(OUT) $(OBJECTSFILESR) $(DEPFILESR)

cleanDebug:
	rm -rf $(DEBUG)$(OUT) $(OBJECTSFILESD) $(DEPFILESD)

diff:
	@git status
	@git diff --stat

-include $(DEPFILESR)
-include $(DEPFILESD)