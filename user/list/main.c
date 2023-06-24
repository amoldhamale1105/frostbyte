#include "stdlib.h"

struct DirEntry entries[1024];

int main(void)
{
    char filename[MAX_FILENAME_BYTES+MAX_EXTNAME_BYTES+2] = {0};
    char filetype;
    int name_count = 0;
    int count = read_root_dir(entries);

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
            while (name_count < MAX_FILENAME_BYTES && *(filename + name_count) != ' ')
            {
                name_count++;
            }
            if (!(entries[i].ext[0] == ' ' || entries[i].ext[0] == 0)){
                filename[name_count] = '.';
                memcpy(filename+name_count+1, entries[i].ext, MAX_EXTNAME_BYTES);
            }
            filetype = entries[i].attributes == ATTR_FILETYPE_DIRECTORY ? 'd' : 'f';

            printf("%s\t%c           %u\r\n", filename, filetype, (uint64_t)entries[i].file_size);

            memset(filename, 0, sizeof(filename));
            name_count = 0;
        }
    }

    return 0;
}