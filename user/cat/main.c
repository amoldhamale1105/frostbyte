#include "stdlib.h"

int main(int argc, char** argv)
{
    if (argc < 2){
        printf("%s: needs file name as argument\n", argv[0]);
        return 1;
    }
    
    int filelen = strlen(argv[1]);
    char filename[filelen+1];
    memcpy(filename, argv[1], filelen);
    filename[filelen] = 0;
    
    to_upper_str(filename);
    int fd = open_file(filename);
    if (fd < 0){
        printf("%s: %s: No such file or directory\n", argv[0], argv[1]);
        return 1;
    }
    int file_size = get_file_size(fd);
    char file_buf[file_size+1];
    int size_read = read_file(fd, file_buf, file_size);
    file_buf[file_size] = 0;

    if (file_size != size_read){
        printf("%s: %s: Error reading file\n", argv[0], argv[1]);
        return 1;
    }
    printf("%s", file_buf);

    return 0;
}