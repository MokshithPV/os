#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <utime.h>

#define MAX_PATH_LENGTH 1024
#define BUFFER_SIZE 4096

void report_change(char symbol, const char *path) {
    printf("[%c] %s\n", symbol, path);
}

void sync_items(const char *src_dir, const char *dst_dir, int flag);

void copy_file(const char *src_path, const char *dst_path) {
    FILE *src_file, *dst_file;
    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    // Open source file for reading
    if ((src_file = fopen(src_path, "r")) == NULL) {
        printf("Error: %s\n", src_path);
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    // Open destination file for writing
    if ((dst_file = fopen(dst_path, "w")) == NULL) {
        printf("Error: %s\n", dst_path);
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    // Copy contents from source to destination
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, src_file)) > 0) {
        if (fwrite(buffer, 1, bytes_read, dst_file) != bytes_read) {
            perror("fwrite");
            exit(EXIT_FAILURE);
        }
    }

    // Close files
    fclose(src_file);
    fclose(dst_file);
}

void sync_files(const char *src_path, const char *dst_path, int flag) {
    struct stat src_stat, dst_stat;

    // Check if src_path exists
    if (stat(src_path, &src_stat) == -1) {
        if (errno == ENOENT) {
            // src_path does not exist, delete dst_path
            remove(dst_path);
            report_change('-', dst_path);
            return;
        } else {
            printf("Error: %s\n", src_path);
            perror("stat");
            exit(EXIT_FAILURE);
        }
    }
    if(flag){
        if(S_ISDIR(src_stat.st_mode)){
            remove(dst_path);
            report_change('-',dst_path);
        }
        return;
    }

    // Check if dst_path exists
    if (stat(dst_path, &dst_stat) == -1) {
        if (errno == ENOENT) {
            // dst_path does not exist, copy src_path to dst_path
            //printf("Copying %s to %s\n", src_path, dst_path);
            copy_file(src_path, dst_path);
            report_change('+', dst_path);
            return;
        } else {
            printf("Error: %s\n", dst_path);
            perror("stat");
            exit(EXIT_FAILURE);
        }
    }

    // Both src_path and dst_path exist
    if (S_ISREG(src_stat.st_mode)) {
        // If src_path is a regular file
        if (src_stat.st_size != dst_stat.st_size || src_stat.st_mtime != dst_stat.st_mtime) {
            // If size or modification time is different, overwrite dst_path with src_path
            //remove(dst_path); // Delete the destination file
            //print sizes and modification times
            // printf("src size: %ld, dst size: %ld\n", src_stat.st_size, dst_stat.st_size);
            // printf("src mtime: %ld, dst mtime: %ld\n", src_stat.st_mtime, dst_stat.st_mtime);
            copy_file(src_path, dst_path);
            report_change('o', dst_path);
        }
    } else if (S_ISDIR(src_stat.st_mode)) {
        // If src_path is a directory
        remove(dst_path); // Delete the destination directory
        report_change('-', dst_path);
    } else {
        // Unexpected file type
        fprintf(stderr, "Unexpected file type for %s\n", src_path);
        exit(EXIT_FAILURE);
    }
}

void sync_items(const char *src_dir, const char *dst_dir, int flag) {
    DIR *dir;
    struct dirent *entry;
    char src_item[MAX_PATH_LENGTH];
    char dst_item[MAX_PATH_LENGTH];
    //check if destination directory exists
    if (access  (dst_dir, F_OK) == -1) {
        //destination directory does not exist, create it
        if (mkdir(dst_dir, 0777) == -1) {
            perror("mkdir");
            exit(EXIT_FAILURE);
        }
        report_change('+', dst_dir);
    }
    else {
        //destination directory exists
        if((dir = opendir(dst_dir)) == NULL) {
            // file exits but not a directory
            snprintf(dst_item, sizeof(dst_item), "%s", dst_dir);
            snprintf(src_item, sizeof(src_item), "%s", src_dir);
            sync_files(src_item, dst_item,1);
        }
        while((dir != NULL) && (entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue; // Skip "." and ".."
            }
            snprintf(dst_item, sizeof(dst_item), "%s/%s", dst_dir, entry->d_name);
            snprintf(src_item, sizeof(src_item), "%s/%s", src_dir, entry->d_name);
            //check if dst_item is a directory
            if (entry->d_type == DT_DIR) {
                // Recursively sync directories
                sync_items(src_item, dst_item,1);
                continue;
            }
            sync_files(src_item, dst_item,1);
        }
        if((dir = opendir(src_dir)) == NULL) {
            //check if source directory exists
            if (access  (src_dir, F_OK) == -1) {
                //source directory does not exist, delete destination directory
                if (rmdir(dst_dir) == -1) {
                    perror("rmdir");
                    exit(EXIT_FAILURE);
                }
                report_change('-', dst_dir);
                return;
            }
            else {
                //source directory exists
                printf("Error: %s\n", src_dir);
                perror("opendir");
                exit(EXIT_FAILURE);
            }
        }
    }
    if(flag) return;

    // Open source directory
    if ((dir = opendir(src_dir)) == NULL) {
        //check if source directory exists
        if (access  (src_dir, F_OK) == -1) {
            //source directory does not exist, delete destination directory
            if (rmdir(dst_dir) == -1) {
                perror("rmdir");
                exit(EXIT_FAILURE);
            }
            report_change('-', dst_dir);
            return;
        }
        else {
            //source directory exists
            printf("Error: %s\n", src_dir);
            perror("opendir");
            exit(EXIT_FAILURE);
        }
    }

    // Traverse source directory
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; // Skip "." and ".."
        }

        snprintf(src_item, sizeof(src_item), "%s/%s", src_dir, entry->d_name);
        snprintf(dst_item, sizeof(dst_item), "%s/%s", dst_dir, entry->d_name);
        //check if src_item is a directory
        if (entry->d_type == DT_DIR) {
            // Recursively sync directories
            sync_items(src_item, dst_item,0);
            continue;
        }else{
            // printf("src_item: %s, dst_item: %s\n", src_item, dst_item);
            sync_files(src_item, dst_item,0);
        }
    }

    closedir(dir);
}

