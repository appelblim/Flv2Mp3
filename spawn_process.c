#include <gtk/gtk.h>
#include "struct.h"

gint button_clicked( GtkButton*, Data* );

static void cb_child_watch( GPid, gint, Data* );

static gboolean cb_out_watch( GIOChannel*, GIOCondition, Data* );

static gboolean cb_err_watch( GIOChannel*, GIOCondition, Data* );


/* Spawn ffmpeg. */
gint button_clicked(GtkButton *button, Data *data)
{
	gchar *outputfilename = g_strconcat( data->inputfilename, ".mp3", NULL );
	gchar *meta_title_conc = g_strconcat( "title=", data->title, NULL );
	gchar *meta_artist_conc = g_strconcat( "artist=", data->artist, NULL );
	gchar *meta_album_conc = g_strconcat( "album=", data->album, NULL );
	gchar *meta_genre_conc = g_strconcat( "genre=", data->genre, NULL );
	gchar *meta_year_conc = g_strconcat( "year=", data->year, NULL );
	gchar *quality_conc = g_strconcat( data->quality, "k", NULL );
	GtkWidget *dialog;


	GPid        pid;
    gchar      *argv[] = { "ffmpeg", "-i", (gchar*)data->inputfilename, "-metadata",
							meta_title_conc, "-metadata",
							meta_artist_conc, "-metadata", meta_album_conc, "-metadata",
							meta_genre_conc, "-metadata", meta_year_conc, "-ab", quality_conc,
							outputfilename, NULL };
    gint        out,
                err;
    GIOChannel *out_ch,
               *err_ch;
    gboolean    ret;

	if( g_utf8_strlen( data->inputfilename, 1 ) == 0 )
	{
		dialog = gtk_message_dialog_new (GTK_WINDOW(data->parent), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Pick a file to transcode" );
  
		gtk_window_set_title (GTK_WINDOW (dialog), "Error");
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		return 1;
	}
	
	GSpawnFlags flags = G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH;
 
    /* Spawn child process */
    ret = g_spawn_async_with_pipes( NULL, argv, NULL, flags, NULL,
                                    NULL, &pid, NULL, &out, &err, NULL );

    if( ! ret )
    {
        g_error( "SPAWNING FFMPEG FAILED" );
        return 1;
    }
 
    /* Add watch function to catch termination of the process. This function
     * will clean any remnants of process. */
    g_child_watch_add( pid, (GChildWatchFunc)cb_child_watch, data );

    /* Create channels that will be used to read data from pipes. */
    out_ch = g_io_channel_unix_new( out );
    err_ch = g_io_channel_unix_new( err );
 
    /* Add watches to channels */
    g_io_add_watch( out_ch, G_IO_IN | G_IO_HUP, (GIOFunc)cb_out_watch, data );
    g_io_add_watch( err_ch, G_IO_IN | G_IO_HUP, (GIOFunc)cb_err_watch, data );
	
	g_free( outputfilename );
	g_free( meta_title_conc );
	g_free( meta_artist_conc );
	g_free( quality_conc );
	g_free( meta_album_conc );
	g_free( meta_genre_conc );
	g_free( meta_year_conc );
	
	return 0;
}


/*  */
static void cb_child_watch( GPid pid, gint status, Data *data )
{
	GtkWidget *dialog;
    /* Remove timeout callback */
    g_source_remove( data->timeout_id );

    /* Close pid */
    g_spawn_close_pid( pid );

	if( status == 0 )
	{
		dialog = gtk_message_dialog_new (GTK_WINDOW(data->parent), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Transcoding successful !" );
  
		gtk_window_set_title (GTK_WINDOW (dialog), "Finished");
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	}
	else
	{
		dialog = gtk_message_dialog_new (GTK_WINDOW(data->parent), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Oops something went wrong !" );
  
		gtk_window_set_title (GTK_WINDOW (dialog), "Error");
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	}
}


/*  */
static gboolean cb_out_watch( GIOChannel *channel, GIOCondition cond, Data *data )
{
    gchar *string;
    gsize  size;
 
    if( cond == G_IO_HUP )
    {
        g_io_channel_unref( channel );
        return( FALSE );
    }
 
    g_io_channel_read_line( channel, &string, &size, NULL, NULL );
    gtk_text_buffer_insert_at_cursor( data->out, string, -1 );
    g_free( string );
 
    return( TRUE );
}


/*  */
static gboolean cb_err_watch( GIOChannel *channel, GIOCondition cond, Data *data )
{
    gchar *string;
    gsize  size;
 
    if( cond == G_IO_HUP )
    {
        g_io_channel_unref( channel );
        return( FALSE );
    }
 
    g_io_channel_read_line( channel, &string, &size, NULL, NULL );
    gtk_text_buffer_insert_at_cursor( data->err, string, -1 );
    g_free( string );
 
    return( TRUE );
}
