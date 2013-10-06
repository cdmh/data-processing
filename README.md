#C++ Data Processing library

This repository will build into a library to ease manipulation and managing data files for data processing.

Stable releases will be merged into the `master` branch.
See the `develop` branch for current status.

## Dataset
A `dataset` is a grid representation of the imported CSV file, with each `column` having a defined type. Each `cell` (a cross section of a row and a column) also has a defined, which is either the same as the column type, or `null_type` where the column has no value.

###Row-wise operations
###Column-wise operations
A representation of a column of data can be accessed by calling the `column()` function on the dataset. A `column_data` object is returned, which provides column-wise operations, which ignore cells in the column that do no contain a value. To see why this is important, consider the calculating the mean average value in a column. A naive implementation using the public interface may be written
```mapped_csv csv("datafile.dat");
```ds = csv.create_dataset();
```csv.read();
```double mean = ds.column(28).sum() / ds.rows();
However, if some fields have no value, then they shouldn't be included in the calculation for the mean average. A better implementation is:
```double mean = ds.column(28).sum() / ds.column(28).count();
This quickly becomes tiresome to write and is error prone, so a function is provided to perform the calculation and provide a consistent result.

Other functions are provided, which ignore cells with no value.
* `count()` returns the number of non-empty cells in a column
* `mean()` calculates the mean average of non-empty cells in a column
* `sum()` calculates the mean average of non-empty cells in a column