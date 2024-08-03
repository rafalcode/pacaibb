// from chatgpt
// // actually didn't know strftime could handle this.
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>

void convert_date0(const char *input_date)
{
    struct tm tm;
    char output_date[100];
    
    // Initialize tm structure to zero
    memset(&tm, 0, sizeof(struct tm));
    
    // Parse the input date string "YYYY:MM:DD"
    sscanf(input_date, "%d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
    
    // Adjust the year and month to fit the tm structure
    tm.tm_year -= 1900; // tm_year is years since 1900
    tm.tm_mon -= 1;     // tm_mon is 0-11

    // Convert to time_t to update the tm_wday field
    time_t t = mktime(&tm);

    // Format the date into "named-day named-month year"
    strftime(output_date, sizeof(output_date), "%A %B %d, %Y", &tm);

    // Print the result
    printf("Converted date: %s\n", output_date);
}

void convert_date(const char *input_date, char *output_date)
{
    struct tm tm;
    
    // Initialize tm structure to zero
    memset(&tm, 0, sizeof(struct tm));
    
    // Parse the input date string "YYYY:MM:DD"
    sscanf(input_date, "%d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
    
    // Adjust the year and month to fit the tm structure
    tm.tm_year -= 1900; // tm_year is years since 1900
    tm.tm_mon -= 1;     // tm_mon is 0-11

    // Convert to time_t to update the tm_wday field
    // time_t t = mktime(&tm); // not sure why, it ain't used.

    // Format the date into "named-day named-month year"
    strftime(output_date, 128, "%a %d %b %Y", &tm);

    // Print the result
    printf("Converted date: %s\n", output_date);
}

int main()
{
    const char *date = "2024:08:03";

    char *odate=calloc(128, sizeof(char));
    convert_date(date, odate);
    return 0;
}
