#ifndef DATA_H
#define DATA_H

typedef struct 
{
    /* Data struct containing data to be passed around
	 * between functions - guess this is not state of
	 * the art but who cares. */
    GtkTextBuffer *out;
    GtkTextBuffer *err;
	const gchar *inputfilename;
	gchar* inputfile_directory;
	GtkEntry *output_entry;
	gchar *outputfilename;
	gchar *title;
	gchar *artist;
	gchar *album;
	gchar *genre;
	gchar *year;
	const gchar *quality;
	GtkWidget *parent;
	GtkLabel *label_pwd;
	GtkLabel *label_pwd_meta;
 
    /* Timeout source id. */
    gint timeout_id;
} Data;

#endif