void sync_timestamps_permissions(const char *src, const char *drc)
{
    struct stat src_stat;
    struct stat drc_stat;
    if (stat(src, &src_stat) == -1) {
        printf("Error %s\n",src);
        perror("stat");
        exit(EXIT_FAILURE);
    }
    if (stat(drc, &drc_stat) == -1) {
        printf("Error %s\n",drc);
        perror("stat");
        exit(EXIT_FAILURE);
    }
    if(drc_stat.st_mode != src_stat.st_mode){
        if (chmod(drc, src_stat.st_mode) == -1) {
            perror("chmod");
            exit(EXIT_FAILURE);
        }
        report_change('p', drc);
    }
    if(drc_stat.st_mtime != src_stat.st_mtime){
        // printf("src mtime: %ld, dst mtime: %ld\n", src_stat.st_mtime, drc_stat.st_mtime);
        // printf("src atime: %ld, dst atime: %ld\n", src_stat.st_atime, drc_stat.st_atime);
        struct utimbuf new_times;
        new_times.actime = src_stat.st_atime;
        new_times.modtime = src_stat.st_mtime;
        if (utime(drc, &new_times) == -1) {
            perror("utime");
            exit(EXIT_FAILURE);
        }
        report_change('t', drc);
    }
    // recursively sync timestamps and permissions
    DIR *dir;
    struct dirent *entry;
    char src_item[MAX_PATH_LENGTH];
    char dst_item[MAX_PATH_LENGTH];
    if((dir = opendir(drc)) == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    while((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; // Skip "." and ".."
        }
        snprintf(dst_item, sizeof(dst_item), "%s/%s", drc, entry->d_name);
        snprintf(src_item, sizeof(src_item), "%s/%s", src, entry->d_name);
        if (stat(src_item, &src_stat) == -1) {
            printf("Error %s\n",src_item);
            perror("stat");
            exit(EXIT_FAILURE);
        }
        if (stat(dst_item, &drc_stat) == -1) {
            printf("Error %s\n",dst_item);
            perror("stat");
            exit(EXIT_FAILURE);
        }//check if dst_item is a directory
        if (entry->d_type == DT_DIR) {
            // Recursively sync directories
            sync_timestamps_permissions(src_item, dst_item);
            continue;
        }
        if(drc_stat.st_mode != src_stat.st_mode){
            if (chmod(dst_item, src_stat.st_mode) == -1) {
                perror("chmod");
                exit(EXIT_FAILURE);
            }
            report_change('p', dst_item);
        }
        if(drc_stat.st_mtime != src_stat.st_mtime){
            // printf("src mtime: %ld, dst mtime: %ld\n", src_stat.st_mtime, drc_stat.st_mtime);
            // printf("src atime: %ld, dst atime: %ld\n", src_stat.st_atime, drc_stat.st_atime);
            struct utimbuf new_times;
            new_times.actime = src_stat.st_atime;
            new_times.modtime = src_stat.st_mtime;
            if (utime(dst_item, &new_times) == -1) {
                perror("utime");
                exit(EXIT_FAILURE);
            }
            report_change('t', dst_item);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_dir> <destination_dir>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *src_dir = argv[1];
    const char *dst_dir = argv[2];

    sync_items(src_dir, dst_dir,0);
    sync_timestamps_permissions(src_dir, dst_dir);

    return 0;
}
