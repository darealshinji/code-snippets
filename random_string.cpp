#include <iostream>
#include <random>
#include <string>
#include <inttypes.h>

std::string random_str(uint32_t length)
{
  if (length == 0) return "";

  const char abc[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789";
  const uint8_t max = sizeof(abc)/sizeof(*abc) - 1;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint8_t> distrib(0, max);
  std::string rndm;
  rndm.reserve(length);

  for (uint32_t i=0; i < length; ++i) {
    rndm.push_back(abc[distrib(gen)]);
  }
  return rndm;
}

int main()
{
  std::cout << random_str(8) << std::endl;
  return 0;
}
