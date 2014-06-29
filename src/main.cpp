#include <hues_logic.hpp>

int main(int argc, char **argv) {
  HuesLogic h;
  if (!h.TryLoadRespack()) {
    exit(EXIT_FAILURE);
  }

  h.InitDisplay();
  h.PlaySong("Madeon - Finale");
}

