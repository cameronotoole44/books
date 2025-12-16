#ifndef MODELS_H
#define MODELS_H

#define STATUS_LEN 16
#define TITLE_LEN 256
#define NOTE_LEN 512

#define STATUS_UNREAD "unread"
#define STATUS_READING "reading"
#define STATUS_FINISHED "finished"

typedef struct {
    int id;
    char title[TITLE_LEN];
    int total_pages;
    int current_page;
    char status[STATUS_LEN];
} Book;

typedef struct {
    char date[11];
    int book_id;
    int pages_read;
    char note[NOTE_LEN];
} LogEntry;

typedef struct {
    int book_id;
    int rating;
    char text[NOTE_LEN];
} Note;

#endif