CFLAGS:=-fPIC -I$(ZABBIX_SOURCE)/include -I$(ZABBIX_SOURCE)/include/common $(CFLAGS)
OBJECTS:=$(patsubst %.c,%.o,$(wildcard src/*.c))

zaipcs.so: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) $(LIBS) -shared -o $@

all: zaipcs.so

clean:
	rm -rf $(OBJECTS) zaipcs.so
