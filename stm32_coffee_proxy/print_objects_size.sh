#!/bin/bash
find . -iname "*.o" -exec arm-none-eabi-size.exe {} \; | grep src | sort -nk4
arm-none-eabi-size.exe Debug/stm32_coffee_proxy.elf