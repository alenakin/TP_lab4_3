#include "sqlite3.h"
#include <stdio.h>
#include "stdbool.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"

enum param{none = -1, id, lastname = 3, country = 5};

void showMenu();
bool checkNum(char*);
void showSubmenu(char*, char*, sqlite3* db);
void insert(sqlite3*);
void select(char*, enum param, sqlite3*);
static int callback(void *data, int argc, char **argv, char **azColName);

char* fields[] = {"ID", "First name", "Patronymic", "Last name", "Date of birth", "Country", "Place of birth",
"Address", "Department", "Post", "Date of hiring"};
char* ps[] = {"WHERE ID = ?", "", "", "WHERE LastName = ?", "", "WHERE Address LIKE ?"};

int main() {
    sqlite3* db;

    char* sqlSelect = "SELECT * FROM Employees ";
    char* sqlDelete = "DELETE FROM Employees ";

    int rc = sqlite3_open("data.db", &db);
    if(rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return(0);
    }

    int c = 0;
    for (;;)
    {
        showMenu();
        printf("Choice: ");
        scanf("%d", &c);
        switch (c)
        {
            case 1:
                showSubmenu(sqlSelect, "Select", db);
                break;
            case 2:
                insert(db);
                break;
            case 3:
                showSubmenu(sqlDelete, "Delete", db);
                break;
            case 4:
                showMenu();
                break;
            case 5:
                return 0;
            default:
                puts("Incorrect mark. Please, try again\n");
        }
    }
}

void showMenu()
{
    printf("\t\t\tMenu\n");
    printf("1 - Show data from database\n");
    printf("2 - Insert data in database\n");
    printf("3 - Delete data from database\n");
    printf("4 - Show menu\n");
    printf("5 - Exit\n");
}

void showChoices()
{
    printf("1 - All\n");
    printf("2 - By ID\n");
    printf("3 - By last name\n");
    printf("4 - By country\n");
    printf("5 - Return to menu\n\n");
    printf("Choose one item: ");
}

void showSubmenu(char* com, char* name, sqlite3* db)
{
    int c = 0;
    for(;;) {
        printf("\n%s: \n", name);
        showChoices();
        scanf("%d", &c);

        switch (c)
        {
            case 1:
                select(com, none, db);
                return;
            case 2:
                select(com, id, db);
                return;
            case 3:
                select(com, lastname, db);
                return;
            case 4:
                select(com, country, db);
                return;
            case 5:
                return;
            default:
                puts("Incorrect mark. Please, try again\n");
        }
    }
}

void select(char* sql, enum param p, sqlite3* db)
{
    int rc = 0;
    char* err_msg = 0;
    if (p != none)
    {
        char param[256];
        char new_sql[256];
        sqlite3_stmt *res;
        strcpy(new_sql, sql);
        strcat(new_sql, ps[p]);

        rc = sqlite3_prepare_v2(db, new_sql, -1, &res, 0);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
            return;
        }

        printf("Enter %s: ", fields[p]);
        scanf("%s", param);
        if (p == id)
        {
            int id = 0;
            if (!checkNum(param))
            {
                printf("Parameter should be a number\n");
                return;
            }
            else
                id = atoi(param);
            sqlite3_bind_int(res, 1, id);
        }
        else if (p == country)
            {
                strcat(param, "%");
                sqlite3_bind_text(res, 1, param, (int) strlen(param), SQLITE_STATIC);
            }
        else
            sqlite3_bind_text(res, 1, param, (int) strlen(param), SQLITE_STATIC);
        if (sql[0] != 'D') {
            while (sqlite3_step(res) == SQLITE_ROW) {
                for (int i = 0; i <= 10; i++) {
                    if (i == 5)
                        continue;
                    printf("%s = %s\n", fields[i], sqlite3_column_text(res, i));
                }
                printf("\n");

                char str[256];
                strcpy(str, sqlite3_column_text(res, 1));
                strcat(str, sqlite3_column_text(res, 3));
                strcat(str, ".jpg");
                FILE *fp = fopen(str, "wb");
                fwrite(sqlite3_column_blob(res, 5),
                       sqlite3_column_bytes(res, 5), 1, fp);
                fclose(fp);
            }
        }
        else
            while(sqlite3_step(res) == SQLITE_ROW);
        sqlite3_finalize(res);
    }
    else
    {
        rc = sqlite3_exec(db, sql, callback, 0, err_msg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", err_msg);
            sqlite3_free(err_msg);
        }
    }
}

bool checkNum(char* a)
{
    for(int i = 0; i < strlen(a); i++)
        if (!atoi(&a[i]))
            return false;
    return true;
}

static int callback(void *data, int argc, char **argv, char **azColName){
    int i;
    for(i=0; i<argc; i++){
        if (i == 5)
            continue;
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

void insert(sqlite3 *db){
    sqlite3_stmt *res;
    char *sql = "INSERT INTO Employees VALUES(?,?,?,?,?,?,?,?,?,?,?);";
    char fields[11][120];
    struct tm hiringdate, birthdate;
    char temp1[80], temp2[80], temp3[80];

    printf("Full name (FirstName Patron Last): ");
    scanf("%s %s %s", fields[1], fields[2], fields[3]);
    printf("Date of birth (dd mm yyyy): ");
    scanf("%d %d %d", &birthdate.tm_mday, &birthdate.tm_mon, &birthdate.tm_year);
    printf("Name of image with photo: ");
    scanf("%s", fields[5]);
    printf("Place of birth(city, country): ");
    scanf("%s %s", fields[6], temp1);
    strcat(fields[6], ", ");
    strcat(fields[6], temp1);
    printf("Living address (country, cite, street, home): ");
    scanf("%s %s %s %s", fields[7], temp1, temp2, temp3);
    strcat(fields[7], ", ");
    strcat(fields[7], temp1);
    strcat(fields[7], ", ");
    strcat(fields[7], temp2);
    strcat(fields[7], " ");
    strcat(fields[7], temp3);
    printf("Department: ");
    scanf("%s", fields[8]);
    printf("Position: ");
    scanf("%s", fields[9]);
    printf("Date of hiring (dd mm yyyy): ");
    scanf("%d %d %d", &hiringdate.tm_mday, &hiringdate.tm_mon, &hiringdate.tm_year);

    birthdate.tm_mon--;
    hiringdate.tm_mon--;
    birthdate.tm_year -= 1900;
    hiringdate.tm_year -= 1900;

    strftime(fields[10], 11, "%Y-%m-%d", &hiringdate);
    strftime(fields[4], 11, "%Y-%m-%d", &birthdate);

    int rc = sqlite3_prepare(db, sql, -1, &res, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    FILE *fp = fopen(fields[5], "rb");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open image file\n");
        return;
    }

    fseek(fp, 0, SEEK_END);
    int flen = ftell(fp);
    if (flen == -1) {

        perror("error occurred");
        fclose(fp);
        return;
    }
    fseek(fp, 0, SEEK_SET);
    char data[flen+1];
    int size = fread(data, 1, flen, fp);
    fclose(fp);
    for(int i = 1; i < 11; i++)
    {
        if (i == 5)
            sqlite3_bind_blob(res, i + 1, data, size, SQLITE_STATIC);
        else
            sqlite3_bind_text(res, i + 1, fields[i], (int) strlen(fields[i]), SQLITE_STATIC);
    }

    int step = sqlite3_step(res);

    if (step == SQLITE_ROW) {
        printf("%s: ", sqlite3_column_text(res, 0));
        printf("%s\n", sqlite3_column_text(res, 1));
    }

    sqlite3_finalize(res);
}



