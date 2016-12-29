/**
 *
 * This file is part of MultiCopy
 * MultiCopy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MultiCopy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with MultiCopy.  If not, see <http://www.gnu.org/licenses/>
 *
 */

/**
 * File:   main.c
 * Author: Davide Airaghi
 * License: GPLv3
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

// Default block size
#define DEFAULT_BLOCKSIZE 4096

// Max Block size
#define MAX_BLOCKSIZE (1024*1024*16)
// Min Block size
#define MIN_BLOCKSIZE 512

// return codes for copy process
#define COPY_SUCCESS       0
#define COPY_ERROR_MEMORY -1
#define COPY_ERROR_READ   -2
#define COPY_ERROR_PERMS  -3
#define COPY_ERROR_OPEN   -4
#define COPY_ERROR_WRITE  -5
#define COPY_FILE_OVERLAP -6

/**
 * get best blocksize for given path
 * @param  string path
 * @return int
 */
int getBlockSize(const char *path) {
    struct stat fi;
    int size;
    if (stat(path, &fi) < 0) {
        char *newPath = strdup(path);
        char *dirName = dirname(newPath);
        if (stat(dirName, &fi) < 0) {
            free(newPath);
            return DEFAULT_BLOCKSIZE;
        }
        free(newPath);
        dirName = NULL;
    }
    size = (int) fi.st_blksize;
    return (size > 0 ? size : DEFAULT_BLOCKSIZE);
}

/**
 * execute the multi copy operation
 * @param  char*   src
 * @param  char**  dst
 * @param  int     tot
 * @return int
 */
int execMultiCopy(const char* src, const char**dst, int tot, int force_blocksize) {
    int blocksize = -1;
    int size;
    int i, j;
    FILE **fdst;
    FILE *fsrc;
    char *buffer;
    unsigned long fileSize;
    struct stat srcStat;
    if (force_blocksize >= MIN_BLOCKSIZE && force_blocksize <= MAX_BLOCKSIZE) {
        // if we have a valid "forced" block size use it
        size = blocksize = force_blocksize;
    } else {
        // get a common minimun block size
        for (i = 0; i < tot; i++) {
            size = getBlockSize(dst[i]);
            if (blocksize < 0 || size < blocksize) {
                blocksize = size;
            }
        }
    }
    // allocate buffer
    buffer = malloc(blocksize);
    if (buffer == NULL) {
        return COPY_ERROR_MEMORY;
    }
    // open source file
    fsrc = fopen(src, "rb");
    if (fsrc == NULL) {
        free(buffer);
        return COPY_ERROR_READ;
    }
    stat(src, &srcStat);
    fileSize = (unsigned long) srcStat.st_size;
    // check if destinations are writable
    for (i = 0; i < tot; i++) {
        if (access(dst[i], W_OK) != 0) {
            char *newPath = strdup(dst[i]);
            char *dirName = dirname(newPath);
            if (access(dirName, W_OK) != 0) {
                dirName = NULL;
                free(newPath);
                fclose(fsrc);
                free(buffer);
                return COPY_ERROR_PERMS;
            }
            dirName = NULL;
            free(newPath);
        }
    }
    // open destination files
    fdst = calloc(sizeof (FILE*), tot);
    if (fdst == NULL) {
        fclose(fsrc);
        free(buffer);
        return COPY_ERROR_MEMORY;
    }
    for (i = 0; i < tot; i++) {
        fdst[i] = fopen(dst[i], "wb");
        if (fdst[i] == NULL) {
            for (j = 0; j < i; j++) {
                fclose(fdst[j]);
            }
            fclose(fsrc);
            free(fdst);
            free(buffer);
            return COPY_ERROR_OPEN;
        }
    }
    // copy data
    int countBytes;
    while (countBytes = fread(buffer, 1, blocksize, fsrc)) {
        fileSize -= countBytes;
        if (ferror(fsrc)) {
            break;
        }
        if (countBytes == 0 && fileSize > 0) {
            countBytes = fileSize;
        }
        for (i = 0; i < tot; i++) {
            if (fwrite(buffer, countBytes, 1, fdst[i]) == 0) {
                for (i = 0; i < tot; i++) {
                    fclose(fdst[i]);
                }
                free(fdst);
                fclose(fsrc);
                free(buffer);
                return COPY_ERROR_WRITE;
            }
        }
        if (feof(fsrc)) {
            break;
        }
    }
    if (!feof(fsrc)) {
        for (i = 0; i < tot; i++) {
            fclose(fdst[i]);
        }
        free(fdst);
        fclose(fsrc);
        free(buffer);
        return COPY_ERROR_READ;
    }
    // close files and clean up
    for (i = 0; i < tot; i++) {
        fclose(fdst[i]);
        // try to set file perms
        chmod(dst[i], srcStat.st_mode);
        // try to set file owner
        chown(dst[i], srcStat.st_uid, srcStat.st_gid);
    }
    free(fdst);
    fclose(fsrc);
    free(buffer);
    // exit
    return COPY_SUCCESS;
}

