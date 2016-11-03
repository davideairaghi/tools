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
 * File:   rfider.h
 * Author: Davide Airaghi - http://www.airaghi.net
 * License: GPLv3
 */

#ifndef RFIDER_H
#define RFIDER_H

#ifdef __cplusplus
extern "C" {
#endif


#define VERSION "0.1"
#define BUFFER_SIZE 64
#define MAX_INPUT_SIZE 4096
#define DEBUG 0

#define K_RELEASED 0
#define K_PRESSED  1
#define K_REPEATED 2

static const char *const evval[3] = {
    "RELEASED",
    "PRESSED ",
    "REPEATED"
};

int run(const char *dev, const char *prg, const int daemon);
void signalHandler(int signal_number);
void registerSignals();
void sendBuffer(const char *buffer,const char *prgname);

#ifdef __cplusplus
}
#endif

#endif /* RFIDER_H */

