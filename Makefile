SERVERFILE=run_server
CLIENTFILE=run_client
INSTALLDIR=build
SRCDIR=src
DEPS=$(SRCDIR)/mp1.h
OUTPUTFILE=$(SERVERFILE) $(CLIENTFILE)
# look for .cpp & .h files in ./src
vpath %.cpp ./src
vpath %.h ./src

.PHONY: all
all: $(OUTPUTFILE)


%: %.cpp $(DEPS)
	g++ -o $@ $<


.PHONY: install
install:
	mkdir -p $(INSTALLDIR)
	cp -p $(SERVERFILE) $(INSTALLDIR)
	cp -p $(CLIENTFILE) $(INSTALLDIR)

.PHONY: clean
clean:
	rm -f $(INSTALLDIR)/$(SERVERFILE) $(INSTALLDIR)/$(CLIENTFILE)
	rm -f $(OUTPUTFILE)