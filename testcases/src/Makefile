MODE ?= release

#bootfs := $(build_dir)/bootfs

ifeq ($(MODE), release)
  build_args += --release
endif

.PHONY: all uart socket clean

all: uart socket

uart: uart-build

socket: socket-build

uart-build:
	rustup target add x86_64-unknown-linux-musl
	cd uart && cargo build --target x86_64-unknown-linux-musl $(build_args)

socket-build:
	cd socket ; pwd
	@echo "To build socket testcases, please copy *.c into libc-test/src/functional/"

clean:
	cd uart
	cargo clean

