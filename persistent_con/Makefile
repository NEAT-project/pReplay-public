SRC=persis_con.c worker.c
CPLR=gcc

add:percon

percon: $(SRC) Makefile
	$(CPLR) $(SRC) -lcurl -o $@

clean:
	rm -rf percon
