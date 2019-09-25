/*--------------------------------------------------------------------------------------------------
 * Copyright (c) 2019 Marcus Geelnard
 *
 * This software is provided 'as-is', without any express or implied warranty. In no event will the
 * authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not claim that you wrote
 *     the original software. If you use this software in a product, an acknowledgment in the
 *     product documentation would be appreciated but is not required.
 *
 *  2. Altered source versions must be plainly marked as such, and must not be misrepresented as
 *     being the original software.
 *
 *  3. This notice may not be removed or altered from any source distribution.
 *------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <us3/us3.h>

#define BUFFER_SIZE 32768
static char s_buffer[BUFFER_SIZE];

static void show_usage(const char* program) {
  fprintf(stderr, "Usage: %s [options] FILE URL\n\n", program);
  fprintf(stderr, "  FILE  Input file\n");
  fprintf(stderr, "  URL   The target S3 object URL\n\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -a, --access-key KEY      The S3 access key\n");
  fprintf(stderr, "  -A, --access-key-env ENV  Name of an environment variable holding the\n");
  fprintf(stderr, "                            S3 access key\n");
  fprintf(stderr, "  -s, --secret-key KEY      The S3 secret key\n");
  fprintf(stderr, "  -S, --secret-key-env ENV  Name of an environment variable holding the\n");
  fprintf(stderr, "                            S3 secret key\n\n");
  fprintf(stderr, "  -v, --verbose             Be verbose\n");
}

int main(const int argc, const char** argv) {
  const char* program = argv[0];
  const char* file_name = NULL;
  const char* url = NULL;
  const char* access_key = NULL;
  const char* secret_key = NULL;
  int verbose = 0;
  int bad_args = 0;
  int i;
  FILE* file;
  size_t file_size;
  int exit_status = EXIT_FAILURE;

  /* Parse arguments. */
  us3_handle_t s3_handle;

  for (i = 1; i < argc; ++i) {
    if ((strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0)) {
      show_usage(program);
      exit(EXIT_SUCCESS);
    } else if ((strcmp(argv[i], "--verbose") == 0) || (strcmp(argv[i], "-v") == 0)) {
      verbose = 1;
    } else if ((strcmp(argv[i], "--access-key") == 0) || (strcmp(argv[i], "-a") == 0)) {
      if (i >= (argc - 1)) {
        bad_args = 1;
        break;
      }
      access_key = argv[i + 1];
      ++i;
    } else if ((strcmp(argv[i], "--access-key-env") == 0) || (strcmp(argv[i], "-A") == 0)) {
      if (i >= (argc - 1)) {
        bad_args = 1;
        break;
      }
      access_key = getenv(argv[i + 1]);
      if (access_key == NULL) {
        fprintf(stderr, "*** No such environment variable: %s\n", argv[i + 1]);
        exit(EXIT_FAILURE);
      }
      ++i;
    } else if ((strcmp(argv[i], "--secret-key") == 0) || (strcmp(argv[i], "-s") == 0)) {
      if (i >= (argc - 1)) {
        bad_args = 1;
        break;
      }
      secret_key = argv[i + 1];
      ++i;
    } else if ((strcmp(argv[i], "--secret-key-env") == 0) || (strcmp(argv[i], "-S") == 0)) {
      if (i >= (argc - 1)) {
        bad_args = 1;
        break;
      }
      secret_key = getenv(argv[i + 1]);
      if (secret_key == NULL) {
        fprintf(stderr, "*** No such environment variable: %s\n", argv[i + 1]);
        exit(EXIT_FAILURE);
      }
      ++i;
    } else if (file_name == NULL) {
      file_name = argv[i];
    } else if (url == NULL) {
      url = argv[i];
    } else {
      bad_args = 1;
      break;
    }
  }
  if (bad_args || (file_name == NULL) || (url == NULL) || (access_key == NULL) ||
      (secret_key == NULL)) {
    show_usage(program);
    exit(EXIT_FAILURE);
  }

  /* Open the input file. */
  file = fopen(file_name, "rb");
  if (file == NULL) {
    fprintf(stderr, "*** Unable to open %s for input\n", file_name);
    exit(EXIT_FAILURE);
  }

  /* Determine the file size. */
  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  /* Open the S3 stream. */
  {
    us3_status_t open_status = us3_open(url,
                                        access_key,
                                        secret_key,
                                        US3_WRITE,
                                        file_size,
                                        US3_NO_TIMEOUT,
                                        US3_NO_TIMEOUT,
                                        &s3_handle);
    if (open_status != US3_SUCCESS) {
      fprintf(stderr, "*** Unable to open %s: %s\n", url, us3_status_str(open_status));
      fclose(file);
      exit(EXIT_FAILURE);
    }
  }

  /* Read & write... */
  {
    size_t bytes_left = file_size;
    while (bytes_left > 0) {
      size_t bytes_to_read;
      size_t bytes_in_buf;
      size_t bytes_written;
      us3_status_t write_status;
      const char* write_ptr;

      /* Read from the file. */
      bytes_to_read = bytes_left < BUFFER_SIZE ? bytes_left : BUFFER_SIZE;
      bytes_in_buf = fread(&s_buffer[0], 1, bytes_to_read, file);
      if (bytes_in_buf != bytes_to_read) {
        fprintf(stderr, "*** Read error\n");
        break;
      }

      /* Write to the S3 stream. */
      write_ptr = &s_buffer[0];
      while (bytes_in_buf > 0) {
        write_status = us3_write(s3_handle, write_ptr, bytes_in_buf, &bytes_written);
        if (write_status != US3_SUCCESS) {
          fprintf(stderr, "*** Write error: %s\n", us3_status_str(write_status));
          break;
        }
        write_ptr += bytes_written;
        bytes_in_buf -= bytes_written;
        bytes_left -= bytes_written;
      }
    }
    if (bytes_left == 0) {
      /* Done! */
      exit_status = EXIT_SUCCESS;
    }
  }

  /* Print some info... */
  if (verbose) {
    const char* status_line;
    if (us3_get_status_line(s3_handle, &status_line) == US3_SUCCESS) {
      fprintf(stderr, "Status: %s\n", status_line);
    }
    fprintf(stderr, "Content length: %lu\n", (unsigned long)file_size);
  }

  /* Close the handles. */
  us3_close(s3_handle);
  fclose(file);

  exit(exit_status);
}
