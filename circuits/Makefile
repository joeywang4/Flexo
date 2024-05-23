EXAMPLES := AES ALU arithmetic gates SHA1 Simon

all: $(EXAMPLES)

$(EXAMPLES):
	$(MAKE) -C $@

clean:
	@for dir in $(EXAMPLES); do \
		$(MAKE) -C $$dir clean; \
	done

.PHONY: all $(EXAMPLES)
