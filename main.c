#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

void create(sqlite3 *database)
{
    char *error;

    const char *query = "CREATE TABLE connections("
    "i INTEGER PRIMARY KEY NOT NULL,"
    "address INTEGER NOT NULL);";

    if(sqlite3_exec(database, query, NULL, NULL, &error) != SQLITE_OK)
    {
        fprintf(stderr, "Failed to create database: %s\n", error);
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
        sqlite3_close(database);
    }
    else
    {
        const char *error = sqlite3_errmsg(database);
	fprintf(stderr, "SQLite encountered an error: %s\n", error);
    }

    return EXIT_SUCCESS;
}
