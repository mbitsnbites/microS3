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
  fprintf(stderr, "Usage: %s [options] URL [FILE]\n\n", program);
  fprintf(stderr, "  URL   The S3 object URL\n");
  fprintf(stderr, "  FILE  Output file (optional)\n\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -a, --access-key KEY      The S3 access key\n");
  fprintf(stderr, "  -A, --access-key-env ENV  Name of an environment variable holding the\n");
  fprintf(stderr, "                            S3 access key\n");
  fprintf(stderr, "  -s, --secret-key KEY      The S3 secret key\n");
  fprintf(stderr, "  -S, --secret-key-env ENV  Name of an environment variable holding the\n");
  fprintf(stderr, "                            S3 secret key\n\n");
  fprintf(stderr, "If FILE is not specified, the data will be written to stdout.\n");
}

int main(const int argc, const char** argv) {
  const char* program = argv[0];
  const char* url = NULL;
  const char* file_name = NULL;
  const char* access_key = NULL;
  const char* secret_key = NULL;
  int bad_args = 0;
  int i;
  int output_to_file;
  FILE* file;
  int exit_status = EXIT_FAILURE;

  /* Parse arguments. */
  us3_handle_t s3_handle;

  for (i = 1; i < argc; ++i) {
    if ((strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0)) {
      show_usage(program);
      exit(EXIT_SUCCESS);
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
    } else if (url == NULL) {
      url = argv[i];
    } else if (file_name == NULL) {
      file_name = argv[i];
    } else {
      bad_args = 1;
      break;
    }
  }
  if (bad_args || (url == NULL) || (access_key == NULL) || (secret_key == NULL)) {
    show_usage(program);
    exit(EXIT_FAILURE);
  }

  /* Open the S3 stream. */
  {
    us3_status_t open_status = us3_open_url(
        url, access_key, secret_key, US3_READ, US3_NO_TIMEOUT, US3_NO_TIMEOUT, &s3_handle);
    if (open_status != US3_SUCCESS) {
      fprintf(stderr, "*** Unable to open %s: %s\n", url, us3_status_str(open_status));
      exit(EXIT_FAILURE);
    }
  }

  /* Open the output file. */
  output_to_file = 0;
  file = stdout;
  if (file_name != NULL) {
    file = fopen(file_name, "wb");
    if (file == NULL) {
      fprintf(stderr, "*** Unable to open %s for output\n", file_name);
      us3_close(s3_handle);
      exit(EXIT_FAILURE);
    }
    output_to_file = 1;
  }

  /* Read & write... */
  while (1) {
    size_t bytes_in_buf;
    size_t bytes_written;
    us3_status_t read_status = us3_read(s3_handle, &s_buffer[0], BUFFER_SIZE, &bytes_in_buf);
    if (read_status != US3_SUCCESS) {
      fprintf(stderr, "*** Read error: %s\n", us3_status_str(read_status));
      break;
    }
    if (bytes_in_buf == 0) {
      /* Done! */
      exit_status = EXIT_SUCCESS;
      break;
    }
    bytes_written = fwrite(&s_buffer[0], 1, bytes_in_buf, file);
    if (bytes_written != bytes_in_buf) {
      fprintf(stderr, "*** Write error\n");
      break;
    }
  }

  /* Close the handles. */
  if (output_to_file) {
    fclose(file);
  }
  us3_close(s3_handle);

  exit(exit_status);
}
