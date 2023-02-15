appname := eStarGo
.PHONY: all external
all: external agent

external:
	$(MAKE) -C external

agent: external
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean
	$(MAKE) -C external clean