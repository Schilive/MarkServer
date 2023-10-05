# What is it?
MarkServer is an HTTP/1.0 server made for me to learn HTTP using 
[RFC 1945](https://datatracker.ietf.org/doc/html/rfc1945), and for fun. The 
name **MarkServer** is an honour/curse to 
[Markiplier](https://www.youtube.com/Markiplier).

# Dependencies

It uses rudimental CMake to build, on the folder `src/`. You can use the 
following.

```
marserver> cmake -S src/ -B build/
```

As a C program, it uses the ISO C library. Since MarkServer needs networking,
the POSIX API is used for POSIX systems and Win32 for Windows systems. The 
minimum Windows version supported should be Windows 2000, which was the first
Windows version using the NT kernel that targeted the general user.

# Licence

I do not care about, really, any use of this source code. I used the MIT 
licence to say this legally.
