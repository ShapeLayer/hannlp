## Tests

<!-- GHA_TESTS_START -->
This section is completed by GitHub Actions for check, release, and manual runs.
<!-- GHA_TESTS_END -->


## Resubmission

This resubmission addresses the review comments:

* Removed the unnecessary `\dontrun{}` wrapper from the morphology example.
* Removed the default user-home dictionary path. Dictionary writes now require
  an explicitly supplied `HANNLP_USER_DATA_DIR`; analysis and current-dictionary
  reads do not create files.
* Backups stay inside the explicitly selected directory. Examples and tests
  use temporary directories and clean up their files.
* Removed cached destination paths so changes to the explicit directory take
  effect immediately.

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
