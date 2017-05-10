#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

void create(sqlite3 *database)
{
    const char *query = "CREATE TABLE connections("
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
	    create(database);

        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET6;
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
                        puts("Received remote connection.");

                        if(remote_address.ss_family == AF_INET)
                        {
                            struct sockaddr_in *cast =
                                (struct sockaddr_in*)&remote_address;
                            printf("%d.%d.%d.%d\n",
                                ((char*)&cast->sin_addr)[0],
                                ((char*)&cast->sin_addr)[1],
                                ((char*)&cast->sin_addr)[2],
                                ((char*)&cast->sin_addr)[3]
                            );
                        }
                        else if(remote_address.ss_family == AF_INET6)
                        {
                            struct sockaddr_in6 *cast =
                                (struct sockaddr_in6*)&remote_address;
                            printf(
                                "%02x%02x:%02x%02x:%02x%02x:%02x%02x:"
                                "%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
                                cast->sin6_addr.s6_addr[0],
                                cast->sin6_addr.s6_addr[1],
                                cast->sin6_addr.s6_addr[2],
                                cast->sin6_addr.s6_addr[3],
                                cast->sin6_addr.s6_addr[4],
                                cast->sin6_addr.s6_addr[5],
                                cast->sin6_addr.s6_addr[6],
                                cast->sin6_addr.s6_addr[7],
                                cast->sin6_addr.s6_addr[8],
                                cast->sin6_addr.s6_addr[9],
                                cast->sin6_addr.s6_addr[10],
                                cast->sin6_addr.s6_addr[11],
                                cast->sin6_addr.s6_addr[12],
                                cast->sin6_addr.s6_addr[13],
                                cast->sin6_addr.s6_addr[14],
                                cast->sin6_addr.s6_addr[15]
                            );
                        }
                        else
                        {
                            fputs("Remote has an invalid address.\n", stderr);
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
