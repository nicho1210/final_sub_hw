PROJECT_DIR := selected_motion_regions/pyzq

.PHONY: all init hw pfm app clean status

all: init hw pfm app

init:
	git submodule update --init --recursive

hw:
	$(MAKE) -C $(PROJECT_DIR)/hw all

pfm:
	$(MAKE) -C $(PROJECT_DIR)/baremetal pfm

app:
	$(MAKE) -C $(PROJECT_DIR)/baremetal app

status:
	git status

clean:
	$(MAKE) -C $(PROJECT_DIR)/hw clean || true
	$(MAKE) -C $(PROJECT_DIR)/baremetal clean || true