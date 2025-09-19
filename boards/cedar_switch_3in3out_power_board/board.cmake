# SPDX-License-Identifier: Apache-2.0

# keep first
board_runner_args(stm32cubeprogrammer "--port=swd" "--reset-mode=hw")
board_runner_args(jlink "--device=STM32F411CE" "--speed=4000")

# keep first
include(${ZEPHYR_BASE}/boards/common/stm32cubeprogrammer.board.cmake)
include(${ZEPHYR_BASE}/boards/common/openocd-stm32.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)

#board_set_flasher(jlink)
#board_set_debugger(jlink)
#board_set_flasher(stm32cubeprogrammer)
#board_set_debugger(stm32cubeprogrammer)

#board_set_flasher_ifnset(jlink)
# board_set_debugger_ifnset(jlink)