#include "types.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

extern State state;

void load_map(char* path) {
    Sector* sectors;
    Wall* walls;

    FILE* input = fopen(path, "r");
    ASSERT(input, "Cannot find Map!");

    enum { SCAN_SECTOR, SCAN_WALL, SCAN_NONE } scan = SCAN_NONE;
    char line[1024], buf[64];
    while (fgets(line, sizeof(line), input)) {
        const char* p = line;
        while(isspace(*p)) {
            p++;
        }

        if (!*p || *p == '#') {
            continue;
        } else if (*p == '[') {
            strncpy(buf, p + 1, sizeof(buf));
            const char* section = strtok(buf, "]");
            if(!section) { fclose(input); ASSERT(false, "Incomplete Header"); };
            if (!strcmp(section, "SECTOR")) { scan = SCAN_SECTOR; }
            else if (!strcmp(section, "WALL")) { scan = SCAN_WALL; }
            else { fclose(input); ASSERT(false, "Unknown Header"); }
        } else {
            switch(scan) {
                case SCAN_WALL: {
                    Wall* wall = &state.walls.arr[state.walls.n++];
                    if (sscanf(p,
                                "%f %f %f %f %d",
                                &wall->a.x,
                                &wall->a.y,
                                &wall->b.x,
                                &wall->b.y,
                                &wall->portal) != 5) {
                        fclose(input);
                        ASSERT(false, "Incomplete Wall Definition");
                    }
                                }break;
                case SCAN_SECTOR: {
                    Sector* sector = &state.sectors.arr[state.sectors.n++];
                    if (sscanf(p,
                                "%lu %lu %lu %f %f",
                                &sector->id,
                                &sector->index,
                                &sector->nwalls,
                                &sector->floor,
                                &sector->ceiling) != 5) {
                        fclose(input);
                        ASSERT(false, "Incomplete Sector Definition");
                    }
                                  } break;
                default: fclose(input); ASSERT(false, "No Header Set");
            }
        }
    }
    return;
}
