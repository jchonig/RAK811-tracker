
# Set for OS
TTY=/dev/tty.SLAB_USBtoUART

STSTM32=$(HOME)/.platformio/platforms/ststm32
RAK811_JSON=$(HOME)/.platformio/platforms/ststm32/boards/rak811.json
STM32L151XBA_LD=$(HOME)/.platformio/platforms/ststm32/ldscripts/stm32l151xba.ld

build: $(RAK811_JSON) $(STM32L151XBA_LD)
	pio run

flash: build ${TTY}
	./stm32flash_src/stm32flash -w .pioenvs/rak811/firmware.bin ${TTY}

test: build ${TTY}
	./stm32flash_src/stm32flash -w .pioenvs/rak811/firmware.bin -v -g 0x0 ${TTY}

$(STSTM32):
	pio platform install ststm32

$(RAK811_JSON): $(STSTM32) rak811.json
	cp -p rak811.json $(RAK811_JSON)

$(STM32L151XBA_LD): $(STSTM32) STM32L151XBA_FLASH.ld
	cp -p STM32L151XBA_FLASH.ld $(STM32L151XBA_LD)
	chmod 755 $(STM32L151XBA_LD)

true: ;
