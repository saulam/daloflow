
/*
 *
 *  Copyright 2019-2021 Jose Rivadeneira Lopez-Bravo, Saul Alonso Monsalve, Felix Garcia Carballeira, Alejandro Calderon Mateos
 *
 *  This file is part of DaLoFlow.
 *
 *  DaLoFlow is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DaLoFlow is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with DaLoFlow.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// TODO:
// * Ahora solo para ficheros con un bloque

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <pthread.h>
#include "hdfs.h"

#define BUFFER_SIZE   (4 * 1024 * 1024)
#define BLOCKSIZE    (64 * 1024 * 1024)


/*
 * Copy from HDFS
 */

int copy_from_to ( hdfsFS fs, char *file_name_org, char *file_name_dst, long buffer_size )
{
     // local buffer for future threads
     unsigned char *buffer ;

     buffer = malloc(buffer_size) ;
     if (NULL == buffer) {
#ifdef DEBUG
         fprintf(stderr, "ERROR: malloc for '%ld'.\n", buffer_size) ;
#endif
         return -1 ;
     }

     /* Read data from HDFS */
     hdfsFile read_file = hdfsOpenFile(fs, file_name_org, O_RDONLY, 0, 0, 0) ;
     if (!read_file) {
         free(buffer) ;
#ifdef DEBUG
         fprintf(stderr, "ERROR: hdfsOpenFile on '%s' for reading.\n", file_name_org) ;
#endif
         return -1 ;
     }

     tSize num_readed_bytes = 0 ;
     tSize read_remaining_bytes = buffer_size ;
     while (read_remaining_bytes > 0)
     {
         num_readed_bytes = hdfsRead(fs, read_file,
                                     (void*)buffer + (buffer_size - read_remaining_bytes),
                                     read_remaining_bytes) ;
         if (num_readed_bytes == -1) {
             free(buffer) ;
#ifdef DEBUG
             fprintf(stderr, "ERROR: hdfsRead fails to read data.\n") ;
#endif
             return -1 ;
         }
         if (num_readed_bytes == 0) {
#ifdef DEBUG
             fprintf(stderr, "WARNING: file smaller than %ld bytes.\n", buffer_size) ;
#endif
	     buffer_size = buffer_size - read_remaining_bytes ;
	     break ;
         }

         read_remaining_bytes -= num_readed_bytes ;
     }

     hdfsCloseFile(fs, read_file) ;

     /* Write data to local file */
     int write_fd = open(file_name_dst, O_WRONLY | O_CREAT, 0700) ;
     if (write_fd < 0) {
         free(buffer) ;
#ifdef DEBUG
         fprintf(stderr, "ERROR: open fails to create '%s' file.\n", file_name_dst) ;
#endif
         return -1 ;
     }

     ssize_t write_num_bytes = 0 ;
     ssize_t write_remaining_bytes = buffer_size ;
     while (write_remaining_bytes > 0)
     {
         write_num_bytes = write(write_fd,
                                 (void*)buffer + (buffer_size - write_remaining_bytes),
                                 write_remaining_bytes) ;
         if (write_num_bytes == -1) {
             free(buffer) ;
#ifdef DEBUG
             fprintf(stderr, "ERROR: write fails to write data.\n") ;
#endif
             return -1 ;
         }

         write_remaining_bytes -= write_num_bytes ;
     }

     close(write_fd) ;

     // Free and return
     free(buffer) ;
     return 0 ;
}


/*
 * Threads
 */

#define NUM_THREADS  (8)
pthread_t threads[NUM_THREADS] ;

int sync_copied = 0 ;
pthread_cond_t sync_cond ;
pthread_mutex_t sync_mutex ;

struct th_args {
       hdfsFS    fs ;
       char      file_name_org   [PATH_MAX] ;
       char      machine_name    [HOST_NAME_MAX + 1] ;
       char      destination_dir [PATH_MAX] ;
} ;

