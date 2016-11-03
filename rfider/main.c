/**
 * 
 * This file is part of RFIDer.
 * RFIDer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * RFIDer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with RFIDer.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

/**
 * File:   main.c
 * Author: Davide Airaghi - http://www.airaghi.net
 * License: GPLv3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "rfider.h"

int fdevice = 0;

/*
 * Main program entrypoint
 */
int main(int argc, char** argv) {
    char *dev = NULL;
    char *prg = NULL;
    int set_daemon = 0;
    switch (argc) {
            // one parameter received
        case 2:
            if (strcmp(argv[1], "-h") == 0) {
                printf("\n");
                printf("RFIDer version %s\n\n", VERSION);
                printf("RFIDer is a simple program, running as a foreground process or a daemon,\n");
                printf("       that gets exclusive access to an input \"event device\" working as a keyboard\n");
                printf("       in order to read digits from it (as keypresses!) and send them, as standard\n");
                printf("       input, to a specified program.\n\n");
                printf("Usage:\n");
                printf("- rfider -h         : help\n");
                printf("- rfider -k         : kill any rfider instace running\n");
                printf("- rfider DEV PRG    : get exclusive access to device DEV (fullpath) and \n");
                printf("                      send characters to program PRG (fullpath)\n");
                printf("- rfider -d DEV PRG : same behaviour as the previous case but rfider will \n");
                printf("                      run as a daemon\n");
                printf("\n\n");
                return EXIT_SUCCESS;
            } else if (strcmp(argv[1], "-k") == 0) {
                return system("killall -s QUIT rfider");
            } else {
                printf("\n");
                printf("RFIDer: Wrong number of parameters. Exeute rfider -h to get usage info.\n");
                printf("\n");
                return EXIT_FAILURE;
            }
            break;
            // three parameters received
        case 4:
            if (strcmp(argv[1], "-d") == 0) {
                set_daemon = 1;
                dev = argv[2];
                prg = argv[3];
            }
            // two parameters received
        case 3:
            if (dev == NULL) {
                dev = argv[1];
            }
            if (prg == NULL) {
                prg = argv[2];
            }
            return run(dev, prg, set_daemon);
            break;
            // other 
        default:
            printf("\n");
            printf("RFIDer: Wrong number of parameters. Exeute rfider -h to get usage info.\n");
            printf("\n");
            return EXIT_FAILURE;
            break;
    }
    return EXIT_SUCCESS;
}

/**
 * do the job!
 * @param dev
 * @param prg
 * @param set_daemon
 * @return int
 */
int run(const char *dev, const char *prg, const int set_daemon) {
    int result;
    int fprg;
    int shift_down;
    int caps_on;
    struct input_event ev[BUFFER_SIZE];
    int size = sizeof (struct input_event);
    char buffer[MAX_INPUT_SIZE+1] = "";
    memset(buffer,0,MAX_INPUT_SIZE+1);
    if (set_daemon > 0) {
        if (daemon(0,0)!=0) {
            printf("\n");
            printf("RFIDer: Unable to daemonize\n");
            printf("\n");
            return EXIT_FAILURE;
        }
    }
    fdevice = open(dev, O_RDONLY);
    if (fdevice <= 0) {
        printf("\n");
        printf("RFIDer: Unable to open device %s\n", dev);
        printf("\n");
        return EXIT_FAILURE;
    }
    fprg = open(prg, O_RDONLY);
    if (fprg <= 0) {
        printf("\n");
        printf("RFIDer: Unable to find program %s\n", prg);
        printf("\n");
        close(fdevice);
        return EXIT_FAILURE;
    }
    close(fprg);
    registerSignals();
#if DEBUG <= 0
    result = ioctl(fdevice, EVIOCGRAB, 1);
    if (result != 0) {
        printf("\n");
        printf("RFIDer: Unable to get exclusive access to device %s\n", dev);
        printf("\n");
        close(fdevice);
        return EXIT_FAILURE;
    }
#endif
    caps_on    = 0;
    shift_down = 0;
    while (1) {
        int n = read(fdevice, &ev, size * BUFFER_SIZE);
        int val = 0;
        if (n != (ssize_t) - 1) {
            if (n >= size) {
                val = ev[0].value;
                if (val != ' ' && ev[1].value == 1 && ev[1].type == EV_KEY) {
                    val = ev[1].code;
                    if (val == KEY_ENTER || strlen(buffer)==MAX_INPUT_SIZE) {
                        sendBuffer(buffer,prg);
                        memset(buffer,0,MAX_INPUT_SIZE+1);
                    } else {
                        char ch;
                        switch (val) {
                            case KEY_1 : ch = '1'; break;
                            case KEY_2 : ch = '2'; break;
                            case KEY_3 : ch = '3'; break;
                            case KEY_4 : ch = '4'; break;
                            case KEY_5 : ch = '5'; break;
                            case KEY_6 : ch = '6'; break;
                            case KEY_7 : ch = '7'; break;
                            case KEY_8 : ch = '8'; break;
                            case KEY_9 : ch = '9'; break;
                            case KEY_0 : ch = '0'; break;
                        }
                        buffer[strlen(buffer)] = ch;
                    }
                }
            }
        }
    }
}

void sendBuffer(const char *buffer,const char *prgname) {
    // printf("\n\nSCANNED: %s --> %s\n\n",buffer,prgname);
    FILE *x = popen(prgname,"ew");
    if (x) {
	fputs(buffer,x);
        pclose(x);
    } else {
        printf("RFIDer: unable to exec %s\n",prgname);
    }
}

/**
 * handle received signals
 * @param signal_number
 */
void signalHandler(int signal_number) {
    switch (signal_number) {
        case SIGQUIT:
        case SIGKILL:
        case SIGUSR1:
            // release exclusive access to device
            if (fdevice > 0) {
                int result = ioctl(fdevice, EVIOCGRAB, 0);
                close(fdevice);
            }
            exit(EXIT_SUCCESS);
            break;
        default:
            // printf("SIGNAL %d", signal_number);
            break;
    }
}

/**
 * register signal hanlders
 */
void registerSignals() {
    signal(SIGQUIT, signalHandler);
    signal(SIGKILL, signalHandler);
    signal(SIGUSR1, signalHandler);
}

