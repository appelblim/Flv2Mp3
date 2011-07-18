/*
 * 
 * Copyright (C) appelblim 2011 <docvanrock@googlemail.com>
 *
 * Various other sources were used for creating this. I do not claim any legal
 * ownership of those various lines I copied from someone. This softwares
 * sole purpose is of educational nature.
 * 
 * flv2mp3 is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * flv2mp3 is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>

typedef struct 
{
    /* Data struct containing data to be passed around
	 * between functions. */
    GtkTextBuffer *out;
    GtkTextBuffer *err;
	const gchar *inputfilename;
	const gchar *title;
	const gchar *artist;
	const gchar *album;
	const gchar *genre;
	const gchar *year;
	const gchar *quality;
	GtkWidget *parent;
	GtkLabel *label_pwd;
 
    /* Timeout source id. */
    gint timeout_id;
} Data;

static void destroy( GtkWidget*, gpointer );
static gint button_clicked( GtkButton*, Data* );
static void quality_changed( GtkComboBoxText*, Data* );
static void meta_okay_clicked( GtkButton*, Data* );
static void file_changed( GtkFileChooser*, Data* );
static void cb_child_watch( GPid pid, gint status, Data *data );
static gboolean cb_out_watch( GIOChannel *channel, GIOCondition cond, Data *data );
static gboolean cb_err_watch( GIOChannel *channel, GIOCondition cond, Data *data );


