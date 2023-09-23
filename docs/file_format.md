# Zero File Format

## Overview
Tensors, Operators, and Bytes (for metadata) are stored in a separate file. The file is a binary file with the following structure:

```
[Header1][Header2]...[HeaderN]
[Data1][Data2]...[DataN]
```

Each header follows the following format:
```C
struct zero_disk_object {
    uint32_t hash;
    uint64_t offset;
    uint32_t size;
};
```

Each data is an unstructured binary blob of size `size` bytes, located at `offset` bytes from the beginning of the file. The `offset` includes the size of the all headers, too.

Since the format is agnostic to which data it is storing, we store tensor and operator in seperate files. The metadata is stored in a separate file, too.
