#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

char *allocateString(const char format[], ...)
{
    va_list arguments1;
    va_list arguments2;

    va_start(arguments1, format);
    va_start(arguments2, format);

    int size = vsnprintf(NULL, 0, format, arguments1) + 1;

    char *output = malloc(size);

    if(output)
    {
	*output = '\0';
        vsprintf(output, format, arguments2);
    }

    va_end(arguments1);
    va_end(arguments2);

    return output;
}

char *ipv4ToString(const struct sockaddr_in *address)
{
   return allocateString("%u.%u.%u.%u\n",
        ((char*)&address->sin_addr)[0],
        ((char*)&address->sin_addr)[1],
        ((char*)&address->sin_addr)[2],
        ((char*)&address->sin_addr)[3]
    );
}

char *ipv6ToString(const struct sockaddr_in6 *address)
{
    return allocateString(
        "%02x%02x:%02x%02x:%02x%02x:%02x%02x:"
        "%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
        address->sin6_addr.s6_addr[0],
        address->sin6_addr.s6_addr[1],
        address->sin6_addr.s6_addr[2],
        address->sin6_addr.s6_addr[3],
        address->sin6_addr.s6_addr[4],
        address->sin6_addr.s6_addr[5],
        address->sin6_addr.s6_addr[6],
        address->sin6_addr.s6_addr[7],
        address->sin6_addr.s6_addr[8],
        address->sin6_addr.s6_addr[9],
        address->sin6_addr.s6_addr[10],
        address->sin6_addr.s6_addr[11],
        address->sin6_addr.s6_addr[12],
        address->sin6_addr.s6_addr[13],
        address->sin6_addr.s6_addr[14],
        address->sin6_addr.s6_addr[15]
    );
}

char *ipToString(const struct sockaddr_storage *address)
{
    if(address->ss_family == AF_INET)
    {
        return ipv4ToString((struct sockaddr_in*)address);
    }

    if(address->ss_family == AF_INET6)
    {
        return ipv6ToString((struct sockaddr_in6*)address);
    }

    fputs("Invalid address type.", stderr);

    return NULL;
}

void createTable(sqlite3 *database)
{
    const char *query = "CREATE TABLE IF NOT EXISTS connections("
    "i INTEGER PRIMARY KEY NOT NULL,"
    "address INTEGER NOT NULL);";

    char *error;

    if(sqlite3_exec(database, query, NULL, NULL, &error) != SQLITE_OK)
    {
        fprintf(stderr, "Failed to create database: %s.\n", error);
        sqlite3_free(error);
    }


}

int main(int argc, char *argv[])
{
    sqlite3 *database;
    int result = sqlite3_open("database.sqlite", &database);

    if(result == SQLITE_OK)
    {
	createTable(database);

        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        struct addrinfo *address;

        if(getaddrinfo(NULL, "12345", &hints, &address) == -1)
        {

        }
        else
        {
            int sock = socket(
                address->ai_family,
                address->ai_socktype,
                address->ai_protocol
            );

            if(sock == -1)
            {
                fputs("Failed to create socket.\n", stderr);
            }
            else
            {
                if(bind(sock, address->ai_addr, address->ai_addrlen) == -1)
                {
                     fputs("Failed to bind socket to port.\n", stderr);
                }
                else if(listen(sock, 16) == -1)
                {
                    fputs("Failed to listen to socket.\n", stderr);
                }
                else
                {
                    struct sockaddr_storage remote_address;
                    socklen_t remote_address_size = sizeof(remote_address);

                    int remote = accept(
                        sock,
                        (struct sockaddr*)&remote_address,
                        &remote_address_size
                    );

                    if(remote == -1)
                    {
                        fputs("Failed to accept remote connection.\n", stderr);
                    }
                    else
                    {
                        char *address_string = ipToString(&remote_address);

                        printf("Received connection from %s\n", address_string);

                        if(address_string)
                        {
                            free(address_string);
                        }

                        close(remote);
                    }
                }

                close(sock);
            }

            freeaddrinfo(address);
        }

        sqlite3_close(database);
    }
    else
    {
        const char *error = sqlite3_errmsg(database);
        fprintf(stderr, "SQLite encountered an error: %s\n", error);
    }

    return EXIT_SUCCESS;
}
