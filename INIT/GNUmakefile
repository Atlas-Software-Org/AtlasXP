CPIO_SRC_DIR := CPIO_Contents
CPIO_TARGET := init.cpio

.PHONY: all clean

all: $(CPIO_TARGET)

$(CPIO_TARGET):
	@echo "Building $(CPIO_TARGET) from $(CPIO_SRC_DIR)..."
	cd $(CPIO_SRC_DIR) && find . | cpio -o -H newc --quiet > ../$(CPIO_TARGET)\

clean:
	rm -f $(CPIO_TARGET)
