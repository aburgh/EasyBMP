# EasyBMP Cross-Platform Windows Bitmap Library

EasyBMP is a simple, cross-platform, open source (revised BSD) C++ library designed for easily reading, writing, and modifying Windows bitmap (BMP) image files. You can learn more about the history and goals of the project here. The library is oriented towards the novice programmer with little formal experience, but it is sufficiently capable for anybody who desires to do I/O and pixel operations on uncompressed 1, 4, 8, 16, 24, and 32 bpp (bits per pixel) BMP files.

EasyBMP is intended to be cross-platform on both little-endian (e.g., x86, x86-64) and big-endian (e.g., IBM PowerPC, Sun Sparc) architectures. So far, it has been tested on x86 with Linux (2.4.x, 2.6.x kernels and g++ and icc) and Windows (XP Pro, XP Home, and 2K Pro with MinGW/g++ and Borland), as well as on a Sun Sparc 4 machine (Solaris 5.9 with g++). EasyBMP should be solid on both little-endian and big-endian architectures.

## Fork Changes

This fork has the following changes:

* C++11 is required.

* It throws exceptions instead of printing warnings and errors to standard out.

* It can perform I/O on memory buffers in addition to files.
