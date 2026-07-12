#include <stdio.h>
#include "quantum_rng.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
int main(int argc,char**argv){
  uint8_t seed[4]={0xDE,0xAD,0xBE,0xEF};
  qrng_ctx*c=NULL; qrng_init(&c,seed,4);
  uint8_t b[8]; qrng_bytes(c,b,8);
  for(int i=0;i<8;i++)printf("%02x",b[i]);
  printf(" pid=%d\n",(int)getpid());
  qrng_free(c); return 0;
}
