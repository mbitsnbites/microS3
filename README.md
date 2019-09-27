# microS3 [![Build Status](https://travis-ci.org/mbitsnbites/microS3.svg?branch=master)](https://travis-ci.org/mbitsnbites/microS3)

* [About](#about)
  * [Architecture](#architecture)
  * [License](#license)
  * [Known limitations](#known-limitations)
* [Usage](#usage)
  * [Building](#building)
  * [Quick start](#quick-start)
  * [Example usage](#example-usage)

# About

microS3 (μS3 for short) is a small, portable client library for interacting with S3 object storage services.

Supported platforms:
* Linux and most POSIX compatible systems (e.g. FreeBSD)
* macOS
* Windows

## Architecture

The library is designed to work even on restricted machines, such as embedded devices.

It is implemented in C++03 and exposes a C89 API. It has no external dependencies except for system level functionality (network sockets).

## License

The library is released under the very liberal [zlib/libpbg license](https://opensource.org/licenses/Zlib).

## Known limitations

As an S3 client library, microS3 is not feature complete. This is by design, since the primary goal is to provide basic S3 I/O capabilities to C/C++ tools and applications that just need to consume or produce data blobs.

* S3 limitations:
  * Only a small subset of the S3 protocol is supported (GET Object, PUT Object).
  * Functionality for listing or deleting objects and buckets is missing.
* HTTP limitations:
  * Only basic HTTP support (e.g. [redirection](https://developer.mozilla.org/en-US/docs/Web/HTTP/Redirections), [basic authentication](https://en.wikipedia.org/wiki/Basic_access_authentication) and [chunked transfers](https://en.wikipedia.org/wiki/Chunked_transfer_encoding) are unsupported).
  * No HTTPS support.
* Also see the [project issues](https://github.com/mbitsnbites/microS3/issues).

Missing features will be added to the library when the need arises.

# Usage

## Building

Use [CMake](https://cmake.org/) to build the library and the tools. For example:

```bash
$ mkdir build && cd build
$ cmake -G Ninja ../
$ ninja
```

To install the library and the tools, do:

```bash
$ sudo ninja install
```

To run the unit tests, do:

```bash
$ ctest --output-on-failure
```

## Quick start

You can easily test microS3 against an S3 server with the `us3get` and `us3put` tools. For example, start a [MinIO](https://min.io/) server using [Docker](https://www.docker.com/) and download a file using `us3get`:

```bash
$ mkdir -p /tmp/s3/mybucket
$ echo "Hello world!" > /tmp/s3/mybucket/hello.txt
$ docker run --rm -d -p 9000:9000 \
    --name s3server \
    -e "MINIO_ACCESS_KEY=myAccessKey" \
    -e "MINIO_SECRET_KEY=SuperSECR3TkEY" \
    -v /tmp/s3:/data \
    minio/minio server /data
$ tools/us3get \
    -a myAccessKey \
    -s SuperSECR3TkEY \
    http://localhost:9000/mybucket/hello.txt \
    /tmp/downloaded-hello.txt
$ cat /tmp/downloaded-hello.txt
```

You can stop the S3 service container with:

```bash
$ docker stop s3server
```

## Example usage

Here is an example C program that reads data from an S3 object:

```c
#include <stdio.h>
#include <stdlib.h>
#include <us3/us3.h>

int main(void) {
  static char buffer[8192];
  us3_status_t status;

  /* Open an S3 stream for reading. */
  us3_handle_t handle;
  status = us3_open("http://localhost:9000/mybucket/hello.txt",
                    "myAccessKey",
                    "SuperSECR3TkEY",
                    US3_READ,
                    0,
                    US3_NO_TIMEOUT,
                    US3_NO_TIMEOUT,
                    &handle);
  if (status != US3_SUCCESS) {
    fprintf(stderr, "*** Error: %s\n", us3_status_str(status));
    exit(1);
  }

  while (1) {
    /* Read data from the stream. */
    size_t bytes_read;
    status = us3_read(handle, &buffer[0], sizeof(buffer), &bytes_read);
    if (status != US3_SUCCESS) {
      fprintf(stderr, "*** Read error: %s\n", us3_status_str(status));
      exit(1);
    } else if (bytes_read == 0) {
      /* Done! */
      break;
    }

    /* Do something with the data (e.g. print to stdout)... */
    fwrite(&buffer[0], 1, bytes_read, stdout);
  }

  /* Close the S3 stream. */
  us3_close(handle);
}
```
