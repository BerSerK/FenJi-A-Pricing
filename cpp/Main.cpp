#include <iostream>
#include <string>

#include "timer.h"
#include "Random.h"
#include "FenJiA.h"

int main() {
  FJASimulator sim("config/config.txt");
  sim.Run();
  return 0;
}
