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
#include <stdbool.h>

#define MAX_ITEMS_PER_ROW 5

struct DirEntry entries[1024];

static void print_usage(void)
{
    printf("Usage:");
    printf("\tls [OPTION]\n");
    printf("\tList information about files in current directory (root directory by default)\n\n");
    printf("\t-h\tdisplay this help and exit\n");
    printf("\t-l\tuse a long listing format with all details\n");
}

int main(int argc, char** argv)
{
    bool long_list = false;
    if (argc > 1){
        int opt = 1;
        while (opt < argc)
        {
            if (argv[opt][0] != '-'){
                printf("%s: bad usage\n", argv[0]);
                printf("Try \'%s -h\' for more information\n", argv[0]);
                return 1;
            }
            char* optstr = &argv[opt][1];
            while (*optstr)
            {
                switch (*optstr)
                {
                case 'h':
                    print_usage();
                    return 0;
                case 'l':
                    long_list = true;
                    break;
                default:
                    printf("%s: invalid option \'%s\'\n", argv[0], argv[opt]);
                    printf("Try \'%s -h\' for more information\n", argv[0]);
                    return 1;
                }
                optstr++;
            }
            opt++;
        }
    }
    char filename[MAX_FILENAME_BYTES+MAX_EXTNAME_BYTES+2] = {0};
    char filetype;
    int name_count = 0, valid_items = 0;
    int count = read_root_dir(entries);

    if (count > 0){
        if (long_list){
            printf("NAME          TYPE          SIZE\r\n");
            printf("---------------------------------\r\n");
        }
        /* Iterate over all files in root directory */
        for(int i = 0; i < count; i++)
        {
            /* Skip an entry if it's empty, deleted, or a volume label
               In this basic version, we will also skip entries with long filenames */
            if (entries[i].name[0] == ENTRY_AVAILABLE || \
                entries[i].name[0] == ENTRY_DELETED || \
                entries[i].attributes == ATTR_LONG_FILENAME || \
                entries[i].attributes == ATTR_VOLUME_LABEL)
                continue;

            memcpy(filename, entries[i].name, MAX_FILENAME_BYTES);
            while (name_count < MAX_FILENAME_BYTES && *(filename + name_count) != ' ')
            {
                name_count++;
            }
            if (!(entries[i].ext[0] == ' ' || entries[i].ext[0] == 0)){
                filename[name_count] = '.';
                memcpy(filename+name_count+1, entries[i].ext, MAX_EXTNAME_BYTES);
            }

            if (long_list){
                filetype = entries[i].attributes == ATTR_FILETYPE_DIRECTORY ? 'd' : 'f';
                printf("%s\t%c           %u\r\n", filename, filetype, (uint64_t)entries[i].file_size);
            }
            else{
                if (valid_items > 0 && valid_items % MAX_ITEMS_PER_ROW == 0)
                    printf("\n");
                printf("%s\t", filename);
            }

            memset(filename, 0, sizeof(filename));
            valid_items++;
            name_count = 0;
        }
        if (!long_list)
            printf("\n");
    }

    return 0;
}