void usage(const char *msg) {
    if (msg!=NULL) {
        printf("\n");
        printf("%s\n",msg);
        printf("\n");
    }
    printf("\n");
    printf("MultiCopy (mcp) copies a single file to a specified set of destination directories (which have to exist)\n");
    printf("or destination files (which have to be created if they do not exist, otherwise they will be overwritten).\n");
    printf("\n");
    printf("Usage examples:\n");
    printf("mcp file1 dest1 dest2 : copy file1 to both dest1 and dest2\n");
    printf("mcp -b 2048 file1 dest1 dest2: copy file1 to both dest1 and dest2 reading 2048 bytes at time\n");
    printf("\n");
    printf("If the parameter -- is used each element after it will be considered as a filename\n");
    printf("Example: mcp -- -b dest1 dest2 : copy a file named -b to dest1 and to dest2\n");
    printf("\n");
}

/**
 * main program entrypoint
 * @param int    argc
 * @param char** argv
 * @return int
 */
int main(int argc, char** argv) {
    if (argc <= 1) {
        usage("Error: Too few parameters");
        return EXIT_SUCCESS;
    }
    int blocksize = -1;
    int ok      = 1;
    int ret     = 0;
    int tot     = 0;
    int i,j     = 0;
    int files   = 0;
    char *buf   = NULL;
    char *src   = NULL;
    char *rpath = NULL;
    char *dst[argc - 1];
    struct stat statInfo;
    // parse parameters
    for (i=1;i<argc;i++) {
        buf = argv[i];
        // check "force files" flag
        if (strcmp(buf,"--")==0) {
            files = 1;
            continue;
        }
        // check "force block size" flag
        if (strcmp(buf,"-b")==0 && !files) {
            if (i < argc-1) {
                blocksize = atoi(argv[i+1]);
                if (blocksize>0 && blocksize >= MIN_BLOCKSIZE && blocksize <= MAX_BLOCKSIZE) {
                    i++;
                    continue;
                } else {
                    if (src!=NULL) {
                        free(src);
                    }
                    for (j=0;j<tot-1;j++) {
                        if (dst[j]!=NULL) {
                            free(dst[j]);
                        }
                    }
                    usage("Error: invalid blocksize given");
                    return EXIT_FAILURE;
                }
            } else {
                if (src!=NULL) {
                    free(src);
                }
                for (j=0;j<tot-1;j++) {
                    if (dst[j]!=NULL) {
                        free(dst[j]);
                    }
                }
                usage("Error: no blocksize given");
                return EXIT_FAILURE;
            }
            break;
        }
        // set files - source
        if (src==NULL) {
            rpath = realpath(buf,NULL);
            src = rpath;
            continue;
        }
        // set files - destination
        tot ++;
        if (stat(buf,&statInfo)==0) {
            if (S_ISDIR(statInfo.st_mode)) {
                char *tmp;
                tmp = malloc( strlen(buf)+1+strlen(src)+1 );
                if (tmp == NULL) {
                    free(src);
                    for (j=0;j<tot-1;j++) {
                        if (dst[j]!=NULL) {
                            free(dst[j]);
                        }
                    }
                    usage("Error: unable to allocate enough system memory for parameters' analysis");
                    return EXIT_FAILURE;
                }
                bzero(tmp,strlen(buf)+1+strlen(src)+1);
                strcat(tmp, buf);
                strcat(tmp,"/");
                strcat(tmp,basename(src));
                rpath = realpath(tmp,NULL);
                free(tmp);
                dst[tot-1] = rpath;
                continue;
            }
        }
        dst[tot-1] = strdup(buf);
    }
    // sanity checks
    if (tot==0) {
        usage("Error: No destinations given/found");
        return EXIT_FAILURE;
    }
    for (i=0;i<tot;i++) {
        if (strcmp(src,dst[i])==0) {
            ok = 0;
            break;
        }
        for (j=0;j<i;j++) {
            if (strcmp(dst[j],dst[i])==0) {
                ok = 0;
                break;
            }
        }
        if (ok==0) {
            break;
        }
    }
    if (ok>0) {
        // execute copy
        ret = execMultiCopy((const char *)src, (const char **)dst, tot, blocksize);
    } else {
        ret = COPY_FILE_OVERLAP;
    }
    // cleanup
    free(src);
    for (i=0;i<tot;i++) {
        if (dst[i]!=NULL) {
            free(dst[i]);
        }
    }
    // handle copy execution status
    switch (ret) {
        case COPY_ERROR_MEMORY :
            usage("Error: unable to allocate enough system memory for buffer creation");
            return EXIT_FAILURE;
            break;
        case COPY_ERROR_OPEN :
            usage("Error: unable to open destination files");
            return EXIT_FAILURE;
            break;
        case COPY_ERROR_PERMS :
            usage("Error: destination files are not writable");
            return EXIT_FAILURE;
            break;
        case COPY_ERROR_READ :
            usage("Error: unable to read source file");
            return EXIT_FAILURE;
            break;
        case COPY_ERROR_WRITE :
            usage("Error: unable to write data to destination files (check them!)");
            return EXIT_FAILURE;
            break;
        case COPY_FILE_OVERLAP :
            usage("Error: make sure source file is not specified also as destination and that all destinations are unique");
            return EXIT_FAILURE;
            break;
        case COPY_SUCCESS :
        default:
            break;
    }
    // everything is fine :-)
    return (EXIT_SUCCESS);
}

