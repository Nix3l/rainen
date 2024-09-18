#ifndef PLATFROM_LAYERH
#define PLATFROM_LAYERH

#include "base.h"
#include "memory/memory.h"

// TODO(nix3l): figure out what exactly should be in the platform layer and what should be in the application layer

#ifndef RESOURCES_PATH
#define RESOURCES_PATH "res"
#endif

// returns all the text in a given filepath
// length of the file is returned in length_read. can be null if not needed
char* platform_load_text_from_file(char* filepath, usize* length_read, arena_s* arena);

// returns all the lines in a given filepath as a 2-d array
// number of files is returned in num_lines. can be null if not needed
char** platform_load_lines_from_file(char* filepath, usize* num_lines, arena_s* arena);

// reads a file and loads all its bytes into memory
// length of the file read is returned in buff_length. may be null if not needed
void* platform_load_file(char* filepath, usize* buff_length, arena_s* arena);

// write/append data from the provided void* to a given filepath
void platform_write_to_file(char* filepath, void* data, usize data_size, bool append);

// returns a string view to the file extension
// be careful as this doesnt return a pointer to independent data
// returns null if the filepath does not have an extension
char* platform_get_file_extension(char* filepath);

// prepends the RESOURCES_PATH to the given filename
char* platform_get_res_path(char* filename, arena_s* arena);
// TODO(nix3l): char* platform_get_shaders_path(char* filename, arena_s* arena);

#endif
