#include "stdlib.h"

struct DirEntry entries[1024];

int main(void)
{
    char filename[13] = {0};
    char filetype[2] = {0};
    int name_count = 0;
    int count = read_root_dir(entries);

    filetype[1] = 0;

    if (count > 0){
        printf("NAME          TYPE          SIZE\r\n");
        printf("---------------------------------\r\n");

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
            while (*(filename + name_count) != ' ')
            {
                name_count++;
            }
            filename[name_count] = '.';
            memcpy(filename+name_count+1, entries[i].ext, MAX_EXTNAME_BYTES);
            *filetype = entries[i].attributes == ATTR_FILETYPE_DIRECTORY ? 'd' : 'f';

            printf("%s\t%s           %u\r\n", filename, filetype, (uint64_t)entries[i].file_size);

            memset(filename, 0, sizeof(filename));
            name_count = 0;
        }
    }

    return 0;
}