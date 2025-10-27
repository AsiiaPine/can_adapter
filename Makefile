# Copyright (C) 2025 Anastasiia Stepanova <asiiapine@gmail.com>
# Distributed under the terms of the GPL v3 license, available in the file LICENSE.

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BUILD_DIR:=$(ROOT_DIR)/build
PLATFORM?=STM32G0B1

all: clean default upload

stm32g0b1: clean
	mkdir -p $(BUILD_DIR)/STM32G0B1/obj
	cd $(BUILD_DIR)/STM32G0B1/obj && cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DPLATFORM=STM32G0B1 -G "Unix Makefiles" ../../.. && make

ubuntu: clean
	mkdir -p $(BUILD_DIR)/UBUNTU/obj
	cd $(BUILD_DIR)/UBUNTU/obj && cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DPLATFORM=UBUNTU -G "Unix Makefiles" ../../.. && make

clean:
	rm -rf $(BUILD_DIR)/${PLATFORM}/obj

upload:
	st-flash --reset write $(BUILD_DIR)/STM32G0B1/obj/node.bin 0x8000000

run:
	$(BUILD_DIR)/UBUNTU/obj/node
