## Test environments

<!-- GHA_TEST_ENVIRONMENTS_START -->
This section is completed by GitHub Actions for releases and manual dispatches.
<!-- GHA_TEST_ENVIRONMENTS_END -->

## R CMD check results

<!-- GHA_CHECK_RESULTS_START -->
This section is completed by GitHub Actions for releases and manual dispatches.
<!-- GHA_CHECK_RESULTS_END -->

## New submission

This is the first CRAN submission of HanNLP.

HanNLP ports the KoNLP/HanNanum morphological analyzer backend to native C and
therefore does not require Java or rJava at runtime.

## Bundled data and package size

The package bundles HanNanum runtime dictionary/statistical data and NIADic data
needed for offline morphological analysis and dictionary management. Copyright
and license notices for bundled third-party materials are provided in
`inst/COPYRIGHTS`.

The installed package size is about 10 MB, most of which is runtime dictionary
data required by the analyzer.
