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

#ifndef US3_US3_H_
#define US3_US3_H_

#include <stddef.h> /* For size_t */

#if defined(US3_SHARED_LIB)
#  if defined(US3_BUILDING_LIBRARY)
#    if defined(_MSC_VER)
#      define US3_API_VISIBILITY __declspec(dllexport)
#    else
#      define US3_API_VISIBILITY __attribute__((visibility("default")))
#    endif
#  else
#    if defined(_MSC_VER)
#      define US3_API_VISIBILITY __declspec(dllimport)
#    else
#      define US3_API_VISIBILITY
#    endif
#  endif
#else
#  define US3_API_VISIBILITY
#endif

#ifdef __cplusplus
#  define US3_API extern "C" US3_API_VISIBILITY
#else
#  define US3_API US3_API_VISIBILITY
#endif

/**
 * @file
 * @mainpage
 *
 * @section intro_sec Introduction
 *
 * microS3 (μS3 for short) is a small client library for interacting with S3 object storage
 * services.
 *
 * It is designed to work even on restricted machines, such as embedded devices.
 *
 * @section funcs_sec Public API functions
 *
 * @li us3_status_str() - Convert a status code to a string.
 *
 * @li us3_open() - Open an S3 stream.
 * @li us3_close() - Close an S3 stream.
 * @li us3_read() - Read data from an S3 stream.
 * @li us3_write() - Write data to an S3 stream.
 *
 * @li us3_get_status_line() - Get the HTTP response status line.
 * @li us3_get_response_field() - Get a HTTP response field value.
 * @li us3_get_content_length() - Get the S3 stream content length (in bytes)
 *
 * @section types_sec About API types
 *
 * All strings are interpreted as UTF-8 encoded, zero-terminated char strings.
 */

/** @brief Return value for μS3 functions. */
typedef int us3_status_t;
#define US3_SUCCESS 0           /**< No error occurred. */
#define US3_ERROR 1             /**< An unspecified error occurred. */
#define US3_INVALID_ARGUMENT 2  /**< An invalid argument was passed to a function. */
#define US3_INVALID_HANDLE 3    /**< An invalid stream handle was passed to a function. */
#define US3_INVALID_OPERATION 4 /**< An invalid operation was requested. */
#define US3_INVALID_URL 5       /**< An invalid URL was passed to a function. */
#define US3_NO_HOST 6           /**< No such host was found. */
#define US3_DENIED 7            /**< Access denied. */
#define US3_REFUSED 8           /**< The connection was refused. */
#define US3_UNREACHABLE 9       /**< The network is unreachable. */
#define US3_CONNECTION_RESET 10 /**< The connection was reset by the peer. */
#define US3_TIMEOUT 11          /**< The operation timed out. */
#define US3_UNSUPPORTED 12      /**< An unsupported protocol function was encountered. */
#define US3_NO_SUCH_FIELD 13    /**< The requested field was not found. */
#define US3_FORBIDDEN 14        /**< The server refused to authorize the request. */
#define US3_NOT_FOUND 15        /**< The object was not found. */

/** @brief Stream mode. */
typedef int us3_mode_t;
#define US3_READ 0  /**< Open a stream in read mode (GET). */
#define US3_WRITE 1 /**< Open a stream in write mode (PUT). */

/** @brief A stream handle. */
typedef struct us3_handle_struct_t* us3_handle_t;
struct us3_handle_struct_t;

/** @brief A timeout value, in microseconds (μs). */
typedef long us3_microseconds_t;

/** @brief A value that requests an infinite timeout. */
#define US3_NO_TIMEOUT 0

/**
 * @brief Convert a status code to a string.
 * @param status The status code.
 * @returns a string that describes the status code.
 */
US3_API const char* us3_status_str(us3_status_t status);

/**
 * @brief Open an S3 stream.
 * @param url Complete S3 URL.
 * @param access_key The S3 access key.
 * @param secret_key The S3 secret key.
 * @param mode Open mode.
 * @param size Number of bytes to write (ignored when mode is not WRITE).
 * @param connect_timeout Connection timeout in microseconds, or US3_NO_TIMEOUT for no timeout.
 * @param socket_timeout Socket timeout in microseconds, or US3_NO_TIMEOUT for no timeout.
 * @param[out] handle The resulting handle.
 * @returns US3_SUCCESS on success, otherwise an error code.
 */
US3_API us3_status_t us3_open(const char* url,
                              const char* access_key,
                              const char* secret_key,
                              us3_mode_t mode,
                              size_t size,
                              us3_microseconds_t connect_timeout,
                              us3_microseconds_t socket_timeout,
                              us3_handle_t* handle);

/**
 * @brief Close an S3 stream.
 * @param handle The stream handle to close.
 * @returns US3_SUCCESS on success, otherwise an error code.
 */
US3_API us3_status_t us3_close(us3_handle_t handle);

/**
 * @brief Read data from an S3 stream.
 * @param handle The stream handle.
 * @param buf The target buffer.
 * @param count The number of bytes to read.
 * @param[out] actual_count The actual number of bytes read.
 * @returns US3_SUCCESS on success, otherwise an error code.
 */
US3_API us3_status_t us3_read(us3_handle_t handle, void* buf, size_t count, size_t* actual_count);

/**
 * @brief Write data to an S3 stream.
 * @param handle The stream handle.
 * @param buf The source data buffer.
 * @param count The number of bytes to write.
 * @param[out] actual_count The actual number of bytes written.
 * @returns US3_SUCCESS on success, otherwise an error code.
 */
US3_API us3_status_t us3_write(us3_handle_t handle,
                               const void* buf,
                               size_t count,
                               size_t* actual_count);

/**
 * @brief Get the HTTP response status line.
 * @param handle The stream handle to query.
 * @param[out] status_line The HTTP response status line value.
 * @returns US3_SUCCESS on success, otherwise an error code.
 */
US3_API us3_status_t us3_get_status_line(us3_handle_t handle, const char** status_line);

/**
 * @brief Get a HTTP response field value.
 * @param handle The stream handle to query.
 * @param name The lower case name of the field (e.g. "content-type").
 * @param[out] value The field value.
 * @returns US3_SUCCESS on success, otherwise an error code. If the field was not given in the HTTP
 * response message, US3_NO_SUCH_FIELD is returned (and *value is set to NULL).
 */
US3_API us3_status_t us3_get_response_field(us3_handle_t handle,
                                            const char* name,
                                            const char** value);

/**
 * @brief Get the S3 stream content length (in bytes).
 * @param handle The stream handle to query.
 * @param[out] content_length The content length as specified by the HTTP response.
 * @returns US3_SUCCESS on success, otherwise an error code. If the content length was not given in
 * the HTTP response message, US3_NO_SUCH_FIELD is returned (and *content_length is set to 0).
 */
US3_API us3_status_t us3_get_content_length(us3_handle_t handle, size_t* content_length);

#endif /* US3_US3_H_ */
