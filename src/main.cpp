#include <cstdlib>
#include <ctime>

#include <hues_logic.hpp>

static const char* pick[] {
  "Madeon - Finale",
  "Aphex Twin - Vordhosbn",
  "Outlaw Star OST - Desire"
};

int main(int argc, char **argv) {
  HuesLogic h;
  if (!h.TryLoadRespack()) {
    exit(EXIT_FAILURE);
  }

  srand(time(NULL));

  h.InitDisplay();
  h.PlaySong(pick[rand() % (sizeof(pick) / sizeof(char**))]);
}

