#include <iostream>
#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <easymedia/rkmedia_api.h>
#include <easymedia/rkmedia_vi.h>

int main(int argc, char** argv) {
#ifdef RKAIQ
    printf("RKAIQ IS ENABLED");
#endif

    return 0;
}
