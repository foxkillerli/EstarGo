appname := eStarGo
.PHONY: all external proto_ice agent
all: external proto_ice agent

external:
	$(MAKE) -C external

proto_ice:
	$(MAKE) -C proto

agent: proto_ice external
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean
	$(MAKE) -C external clean
	$(MAKE) -C proto clean