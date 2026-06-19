## Tests

<!-- GHA_TESTS_START -->
This section is completed by GitHub Actions for check, release, and manual runs.
<!-- GHA_TESTS_END -->

## New submission

This is the first CRAN submission of HanNLP.

HanNLP ports the KoNLP/HanNanum morphological analyzer backend to native C and
therefore does not require Java or rJava at runtime.

## Notes for the reviewer

* Installed size is around 9.7 MB. The bulk is bundled dictionary and
  statistical data (the `modules` sub-directory) that the morphological
  analyzer needs to run out of the box. The data cannot be reduced further
  without breaking analysis for users who do not install the optional
  external dictionary packages.

* The package code is licensed under GPL-3. It additionally bundles
  third-party code and data derived from the GPL-3 licensed JHanNanum /
  HanNanum materials, plus small MIT-licensed string buffer utilities
  derived from commonmark/cmark. The MIT license is compatible with the
  GPL. All third-party components, their copyright holders, and their
  licenses are documented in `inst/COPYRIGHTS` and the `LICENSE` file, and
  the DESCRIPTION uses `License: GPL-3 + file LICENSE`.

* `Sejong` and `NIADic` are optional and are not available on CRAN, so they
  are not listed under `Suggests`. No NIADic data is bundled or distributed
  (NIADic is CC BY-SA, which is incompatible with the GPL); `useNIADic()`
  works only when a user has separately installed the `NIADic` package and
  loaded it in their session. These packages are accessed defensively at
  runtime (via `isNamespaceLoaded()` / `system.file()`) and are never
  required by examples or tests.

* There are no references describing the methods in this package.
