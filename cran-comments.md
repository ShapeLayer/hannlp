## Tests

<!-- GHA_TESTS_START -->
This section is completed by GitHub Actions for check, release, and manual runs.
<!-- GHA_TESTS_END -->

## New submission

This is the first CRAN submission of HanNLP.

HanNLP ports the KoNLP/HanNanum morphological analyzer backend to native C and
therefore does not require Java or rJava at runtime.

## Notes for the reviewer

* Installed size is about 5 MB. Most of it is the bundled dictionary and
  statistical data required for out-of-the-box analysis.

* The package is GPL-3 and bundles GPL-3-licensed 'JHanNanum'/'HanNanum'
  derived code and data. Copyright and license details are in
  `inst/COPYRIGHTS` and `LICENSE`.

* 'Sejong' and 'NIADic' are optional, non-CRAN packages. They are not listed
  in `Suggests`, are accessed only conditionally at runtime, and are not used
  by examples or tests. No 'NIADic' data is bundled.
