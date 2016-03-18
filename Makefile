SRC= parseDependencyGraph.c cJSON.c
HDRS=cJSON.h
CPLR=gcc

all:dParser

dParser: $(SRC) Makefile
	$(CPLR) $(SRC) -lm -o $@

clean:
	rm -rf dParser 	 
