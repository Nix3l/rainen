#include "platform.h"

#include <errno.h>

#include "game.h"
#include "util/log.h"

#if OS_LINUX

#define FILE_PATH_DELIM "/"

// TODO(nix3l): redo this whole thing to somehow use the executable's absolute path
// for now however i can do the hacky approach

static usize get_file_length(FILE* file) {
    fseek(file, 0L, SEEK_END);
    usize length = ftell(file);
    rewind(file);
    return length;
}

char* platform_load_text_from_file(char* filename, usize* out_length, arena_s* arena) {
    FILE* file = fopen(filename, "r");
    if(!file) {
        LOG_ERR("failed to open [%s]: err %d\n%s\n", filename, errno, strerror(errno));
        return NULL;
    }

    usize length = get_file_length(file);

    if(!length) {
        LOG_ERR("failed to read length of [%s]: err %d\n%s\n", filename, errno, strerror(errno));
        fclose(file);
        return NULL;
    }

    if(!arena_fits(arena, length)) {
        LOG_ERR("not enough space in arena to output contents of file [%s]\n", filename);
        fclose(file);
        return NULL;
    }

    char* output = arena_push(arena, length + 1);
    MEM_ZERO(output, length + 1);
    usize read_length = fread(output, 1, length, file);
    if(read_length != length) {
        LOG_ERR("failed to read contents of [%s]: err %d\n%s\n", filename, errno, strerror(errno));
        fclose(file);
        return NULL;
    }

    output[length + 1] = '\0';

    if(out_length) *out_length = length + 1;

    fclose(file);
    return output;
}

char** platform_load_lines_from_file(char* filepath, usize* out_num_lines, arena_s* arena) {
    FILE* file = fopen(filepath, "r");
    if(!file) {
        LOG_ERR("failed to open [%s]: err %d\n%s\n", filepath, errno, strerror(errno));
        return NULL;
    }

    // TODO(nix3l): error checking for failures in the std functions

    usize num_lines = 0;

    // load the entire file into transient memory
    // and get the number of new lines in the process
    usize length = get_file_length(file);
    char* file_contents = arena_push(arena, length + 1);

    usize read_length = fread(file_contents, 1, length, file);
    if(read_length != length) {
        LOG_ERR("failed to read contents of [%s]: err %d\n%s\n", filepath, errno, strerror(errno));
        fclose(file);
        return NULL;
    }

    for(usize i = 0; i < length; i ++)
        if(file_contents[i] == '\n')
            num_lines ++;

    file_contents[length + 1] = '\0';
    fclose(file);

    char** output = arena_push(arena, num_lines * sizeof(char*));

    char* curr_str_view = file_contents;
    output[0] = file_contents;
    for(usize i = 1; i < num_lines; i ++) {
        // loop through the file and add a pointer to wherever
        // a new line is found
        for(;;) {
            if(*curr_str_view == '\n') {
                output[i] = curr_str_view + 1;
                // change the \n to \0 to make the strings separate in memory
                *curr_str_view = '\0';
                break;
            }

            curr_str_view++;
        }
    }

    if(out_num_lines) *out_num_lines = num_lines;
    return output;
}

void* platform_load_file(char* filepath, usize* buff_length, arena_s* arena) {
    FILE* file = fopen(filepath, "rb");
    if(!file) {
        LOG_ERR("failed to open [%s]: err %d\n%s\n", filepath, errno, strerror(errno));
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    usize length = ftell(file);
    rewind(file);

    if(!length) {
        LOG_ERR("failed to read length of [%s]: err %d\n%s\n", filepath, errno, strerror(errno));
        fclose(file);
        return NULL;
    }

    if(!arena_fits(arena, length)) {
        LOG_ERR("not enough space in arena to output contents of file [%s]\n", filepath);
        fclose(file);
        return NULL;
    }

    void* output = arena_push(arena, length);
    usize read_length = fread(output, 1, length, file);
    if(read_length != length) {
        LOG_ERR("failed to read contents of [%s]: err %d\n%s\n", filepath, errno, strerror(errno));
        fclose(file);
        return NULL;
    }

    if(buff_length) *buff_length = length;

    fclose(file);
    return output;
}

void platform_write_to_file(char* filepath, void* data, usize data_size, bool append) {
    FILE* file = fopen(filepath, append ? "ab" : "wb");
    if(!file) {
        LOG_ERR("failed to open [%s]: err %d\n%s\n", filepath, errno, strerror(errno));
        return;
    }

    if(fwrite(data, data_size, 1, file) != 1)
        LOG_ERR("error writing data to file [%s]\n", filepath);

    fclose(file);
}

char* platform_get_file_extension(char* filepath) {
    // returns a pointer starting from the last '.' in the filepath
    return strrchr(filepath, '.');
}

// NOTE(nix3l): unsure whether using the std provided string functions is good or not
// but it works for now
char* platform_get_res_path(char* filename, arena_s* arena) {
    usize length = strlen(RESOURCES_PATH) + strlen(FILE_PATH_DELIM) + strlen(filename) + 1;
    char* str = arena_push(arena, length);
    strcat(str, RESOURCES_PATH);
    strcat(str, FILE_PATH_DELIM);
    strcat(str, filename);
    str[length] = '\0';
    return str;
}

#endif // OS_LINUX