void * th_copy_from_hdfs_to_local ( void *arg )
{
       int             ret ;
       char          * msg ;
       struct th_args  thargs ;
       char            file_name_dst[2*PATH_MAX] ;
       char            file_name_org[2*PATH_MAX] ;
       char              ln_name_org[2*PATH_MAX] ;
       char        *** blocks_information;
       int             is_remote ;

       // Copy arguments
       pthread_mutex_lock(&sync_mutex) ;
       memmove((char *)&thargs, (char *)arg, sizeof(struct th_args)) ;
       sync_copied = 1 ;
       pthread_cond_signal(&sync_cond) ;
       pthread_mutex_unlock(&sync_mutex) ;

       // Get HDFS information
       blocks_information = hdfsGetHosts(thargs.fs, thargs.file_name_org, 0, BLOCKSIZE) ;
       if (NULL == blocks_information) {
#ifdef DEBUG
           fprintf(stderr, "ERROR: hdfsGetHosts for '%s'.\n", thargs.file_name_org) ;
#endif
           pthread_exit((void *)0) ;
       }

       // Set the initial org/dst file name...
       sprintf(file_name_org, "%s",    basename(thargs.file_name_org)) ;
       sprintf(file_name_dst, "%s/%s", thargs.destination_dir, file_name_org) ;

       // If local file then symlink, else copy from hdfs
       is_remote = strncmp(thargs.machine_name, blocks_information[0][0], strlen(thargs.machine_name)) ;
       if (0 == is_remote)
       {
           sprintf(ln_name_org, "%s/../fuse/%s", thargs.destination_dir, file_name_org) ;
           ret = symlink(file_name_dst, ln_name_org) ;
           if (ret < 0) { perror("symlink: ") ; }
       }
       else
       {
           ret = copy_from_to(thargs.fs, thargs.file_name_org, file_name_dst, BUFFER_SIZE) ;
       }

       // Show message...
       msg = (ret < 0) ? "Error found" : "Done" ;
#ifdef DEBUG
       printf("'%s' from node '%s' to node '%s': %s\n",
              thargs.file_name_org,
              blocks_information[0][0],
              thargs.machine_name,
              msg) ;
#endif

       // The End
       pthread_exit((void *)(long)ret) ;
}


/*
 * Read string from file
 */

char * fgets2 ( FILE *fd, char *str, int len )
{
       bzero(str, len) ;
       char *ret1 = fgets(str, len-1, fd) ;
       if (NULL == ret1) {
           return NULL ;
       }

       str[strlen(str)-1] = '\0' ;
       return str ;
}


// Main
int main ( int argc, char* argv[] )
{
    struct th_args th_args ;
    int            num_threads ;
    char  *ret1 ;
    int    ret2 ;
    void  *retval ;

    // Check arguments
    if (argc != 3) {
        printf("Usage: %s <file> <path>\n", argv[0]) ;
        exit(-1) ;
    }

    // Initialize th_args...
    bzero(&th_args, sizeof(struct th_args)) ;
    strcpy(th_args.destination_dir, argv[2]) ;
    gethostname(th_args.machine_name, HOST_NAME_MAX + 1) ;

    // HFDS connect
    th_args.fs = hdfsConnect("default", 0) ;
    if (NULL == th_args.fs) {
        perror("hdfsConnect: ") ;
        exit(-1) ;
    }

    // Open listing file
    FILE *list_fd = fopen(argv[1], "ro") ;
    if (NULL == list_fd) {
        hdfsDisconnect(th_args.fs) ;
        perror("fopen: ") ;
        exit(-1) ;
    }

    do
    {
       for (num_threads=0; num_threads<NUM_THREADS; num_threads++)
       {
            // Get file name from listing
            ret1 = fgets2(list_fd, th_args.file_name_org, sizeof(th_args.file_name_org)) ;
            if (NULL == ret1) {
                break ; // end for loop...
            }

            // Create thread...
            ret2 = pthread_create(&(threads[num_threads]), NULL, th_copy_from_hdfs_to_local, (void *)&(th_args)) ;
            if (ret2 != 0) {
                perror("pthread_create: ") ;
                exit(-1) ;
            }

            pthread_mutex_lock(&sync_mutex) ;
            while (sync_copied == 0) {
                   pthread_cond_wait(&sync_cond, &sync_mutex) ;
            }
            sync_copied = 0 ;
            pthread_mutex_unlock(&sync_mutex) ;
       }

       for (int i=0; i<num_threads; i++)
       {
            pthread_join(threads[i], &retval) ;
       }

    } while (ret1 != NULL) ;

    hdfsDisconnect(th_args.fs) ;
    fclose(list_fd) ;

    // The end
    return 0;
}
