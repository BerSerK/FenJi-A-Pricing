#include <iostream>
#include <string>

#include "timer.h"
#include "Random.h"
#include "FenJiA.h"

int main(int nargv, char *argc[] ) {
  if ( nargv == 1 ) {
    FJASimulator sim("config/config.txt");
    sim.Run();
  } else if ( nargv == 2 ) {
    FJASimulator sim( argc[1] );
    sim.Run();
  }

  return 0;
}
