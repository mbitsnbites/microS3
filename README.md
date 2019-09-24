# microS3

microS3 (μS3 for short) is a small client library for interacting with S3 object storage services.

It is designed to work even on restricted machines, such as embedded devices.

## Architecture

The library is implemented in C++03, and exposes a C89 API. It has no external dependencies except for system level functionality (network sockets and crypto).

## License

The library is released under the very liberal [zlib/libpbg license](https://opensource.org/licenses/Zlib).

## Building

Use [CMake](https://cmake.org/) to build the library and tools. For example:

```bash
$ mkdir build && cd build
$ cmake ../
$ cmake --build .
```

## Quick start

You can easily test microS3 against an S3 server with the `us3get` and `us3put` tools. For example, start a [MinIO](https://min.io/) server using [Docker](https://www.docker.com/) and download a file using `us3get`:

```bash
$ mkdir -p /tmp/myminio/mybucket
$ echo "Hello world!" > /tmp/myminio/mybucket/hello.txt
$ docker run --rm -p 9000:9000 \
    -e "MINIO_ACCESS_KEY=myAccessKey" \
    -e "MINIO_SECRET_KEY=SuperSECR3TkEY" \
    -v /tmp/myminio:/data \
    minio/minio server /data &
$ tools/us3get \
    -a myAccessKey \
    -s SuperSECR3TkEY \
    http://localhost:9000/mybucket/hello.txt \
    /tmp/downloaded-hello.txt
$ cat /tmp/downloaded-hello.txt
```

## Example usage

Here is an example C program that reads data from an S3 object:

```c
#include <us3/us3.h>
#include <stdio.h>
#include <stdlib.h>

void my_read_function() {
    static char buffer[8192];
    us3_status_t status;

    /* Open an S3 stream for reading. */
    us3_handle_t handle;
    status = us3_open("http://myhost:9000/abucket/somefile.foo",
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

        /* Do something with the data... */
        /* ... */
    }

    /* Close the S3 stream. */
    us3_close(handle);
}
```
