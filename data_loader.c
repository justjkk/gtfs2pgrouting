/*
 * data_loader.c
 *
 *  Created on: 18-Aug-2011
 *      Author: jkk
 */

#include "data_loader.h"
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 1024

const char *GTFS_FILENAME[] = { "agency.txt",
                                "stops.txt",
                                "routes.txt",
                                "calendar.txt",
                                "shapes.txt",
                                "trips.txt",
                                "stop_times.txt",
                                "frequencies.txt" };

const char *GTFS_TABLENAME[] = { "agency",
                                 "stops",
                                 "routes",
                                 "calendar",
                                 "shapes",
                                 "trips",
                                 "stop_times",
                                 "frequencies" };

int import_from_folder(PGconn *conn, char *schema, char *folder_path) {
  GTFS_FILE i;
  int ci;
  FILE *fp;
  char sql[1024];
  char buf[BUF_SIZE];
  PGresult *result;
  char *err = NULL;

  if (chdir(folder_path)) {
    fprintf(stderr, "Unknown folder : %s\n", folder_path);
    return -1;
  }

  for (i = 0; i < G_COUNT; ++i) {
    fp = fopen(GTFS_FILENAME[i], "r");
    if (fp == NULL)
    {
      if (i == G_SHAPES || i == G_FREQUENCIES) {
        continue;
      } else {
        fprintf(stderr, "Required file '%s' not found\n", GTFS_FILENAME[i]);
        return -1;
      }
    } else {
      ci = 0;
      while((buf[ci++] = getc(fp)) != '\n');
      buf[ci - 1] = '\0';
      sprintf(sql, "COPY %s(%s) FROM STDIN WITH CSV", GTFS_TABLENAME[i], buf);
      result = PQexec(conn, sql);
      if(PQresultStatus(result) != PGRES_COPY_IN)
      {
        fprintf(stderr, "Error trying to copy into '%s'.\n", GTFS_TABLENAME[i]);
        return -1;
      }
      fgets(buf, BUF_SIZE, fp);
      while(!feof(fp))
      {
        PQputCopyData(conn, buf, strlen(buf));
        fgets(buf, BUF_SIZE, fp);
      }
      PQputCopyEnd(conn, err);
    }
    fclose(fp);
  }
  return 0;
}
