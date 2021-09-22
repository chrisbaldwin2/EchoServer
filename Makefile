#
SERVERFILE=run_server 
CLIENTFILE=run_client
INSTALLDIR=build
OUTPUTFILE=$(SERVERFILE) $(CLIENTFILE)
# look for .cpp & .h files in ./src
vpath %.cpp ./src
vpath %.h ./src

.PHONY: all
all: $(OUTPUTFILE)

%: %.cpp
	g++ -o $(INSTALLDIR)/$@ $<

.PHONY: clean
clean:
	rm -f $(INSTALLDIR)/$(SERVERFILE) $(INSTALLDIR)/$(CLIENTFILE)