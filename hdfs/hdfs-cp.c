
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
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/time.h>
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
     //if (ret < 0) {
     //    DEBUG_PRINT("ERROR: creating directory '%s'.\n", path) ;
     //    perror("mkdir:") ;
     //}

     // free duplicate string
     free(fullpath);
}

int create_local_file ( char *file_name_dst )
{
     /* mkdir directory structure */
     struct stat st = {0} ;
     char        basename_org[PATH_MAX] ;
     char       *dirname_org ;

     strcpy(basename_org, file_name_dst) ;
     dirname_org = dirname(basename_org) ;
     if (stat(dirname_org, &st) == -1) {
         mkdir_recursive(dirname_org) ;
     }

     /* open local file */
     int write_fd = open(file_name_dst, O_WRONLY | O_CREAT, 0700) ;
     if (write_fd < 0) {
         DEBUG_PRINT("ERROR: open fails to create '%s' file.\n", file_name_dst) ;
     }

     return write_fd ;
}

hdfsFile create_hdfs_file ( hdfsFS fs, char *file_name_dst )
{
     hdfsFile write_fd ;
     char   basename_org[PATH_MAX] ;
     char  *dirname_org ;

     // mkdir directory structure
     strcpy(basename_org, file_name_dst) ;
     dirname_org = dirname(basename_org) ;
     hdfsCreateDirectory(fs, dirname_org);

     /* open hdfs file */
     write_fd = hdfsOpenFile(fs, file_name_dst, O_WRONLY|O_CREAT, 0, 0, 0) ;
     if (!write_fd) {
         DEBUG_PRINT("ERROR: hdfsOpenFile fails to create '%s'.\n", file_name_dst) ;
     }

     return write_fd ;
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

     return num_readed_bytes ;
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

	 // check end of file
         if (read_num_bytes == 0) {
	     return (buffer_size - read_remaining_bytes) ;
         }

         read_remaining_bytes -= read_num_bytes ;
     }

     return buffer_size ;
}


