# RFunctionGrabber

Simple `C`-function that uses `regex.h` to grep all function definitions from
an `R`-script, open a new file with the name of the function and write into
this file until a new function definition is encountered. The script currently
assumes that all function definitions are written as:

```{.r}
name <- function (
```

where the number of spaces between all tokens can be variable and the name
adheres to the conventions specified by `R`. Closures are not supported,
otherwise this would require writing a full-blown parser.
