#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

void read_params(char *path)
{
    FILE *f = fopen(path, "r");
    ip = (char *)malloc(20);

    char *s_port = (char *)malloc(20);
    fscanf(f, "ip=%s\nport=%s", ip, s_port);

    if (f)
        fclose(f);

    port = atoi(s_port);
    printf("%s %d\n", ip, port);
}

typedef struct file_metadata
{
    char path[255];
    unsigned int size;
    unsigned int timestamp;
    short is_regular_file; // 1 if regular file; 0 if directory
} fm;

int paths_count = 0;
char paths[255][255];


void getAllFilesPaths(char *path)
{
    //parcurge recursiv arborele de fisiere
    //cand ajunge la o frunza (fisier sau director gol) citeste-i calea absoluta
    //pune-o in vectorul de string-uri "paths" si incrementeaza paths_count

    /*
        your code
        
    */
}

fm *own_files;

void getAllFilesMetadata()
{
    own_files = (fm *)malloc(paths_count * sizeof(fm));
    //pentru fiecare path din paths, creaza un nou obiect file_metadata,
    //populeaza-l cu date apeland lstat SI adauga-l la own_files_metadata
    
    int i;
    for(i = 0; i < paths_count; i++)
    {
        strcpy(own_files[i].path, paths[i]);
        struct stat s;
        stat(own_files[i].path, &s);
        own_files[i].size = s.st_size;
        own_files[i].timestamp = s.st_mtime;
        if(S_ISDIR(s.st_mode))
        {
            own_files[i].is_regular_file = 0;
        }
        else
        {
            own_files[i].is_regular_file = 1;
        }
    }

}

fm *files_to_delete;
int files_to_delete_count = 0;

void getFilesToDelete(fm *server_files, int server_files_count)
{
    //vrem sa le returnam doar pe cele care se gasesc pe client si nu se gasesc pe server
    //mijlocul de comparatie este campul "path" din structura file_metadata

    /*
        your code
    */
}

fm *files_to_update;
int files_to_update_count = 0;

void getFilesToUpdate(fm *server_files, int server_files_count)
{
    //vrem sa returnam file_metadata[] de pe client care difera de cele de pe server
    //la cel putin una din urmatoarele categorii
    //fm->size SAU fm->timestamp SAU fisierul e pe server si NU e pe client

    /*
        your code
    */

}