int copy_from_hdfs_to_local ( hdfsFS fs, char *file_name_org, char *file_name_dst )
{
     int ret = -1 ;
     int write_fd ;
     hdfsFile read_file ;
     tSize num_readed_bytes = 0 ;
     unsigned char *buffer = NULL ;

     // allocate intermediate buffer
     buffer = malloc(BUFFER_SIZE) ;
     if (NULL == buffer) {
         DEBUG_PRINT("ERROR: malloc for '%d'.\n", BUFFER_SIZE) ;
         return -1 ;
     }

     // DEBUG
     DEBUG_PRINT("INFO: copy from '%s' to '%s'...\n", file_name_org, file_name_dst) ;

     /* Data from HDFS */
     read_file = hdfsOpenFile(fs, file_name_org, O_RDONLY, 0, 0, 0) ;
     if (!read_file) {
         free(buffer) ;
         DEBUG_PRINT("ERROR: hdfsOpenFile on '%s' for reading.\n", file_name_org) ;
         return -1 ;
     }

     /* Data to local file */
     write_fd = create_local_file(file_name_dst) ;
     if (write_fd < 0) {
         hdfsCloseFile(fs, read_file) ;
         free(buffer) ;
         return -1 ;
     }

     /* Copy from HDFS to local */
     do
     {
         num_readed_bytes = read_buffer(fs, read_file, -1, (void *)buffer, BUFFER_SIZE) ;
         if (num_readed_bytes != -1) {
             ret = write_buffer(NULL, NULL, write_fd, (void *)buffer, BUFFER_SIZE, num_readed_bytes) ;
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
     int ret = -1 ;
     hdfsFile write_fd ;
     int read_file ;
     tSize num_readed_bytes = 0 ;
     unsigned char *buffer = NULL ;

     // allocate intermediate buffer
     buffer = malloc(BUFFER_SIZE) ;
     if (NULL == buffer) {
         DEBUG_PRINT("ERROR: malloc for '%d'.\n", BUFFER_SIZE) ;
         return -1 ;
     }

     // DEBUG
     DEBUG_PRINT("INFO: copy from '%s' to '%s'...\n", file_name_org, file_name_dst) ;

     /* Data to HDFS */
     write_fd = create_hdfs_file(fs, file_name_org) ;
     if (!write_fd) {
         free(buffer) ;
         return -1 ;
     }

     /* Data from local file */
     read_file = open(file_name_dst, O_RDONLY, 0700) ;
     if (read_file < 0) {
         hdfsCloseFile(fs, write_fd) ;
         free(buffer) ;
         DEBUG_PRINT("ERROR: open fails for reading from '%s' file.\n", file_name_dst) ;
         return -1 ;
     }

     /* Copy from local to HDFS */
     do
     {
         num_readed_bytes = read_buffer(NULL, NULL, read_file, (void *)buffer, BUFFER_SIZE) ;
         if (num_readed_bytes != -1) {
             ret = write_buffer(fs, write_fd, -1, (void *)buffer, BUFFER_SIZE, num_readed_bytes) ;
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
     strcpy(hostname_list, "") ;
     for (int i=0; blocks_information[0][i] != NULL; i++) {
	  sprintf(hostname_list, "%s+%s", hostname_list, blocks_information[0][i]) ;
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

typedef struct thargs {
       hdfsFS    fs ;
       char      hdfs_path_org   [PATH_MAX] ;
       char      file_name_org   [PATH_MAX] ;
       char      machine_name    [HOST_NAME_MAX + 1] ;
       char      destination_dir [PATH_MAX] ;
       char      action          [PATH_MAX] ;
       char      list_files      [PATH_MAX] ;
} thargs_t ;

#define MAX_BUFFER 128
thargs_t buffer[MAX_BUFFER];

int  ha_arrancado = 0;
int  fin = 0;
int  n_elementos  = 0;
int  pos_receptor = 0;
int  pos_servicio = 0;

pthread_mutex_t  mutex;
pthread_cond_t   no_lleno;
pthread_cond_t   no_vacio;
pthread_cond_t   arrancado;
pthread_cond_t   parado;

char * do_reception ( FILE *fd, thargs_t *p )
{
       char *str = p->file_name_org ;
       int   len = sizeof(p->file_name_org) ;

       bzero(str, len) ;
       char *ret = fgets(str, len-1, fd) ;
       if (NULL == ret) {
           return NULL ;
       }

       str[strlen(str)-1] = '\0' ;
       return str ;
}

void * receptor ( void * param )
{
      char    *ret ;
      thargs_t p ;

      // Initializate...
      memcpy(&p, param, sizeof(thargs_t)) ;
      DEBUG_PRINT("INFO: 'receptor' initialized...\n");

      // Open listing file
      FILE *list_fd = fopen(p.list_files, "ro") ;
      if (NULL == list_fd) {
          hdfsDisconnect(p.fs) ;
          perror("fopen: ") ;
          exit(-1) ;
      }

      // Get file name from listing
      ret = do_reception(list_fd, &p) ;
      while (ret != NULL)
      {
	    // lock when not full...
            pthread_mutex_lock(&mutex);
            while (n_elementos == MAX_BUFFER) {
                   pthread_cond_wait(&no_lleno, &mutex);
	    }

	    // inserting element into the buffer
            DEBUG_PRINT("INFO: 'receptor' enqueue request for '%s' at %d.\n", p.file_name_org, pos_receptor);
            memcpy((void *)&(buffer[pos_receptor]), (void *)&p, sizeof(thargs_t)) ;
            pos_receptor = (pos_receptor + 1) % MAX_BUFFER;
            n_elementos++;

	    // signal not empty...
            pthread_cond_signal(&no_vacio);
            pthread_mutex_unlock(&mutex);

            ret = do_reception(list_fd, &p) ;
      }

      // signal end
      pthread_mutex_lock(&mutex);
      fin=1;
      pthread_cond_broadcast(&no_vacio);
      pthread_mutex_unlock(&mutex);
      DEBUG_PRINT("INFO: 'receptor' finalized...\n");

      // close local
      fclose(list_fd) ;

      pthread_exit(0);
      return NULL;
}

void * do_service ( void *params )
{
       int       ret ;
       thargs_t  thargs ;
       char      file_name_dst[2*PATH_MAX] ;
       char      file_name_org[2*PATH_MAX] ;
       char  *** blocks_information;

       // Default return value
       ret = 0 ;

       // Set the initial org/dst file name...
       memcpy(&thargs, params, sizeof(thargs_t)) ;
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
           DEBUG_PRINT("INFO: '%s' from node '%s' to node '%s': %s\n",
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
       return NULL ;
}

void * servicio ( void * param )
{
      thargs_t p;

      // signal initializate...
      pthread_mutex_lock(&mutex);
      ha_arrancado = 1;
      pthread_cond_signal(&arrancado);
      pthread_mutex_unlock(&mutex);
      DEBUG_PRINT("INFO: 'service' initialized...\n");

      for (;;)
      {
	   // lock when not empty and not ended...
           pthread_mutex_lock(&mutex);
           while (n_elementos == 0)
	   {
                if (fin==1) {
                    DEBUG_PRINT("INFO: 'service' finalized.\n");
                    pthread_cond_signal(&parado);
                    pthread_mutex_unlock(&mutex);
                    pthread_exit(0);
                }

                pthread_cond_wait(&no_vacio, &mutex);
           } // while

	   // removing element from buffer...
           DEBUG_PRINT("INFO: 'service' dequeue request at %d\n", pos_servicio);
           memcpy((void *)&p, (void *)&(buffer[pos_servicio]), sizeof(thargs_t)) ;
           pos_servicio = (pos_servicio + 1) % MAX_BUFFER;
           n_elementos--;

	   // signal not full...
           pthread_cond_signal(&no_lleno);
           pthread_mutex_unlock(&mutex);

	   // process and response...
           do_service(&p) ;
    }

    pthread_exit(0);
    return NULL;
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

int main ( int argc, char *argv[] )
{
    struct timeval timenow;
    long t1, t2;
    pthread_t  thr;
    pthread_t *ths;
    int MAX_SERVICIO;
    thargs_t p;

    // Check arguments
    if (argc != 5) {
        main_usage(argv[0]) ;
        exit(-1) ;
    }

    // t1
    gettimeofday(&timenow, NULL) ;
    t1 = (long)timenow.tv_sec * 1000 + (long)timenow.tv_usec / 1000 ;

    // Initialize threads...
    MAX_SERVICIO = 3 * get_nprocs_conf() ;
    ths = malloc(MAX_SERVICIO * sizeof(pthread_t)) ;

    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&no_lleno, NULL);
    pthread_cond_init(&no_vacio, NULL);
    pthread_cond_init(&arrancado, NULL);
    pthread_cond_init(&parado, NULL);

    // Initialize th_args...
    bzero(&p, sizeof(thargs_t)) ;
    strcpy(p.action,          argv[1]) ;
    strcpy(p.hdfs_path_org,   argv[2]) ;
    strcpy(p.list_files,      argv[3]) ;
    strcpy(p.destination_dir, argv[4]) ;
    gethostname(p.machine_name, HOST_NAME_MAX + 1) ;
    p.fs = hdfsConnect("default", 0) ;
    if (NULL == p.fs) {
        perror("hdfsConnect: ") ;
        exit(-1) ;
    }

    // Create threads
    for (int i=0; i<MAX_SERVICIO; i++)
    {
          pthread_create(&ths[i], NULL, servicio, &p);

          // wait thread is started
          pthread_mutex_lock(&mutex) ;
	  while (!ha_arrancado) {
                 pthread_cond_wait(&arrancado, &mutex) ;
	  }
          ha_arrancado = 0 ;
          pthread_mutex_unlock(&mutex) ;
    }

    pthread_create(&thr, NULL,receptor, &p);

    // Wait for all thread end
    pthread_mutex_lock(&mutex) ;
    while ( (!fin) || (n_elementos > 0) ) {
             pthread_cond_wait(&parado, &mutex) ;
    }
    pthread_mutex_unlock(&mutex) ;

    // Join threads
    pthread_join(thr, NULL);
    for (int i=0; i<MAX_SERVICIO; i++) {
         pthread_join(ths[i], NULL);
    }

    // Finalize
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&no_lleno);
    pthread_cond_destroy(&no_vacio);
    pthread_cond_destroy(&arrancado);
    pthread_cond_destroy(&parado);

    free(ths) ;
    hdfsDisconnect(p.fs) ;

    // t2
    gettimeofday(&timenow, NULL) ;
    t2 = (long)timenow.tv_sec * 1000 + (long)timenow.tv_usec / 1000 ;

    // Imprimir t2-t1...
    printf("Tiempo total: %lf seconds.\n", (t2-t1)/1000.0);
    return 0;
}

