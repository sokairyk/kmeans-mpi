#include <iostream>
#include <fstream>
#include <time.h>
#include <stdlib.h>
using namespace std;

int main () {
  ofstream random_seed ("random");
  if (random_seed.is_open())
  {
    srand(time(NULL));

    for (int i = 0; i < 30; i++)
            for (int j = 0; j < 100000; j++)
                random_seed << (float)rand() / static_cast<float>(RAND_MAX) << endl;
    
    random_seed.close();
  }
  else cout << "Unable to open file: random";
  return 0;
}

