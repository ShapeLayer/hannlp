#!/usr/bin/env bash

# Build an R source package and run the CRAN checks against the tarball.
# The package is staged in a temporary directory so R CMD build does not
# leave generated files in the working tree.

set -euo pipefail

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
package_dir=$(CDPATH= cd -- "$script_dir/.." && pwd)
package_name=$(basename -- "$package_dir")
output_dir=${CRAN_CHECK_OUTPUT_DIR:-"$package_dir/cran-submission"}

if ! command -v R >/dev/null 2>&1; then
  echo "error: R is not available on PATH" >&2
  exit 127
fi

mkdir -p "$output_dir"
staging_dir=$(mktemp -d "${TMPDIR:-/tmp}/${package_name}-cran.XXXXXX")
trap 'rm -rf "$staging_dir"' EXIT

cp -R "$package_dir" "$staging_dir/$package_name"
rm -rf "$staging_dir/$package_name/.git" "$staging_dir/$package_name/.Rproj.user"

echo "==> R CMD build"
(
  cd "$staging_dir"
  R CMD build --md5 "$package_name" 2>&1 | tee "$output_dir/${package_name}-build.log"
)

tarball=$(find "$staging_dir" -maxdepth 1 -type f -name "${package_name}_*.tar.gz" -print -quit)
if [[ -z "$tarball" ]]; then
  echo "error: R CMD build did not produce ${package_name}_*.tar.gz" >&2
  exit 1
fi

final_tarball="$output_dir/$(basename -- "$tarball")"
cp "$tarball" "$final_tarball"

echo "==> R CMD check --as-cran"
set +e
R CMD check --as-cran --output="$output_dir" "$final_tarball" 2>&1 | tee "$output_dir/${package_name}-check.log"
check_status=${PIPESTATUS[0]}
set -e

if [[ "$check_status" -ne 0 ]]; then
  echo "CRAN check failed (exit status: $check_status)" >&2
  exit "$check_status"
fi

echo "CRAN check passed: $final_tarball"
