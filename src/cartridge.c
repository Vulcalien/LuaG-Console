/* Copyright 2022 Vulcalien
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "cartridge.h"

#include "terminal.h"
#include "display.h"
#include "map.h"

#include <stdio.h>
#include <limits.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <archive.h>
#include <archive_entry.h>

#define TEMP_DIR "/tmp/luag-console"

static int load_cartridge_info(void);
static int load_atlas(void);
static int load_map(void);

struct cartridge_Info cartridge_info;

static char *cartridge_folder;

int cartridge_init(void) {
    mkdir(TEMP_DIR, 0700);
    return 0;
}

void cartridge_destroy(void) {
    if(cartridge_folder)
        free(cartridge_folder);

    // TODO remove TEMP_DIR
}

char *cartridge_extract(const char *filename) {
    char *result = NULL;

    char *entry_path = NULL;

    struct archive *a = archive_read_new();

    archive_read_support_format_tar(a);
    archive_read_support_format_zip(a);

    archive_read_support_filter_gzip(a);

    struct archive *ext = archive_write_disk_new();
    archive_write_disk_set_standard_lookup(ext);

    // open cartridge file
    if(archive_read_open_filename(a, filename, 4096)) {
        fprintf(
            stderr,
            "Cartrige: could not load file '%s'\n"
            " - archive_read_open_filename: %s\n",
            filename, archive_error_string(a)
        );
        goto exit;
    }

    // prepare temporary folder
    if(cartridge_folder)
        free(cartridge_folder);
    cartridge_folder = malloc(PATH_MAX * sizeof(char));

    snprintf(cartridge_folder, PATH_MAX, TEMP_DIR "/XXXXXX");
    cartridge_folder = mkdtemp(cartridge_folder);

    // extract entries
    struct archive_entry *entry;
    entry_path = malloc(PATH_MAX * sizeof(char));
    while(true) {
        // read header
        int r = archive_read_next_header(a, &entry);

        if(r == ARCHIVE_EOF) {
            break;
        } else if(r != ARCHIVE_OK) {
            fprintf(
                stderr,
                "Cartridge: error reading a cartridge file\n"
                " - archive_read_next_header: %s\n",
                archive_error_string(a)
            );
            goto exit;
        }

        // change output path
        snprintf(
            entry_path, PATH_MAX,
            "%s/%s", cartridge_folder, archive_entry_pathname(entry)
        );
        archive_entry_set_pathname(entry, entry_path);

        // write header
        r = archive_write_header(ext, entry);

        if(r != ARCHIVE_OK) {
            fprintf(
                stderr,
                "Cartridge: error extracting a cartridge file\n"
                " - archive_write_header: %s\n",
                archive_error_string(ext)
            );
            goto exit;
        }

        // copy data
        const void *buffer;
        size_t size;
        la_int64_t offset;

        while(true) {
            // read data
            r = archive_read_data_block(a, &buffer, &size, &offset);

            if(r == ARCHIVE_EOF) {
                break;
            } else if(r != ARCHIVE_OK) {
                fprintf(
                    stderr,
                    "Cartridge: error reading data of a cartridge file\n"
                    " - archive_read_data_block: %s\n",
                    archive_error_string(a)
                );
                break; // is this correct to just ignore it?
            }

            // write data
            r = archive_write_data_block(ext, buffer, size, offset);

            if(r != ARCHIVE_OK) {
                fprintf(
                    stderr,
                    "Cartridge: error writing data of a cartridge file\n"
                    " - archive_write_data_block: %s\n",
                    archive_error_string(ext)
                );
                break; // does this leave the file open?
            }
        }

        // close entry file
        r = archive_write_finish_entry(ext);

        if(r != ARCHIVE_OK) {
            fprintf(
                stderr,
                "Cartridge: error closing a cartridge file\n"
                "- archive_write_finish_entry: %s\n",
                archive_error_string(ext)
            );
            goto exit;
        }
    }
    result = cartridge_folder;

    exit:
    archive_read_close(a);
    archive_read_free(a);

    archive_write_close(ext);
    archive_write_free(ext);

    if(entry_path)
        free(entry_path);

    return result;
}

int cartridge_load_files(void) {
    if(load_cartridge_info())
        return -1;
    if(load_atlas())
        return -2;
    if(load_map())
        return -3;
    return 0;
}

static int load_cartridge_info(void) {
    char *filename = malloc(PATH_MAX * sizeof(char));
    snprintf(
        filename, PATH_MAX,
        "%s/cartridge-info", game_folder
    );
    FILE *file = fopen(filename, "r");
    free(filename);

    if(!file) {
        // defaults
        cartridge_info.major_v = CARTRIDGE_DEFAULT_MAJOR_V;
        cartridge_info.minor_v = CARTRIDGE_DEFAULT_MINOR_V;
        return 0;
    }

    int result = fscanf(
        file,
        " library-version = %u.%u",
        &cartridge_info.major_v,
        &cartridge_info.minor_v
    );

    int err = 0;
    if(result != 2) {
        terminal_write(
            "Error:\n"
            "'cartridge-info'\n"
            "is invalid",
            true
        );
        err = -1;
    }

    fclose(file);
    return err;
}

static int load_atlas(void) {
    char *filename = malloc(PATH_MAX * sizeof(char));
    snprintf(
        filename, PATH_MAX,
        "%s/atlas.png", game_folder
    );
    int err = display_load_atlas(filename, NULL, NULL);
    free(filename);

    return err;
}

static int load_map(void) {
    char *filename = malloc(PATH_MAX * sizeof(char));
    snprintf(
        filename, PATH_MAX,
        "%s/map", game_folder
    );
    int err = map_load(filename);
    free(filename);

    return err;
}
