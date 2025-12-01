
<style>
mark{
    background:grey;
}
</style>
1. Byte order, network is big endian
    *Big-Endian* is also called *Network Byte Order*
    ```c
    Function	Description
    htons()	host to network short
    htonl()	host to network long
    ntohs()	network to host short
    ntohl()	network to host long
    ```
1. __*struct addrinfo*__ -> used to prep the socket address structures for subsequent use.

    used to host name lookups and service name lookup
    ```c
    struct addrinfo {
        int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
        int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
        int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
        int              ai_protocol;  // use 0 for "any"
        size_t           ai_addrlen;   // size of ai_addr in bytes
        struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
        char            *ai_canonname; // full canonical hostname

        struct addrinfo *ai_next;      // linked list, next node
    };
    ```

    ```c
    struct sockaddr {
        unsigned short    sa_family;    // address family, AF_xxx
        char              sa_data[14];  // 14 bytes of protocol address
    };
    ```
    To deal with <mark>*struct sockaddr*</mark>, programmers created a parallel structure: <mark>*struct sockaddr_in*</mark> (“in” for “Internet”) to be used with IPv4.

    A pointer to a struct <mark>*sockaddr_in*</mark> can be cast to a pointer to a <mark>*struct sockaddr*</mark> and vice-versa.

    ```c
    // (IPv4 only--see struct sockaddr_in6 for IPv6)

    struct sockaddr_in {
        short int          sin_family;  // Address family, AF_INET
        unsigned short int sin_port;    // Port number
        struct in_addr     sin_addr;    // Internet address
        unsigned char      sin_zero[8]; // Same size as struct sockaddr
    };
    
    // Internet address (a structure for historical reasons)
    struct in_addr {
        uint32_t s_addr; // that's a 32-bit int (4 bytes)
    };
    ```

    ```c
    // (IPv6 only--see struct sockaddr_in and struct in_addr for IPv4)

    struct sockaddr_in6 {
        u_int16_t       sin6_family;   // address family, AF_INET6
        u_int16_t       sin6_port;     // port number, Network Byte Order
        u_int32_t       sin6_flowinfo; // IPv6 flow information
        struct in6_addr sin6_addr;     // IPv6 address
        u_int32_t       sin6_scope_id; // Scope ID
    };

    struct in6_addr {
        unsigned char   s6_addr[16];   // IPv6 address
    };
    ```
    <mark>struct sockaddr_storage</mark> that is designed to be large enough to hold both IPv4 and IPv6 structures. See, for some calls, sometimes you don’t know in advance if it’s going to fill out your struct sockaddr with an IPv4 or IPv6 address. So you pass in this parallel structure, very similar to struct sockaddr except larger, and then cast it to the type you need:

    ```c
    struct sockaddr_storage {
        sa_family_t  ss_family;     // address family

        // all this is padding, implementation specific, ignore it:
        char      __ss_pad1[_SS_PAD1SIZE];
        int64_t   __ss_align;
        char      __ss_pad2[_SS_PAD2SIZE];
    };
    ```
1. string to ip4 & ip6 and vise versa

    ```c
    struct sockaddr_in sa; // IPv4
    struct sockaddr_in6 sa6; // IPv6

    inet_pton(AF_INET, "10.12.110.57", &(sa.sin_addr)); // IPv4
    inet_pton(AF_INET6, "2001:db8:63b3:1::3490", &(sa6.sin6_addr)); // IPv6
    ```
    <mark>__“pton” stands for “presentation to network"__</mark>

    ```c
    // IPv4:
    char ip4[INET_ADDRSTRLEN];  // space to hold the IPv4 string
    struct sockaddr_in sa;      // pretend this is loaded with something

    inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);
    printf("The IPv4 address is: %s\n", ip4);


    // IPv6:
    char ip6[INET6_ADDRSTRLEN]; // space to hold the IPv6 string
    struct sockaddr_in6 sa6;    // pretend this is loaded with something

    inet_ntop(AF_INET6, &(sa6.sin6_addr), ip6, INET6_ADDRSTRLEN);
    printf("The address is: %s\n", ip6);
    ```
    <mark>__“ntop” stands for “network to presentation"__</mark>

