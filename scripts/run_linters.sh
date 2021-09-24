#!/usr/bin/env bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
REPO_ROOT=${SCRIPT_DIR}/../

# The build folder must be passed as the first argument.
if [ "$#" -ne 1 ]; then
  echo "Usage: $0 BUILD_DIR"
  exit 1
fi
BUILD_DIR=$1

# Run shellcheck (aah, the meta).
c="$(command -v shellcheck)"
if [ -n "$c" ]; then
  echo "Running $c"
  # shellcheck disable=SC2046
  "$c" $(git -C "${REPO_ROOT}" ls-files '*.sh') || exit 1
else
  echo "WARNING: shellcheck not found!"
fi

# Run clang-format.
c="$(command -v clang-format)"
if [ -n "$c" ]; then
  echo "Running $c"
  # shellcheck disable=SC2046
  "$c" --dry-run --Werror $(git -C "${REPO_ROOT}" ls-files '*.cpp' '*.hpp' '*.c' '*.h' | grep -v 'ext/') || exit 1
else
  echo "WARNING: clang-format not found!"
fi

# Run clang-tidy.
c="$(command -v clang-tidy)"
if [ -n "$c" ]; then
  echo "Running $c"
  # Collect all the C/C++ source files from the compileation database.
  SRC_FILES=$(grep '"file":' "${BUILD_DIR}/compile_commands.json" | grep -v 'ext/' | sed 's| \+"file": \+||g' | sed 's|"||g')
  # shellcheck disable=SC2086
  "$c" --quiet -p "${BUILD_DIR}" ${SRC_FILES} 2>/dev/null || exit 1
else
  echo "WARNING: clang-tidy not found!"
fi

echo "All tests passed! 👍"

