#ifndef VERSION_H_
#define VERSION_H_

#include <stdio.h>

/// !! Generated by 'genver.sh' DON'T EDIT !! ///

// The name of this program.
const char * program_name = "Glaze-Arc";
// The version of this program.
const char * version = "0.2.0";
// The last commit date time.
const char * last_commit_date = "Mon Jul 30 20:09:01 2012 +0900";

// Print an ID stamp for the program.
int _program_id_stamp(FILE* output)
{
    fprintf (output, "%s version %s (%s) compiled at %s %s\n",
             program_name, version, last_commit_date, __DATE__, __TIME__ );

    return 0;
}

#endif /* VERSION_H_ */
