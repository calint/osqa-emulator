#include <cstdint>
#include <fstream>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <vector>
// #define RISCV_DEBUG
#include "riscv.h"

#define UART_OUT 0xfffffffe
#define UART_IN 0xfffffffd
#define LED 0xffffffff

// Define the RAM
std::vector<int8_t> ram(2 * 1024 * 1024, -1);

// Function to read from or write to the RAM
rv_uint32 ram_access(rv_uint32 addr, RISCV_BUSWIDTH width, rv_uint32 is_store,
                     rv_uint32 *data) {
  if (addr + width > ram.size() && addr != UART_OUT && addr != UART_IN &&
      addr != LED) {
    return 1; // Address out of bounds
  }

  if (is_store) {
    if (addr == UART_OUT) {
      const char ch = static_cast<char>(*data & 0xFF);
      if (ch == 0x7f) {
        // Convert to backspace
        std::cout << "\b \b";
      } else {
        // Write the byte to the console
        std::cout << ch;
      }
      std::cout.flush(); // Ensure the output is flushed
    } else if (addr != LED) {
      // Write to RAM
      for (rv_uint32 i = 0; i < width; ++i) {
        ram[addr + i] = (*data >> (i * 8)) & 0xFF;
      }
    }
  } else {
    if (addr == UART_OUT && width == RVBUS_BYTE) {
      // Always return 0 when reading from address UART_OUT
      *data = 0;
    } else if (addr == UART_IN && width == RVBUS_BYTE) {
      // Read a character from the console without blocking
      struct termios oldt, newt;
      tcgetattr(STDIN_FILENO, &oldt);
      newt = oldt;
      newt.c_lflag &= ~(ICANON | ECHO);
      tcsetattr(STDIN_FILENO, TCSANOW, &newt);

      int ch = getchar();
      if (ch != EOF) {
        if (ch == 0x0a) {
          *data = 0x0d;
        } else {
          *data = ch;
        }
      } else {
        *data = 0;
      }

      tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    } else if (addr != LED) {
      // Read from RAM
      *data = 0;
      for (rv_uint32 i = 0; i < width; ++i) {
        *data |= (ram[addr + i] & 0xFF) << (i * 8);
      }
    }
  }

  return 0; // Success
}

auto main(int argc, char **argv) -> int {
  // Open the binary file
  std::ifstream file("firmware.bin", std::ios::binary | std::ios::ate);
  if (!file) {
    std::cerr << "Error opening file" << std::endl;
    return 1;
  }

  // Get the size of the file
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  // Read the file into the RAM
  if (size > static_cast<std::streamsize>(ram.size())) {
    std::cerr << "Firmware size exceeds RAM size" << std::endl;
    return 1;
  }

  if (!file.read(reinterpret_cast<char *>(ram.data()), size)) {
    std::cerr << "Error reading file" << std::endl;
    return 1;
  }

  // Close the file
  file.close();

  // Initialize the CPU
  RISCV cpu;
  riscv_init(&cpu, ram_access, 0x0);

  // Run the CPU cycle
  while (true) {
    if (rv_uint32 err = riscv_cycle(&cpu)) {
      std::cout << "Error: " << err << std::endl;
      return err;
    }
  }

  return 0;
}