1. System calls

    1. <mark>__getaddrinfo(...)__</mark> : DNS and service name lookups
        ```c
        int getaddrinfo(const char *node,     // e.g. "www.example.com" or IP
                const char *service,  // e.g. "http" or port number
                const struct addrinfo *hints,
                struct addrinfo **res);
        ```
        Here’s a sample call if you’re a server who wants to listen on your host’s IP address, port 3490.
        ```c
        int status;
        struct addrinfo hints;
        struct addrinfo *servinfo;  // will point to the results

        memset(&hints, 0, sizeof hints); // make sure the struct is empty
        hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
        hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

        if ((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
            exit(1);
        }

        // servinfo now points to a linked list of 1 or more struct addrinfos

        // ... do everything until you don't need servinfo anymore ....

        freeaddrinfo(servinfo); 
        ```
        Here’s a sample call if you’re a client who wants to connect to a particular server, say “www.example.net” port 3490.
        ```c
        int status;
        struct addrinfo hints;
        struct addrinfo *servinfo;  // will point to the results

        memset(&hints, 0, sizeof hints); // make sure the struct is empty
        hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

        // get ready to connect
        status = getaddrinfo("www.example.net", "3490", &hints, &servinfo);

        // servinfo now points to a linked list of 1 or more struct addrinfos

        // etc.
        ```
        ```c
        /*
        ** showip.c -- show IP addresses for a host given on the command line
        */
        ...

        int main(int argc, char *argv[])
        {
            struct addrinfo hints, *res, *p;
            int status;
            char ipstr[INET6_ADDRSTRLEN];

            if (argc != 2) {
                fprintf(stderr,"usage: showip hostname\n");
                return 1;
            }

            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
            hints.ai_socktype = SOCK_STREAM;

            if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
                return 2;
            }

            printf("IP addresses for %s:\n\n", argv[1]);

            for(p = res;p != NULL; p = p->ai_next) {
                void *addr;
                char *ipver;

                // get the pointer to the address itself,
                // different fields in IPv4 and IPv6:
                if (p->ai_family == AF_INET) { // IPv4
                    struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
                    addr = &(ipv4->sin_addr);
                    ipver = "IPv4";
                } else { // IPv6
                    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
                    addr = &(ipv6->sin6_addr);
                    ipver = "IPv6";
                }

                // convert the IP to a string and print it:
                inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
                printf("  %s: %s\n", ipver, ipstr);
            }

            freeaddrinfo(res); // free the linked list

            return 0;
        }
        ```
        ```bash
        $ showip www.example.net
        IP addresses for www.example.net:

        IPv4: 192.0.2.88

        $ showip ipv6.example.com
        IP addresses for ipv6.example.com:

        IPv4: 192.0.2.101
        IPv6: 2001:db8:8c00:22::171
        ```
    1. socket()
        ```c
        #include <sys/types.h>
        #include <sys/socket.h>

        int socket(int domain, int type, int protocol); 
        ```
        *domain* is PF_INET or PF_INET6, *type* is SOCK_STREAM or SOCK_DGRAM, and *protocol* can be set to 0 to choose the proper protocol for the given type. Or you can call getprotobyname() to look up the protocol you want, “tcp” or “udp”.

        > The most correct thing to do is to use __AF_INET__ in your *struct sockaddr_in* and __PF_INET__ in your call to *socket()*.
        
        ```c
        int s;
        struct addrinfo hints, *res;

        // do the lookup
        // [pretend we already filled out the "hints" struct]
        getaddrinfo("www.example.com", "http", &hints, &res);

        // again, you should do error-checking on getaddrinfo(), and walk
        // the "res" linked list looking for valid entries instead of just
        // assuming the first one is good (like many of these examples do).
        // See the section on client/server for real examples.

        s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        ```
    1. bind()
        ```c
        #include <sys/types.h>
        #include <sys/socket.h>

        int bind(int sockfd, struct sockaddr *my_addr, int addrlen);

        sockfd is the socket file descriptor returned by socket(). my_addr is a pointer to a struct sockaddr that contains information about your address, namely, port and IP address. addrlen is the length in bytes of that address.
        ```
        ```c
        struct addrinfo hints, *res;
        int sockfd;

        // first, load up address structs with getaddrinfo():

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

        getaddrinfo(NULL, "3490", &hints, &res);

        // make a socket:

        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

        // bind it to the port we passed in to getaddrinfo():

        bind(sockfd, res->ai_addr, res->ai_addrlen);
        ```

        ```c
        // !!! THIS IS THE OLD WAY !!!

        int sockfd;
        struct sockaddr_in my_addr;

        sockfd = socket(PF_INET, SOCK_STREAM, 0);

        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(MYPORT);     // short, network byte order
        my_addr.sin_addr.s_addr = inet_addr("10.12.110.57");
        memset(my_addr.sin_zero, '\0', sizeof my_addr.sin_zero);

        bind(sockfd, (struct sockaddr *)&my_addr, sizeof my_addr);
        ```
        Sometimes, you might notice, you try to rerun a server and bind() fails, claiming “Address already in use.” What does that mean? Well, a little bit of a socket that was connected is still hanging around in the kernel, and it’s hogging the port. You can either wait for it to clear (a minute or so), or add code to your program allowing it to reuse the port, like this:
        ```c
        int yes=1;
        //char yes='1'; // Solaris people use this

        // lose the pesky "Address already in use" error message
        if (setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
            perror("setsockopt");
            exit(1);
        }
        ```
    1. connect()
        ```c
        #include <sys/types.h>
        #include <sys/socket.h>

        int connect(int sockfd, struct sockaddr *serv_addr, int addrlen); 
        ```
        eg Let’s have an example where we make a socket connection to “www.example.com”, port 3490:
        ```c
        struct addrinfo hints, *res;
        int sockfd;

        // first, load up address structs with getaddrinfo():

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        getaddrinfo("www.example.com", "3490", &hints, &res);

        // make a socket:
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

        // connect!
        connect(sockfd, res->ai_addr, res->ai_addrlen);
        ```
        > The kernel will choose a local port for us, and the site we connect to will automatically get this information from us.
    1. listen()
        ```c
        int listen(int sockfd, int backlog);

        sockfd is the usual socket file descriptor from the socket() system call. backlog is the number of connections allowed on the incoming queue.
        ```
    1. accept()
        ```c
        #include <sys/types.h>
        #include <sys/socket.h>

        int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

        eg .

        struct sockaddr_storage their_addr;
        socklen_t addr_size;

        addr_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        ```
    1. send and recv -> for stream socket
        ```c
        int send(int sockfd, const void *msg, int len, int flags);

        sockfd is the socket descriptor you want to send data to (whether it’s the one returned by socket() or the one you got with accept()). msg is a pointer to the data you want to send, and len is the length of that data in bytes. Just set flags to 0
        ```
        eg 
        ```c
        char *msg = "Beej was here!";
        int len, bytes_sent;

        len = strlen(msg);
        bytes_sent = send(sockfd, msg, len, 0);
        ```
        > sometimes you tell it to send a whole gob of data and it just can’t handle it. It’ll fire off as much of the data as it can, and trust you to send the rest later. Remember, if the value returned by send() doesn’t match the value in len, it’s up to you to send the rest of the string. The good news is this: if the packet is small (less than 1K or so) it will probably manage to send the whole thing all in one go.

        ```c
        int recv(int sockfd, void *buf, int len, int flags);

        sockfd is the socket descriptor to read from, buf is the buffer to read the information into, len is the maximum length of the buffer, and flags can again be set to 0.
        ```
        > recv() can return 0. This can mean only one thing: the remote side has closed the connection on you! A return value of 0 is recv()’s way of letting you know this has occurred.
    1. sendto() and recvfrom()—Talk to me, DGRAM-style

        Since datagram sockets aren’t connected to a remote host, guess which piece of information we need to give before we send a packet? That’s right! The destination address! Here’s the scoop:
        ```c
        int sendto(int sockfd, const void *msg, int len, unsigned int flags,
        const struct sockaddr *to, socklen_t tolen);

        to is a pointer to a struct sockaddr (which will probably be another struct sockaddr_in or struct sockaddr_in6 or struct sockaddr_storage that you cast at the last minute) which contains the destination IP address and port. tolen, an int deep-down, can simply be set to sizeof *to or sizeof(struct sockaddr_storage).
        ```

        ```c
        int recvfrom(int sockfd, void *buf, int len, unsigned int flags,
             struct sockaddr *from, int *fromlen);
        
        from is a pointer to a local struct sockaddr_storage that will be filled with the IP address and port of the originating machine. fromlen is a pointer to a local int that should be initialized to sizeof *from or sizeof(struct sockaddr_storage). When the function returns, fromlen will contain the length of the address actually stored in from.
        ```
        > Remember, if you connect() a datagram socket, you can then simply use send() and recv() for all your transactions. The socket itself is still a datagram socket and the packets still use UDP, but the socket interface will automatically add the destination and source information for you.
    1. close and shutdown

        ```c
        close(sockfd); 
        
        This will prevent any more reads and writes to the socket. Anyone attempting to read or write the socket on the remote end will receive an error.
        ```
        shutdown allows you to cut off communication in a certain direction, or both ways (just like close() does). Synopsis:
        ```c
        int shutdown(int sockfd, int how); 
        ```

        sockfd is the socket file descriptor you want to shutdown, and how is one of the following:
        ```bash
        how	Effect
        0	Further receives are disallowed
        1	Further sends are disallowed
        2	Further sends and receives are disallowed (like close())
        ```

        > If you deign to use shutdown() on unconnected datagram sockets, it will simply make the socket unavailable for further send() and recv() calls (remember that you can use these if you connect() your datagram socket).

        > It’s important to note that shutdown() doesn’t actually close the file descriptor—it just changes its usability. To free a socket descriptor, you need to use close().
    1. getpeername() -> will tell you who is at the other end of a connected stream socket.
        ```c
        #include <sys/socket.h>

        int getpeername(int sockfd, struct sockaddr *addr, int *addrlen); 
        
        sockfd is the descriptor of the connected stream socket, addr is a pointer to a struct sockaddr (or a struct sockaddr_in) that will hold the information about the other side of the connection, and addrlen is a pointer to an int, that should be initialized to sizeof *addr or sizeof(struct sockaddr).
        ```

        > you can use inet_ntop(), getnameinfo(), or gethostbyaddr() to print or get more information. No, you can’t get their login name. (Ok, ok. If the other computer is running an ident daemon, this is possible
    1. gerhostname() -> It returns the name of the computer that your program is running on. The name can  then be used by getaddrinfo(), above, to determine the IP address of your local machine.

        ```c
        #include <unistd.h>

        int gethostname(char *hostname, size_t size); 
        
        The arguments are simple: hostname is a pointer to an array of chars that will contain the hostname upon the function’s return, and size is the length in bytes of the hostname array.
        ```