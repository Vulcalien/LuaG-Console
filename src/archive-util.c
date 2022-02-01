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
#include "archive-util.h"

#include <limits.h>

#include <archive.h>
#include <archive_entry.h>

static int copy_entries(struct archive *in, struct archive *out,
                        const char *dest_folder);

int archiveutil_extract(const char *archive_filename,
                        const char *dest_folder) {
    int err = -1;

    // in
    struct archive *in = archive_read_new();

    archive_read_support_format_tar(in);
    archive_read_support_format_zip(in);

    archive_read_support_filter_gzip(in);

    // out
    struct archive *out = archive_write_disk_new();
    archive_write_disk_set_standard_lookup(out);

    // open archive file
    if(archive_read_open_filename(in, archive_filename, 4096)) {
        fprintf(
            stderr,
            "Archive Util: could not load file '%s'\n"
            " - archive_read_open_filename: %s\n",
            archive_filename, archive_error_string(in)
        );
        goto exit;
    }

    err = copy_entries(in, out, dest_folder);

    exit:
    archive_read_close(in);
    archive_read_free(in);

    archive_write_close(out);
    archive_write_free(out);

    return err;
}

static int copy_entries(struct archive *in, struct archive *out,
                        const char *dest_folder) {
    int err = -1;

    struct archive_entry *entry;
    char *entry_path = NULL;

    while(true) {
        // read header
        int r = archive_read_next_header(in, &entry);

        if(r == ARCHIVE_EOF) {
            break;
        } else if(r != ARCHIVE_OK) {
            fprintf(
                stderr,
                "Archive Util: error reading an archive file\n"
                " - archive_read_next_header: %s\n",
                archive_error_string(in)
            );
            goto exit;
        }

        // change output path
        if(dest_folder) {
            if(!entry_path)
                entry_path = malloc(PATH_MAX * sizeof(char));

            snprintf(
                entry_path, PATH_MAX,
                "%s/%s", dest_folder, archive_entry_pathname(entry)
            );
            archive_entry_set_pathname(entry, entry_path);
        }

        // write header
        r = archive_write_header(out, entry);

        if(r != ARCHIVE_OK) {
            fprintf(
                stderr,
                "Archive Util: error extracting an archive file\n"
                " - archive_write_header: %s\n",
                archive_error_string(out)
            );
            goto exit;
        }

        // copy data
        const void *buffer;
        size_t size;
        la_int64_t offset;

        while(true) {
            // read data
            r = archive_read_data_block(in, &buffer, &size, &offset);

            if(r == ARCHIVE_EOF) {
                break;
            } else if(r != ARCHIVE_OK) {
                fprintf(
                    stderr,
                    "Archive Util: error reading data of an archive file\n"
                    " - archive_read_data_block: %s\n",
                    archive_error_string(in)
                );
                break;
            }

            // write data
            r = archive_write_data_block(out, buffer, size, offset);

            if(r != ARCHIVE_OK) {
                fprintf(
                    stderr,
                    "Archive Util: error writing data of an archive file\n"
                    " - archive_write_data_block: %s\n",
                    archive_error_string(out)
                );
                break;
            }
        }

        // close entry file
        r = archive_write_finish_entry(out);

        if(r != ARCHIVE_OK) {
            fprintf(
                stderr,
                "Archive Util: error closing a cartridge file\n"
                " - archive_write_finish_entry: %s\n",
                archive_error_string(out)
            );
            goto exit;
        }
    }
    err = 0;

    exit:
    if(entry_path)
        free(entry_path);

    return err;
}
