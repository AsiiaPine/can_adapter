# Copyright (C) 2025 Anastasiia Stepanova <asiiapine@gmail.com>
# Distributed under the terms of the GPL v3 license, available in the file LICENSE.

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BUILD_DIR:=$(ROOT_DIR)/build

all: clean default upload

default: clean
	mkdir -p $(BUILD_DIR)/obj
	cd $(BUILD_DIR)/obj && cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}  -G "Unix Makefiles" ../.. && make

clean:
	rm -rf $(BUILD_DIR)/obj

upload:
	st-flash --reset write $(BUILD_DIR)/obj/node.bin 0x8000000
