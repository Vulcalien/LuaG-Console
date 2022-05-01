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

#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <archive.h>
#include <archive_entry.h>

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

    // copy entries
    while(true) {
        char entry_path[PATH_MAX];
        struct archive_entry *entry;

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
        while(true) {
            const void *buffer;
            size_t size;
            la_int64_t offset;

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
    archive_read_close(in);
    archive_read_free(in);

    archive_write_close(out);
    archive_write_free(out);

    return err;
}

int archiveutil_pack(const char *archive_filename,
                     const char *src_folder) {
    int err = -1;

    // in
    struct archive *in = archive_read_disk_new();
    archive_read_disk_set_standard_lookup(in);

    // out
    struct archive *out = archive_write_new();
    archive_write_set_format_ustar(out);
    archive_write_add_filter_gzip(out);

    // open folder (in)
    if(archive_read_disk_open(in, src_folder)) {
        fprintf(
            stderr,
            "Archive Util: could not open folder '%s\n'"
            " - archive_read_disk_open: %s\n",
            src_folder, archive_error_string(in)
        );
    }

    // open archive file (out)
    if(archive_write_open_filename(out, archive_filename)) {
        fprintf(
            stderr,
            "Archive Util: coult not create file '%s'\n"
            " - archive_write_open_filename: %s\n",
            archive_filename, archive_error_string(out)
        );
        goto exit;
    }

    // copy entries
    u32 root_name_len = -1;
    while(true) {
        struct archive_entry *entry = archive_entry_new();

        // read header
        int r = archive_read_next_header2(in, entry);

        if(r == ARCHIVE_EOF) {
            break;
        } else if(r != ARCHIVE_OK) {
            fprintf(
                stderr,
                "Archive Util: error\n" // TODO what error?
                " - archive_read_next_header2: %s\n",
                archive_error_string(in)
            );
            goto exit;
        }

        archive_read_disk_descend(in);

        // remove root directory from output path
        if(root_name_len == -1) {
            root_name_len = strlen(archive_entry_sourcepath(entry));

            archive_entry_set_pathname(entry, "");
        } else {
            archive_entry_set_pathname(
                entry, archive_entry_sourcepath(entry) + root_name_len + 1
            );
        }

        // delete user and group
        archive_entry_set_uid(entry, 0);
        archive_entry_set_uname(entry, "");
        archive_entry_set_gid(entry, 0);
        archive_entry_set_gname(entry, "");

        // write header
        r = archive_write_header(out, entry);

        if(r != ARCHIVE_OK) {
            fprintf(
                stderr,
                "Archive Util: error\n" // TODO what error?
                " - archive_write_header: %s\n",
                archive_error_string(out)
            );
            goto exit;
        }

        // copy data
        FILE *file = fopen(archive_entry_sourcepath(entry), "rb");

        while(true) {
            char buffer[4096];
            u32 len = fread(
                buffer,
                sizeof(char), sizeof(buffer) / sizeof(char),
                file
            );
            if(len <= 0)
                break;

            archive_write_data(out, buffer, len);
        }
        fclose(file);

        archive_entry_free(entry);
    }
    err = 0;

    exit:
    archive_read_close(in);
    archive_read_free(in);

    archive_write_close(out);
    archive_write_free(out);

    return err;
}
