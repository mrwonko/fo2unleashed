#include <iostream>

extern "C" int getGlobal();

extern "C" int g_global = 42;

int main( int argc, char** argv )
{
  std::cout << getGlobal() << std::endl;
  return 0;
}