int main (int argc, char *argv[])
{
  GtkWidget *window, *notebook;
  GtkWidget *label1, *label2, *btn_close, *btn_go, *btn_meta_ok, *meta_table, *meta_track_title, *meta_track_artist, *meta_track_year, *meta_track_genre, *meta_track_album, *label_title, *label_artist, *label_quality;
  GtkWidget *label_kbs, *meta_vbox, *main_vbox, *main_hbox, *main_hbox2, *combobox_quality, *picker, *label_pwd, *text_view_ffmpeg, *sw, *vpaned;
	GtkWidget *about_vbox, *label3, *about_text;
	GtkWidget *label_genre, *label_year, *label_album;
  Data *data;
	GtkTextBuffer *about_buffer;
	GtkTextIter iter;

  data = g_slice_new( Data );

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Flv2Mp3");
  gtk_container_set_border_width (GTK_CONTAINER (window), 10);
  gtk_window_set_resizable( GTK_WINDOW(window), FALSE );
  gtk_window_set_icon_from_file( GTK_WINDOW(window), "/usr/share/pixmaps/faces/baseball.png", NULL );

  g_signal_connect (G_OBJECT (window), "destroy",
                    G_CALLBACK (destroy), NULL);

	notebook = gtk_notebook_new ();
	label1 = gtk_label_new ("Transcode");
	label2 = gtk_label_new ("Metadata");
	label3 = gtk_label_new ("About");
	
  combobox_quality = gtk_combo_box_text_new();
  btn_close = gtk_button_new_from_stock( GTK_STOCK_CLOSE );
  btn_go = gtk_button_new_with_label ("Start");
  btn_meta_ok = gtk_button_new_with_label ("Apply");
  
  meta_table = gtk_table_new (2, 2, TRUE);
  
  meta_track_title = gtk_entry_new();
  meta_track_artist = gtk_entry_new();
	meta_track_year = gtk_entry_new();
	meta_track_genre = gtk_entry_new();
	meta_track_album = gtk_entry_new();
  
  label_title = gtk_label_new( "Title:" );
  label_artist = gtk_label_new( "Artist:" );
	label_genre = gtk_label_new( "Genre:" );
	label_year = gtk_label_new( "Year:" );
	label_album = gtk_label_new( "Album:" );
  label_kbs = gtk_label_new( " kb/s" );
  label_quality = gtk_label_new( "Quality: " );
  label_pwd = gtk_label_new( "" );
  data->quality = "256";
  data->label_pwd = GTK_LABEL(label_pwd);

  picker = gtk_file_chooser_button_new ("Pick a File", GTK_FILE_CHOOSER_ACTION_OPEN);
  gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER (picker), g_get_home_dir() );


	text_view_ffmpeg = gtk_text_view_new();
	gtk_text_view_set_cursor_visible( GTK_TEXT_VIEW(text_view_ffmpeg), FALSE );
  /* Set the line breaking behaviour. */
  gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW(text_view_ffmpeg), GTK_WRAP_WORD );

	vpaned = gtk_vpaned_new ();
	gtk_container_set_border_width (GTK_CONTAINER(vpaned), 5);
	
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_paned_add2 (GTK_PANED (vpaned), sw);

    gtk_container_add (GTK_CONTAINER (sw), text_view_ffmpeg);
	
  data->out = gtk_text_view_get_buffer( GTK_TEXT_VIEW( text_view_ffmpeg ) );
  data->err = gtk_text_view_get_buffer( GTK_TEXT_VIEW( text_view_ffmpeg ) );

  data->parent = window;
	
	about_text = gtk_text_view_new();
	gtk_text_view_set_cursor_visible( GTK_TEXT_VIEW(about_text), FALSE );
	gtk_text_view_set_justification( GTK_TEXT_VIEW(about_text), GTK_JUSTIFY_CENTER );
	gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW(about_text), GTK_WRAP_WORD );
	about_buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( about_text ) );
	gtk_text_buffer_get_iter_at_offset (about_buffer, &iter, 0);
	gtk_text_buffer_insert (about_buffer, &iter, "\nFlv2Mp3 - 1.2 \n\n by appelblim \n 2011 \n\n requires any recent FFmpeg version & gstreamer1.0-ugly Plugins \n\n Licence: GPL", -1);
	
  gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(combobox_quality), "192" );
  gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(combobox_quality), "256" );  
  gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(combobox_quality), "320" );

  gtk_combo_box_set_active( GTK_COMBO_BOX(combobox_quality), 1 );
  
	meta_vbox = gtk_vbox_new (FALSE, 5);
	main_hbox = gtk_hbox_new (FALSE, 5);
	main_vbox = gtk_vbox_new (FALSE, 5);
	main_hbox2 = gtk_hbox_new (FALSE, 5);
	about_vbox = gtk_vbox_new (FALSE, 5);

	gtk_box_pack_start( GTK_BOX(main_vbox), vpaned, TRUE, TRUE, 0 );
  
	/* Pack the table. */
	gtk_table_attach( GTK_TABLE(meta_table), label_title, 0, 1, 1, 2, GTK_EXPAND, GTK_SHRINK, 0, 0 );
	gtk_table_attach( GTK_TABLE(meta_table), label_artist, 0, 1, 2, 3, GTK_EXPAND, GTK_SHRINK, 0, 0 );
	gtk_table_attach( GTK_TABLE(meta_table), meta_track_title, 1, 2, 1, 2, GTK_EXPAND, GTK_SHRINK, 0, 0 );
	gtk_table_attach( GTK_TABLE(meta_table), meta_track_artist, 1, 2, 2, 3, GTK_EXPAND, GTK_SHRINK, 0, 0 );
	gtk_table_attach( GTK_TABLE(meta_table), label_album, 0, 1, 3, 4, GTK_EXPAND, GTK_SHRINK, 0, 0 );
	gtk_table_attach( GTK_TABLE(meta_table), label_genre, 0, 1, 4, 5, GTK_EXPAND, GTK_SHRINK, 0, 0 );
	gtk_table_attach( GTK_TABLE(meta_table), label_year, 0, 1, 5, 6, GTK_EXPAND, GTK_SHRINK, 0, 0 );
	gtk_table_attach( GTK_TABLE(meta_table), meta_track_year, 1, 2, 3, 4, GTK_EXPAND, GTK_SHRINK, 0, 0 );
	gtk_table_attach( GTK_TABLE(meta_table), meta_track_genre, 1, 2, 4, 5, GTK_EXPAND, GTK_SHRINK, 0, 0 );
	gtk_table_attach( GTK_TABLE(meta_table), meta_track_album, 1, 2, 5, 6, GTK_EXPAND, GTK_SHRINK, 0, 0 );

  /* Set the spacings for the table. */
  gtk_table_set_row_spacings(GTK_TABLE(meta_table), 5);
  gtk_table_set_col_spacings(GTK_TABLE(meta_table), 5);

  g_object_set_data(G_OBJECT(btn_meta_ok), "entry1", meta_track_title);
  g_object_set_data(G_OBJECT(btn_meta_ok), "entry2", meta_track_artist);
	g_object_set_data(G_OBJECT(btn_meta_ok), "entry3", meta_track_album);
	g_object_set_data(G_OBJECT(btn_meta_ok), "entry4", meta_track_genre);
	g_object_set_data(G_OBJECT(btn_meta_ok), "entry5", meta_track_year);
  
  /* Signal connections. */
  g_signal_connect( G_OBJECT(btn_meta_ok), "clicked", G_CALLBACK(meta_okay_clicked), data );

  g_signal_connect_swapped (G_OBJECT(btn_close), "clicked", G_CALLBACK(gtk_widget_destroy), (gpointer)window );
  g_signal_connect( G_OBJECT(btn_go), "clicked", G_CALLBACK(button_clicked), data );
  
  g_signal_connect( G_OBJECT(combobox_quality), "changed", G_CALLBACK(quality_changed), data );

  g_signal_connect( G_OBJECT(picker), "selection_changed", G_CALLBACK(file_changed), data );


  
	/* Pack the table and the button into the main_vbox. */
	gtk_box_pack_start( GTK_BOX(main_hbox), btn_close, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_hbox), btn_go, TRUE, TRUE, 0 );

	gtk_box_pack_start( GTK_BOX(main_hbox2), label_quality, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_hbox2), combobox_quality, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_hbox2), label_kbs, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_vbox), picker, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_vbox), label_pwd, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_vbox), sw, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_vbox), main_hbox2, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_vbox), main_hbox, TRUE, TRUE, 0 );

	/* Pack the table and the button into the meta_vbox. */
	gtk_box_pack_start(GTK_BOX(meta_vbox), meta_table, TRUE, TRUE, 0 );
	gtk_box_pack_start(GTK_BOX(meta_vbox), btn_meta_ok, FALSE, FALSE, 0 );

	/* Pack the stuff into the about_vbox. */
	gtk_box_pack_start( GTK_BOX(about_vbox), about_text, TRUE, TRUE, 0 );

	/* Append to pages to the notebook container. */
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), main_vbox, label1);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), meta_vbox, label2);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), about_vbox, label3);
  
	gtk_container_add (GTK_CONTAINER (window), notebook);
	gtk_widget_show_all(window);
	
	gtk_main();
	
	g_slice_free( Data, data );

	return 0;
}


