# help for makefile project

该工程用于试验makefile的使用。

其中，包含了几个例子：

# sam_NestMake

嵌套编译, 在工程根目录, 敲make, 就可以完成编译. 自动完成子目录迭代编译.

这里面有三种Makefile:
1.根目录makefile
2.debug目录makefile
3.其他目录makefile

# sam_OneMakefile

只在根目录下有一个Makefile，自动搜索全部的 .c 和 .h 完成编译，编译的 .obj 都统一存放到 obj 文件夹下。然后完成链接，生成可执行文件。

# sam_SeparateSrcInc

独立存放 源文件 和 头文件，并进行编译。

