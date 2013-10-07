#C++ Data Processing library

This repository will build into a library to ease manipulation and managing data files for data processing.

Stable releases will be merged into the `master` branch.
See the `develop` branch for current status.

## Datasets
A `dataset` is a grid representation of imported CSV data, either from a file or from a memory buffer. Each `column` has a defined type and each`cell` (a cross section of a row and a column) also has a defined, which is either the same as the column type, or `null_type` where the column has no value.

###Column-wise operations
A representation of a column of data can be accessed by calling the `column()` function on the dataset. A `column_data` object is returned, which provides column-wise operations, which ignore cells in the column that do no contain a value. To see why this is important, consider the calculating the mean average value in a column. A naive implementation using the public interface may be written

    mapped_csv csv("datafile.dat");
    csv.read();
    ds = csv.create_dataset();
    double mean = ds.column(28).sum() / ds.rows();

However, if some fields have no value, then they shouldn't be included in the calculation for the mean average. A better implementation is:

    double mean = ds.column(28).sum() / ds.column(28).count();

This quickly becomes tiresome to write and is error prone, so a function is provided to perform the calculation and provide a consistent result.

Other functions are provided, which ignore cells with no value.
* `count()` returns the number of non-empty cells in a column
* `count_null()` returns the number of empty cells in a column
* `mean()` calculates the mean average of non-empty cells in a column
* `sum()` calculates the mean average of non-empty cells in a column

###Extracting and Deleting column data
If a column of data is no longer needed, it can be removed from the dataset by calling `erase`.

    ds.column("image_data").erase();

Data can be extracted from a data column using `extract`.

    auto extracted_data = ds.column(2).extract<std::uint32_t>();

If the column no longer needs to be a part of the dataset, then calling `detach` will `extract` and `erase` the column with a single call.

    auto column = ds.column(2).detach<std::uint32_t>();


#Serialization
A dataset can easily be serialized to a stream as a CSV data file

    mapped_csv csv("datafile.dat");
    csv.read();
    ds = csv.create_dataset();
    std::cout << ds;
