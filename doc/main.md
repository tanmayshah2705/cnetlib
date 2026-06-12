@mainpage
@author Tanmay Shah

<b>cnetlib</b> - is a Network Layer Library to be used on application layer.
This library will help you write network application without worrying about socket programming.

The library has been divided in two parts, the client side part and the server side part.

The client side part contains \ref csnl.h and libcsnl.a whereas the server side part has  \ref ssnl.h and libssnl.a
<hr/>
<h1>How to compile</h1>
<h2>Client side application</h2>
Place the header file (csnl.h) and library file (libcsnl.a) in folders of your choice.
Include the csnl.h in your source code that uses client side network layer library functions.

To compile
<pre>
gcc   your_source_code.c   -I   path_to_header_file_folder   -L   path_to_library_file_folder   -l   csnl   -o   executable_file_name
</pre>
<hr/>
<h2>Server side application</h2>
Place the header file (ssnl.h) and library file (libssnl.a) in folders of your choice.
Include the csnl.h in your source code that uses client side network layer library functions.

To compile
<pre>
gcc   your_source_code.c   -I   path_to_header_file_folder   -L   path_to_library_file_folder   -l   ssnl   -o   executable_file_name
</pre>

