SRC= parseDependencyGraph.c cJSON.c
HDRS=cJSON.h
CPLR=gcc

all:dParser

dParser: $(SRC) Makefile
	$(CPLR) $(SRC) -lm -lpthread -o $@

clean:
	rm -rf dParser 	 
