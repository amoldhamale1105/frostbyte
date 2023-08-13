/**
    Frostbyte kernel and operating system
    Copyright (C) 2023  Amol Dhamale <amoldhamale1105@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "flib.h"

static void print_usage(void)
{
    printf("Usage:\n");
    printf("\tcat [OPTION] <FILE>\n");
    printf("Concatenate FILE to standard output (shell)\n\n");
    printf("\t-h\tdisplay this help and exit\n");
}

int main(int argc, char** argv)
{
    if (argc < 2){
        printf("%s: bad usage\n", argv[0]);
        printf("Try \'%s -h\' for more information\n", argv[0]);
        return 1;
    }
    if (argc > 1){
        if (argv[1][0] == '-'){
            if (strlen(argv[1]) == 2){
                switch (argv[1][1])
                {
                case 'h':
                    print_usage();
                    return 0;
                default:
                    printf("%s: invalid option \'%s\'\n", argv[0], argv[1]);
                    printf("Try \'%s -h\' for more information\n", argv[0]);
                    return 1;
                }
            }
            else{
                printf("%s: invalid option \'%s\'\n", argv[0], argv[1]);
                printf("Try \'%s -h\' for more information\n", argv[0]);
                return 1;
            }
        }
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