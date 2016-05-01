include ../makefileinc

SOURCES_PARSE=parse_test.c
SOURCES_RANGE=range_test.c

OBJECTS_PARSE=$(SOURCES_PARSE:.c=.o)
OBJECTS_RANGE=$(SOURCES_RANGE:.c=.o)

EXE_PARSE=parse_test
EXE_RANGE=range_test
EXECUTABLES=$(EXE_PARSE) $(EXE_RANGE)

.PHONY: all clean $(EXECUTABLES)

all: $(EXECUTABLES)

$(EXE_PARSE): $(OBJECTS_PARSE)
	$(CC) -o $@ $^ $(INC_PATH) $(LIB_PATH) -l$(LIBRARY) $(STDLIBS)
	$(STRIP) $(addsuffix .exe, $(EXE_PARSE))

$(EXE_RANGE): $(OBJECTS_RANGE)
	$(CC) -o $@ $^ $(INC_PATH) $(LIB_PATH) -l$(LIBRARY) $(STDLIBS)
	$(STRIP) $(addsuffix .exe, $(EXE_RANGE))

# $(LIBRARY): $(OBJECTS)
# 	$(AR) $(ARARGS) $@ $^

.c.o:
	$(CC) $(INC_PATH) $(CFLAGS) $< -o $@

clean:
	$(info removing tests...)
	$(RM) *.o
	$(RM) $(addsuffix .exe, $(EXECUTABLES))