static void destroy( GtkWidget *window, gpointer data )
{
  gtk_main_quit();
}


/* Create a new message dialog that tells the user that the button was clicked. */
static gint button_clicked(GtkButton *button, Data *data)
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

/* Function handling changes in the combo box for the quality. */
static void quality_changed( GtkComboBoxText *combobox_quality, Data* data )
{	
	data->quality = gtk_combo_box_text_get_active_text( GTK_COMBO_BOX_TEXT(combobox_quality) );
}

/* Callback function for when the "Okay" button on the meta page is clicked. */
static void meta_okay_clicked( GtkButton *btn_meta_ok, Data *data )
{	
	GtkEntry *entry1 = g_object_get_data(G_OBJECT(btn_meta_ok), "entry1");
	GtkEntry *entry2 = g_object_get_data(G_OBJECT(btn_meta_ok), "entry2");
	GtkEntry *entry3 = g_object_get_data(G_OBJECT(btn_meta_ok), "entry3");
	GtkEntry *entry4 = g_object_get_data(G_OBJECT(btn_meta_ok), "entry4");
	GtkEntry *entry5 = g_object_get_data(G_OBJECT(btn_meta_ok), "entry5");

	
	data->title = gtk_entry_get_text(GTK_ENTRY(entry1));
	data->artist = gtk_entry_get_text(GTK_ENTRY(entry2));
	data->album = gtk_entry_get_text(GTK_ENTRY(entry5));
	data->genre = gtk_entry_get_text(GTK_ENTRY(entry4));
	data->year = gtk_entry_get_text(GTK_ENTRY(entry3));
}


static void file_changed( GtkFileChooser *picker, Data *data )
{
	gchar *file = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(picker) );
	gtk_label_set_text( data->label_pwd, file );
	data->inputfilename = file;
}


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