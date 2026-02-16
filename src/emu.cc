#include "emu.h"

static uint8_t random_byte() {
    return static_cast<uint8_t>(rand() % 256);
}

Chip::Chip(const char *rom) {
  std::fstream file(rom, std::ios::in | std::ios::binary);
  uint8_t bytes[2];
  
  constexpr uint8_t font[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80   // F
  };

  std::copy(font, font + sizeof(font), this->memory);

  file.read(reinterpret_cast<char*>(memory + 0x200), 
         4096 - 0x200);
}

Chip::Chip() {}
Chip::~Chip() {}

void Chip::step() {
  uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];

  uint8_t nibble = (opcode & 0xF000) >> 12;
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;
  uint8_t n = opcode & 0x000F;
  uint8_t nn = opcode & 0x00FF;
  uint16_t nnn = opcode & 0x0FFF;

  bool inc = true;
  switch (nibble) {
    case 0x0:
      if (opcode == 0x00E0) {
        std::fill(this->screen, this->screen + 64 * 32, 0);
      } else if (opcode == 0x00EE) {
        this->pc = this->stack[this->sp];
        this->sp--;
        inc = false;
      }
      break;
    case 0x1:
      this->pc = nnn;
      inc = false;
      break;
    case 0x2:
      if (this->sp >= 16) {
        break;
      }
      this->sp++;
      this->stack[this->sp] = this->pc + 2;
      this->pc = nnn;
      inc = false;
      break;
    case 0x3:
      if (this->v[x] == nn) this->pc += 2;
      break;
    case 0x4:
      if (this->v[x] != nn) this->pc += 2;
      break;
    case 0x5:
      if (this->v[x] == this->v[y]) this->pc += 2;
      break;
    case 0x6:
      this->v[x] = nn;
      break;
    case 0x7:
      this->v[x] += nn;
      break;
    case 0x8: {
      switch (n) {
        case 0x0:
          this->v[x] = this->v[y];
          break;
        case 0x1:
          this->v[x] |= this->v[y];
          break;
        case 0x2:
          this->v[x] &= this->v[y];
          break;
        case 0x3:
          this->v[x] ^= this->v[y];
          break;
        case 0x4: {
          uint16_t sum = this->v[x] + this->v[y];
          this->v[0xF] = (sum > 0xFF);
          this->v[x] = sum & 0xFF;
          break;
        }
        case 0x5: {
          this->v[0xF] = (this->v[x] >= this->v[y]);
          this->v[x] -= this->v[y];
          break;
        }
        case 0x6: {
          this->v[0xF] = this->v[x] & 0x1;
          this->v[x] >>= 1;
          break;
        }
        case 0x7: {
          this->v[0xF] = (this->v[y] >= this->v[x]);
          this->v[x] = this->v[y] - this->v[x];
          break;
        }
        case 0xE: {
          this->v[0xF] = (this->v[x] & 0x80) >> 7;
          this->v[x] <<= 1;
          break;
        }
      }
      break;
    }
    case 0x9:
      if (this->v[x] != this->v[y]) this->pc += 2;
      break;
    case 0xA:
      this->i = nnn;
      break;
    case 0xB:
      this->pc = nnn + this->v[0];
      inc = false;
      break;
    case 0xC:
      this->v[x] = random_byte() & nn;
      break;
    case 0xD: {
      const uint8_t x_pos = this->v[x] % 64;
      const uint8_t y_pos = this->v[y] % 32;
      this->v[0xF] = 0;

      for (uint8_t row = 0; row < n && y_pos + row < 32; row++) {
        uint8_t sprite_byte = this->memory[this->i + row];
        for (uint8_t col = 0; col < 8 && x_pos + col < 64; col++) {
          uint8_t pixel = (sprite_byte >> (7 - col)) & 0x1;
          uint16_t screen_pos = (y_pos + row) * 64 + (x_pos + col);

          if (pixel && this->screen[screen_pos]) {
            this->v[0xF] = 1;
          }

          this->screen[screen_pos] ^= pixel;
        }
      }
      break;
    }
    case 0xE:
      if (nn == 0x9E) {
        if (this->keypad[this->v[x]]) this->pc += 2;
      } else if (nn == 0xA1) {
        if (!this->keypad[this->v[x]]) this->pc += 2;
      }
      break;
    case 0xF: {
      switch (nn) {
        case 0x07:
          this->v[x] = this->dt;
          break;
        case 0x0A: {
          bool key_pressed = false;
          for (uint8_t i = 0; i < 16; i++) {
            if (this->keypad[i]) {
              this->v[x] = i;
              key_pressed = true;
              break;
            }
          }
          if (!key_pressed) {
            this->pc -= 2;
            inc = false;
          }
          break;
        }
        case 0x15:
          this->dt = this->v[x];
          break;
        case 0x18:
          this->st = this->v[x];
          break;
        case 0x1E:
          this->i += this->v[x];
          break;
        case 0x29:
          this->i = this->v[x] * 5;
          break;
        case 0x33: {
          uint8_t value = this->v[x];
          this->memory[this->i] = value / 100;
          this->memory[this->i + 1] = (value / 10) % 10;
          this->memory[this->i + 2] = value % 10;
          break;
        }
        case 0x55:
          for (uint8_t i = 0; i <= x; i++) {
            this->memory[this->i + i] = this->v[i];
          }
          break;
        case 0x65:
          for (uint8_t i = 0; i <= x; i++) {
            this->v[i] = this->memory[this->i + i];
          }
          break;
      }
      break;
    }
  }

  if (inc) this->pc += 2;
}
