#include <stdio.h>
#include "macho_reader.h"

#include <mach-o/loader.h>
#include <string.h>
#include <stdlib.h>

FILE *create_output_file(const char *cpuname) {
  const char *ext = ".xar";
  int length = strlen(cpuname) + strlen(ext);
  char *filename = calloc(sizeof(char), length);
  strcpy(filename, cpuname);
  strcat(filename, ext);

  FILE *output = fopen(filename, "wb");
  free(filename);

  if (!output) {
    printf("Cannot open '%s' for writing\n", filename);
    exit(1);
  }

  return output;
}

void extract_bitcode(FILE *stream, const char *cpuname, const uint32_t offset, const uint32_t size) {
  char *buffer = calloc(sizeof(char), size);
  fseek(stream, offset, SEEK_SET);
  fread(buffer, sizeof(char), size, stream);

  FILE *output = create_output_file(cpuname);
  fwrite(buffer, sizeof(char), size, output);
  free(buffer);
  fclose(output);
}

void extract_bitcode_64(FILE *stream, const char *cpuname, const uint64_t offset, const uint64_t size) {
  char *buffer = calloc(sizeof(char), size);
  fseek(stream, offset, SEEK_SET);
  fread(buffer, sizeof(char), size, stream);

  FILE *output = create_output_file(cpuname);
  fwrite(buffer, sizeof(char), size, output);
  free(buffer);
  fclose(output);
}

void retrieve_bitcode(FILE *stream, const int offset, const int swap_bytes) {
  struct mach_header *header = load_mach_header(stream, offset, swap_bytes);
  const char *cpu_name = get_cpu_type_name(header);
  struct segment_command *segment = load_llvm_segment_command(stream, header, offset, swap_bytes);
  if (segment) {
    extract_bitcode(stream, cpu_name, segment->fileoff, segment->filesize);
    free(segment);
  }

  free(header);
}

void retrieve_bitcode_64(FILE *stream, const int offset, const int swap_bytes) {
  struct mach_header_64 *header = load_mach_header_64(stream, offset, swap_bytes);
  const char *cpu_name = get_cpu_type_name_64(header);
  struct segment_command_64 *segment = load_llvm_segment_command_64(stream, header, offset, swap_bytes);
  if (segment) {
    extract_bitcode_64(stream, cpu_name, segment->fileoff, segment->filesize);
    free(segment);
  }

  free(header);
}

int main(int argc, char *argv[]) {
  const char *filename = argv[1];
  FILE *stream = fopen(filename, "rb");

  uint32_t magic = get_magic(stream, 0);
  int is64 = is_magic_64(magic);
  int swap_bytes = is_should_swap_bytes(magic);

  if (is64) {
    retrieve_bitcode_64(stream, 0, swap_bytes);
  } else {
    retrieve_bitcode(stream, 0, swap_bytes);
  }

  fclose(stream);
  return 0;
}

