// Project1
//

#include <stdio.h>
#include <string.h>
#include <mysql.h>

int main(int argc, char * argv[])
{
   MYSQL *myData;
   MYSQL_RES *res;
   MYSQL_FIELD *fd;
   MYSQL_ROW row;
   char szDB[200], szSQL[200];
   int i, j, l, x, k;

   strcpy(szDB,"peoples");
   strcpy(szSQL,"select * from book");

   if ((myData = mysql_init((MYSQL*) 0)) &&
      mysql_real_connect(myData, NULL, NULL, NULL, NULL, MYSQL_PORT,NULL, 0))
   {
      if (mysql_select_db(myData,szDB) < 0)
      {
         printf("Can't select the %s database!\n", szDB);
         mysql_close(myData);
         return 2;
      }
   }
   else
   {
      printf("Can't connect to the mysql server on port %d !\n",MYSQL_PORT);
      mysql_close(myData);
      return 1;
   }

   if (!mysql_query(myData,szSQL))
   {
      res = mysql_store_result(myData);
      i = (int)mysql_num_rows(res); 
      l = 1;

      printf("Query:  %s\nNumber of records found:  %ld\n", szSQL, i);

      // Display Data

      while (row=mysql_fetch_row(res))
      {
         printf("%s\n",row[1]);
//         j=mysql_num_fields(res);
//         printf("Record #%ld:-\n", l++);
//         for (k = 0 ; k < j ; k++)
//            printf("%s\n",(((row[k]==NULL)||(!strlen(row[k])))?"NULL":row[k]));
      }
      mysql_free_result(res);
   }
   else
      printf( "Couldn't execute %s on the server !\n", szSQL ) ;

   mysql_close(myData);
   return 0;
}

