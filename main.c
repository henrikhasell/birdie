#include <stdlib.h>
#include <sqlite3.h>

int main(int argc, char *argv[])
{
    sqlite3 *database;
    int result = sqlite3_open("database.sqlite", &database);

    if(result == SQLITE_OK)
    {
        
    }
    else
    {

    }

    return EXIT_SUCCESS;
}
