#ifndef __DANA_SRV_H__

struct Globals {
  struct options {
    int count;
    int trace;
    int verbose;  
    char *ifname;
  } opts;
};

extern struct Globals  globals;

#define S1(x) #x
#define S2(x) S1(x)
#define LOC_INFO __FILE__ " : " S2(__LINE__)


#endif
