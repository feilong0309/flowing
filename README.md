flowing
=======

Stream community detection for large graphs

===Compilation===

Execute the following comands inside the flowing root folder:

```
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make
```

===Execution===

```
$ cd build
$ ./flowing < PATH_TO_GRAPH
```
The communities are output into a file named "communities.dat".

