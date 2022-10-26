/* 
Outline for ls project

First step:

Read the arguments that are passed with ls, which are -a, -l, any number of files and any
number of directories

First: find and store the options -l and -a

Second: loop thru the arguments and check if they are files or directory

Third: Call each argument to ls_dir or ls_file, which prints the info


Behavior of ls:

Files and directories listed must be in the current directory

ls of a file just shows info for that file

ls of a directory shows the files and directories (ls of a directory in the current directory 
is allowed)

-l shows the long version of the info about that file/directory

-l => permissions, user, group, date/time last modified, filename

*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>




void perror(const char *s);
void printlong(struct stat statbuf);
void ls_dir(int show_hidden_files, int print_long, char* dir_path);
void ls_file(int show_hidden_files, int print_long, char* dir_path);



void ls_file(int show_hidden_files, int print_long, char* file_name)
{
    //this funciton is called for printing information on a single file

    struct stat statbuf;

    // printf("Is a file\n");
    if(stat(file_name, &statbuf) == -1)
    {
        printf("error reading file in ls_file");
        return;
    }

    // check if the -l option was passed, simply prints the file name if not and calls 
    // printlong if it was
    if(print_long == 0)
    {
        printf("%s\n", file_name);
    }
    else
    {
        if(stat(file_name, &statbuf) == -1)
        {
            perror("Error: ");
            exit(-1);
        }
        printlong(statbuf);
        printf("%s\n", file_name);
    }

}

void ls_dir(int show_hidden_files, int print_long, char* dir_path)
{
    //this function is called from printing information on the files in a directory

    struct dirent *dp;
    struct stat statbuf;

    DIR *dir = opendir(dir_path);

    if(dir == NULL)
    {
        printf("error opneing directory: %s\n", dir_path);
        return; 
    }
    // printf("%s:\n", dir_path);

    // checks if the -l option was passed by the user and acts accordingly 
    if(print_long ==1) // -l was passed
    {
        while((dp = readdir(dir)) != NULL)
        {   
            // printf("file in directory: %s\n", dp->d_name);
            // Get entry's info
            if((show_hidden_files == 0) & (dp->d_name[0] == '.'))
            {
                //do nothing bc this file is hidden and -a was not passed
            }
            else
            {   
                //build the path to the file that we need information on
                char dir_path_temp[PATH_MAX];
                strcpy(dir_path_temp, dir_path);
                char *file_path = strcat(strcat(dir_path_temp, "/"), dp->d_name);
                if(stat(file_path, &statbuf) == -1)
                {
                    printf("statbuf passed -1");
                    continue;
                }
                printlong(statbuf);
                printf("%s\n", dp->d_name);
            }
        }
    }
    else // -l was not passed
    {
        while((dp = readdir(dir)) != NULL)
        {   
            // printf("file in directory: %s, dir_path = %s\n", dp->d_name, dir_path);
            if((show_hidden_files == 0) & (dp->d_name[0] == '.'))
            {
                //do nothing bc this file is hidden and -a was not passed
            }
            else
            {
                //build the path to the file that we need information on
                char dir_path_temp[PATH_MAX];
                strcpy(dir_path_temp, dir_path);
                char *file_path = strcat(strcat(dir_path_temp, "/"), dp->d_name);

                // printf("reading file at path: %s\n", file_path);
                if(stat(file_path, &statbuf) == -1)
                {
                    printf("statbuf passed -1");
                    continue;
                }
                printf("%s ", dp->d_name);
            }
        }
        printf("\n");
    }
    printf("\n");
}

void printlong(struct stat statbuf)
{
    //this function is called for printing the information needed for -l option

    // struct stat statbuf;
    struct passwd *pwd;
    struct group *grp;
    struct tm *tm;
    char datestring[256];

    // print file type, permissions, and number of links
    printf( (S_ISDIR(statbuf.st_mode)) ? "d" : "-");
    printf( (statbuf.st_mode & S_IRUSR) ? "r" : "-");
    printf( (statbuf.st_mode & S_IWUSR) ? "w" : "-");
    printf( (statbuf.st_mode & S_IXUSR) ? "x" : "-");
    printf( (statbuf.st_mode & S_IRGRP) ? "r" : "-");
    printf( (statbuf.st_mode & S_IWGRP) ? "w" : "-");
    printf( (statbuf.st_mode & S_IXGRP) ? "x" : "-");
    printf( (statbuf.st_mode & S_IROTH) ? "r" : "-");
    printf( (statbuf.st_mode & S_IWOTH) ? "w" : "-");
    printf( (statbuf.st_mode & S_IXOTH) ? "x" : "-");

    printf("%4ld", statbuf.st_nlink); //# of links

    //print owners name if found using getpwuid
    if( (pwd = getpwuid(statbuf.st_uid)) != NULL )
    {
        printf(" %-8.8s", pwd->pw_name);
    }
    else
    {
        printf(" %-8d", statbuf.st_uid);
    }

    // print group name if found using getgrgid, using just stat info if not
    if( (grp = getgrgid(statbuf.st_gid)) != NULL)
    {
        printf("%-8.8s", grp->gr_name);
    }
    else
    {
        printf("%-8d", statbuf.st_gid);
    }

    //print size of file
    printf("%9jd", (intmax_t)statbuf.st_size);
    printf(" ");

    tm = localtime(&statbuf.st_atime); // save time last modified to variable tm

    //get localalize date string in correct format
    strftime(datestring, sizeof(datestring), "%b %e %H:%M", tm);

    printf("%s ", datestring); // print time
}

int main(int argc, char *argv[])
{

    int show_hidden_files = 0;
    int print_long = 0;
    int opt;

    //this while loop checks whether the -l or -a options were passed
    while((opt = getopt(argc, argv, "al")) != -1)
    {
        switch(opt)
        {
            case 'a':
                // printf("option: %c\n", opt); 
                show_hidden_files = 1;
                break;
            case 'l':
                // printf("option: %c\n", opt);
                print_long = 1;
                break;
            case '?': 
                perror("Error in main");
                exit(EXIT_FAILURE);
        }
    }

    // printf("show_hidden_files = %d\n", show_hidden_files);
    // printf("print_long = %d\n", print_long);
    // printf("optind = %d\n", optind);

    if(argv[optind] == NULL)
    {
        // not directory or file was specified, so call with cwd
        ls_dir(show_hidden_files, print_long, ".");
    }
    else // there were additional arguments specifying files/directories
    {
        int index = optind;
        while(argv[index] != NULL) // loops thru arguments passed
        {
            struct stat statbuf;
            char *name = argv[index];
            int is_dir = 0;
            int is_file = 0;

            if(stat(name, &statbuf) == -1) // get stats
            {
                printf("ls: cannot access '%s': No such file or directory\n", name);
                index++;
                continue;
            }

            //check if argument is a directory
            if(S_ISDIR(statbuf.st_mode))
            {
                is_dir = 1;
            }
            else if(S_ISREG(statbuf.st_mode)) //check if argument is a file
            {
                is_file = 1;
            }
            else
            {
                printf("error reading name");
            }

            if(is_dir == 1)
            {   
                //get path to current directory
                char cwd[PATH_MAX];
                if(getcwd(cwd, sizeof(cwd)) == NULL)
                {
                    perror("Error: ");
                    exit(-1);
                }

                // unless there is only one directory passed, print the directory name
                // printf("optind = %d, argc = %d\n", optind, argc);
                if((optind+1) != argc)
                {
                    printf("%s:\n", name);
                }
                
                //if directory is a is not relative to the cwd but instead just from root
                if(name[0] == '/')
                {
                    // printf("Path: %s\n", name);
                    ls_dir(show_hidden_files, print_long, name);
                }
                else
                {   
                    //create the path from the cwd to the new directory
                    char *path = strcat(strcat(cwd, "/"), name);
                    // printf("Path: %s\n", path);
                    ls_dir(show_hidden_files, print_long, path);
                }

            }
            else if(is_file == 1) // if the user passes a file in the cwd
            {
                ls_file(show_hidden_files, print_long, name);
            }
            
            index++;
        }
    }
}