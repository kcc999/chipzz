#include <raylib.h>
#include "emu.h"

#include <stdint.h>
#include <array>
#include <algorithm>
#include <chrono>

enum State
{
  MENU,
  PAUSE,
  PLAYING
};

int main(int argc, char *argv[])
{
  constexpr int32_t base_screen_w = 64;
  constexpr int32_t base_screen_h = 32;
  constexpr std::array<int32_t, 6> scale_steps = {8, 10, 12, 14, 16, 20};
  constexpr uint8_t instructions_per_frame = 10;
  constexpr int32_t help_font_size = 12;
  constexpr int32_t help_padding = 8;

  Chip chip(argv[1]);
  auto state = State::PLAYING;
  bool show_controls = true;
  int32_t scale_index = 1; // default 10x
  int32_t pixel_scale = scale_steps[scale_index];
  int32_t screen_w = base_screen_w * pixel_scale;
  int32_t screen_h = base_screen_h * pixel_scale;

  InitWindow(screen_w, screen_h, "chipzz");
  SetWindowMinSize(base_screen_w * scale_steps.front(), base_screen_h * scale_steps.front());
  InitAudioDevice();
  SetTargetFPS(60);

  Sound buzzer_wav = LoadSound("resources/game_buzzer.wav");

  auto last_time = std::chrono::high_resolution_clock::now();

  const char *l1 = "TAB: Pause/Resume";
  const char *l2 = "H/F1: Hide help";
  const char *l3 = "-/+: Scale";
  const char *l4 = "1-4, Q-R, A-F, Z-V: Keys";
  const char *l5 = "ESC: Quit";

  while (!WindowShouldClose())
  {

    if (IsKeyPressed(KEY_TAB))
    {
      if (state == State::PAUSE)
      {
        state = State::PLAYING;
      }
      else
      {
        state = State::PAUSE;
      }
    }
    if (IsKeyPressed(KEY_H) || IsKeyPressed(KEY_F1))
    {
      show_controls = !show_controls;
    }
    if ((IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) &&
        scale_index < static_cast<int32_t>(scale_steps.size() - 1))
    {
      scale_index++;
      pixel_scale = scale_steps[scale_index];
      screen_w = base_screen_w * pixel_scale;
      screen_h = base_screen_h * pixel_scale;
      SetWindowSize(screen_w, screen_h);
    }
    if ((IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) && scale_index > 0)
    {
      scale_index--;
      pixel_scale = scale_steps[scale_index];
      screen_w = base_screen_w * pixel_scale;
      screen_h = base_screen_h * pixel_scale;
      SetWindowSize(screen_w, screen_h);
    }

    if (state == State::PLAYING)
    {
      chip.keypad[0x1] = IsKeyDown(KEY_ONE);
      chip.keypad[0x2] = IsKeyDown(KEY_TWO);
      chip.keypad[0x3] = IsKeyDown(KEY_THREE);
      chip.keypad[0xC] = IsKeyDown(KEY_FOUR);
      chip.keypad[0x4] = IsKeyDown(KEY_Q);
      chip.keypad[0x5] = IsKeyDown(KEY_W);
      chip.keypad[0x6] = IsKeyDown(KEY_E);
      chip.keypad[0xD] = IsKeyDown(KEY_R);
      chip.keypad[0x7] = IsKeyDown(KEY_A);
      chip.keypad[0x8] = IsKeyDown(KEY_S);
      chip.keypad[0x9] = IsKeyDown(KEY_D);
      chip.keypad[0xE] = IsKeyDown(KEY_F);
      chip.keypad[0xA] = IsKeyDown(KEY_Z);
      chip.keypad[0x0] = IsKeyDown(KEY_X);
      chip.keypad[0xB] = IsKeyDown(KEY_C);
      chip.keypad[0xF] = IsKeyDown(KEY_V);

      auto current_time = std::chrono::high_resolution_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_time).count();

      if (elapsed >= 16)
      {
        if (chip.dt > 0)
          chip.dt--;
        if (chip.st > 0)
        {
          PlaySound(buzzer_wav);
          chip.st--;
        }

        last_time = current_time;
      }

      for (int i = 0; i < instructions_per_frame; i++)
        chip.step();
    }

    BeginDrawing();
    ClearBackground(BLACK);

    for (int y = 0; y < 32; y++)
    {
      for (int x = 0; x < 64; x++)
      {
        int index = y * 64 + x;
        bool pixel = chip.screen[index];
        if (pixel == true)
        {
          DrawRectangle(x * pixel_scale, y * pixel_scale, pixel_scale, pixel_scale, RAYWHITE);
        }
      }
    }

    if (show_controls)
    {
      const char *scale_line = TextFormat("Scale: %dx (%dx%d)", pixel_scale, screen_w, screen_h);
      int box_w = 0;
      box_w = std::max(box_w, MeasureText(l1, help_font_size));
      box_w = std::max(box_w, MeasureText(l2, help_font_size));
      box_w = std::max(box_w, MeasureText(l3, help_font_size));
      box_w = std::max(box_w, MeasureText(l4, help_font_size));
      box_w = std::max(box_w, MeasureText(l5, help_font_size));
      box_w = std::max(box_w, MeasureText(scale_line, help_font_size));
      box_w += help_padding * 2;

      const int line_h = help_font_size + 4;
      const int box_h = line_h * 6 + help_padding * 2;
      const int box_x = GetScreenWidth() - box_w - 6;
      const int box_y = GetScreenHeight() - box_h - 6;

      DrawRectangle(box_x, box_y, box_w, box_h, Fade(BLACK, 0.7f));
      DrawText(l1, box_x + help_padding, box_y + help_padding + line_h * 0, help_font_size, RAYWHITE);
      DrawText(l2, box_x + help_padding, box_y + help_padding + line_h * 1, help_font_size, RAYWHITE);
      DrawText(l3, box_x + help_padding, box_y + help_padding + line_h * 2, help_font_size, RAYWHITE);
      DrawText(l4, box_x + help_padding, box_y + help_padding + line_h * 3, help_font_size, RAYWHITE);
      DrawText(l5, box_x + help_padding, box_y + help_padding + line_h * 4, help_font_size, RAYWHITE);
      DrawText(scale_line, box_x + help_padding, box_y + help_padding + line_h * 5, help_font_size, RAYWHITE);
    }

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
