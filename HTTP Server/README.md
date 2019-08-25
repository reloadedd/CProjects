# A simple HTTP Server written in C for Linux
* It handles GET and HEAD requests and responds with a 200 / 404 HTTP code.  
* The main reason I written it is because I was frustrated because I couldn't understand the Tiny Webserver written in the book "Hacking: The Art of Exploitation, 2nd ed."  
* It took me like a week or so to make my research, but I finally understood what that code did by doing my own. I've learned some valuable things in the process.
* Don't mind about the webpages if they don't make sense, because they don't. I needed to write something that worked, so I've written what was in my head at that time.

# Documentation used
* [Socket Programming in C - Part 1](https://www.youtube.com/watch?v=LtXEMwSG5-8&t=6s)
* [Socket Programming in C - Part 2](https://www.youtube.com/watch?v=mStnzIEprH8)
* [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
* Hacking: The Art of Exploitation, 2nd ed. (lots of re-readings)
* [Introduction to Sockets Programming in C using TCP/IP](https://www.csd.uoc.gr/~hy556/material/tutorials/cs556-3rd-tutorial.pdf) (amazing presentation)
* A lot of duckduckgo'ing

# How to compile it?
* It's simple, just: ```gcc -o HTTPServer HTTPServer.c```
* I've also uploaded the precompiled binary.

# How to use it?
* Give it execute permission: ```sudo chmod u+x HTTPServer```
* Execute it: ```sudo ./HTTPServer```
* That's all, folks!

# Notes
* You need root access to use it, because it binds port 80 (one of those "special" ports). Don't like this ? Change the macro (```#define PORT 80```) to something above 1024.
* After you've executed the script with root privileges, you still get ```[Error] when binding the socket. Have you tried running it with sudo?```, then it's very likely that you already have a service using that port (maybe another webserver, like Apache - who knows?).
