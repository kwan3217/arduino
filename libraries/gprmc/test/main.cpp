#include <cstdio>
#include "gprmc.h"
#include "Arduino.h"


int main(int argc, char** argv) {
  GPRMC gps;
  FILE* inf=fopen(argv[1],"rb");
  while(!feof(inf)) {
    char c;
    fread(&c,1,1,inf);
    if(gps.process(c)) {
        printf("Time: %04d-%02d-%02d %02d:%02d:%06.3f\n",gps.y,gps.m,gps.d,gps.h,gps.n,gps.s);
        printf("Lat: %f   Lon: %f\n",gps.lat,gps.lon);
    }
  }
  fclose(inf);
}
