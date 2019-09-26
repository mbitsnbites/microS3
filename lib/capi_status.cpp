//--------------------------------------------------------------------------------------------------
// Copyright (c) 2019 Marcus Geelnard
//
// This software is provided 'as-is', without any express or implied warranty. In no event will the
// authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including commercial
// applications, and to alter it and redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not claim that you wrote
//     the original software. If you use this software in a product, an acknowledgment in the
//     product documentation would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and must not be misrepresented as
//     being the original software.
//
//  3. This notice may not be removed or altered from any source distribution.
//--------------------------------------------------------------------------------------------------

#include <us3/us3.h>

US3_EXTERN const char* us3_status_str(const us3_status_t status) {
  switch (status) {
    case US3_SUCCESS:
      return "No error occurred";
    case US3_ERROR:
      return "An unspecified error occurred";
    case US3_INVALID_ARGUMENT:
      return "An invalid argument was passed to a function";
    case US3_INVALID_HANDLE:
      return "An invalid stream handle was passed to a function";
    case US3_INVALID_OPERATION:
      return "An invalid operation was requested";
    case US3_INVALID_URL:
      return "An invalid URL was passed to a function";
    case US3_NO_HOST:
      return "No such host was found";
    case US3_DENIED:
      return "Access denied";
    case US3_REFUSED:
      return "The connection was refused";
    case US3_UNREACHABLE:
      return "The network is unreachable";
    case US3_CONNECTION_RESET:
      return "The connection was reset by the peer";
    case US3_TIMEOUT:
      return "The operation timed out";
    case US3_UNSUPPORTED:
      return "An unsupported protocol function was encountered";
    case US3_NO_SUCH_FIELD:
      return "The requested field was not found";
    case US3_FORBIDDEN:
      return "The server refused to authorize the request";
    case US3_NOT_FOUND:
      return "The object was not found";
    default:
      return "(invalid status code)";
  }
}
