
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include "hdfs.h"

#define BUFFER_SIZE   (     128 * 1024)
#define BLOCKSIZE    (64 * 1024 * 1024)

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    fprintf(stderr, fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif


/*
 * Copy from/to HDFS
 */

void mkdir_recursive ( const char *path )
{
     int    ret ;
     char  *subpath, *fullpath;
     struct stat s;

     // if directories exist then return
     ret = stat(path, &s);
     if (ret >= 0)
     {
         if (!S_ISDIR(s.st_mode)) {
             DEBUG_PRINT("ERROR: path '%s' is not a directory.\n", path) ;
         }

         return ;
     }

     // duplicate string (malloc inside)
     fullpath = strdup(path);

     // get last directory
     subpath = dirname(fullpath);
     if (strlen(subpath) > 1) {
         mkdir_recursive(subpath);
     }

     // mkdir last directory
     ret = mkdir(path, 0700);
     if (ret < 0) {
         DEBUG_PRINT("ERROR: creating directory '%s'.\n", path) ;
     }

     // free duplicate string
     free(fullpath);
}

int write_buffer ( hdfsFS fs, hdfsFile write_fd1, int write_fd2, void *buffer, int buffer_size, int num_readed_bytes )
{
     ssize_t write_num_bytes       = -1 ;
     ssize_t write_remaining_bytes = num_readed_bytes ;

     while (write_remaining_bytes > 0)
     {
	 // Write into hdfs (fs+write_fd1) or local file (write_fd2)...
	 if (fs != NULL) {
             write_num_bytes = hdfsWrite(fs, write_fd1,
				         buffer + (buffer_size - write_remaining_bytes),
				         write_remaining_bytes) ;
	 }
	 if (write_fd2 != -1) {
             write_num_bytes = write(write_fd2,
    	    		             buffer + (buffer_size - write_remaining_bytes),
			             write_remaining_bytes) ;
	 }

	 // check errors
         if (write_num_bytes == -1) {
	     DEBUG_PRINT("ERROR: write fails to write data.\n") ;
	     return -1 ;
         }

         write_remaining_bytes -= write_num_bytes ;
     }

     return 0 ;
}

int read_buffer ( hdfsFS fs, hdfsFile read_fd1, int read_fd2, void *buffer, int buffer_size )
{
     ssize_t read_num_bytes       = -1 ;
     ssize_t read_remaining_bytes = buffer_size ;

     while (read_remaining_bytes > 0)
     {
	 // Read from hdfs (fs+read_fd1) or local file (read_fd2)...
	 if (fs != NULL) {
             read_num_bytes = hdfsRead(fs, read_fd1,
				       buffer + (buffer_size - read_remaining_bytes),
				       read_remaining_bytes) ;
	 }
	 if (read_fd2 != -1) {
             read_num_bytes = read(read_fd2,
    	    		           buffer + (buffer_size - read_remaining_bytes),
			           read_remaining_bytes) ;
	 }

	 // check errors
         if (read_num_bytes == -1) {
	     DEBUG_PRINT("ERROR: read fails to read data.\n") ;
	     return -1 ;
         }

         read_remaining_bytes -= read_num_bytes ;
     }

     return 0 ;
}


int copy_from_hdfs_to_local ( hdfsFS fs, char *file_name_org, char *file_name_dst )
{
     // allocate intermediate buffer
     unsigned char *buffer = malloc(BUFFER_SIZE) ;
     if (NULL == buffer) {
         DEBUG_PRINT("ERROR: malloc for '%ld'.\n", BUFFER_SIZE) ;
         return -1 ;
     }

     // mkdir directory structure
     struct stat st = {0} ;
     char        basename_org[PATH_MAX] ;
     char       *dirname_org ;

     strcpy(basename_org, file_name_dst) ;
     dirname_org = dirname(basename_org) ;
     if (stat(dirname_org, &st) == -1) {
         mkdir_recursive(dirname_org) ;
     }

     // DEBUG
     DEBUG_PRINT("INFO: copy from '%s' to '%s' in '%s'...\n",
                 file_name_org, file_name_dst, dirname_org) ;

     /* Data from HDFS */
     hdfsFile read_file = hdfsOpenFile(fs, file_name_org, O_RDONLY, 0, 0, 0) ;
     if (!read_file) {
         free(buffer) ;
         DEBUG_PRINT("ERROR: hdfsOpenFile on '%s' for reading.\n", file_name_org) ;
         return -1 ;
     }

     /* Data to local file */
     int write_fd = open(file_name_dst, O_WRONLY | O_CREAT, 0700) ;
     if (write_fd < 0) {
         hdfsCloseFile(fs, read_file) ;
         free(buffer) ;
         DEBUG_PRINT("ERROR: open fails to create '%s' file.\n", file_name_dst) ;
         return -1 ;
     }

     /* Copy from HDFS to local */
     tSize num_readed_bytes = 0 ;
     int ret = -1 ;

     do
     {
         num_readed_bytes = read_buffer(fs, read_file, -1, (void *)buffer, BUFFER_SIZE) ;
         if (num_readed_bytes == -1) {
             break ;
         }
         ret = write_buffer(NULL, NULL, write_fd, (void *)buffer, BUFFER_SIZE, num_readed_bytes) ;
         if (ret == -1) {
             break ;
         }
     }
     while ( (ret != -1) && (num_readed_bytes > 0) ) ;

     // Free resources (ok)
     hdfsCloseFile(fs, read_file) ;
     close(write_fd) ;
     free(buffer) ;
     return ret ;
}

int copy_from_local_to_hdfs ( hdfsFS fs, char *file_name_org, char *file_name_dst )
{
     // allocate intermediate buffer
     unsigned char *buffer = malloc(BUFFER_SIZE) ;
     if (NULL == buffer) {
         DEBUG_PRINT("ERROR: malloc for '%ld'.\n", BUFFER_SIZE) ;
         return -1 ;
     }

     // mkdir directory structure
     char   basename_org[PATH_MAX] ;
     char  *dirname_org ;

     strcpy(basename_org, file_name_dst) ;
     dirname_org = dirname(basename_org) ;
     hdfsCreateDirectory(fs, dirname_org);

     // DEBUG
     DEBUG_PRINT("INFO: copy from '%s' to '%s' in '%s'...\n",
                 file_name_org, file_name_dst, dirname_org) ;

     /* Data from HDFS */
     hdfsFile write_fd = hdfsOpenFile(fs, file_name_org, O_WRONLY|O_CREAT, 0, 0, 0) ;
     if (!write_fd) {
         free(buffer) ;
         DEBUG_PRINT("ERROR: hdfsOpenFile fails to create '%s'.\n", file_name_org) ;
         return -1 ;
     }

     /* Data to local file */
     int read_file = open(file_name_dst, O_RDONLY, 0700) ;
     if (read_file < 0) {
         hdfsCloseFile(fs, write_fd) ;
         free(buffer) ;
         DEBUG_PRINT("ERROR: open fails for reading from '%s' file.\n", file_name_dst) ;
         return -1 ;
     }

     /* Copy from HDFS to local */
     tSize num_readed_bytes = 0 ;
     int ret = -1 ;

     do
     {
         num_readed_bytes = read_buffer(NULL, NULL, read_file, (void *)buffer, BUFFER_SIZE) ;
         if (num_readed_bytes == -1) {
             break ;
         }
         ret = write_buffer(fs, write_fd, -1, (void *)buffer, BUFFER_SIZE, num_readed_bytes) ;
         if (ret == -1) {
             break ;
         }
     }
     while ( (ret != -1) && (num_readed_bytes > 0) ) ;

     // Free resources (ok)
     hdfsFlush(fs, write_fd) ;
     hdfsCloseFile(fs, write_fd) ;
     close(read_file) ;
     free(buffer) ;
     return ret ;
}

int hdfs_stats ( char *file_name_org, char *machine_name, char ***blocks_information )
{
     char hostname_list[1024] ;

     // hostnames
     int i = 0 ;
     strcpy(hostname_list, "") ;
     while (blocks_information[0][i] != NULL)
     {
	  sprintf(hostname_list, "%s", blocks_information[0][i]) ;
	  i++ ;
	  if (blocks_information[0][i] != NULL) {
	      sprintf(hostname_list, "+") ;
	  }
     }

     // is_remote
     int is_remote = (strncmp(machine_name, blocks_information[0][0], strlen(machine_name)) != 0) ;

     // print metadata for this file
     printf("{ name:'%s', is_remote:%d, hostnames:'%s' },\n", file_name_org, is_remote, hostname_list) ;

     return 0 ;
}


/*
 * Threads
 */

int NUM_THREADS = 1 ;
pthread_t *threads ;

int sync_copied = 0 ;
pthread_cond_t sync_cond ;
pthread_mutex_t sync_mutex ;

struct th_args {
       hdfsFS    fs ;
       char      hdfs_path_org   [PATH_MAX] ;
       char      file_name_org   [PATH_MAX] ;
       char      machine_name    [HOST_NAME_MAX + 1] ;
       char      destination_dir [PATH_MAX] ;
       char      action          [PATH_MAX] ;
} ;

void * th_do_action ( void *arg )
{
       int             ret ;
       struct th_args  thargs ;
       char            file_name_dst[2*PATH_MAX] ;
       char            file_name_org[2*PATH_MAX] ;
       char        *** blocks_information;

       // Default return value
       ret = 0 ;

       // Copy arguments
       pthread_mutex_lock(&sync_mutex) ;
       memmove((char *)&thargs, (char *)arg, sizeof(struct th_args)) ;
       sync_copied = 1 ;
       pthread_cond_signal(&sync_cond) ;
       pthread_mutex_unlock(&sync_mutex) ;

       // Set the initial org/dst file name...
       sprintf(file_name_org, "%s/%s", thargs.hdfs_path_org,   thargs.file_name_org) ;
       sprintf(file_name_dst, "%s/%s", thargs.destination_dir, thargs.file_name_org) ;

       // Get HDFS information
       blocks_information = hdfsGetHosts(thargs.fs, file_name_org, 0, BLOCKSIZE) ;
       if (NULL == blocks_information) {
           DEBUG_PRINT("ERROR: hdfsGetHosts for '%s'.\n", thargs.file_name_org) ;
           pthread_exit((void *)(long)ret) ;
       }

       // Do action with file...
       if (!strcmp(thargs.action, "hdfs2local"))
       {
           //int is_remote = strncmp(thargs.machine_name, blocks_information[0][0], strlen(thargs.machine_name)) ;
           //if (0 != is_remote) {
                 ret = copy_from_hdfs_to_local(thargs.fs, file_name_org, file_name_dst) ;
           //}

           // Show message...
           DEBUG_PRINT("'%s' from node '%s' to node '%s': %s\n",
                       thargs.file_name_org,
                       blocks_information[0][0],
                       thargs.machine_name,
                       (ret < 0) ? "Error found" : "Done") ;

       }
       if (!strcmp(thargs.action, "local2hdfs"))
       {
           ret = copy_from_local_to_hdfs(thargs.fs, file_name_org, file_name_dst) ;
       }
       if (!strcmp(thargs.action, "stats4hdfs"))
       {
           ret = hdfs_stats(file_name_org, thargs.machine_name, blocks_information) ;
       }

       // The End
       hdfsFreeHosts(blocks_information);
       pthread_exit((void *)(long)ret) ;
}


/*
 * Read string from file
 */

char * main_fgets2 ( FILE *fd, char *str, int len )
{
       bzero(str, len) ;
       char *ret1 = fgets(str, len-1, fd) ;
       if (NULL == ret1) {
           return NULL ;
       }

       str[strlen(str)-1] = '\0' ;
       return str ;
}

void main_usage ( char *app_name )
{
       printf("\n") ;
       printf("  HDFS Copy\n") ;
       printf(" -----------\n") ;
       printf("\n") ;
       printf("  Usage:\n") ;
       printf("\n") ;
       printf("  > %s hdfs2local <hdfs/path> <file_list.txt> <cache/path>\n", app_name) ;
       printf("    Copy from a HDFS path to a local cache path a list of files (within file_list.txt).\n") ;
       printf("\n") ;
       printf("  > %s stats4hdfs <hdfs/path> <file_list.txt> <cache/path>\n", app_name) ;
       printf("    List HDFS metadata from the list of files within file_list.txt.\n") ;
       printf("\n") ;
       printf("\n") ;
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
    if (argc != 5) {
        main_usage(argv[0]) ;
        exit(-1) ;
    }

    // NUM_THREADS
    NUM_THREADS = 3 * get_nprocs_conf() ;
    threads     = malloc(NUM_THREADS * sizeof(pthread_t)) ;

    // Initialize th_args...
    bzero(&th_args, sizeof(struct th_args)) ;
    strcpy(th_args.action,          argv[1]) ;
    strcpy(th_args.hdfs_path_org,   argv[2]) ;
    strcpy(th_args.destination_dir, argv[4]) ;
    gethostname(th_args.machine_name, HOST_NAME_MAX + 1) ;

    // HFDS connect
    th_args.fs = hdfsConnect("default", 0) ;
    if (NULL == th_args.fs) {
        perror("hdfsConnect: ") ;
        exit(-1) ;
    }

    // Open listing file
    FILE *list_fd = fopen(argv[3], "ro") ;
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
            ret1 = main_fgets2(list_fd, th_args.file_name_org, sizeof(th_args.file_name_org)) ;
            if (NULL == ret1) {
                break ; // end for loop...
            }

            // Create thread...
            ret2 = pthread_create(&(threads[num_threads]), NULL, th_do_action, (void *)&(th_args)) ;
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
    free(threads) ;

    // The end
    return 0;
}

