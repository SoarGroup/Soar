/*  XMMS - Cross-platform multimedia player
 *  Copyright (C) 1998-2004  Peter Alm, Mikael Alm, Olle Hallnas,
 *                           Thomas Nilsson and 4Front Technologies
 *  Copyright (C) 1999-2004  Haavard Kvaalen
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#define VERSION "0.0.0"

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <xmms/plugin.h>
#include <xmms/xmmsctrl.h>
#include <xmms/dirbrowser.h>
#include <xmms/configfile.h>
#include <xmms/util.h>

#include <libplayerc/playerc.h>

static GtkWidget *configure_win = NULL, *configure_vbox;
static GtkWidget *server_address_entry;
static GtkWidget *server_port_entry;
static GtkWidget *buffer_entry;
static GtkWidget *server_index_entry;
static GtkWidget *configure_separator;
static GtkWidget *configure_bbox, *configure_ok, *configure_cancel;

static gchar *server_address = NULL;
static guint32 server_port = 6665;
static guint32 server_index = 0;
static playerc_client_t *client;
static playerc_audio_t *audio_proxy;
static guint32 written = 0;
static AFormat afmt;
static guint32 sampleRate = 44100;
static guint8 numChannels = 2;
static gint bytesPerSecond = 176400;
static guint8 *buffer = NULL;
static guint32 bufferTime = 5000;
static guint32 bufferLength = 0;
static guint32 bufferPos = 0;
static double startTime = 0.0f;
static double pauseStartTime = 0.0f;
static double pausedTime = 0.0f;
gint ctrlsocket_get_session_id(void);		/* FIXME */

static void playerout_init(void);
static void playerout_get_volume (int *l, int *r);
static void playerout_set_volume (int l, int r);
static gint playerout_open(AFormat fmt, gint rate, gint nch);
static void playerout_write(void *ptr, gint length);
static void playerout_close(void);
static void playerout_flush(gint time);
static void playerout_pause(short p);
static gint playerout_free(void);
static gint playerout_playing(void);
static gint playerout_get_written_time(void);
static gint playerout_get_output_time(void);
static void playerout_configure(void);

OutputPlugin playerout_op =
{
	NULL,
	NULL,
	NULL,			/* Description */
	playerout_init,
	NULL,			/* about */
	playerout_configure,		/* configure */
	playerout_get_volume,			/* get_volume */
	playerout_set_volume,			/* set_volume */
	playerout_open,
	playerout_write,
	playerout_close,
	playerout_flush,
	playerout_pause,
	playerout_free,
	playerout_playing,
	playerout_get_output_time,
	playerout_get_written_time,
};

OutputPlugin *get_oplugin_info(void)
{
	playerout_op.description = g_strdup_printf("Player Output Driver %s", VERSION);
	return &playerout_op;
}

static void playerout_init(void)
{
	ConfigFile *cfgfile;

	cfgfile = xmms_cfg_open_default_file();
	if (cfgfile)
	{
		xmms_cfg_read_string(cfgfile, "playerout", "server_address", &server_address);
		xmms_cfg_read_int(cfgfile, "playerout", "server_port", &server_port);
		xmms_cfg_read_int(cfgfile, "playerout", "server_index", &server_index);
		xmms_cfg_read_int (cfgfile, "playerout", "buffer_time", &bufferTime);
		xmms_cfg_free(cfgfile);
	}

	if (!server_address)
		server_address = g_strdup ("localhost");
}

static void playerout_get_volume (int *l, int *r)
{
	if (audio_proxy)
	{
// 		playerc_audio_get_mixer_levels (audio_proxy);
		*l = (int) (audio_proxy->mixer_data.channels[0].amplitude * 100);
		*r = (int) (audio_proxy->mixer_data.channels[0].amplitude * 100);
	}
	else
		*l = *r = 0;
}

static void playerout_set_volume (int l, int r)
{
	if (audio_proxy)
	{
		// Find the average of l and r
		float avg = ((float) (l + r) / 2.0f) / 100.0f;
		playerc_audio_mixer_channel_cmd (audio_proxy, 0, avg, 1);
	}
}

static gint playerout_open(AFormat fmt, gint rate, gint nch)
{
	gint pos;

	written = 0;
	afmt = fmt;
	sampleRate = rate;
	numChannels = nch;
	startTime = 0.0f;
	pausedTime = 0.0f;

	if (xmms_check_realtime_priority())
	{
		xmms_show_message("Error",
				  "You cannot use the Player Output plugin\n"
				    "when you're running in realtime mode.",
				  "Ok", FALSE, NULL, NULL);
		return 0;
	}

	/* do player server connection here */
	// Create a client object and connect to the server; the server must
	// be running on "localhost" at port 6665
	client = playerc_client_create(NULL, server_address, server_port);
	if (playerc_client_connect(client) != 0)
	{
		fprintf(stderr, "error: %s\n", playerc_error_str());
		client = NULL;
		return 0;
	}

	// Create a audio proxy susbscribe
	audio_proxy = playerc_audio_create(client, server_index);
	if (playerc_audio_subscribe(audio_proxy, PLAYERC_OPEN_MODE) != 0)
	{
		fprintf(stderr, "error: %s\n", playerc_error_str());
		return 0;
	}

	// Set to PULL data mode
	if (playerc_client_datamode (client, PLAYERC_DATAMODE_PULL) != 0)
	{
		fprintf(stderr, "error: %s\n", playerc_error_str());
		return 0;
	}
	if (playerc_client_set_replace_rule (client, -1, -1, PLAYER_MSGTYPE_DATA, -1, 1) != 0)
	{
		fprintf(stderr, "error: %s\n", playerc_error_str());
		return 0;
	}

	bytesPerSecond = rate * nch;
	if (fmt == FMT_S16_LE || fmt == FMT_S16_BE || fmt == FMT_S16_NE ||
		   fmt == FMT_U16_LE || fmt == FMT_U16_BE || fmt == FMT_U16_NE)
		bytesPerSecond *= 2;

	// Allocate a buffer
	bufferLength = (int) (((float) bufferTime / 1000.0f) * bytesPerSecond);
	if (buffer != NULL)
		free (buffer);
	buffer = malloc (bufferLength);
	bufferPos = 0;

	return 1;
}

static void convert_buffer(gpointer buffer, gint length)
{
	gint i;

	if (afmt == FMT_S8)
	{
		guint8 *ptr1 = buffer;
		gint8 *ptr2 = buffer;

		for (i = 0; i < length; i++)
			*(ptr1++) = *(ptr2++) ^ 128;
	}
	if (afmt == FMT_S16_BE)
	{
		gint16 *ptr = buffer;

		for (i = 0; i < length >> 1; i++, ptr++)
			*ptr = GUINT16_SWAP_LE_BE(*ptr);
	}
	if (afmt == FMT_S16_NE)
	{
		gint16 *ptr = buffer;

		for (i = 0; i < length >> 1; i++, ptr++)
			*ptr = GINT16_TO_LE(*ptr);
	}
	if (afmt == FMT_U16_BE)
	{
		gint16 *ptr1 = buffer;
		guint16 *ptr2 = buffer;

		for (i = 0; i < length >> 1; i++, ptr2++)
			*(ptr1++) = GINT16_TO_LE(GUINT16_FROM_BE(*ptr2) ^ 32768);
	}
	if (afmt == FMT_U16_LE)
	{
		gint16 *ptr1 = buffer;
		guint16 *ptr2 = buffer;

		for (i = 0; i < length >> 1; i++, ptr2++)
			*(ptr1++) = GINT16_TO_LE(GUINT16_FROM_LE(*ptr2) ^ 32768);
	}
	if (afmt == FMT_U16_NE)
	{
		gint16 *ptr1 = buffer;
		guint16 *ptr2 = buffer;

		for (i = 0; i < length >> 1; i++, ptr2++)
			*(ptr1++) = GINT16_TO_LE((*ptr2) ^ 32768);
	}
}

static void playerout_write(void *ptr, gint length)
{
	unsigned int format = 0;

	if (startTime == 0.0f)
	{
		struct timeval timeVal;
		gettimeofday (&timeVal, NULL);
		startTime = (double) timeVal.tv_sec + ((double) timeVal.tv_usec) / 1e6;
	}

	// Add to buffer if there is space
	if (length + bufferPos > bufferLength)
	{
		// Copy what we can fit into the buffer
// 		printf ("Filling up remainder of buffer with %d bytes at position %d\n", (bufferLength - bufferPos), bufferPos);
		memcpy (&buffer[bufferPos], ptr, (bufferLength - bufferPos));
		ptr += bufferLength - bufferPos;
		length -= bufferLength - bufferPos;

		// Send out the buffer
		if (afmt == FMT_S8 || afmt == FMT_S16_BE ||
				  afmt == FMT_U16_LE || afmt == FMT_U16_BE || afmt == FMT_U16_NE)
			convert_buffer(ptr, length);
		#ifdef WORDS_BIGENDIAN
		if (afmt == FMT_S16_NE)
			convert_buffer(ptr, length);
		#endif

		format = PLAYER_AUDIO_FORMAT_RAW;
		if (numChannels == 2)
			format |= PLAYER_AUDIO_STEREO;
		format |= PLAYER_AUDIO_16BIT;
		if (sampleRate == 11025)
			format |= PLAYER_AUDIO_FREQ_11k;
		else if (sampleRate == 22050)
			format |= PLAYER_AUDIO_FREQ_22k;
		else if (sampleRate == 48000)
			format |= PLAYER_AUDIO_FREQ_48k;
		else
			format |= PLAYER_AUDIO_FREQ_44k;

		playerc_audio_wav_play_cmd (audio_proxy, bufferLength, buffer, format);
		written += bufferLength - bufferPos;

		// Reset the buffer
		bufferPos = 0;
	}

	// Copy data into the buffer
// 	printf ("Copying %d bytes into buffer at position %d\n", length, bufferPos);
	memcpy (&buffer[bufferPos], ptr, length);
	bufferPos += length;
	written += length;

}

static void playerout_close(void)
{
	if (client)
	{
		// Shutdown and tidy up
		playerc_audio_unsubscribe(audio_proxy);
		playerc_audio_destroy(audio_proxy);
		playerc_client_disconnect(client);
		playerc_client_destroy(client);

		audio_proxy = NULL;
		client = NULL;

		free (buffer);
		buffer = NULL;
	}
}

static void playerout_flush(gint time)
{
}

static void playerout_pause(short p)
{
	// Audio protocol doesn't support pausing playback
/*	printf ("Told to %s\n", p ? "pause" : "unpause");
	if (p)
	{
		struct timeval timeVal;
		gettimeofday (&timeVal, NULL);
		pauseStartTime = (double) timeVal.tv_sec + ((double) timeVal.tv_usec) / 1e6;
		printf ("started pause at %f\n", pauseStartTime);
	}
	else
	{
		double thisPausedTime = 0.0f;
		struct timeval timeVal;
		gettimeofday (&timeVal, NULL);
		thisPausedTime = (double) timeVal.tv_sec + ((double) timeVal.tv_usec) / 1e6;
		thisPausedTime -= pauseStartTime;
		pausedTime += thisPausedTime;
		printf ("paused for %f\n", thisPausedTime);
	}*/
}

static gint playerout_free(void)
{
	// Use the max size of a wave message as free space size
	return 1048576;
}

static gint playerout_playing(void)
{
	playerc_client_read (client);
	if (audio_proxy->state & PLAYER_AUDIO_STATE_PLAYING)
		return 1;
	else
		return 0;
}

static gint playerout_get_written_time(void)
{
	double result = 0;
	result = ((double) written) / (sampleRate * numChannels);
	if (afmt == FMT_S16_LE || afmt == FMT_S16_BE || afmt == FMT_S16_NE ||
			afmt == FMT_U16_LE || afmt == FMT_U16_BE || afmt == FMT_U16_NE)
		result /= 2.0f;
	return result;
}

static gint playerout_get_output_time(void)
{
	if (startTime == 0.0f)
		return 0;

	struct timeval timeVal;
	gettimeofday (&timeVal, NULL);
	double currentTime = (double) timeVal.tv_sec + ((double) timeVal.tv_usec) / 1e6;
	return (currentTime - startTime);// - pausedTime);
}

static void configure_ok_cb(gpointer data)
{
	ConfigFile *cfgfile;

	if (server_address)
		g_free(server_address);
	server_address = g_strdup(gtk_entry_get_text(GTK_ENTRY(server_address_entry)));

	server_port = atoi (gtk_entry_get_text (GTK_ENTRY (server_port_entry)));
	server_index = atoi (gtk_entry_get_text( GTK_ENTRY (server_index_entry)));
	bufferTime = atoi (gtk_entry_get_text (GTK_ENTRY (buffer_entry)));

	cfgfile = xmms_cfg_open_default_file();
	if (!cfgfile)
		cfgfile = xmms_cfg_new();

	xmms_cfg_write_string(cfgfile, "playerout", "server_address", server_address);
	xmms_cfg_write_int(cfgfile, "playerout", "server_port", server_port);
	xmms_cfg_write_int(cfgfile, "playerout", "server_index", server_index);
	xmms_cfg_write_int (cfgfile, "playerout", "buffer_time", bufferTime);

	xmms_cfg_free(cfgfile);

	gtk_widget_destroy(configure_win);
}

static void configure_destroy(void)
{
}

static void playerout_configure(void)
{
	char temp[10];
	GtkWidget *server_hbox, *server_label;
	GtkWidget *port_hbox, *port_label;
	GtkWidget *index_hbox, *index_label;
	GtkWidget *buffer_hbox, *buffer_label;

	if(configure_win)
		return;

	configure_win = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_signal_connect(GTK_OBJECT(configure_win), "destroy", GTK_SIGNAL_FUNC(configure_destroy), NULL);
	gtk_signal_connect(GTK_OBJECT(configure_win), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &configure_win);
	gtk_window_set_title(GTK_WINDOW(configure_win), "Player Output Configuration");
	gtk_window_set_position(GTK_WINDOW(configure_win), GTK_WIN_POS_MOUSE);

	gtk_container_set_border_width(GTK_CONTAINER(configure_win), 10);

	configure_vbox = gtk_vbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(configure_win), configure_vbox);



	server_hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(configure_vbox), server_hbox, FALSE, FALSE, 0);

	server_label = gtk_label_new("Server Address:");
	gtk_box_pack_start(GTK_BOX(server_hbox), server_label, FALSE, FALSE, 0);
	gtk_widget_show(server_label);

	server_address_entry = gtk_entry_new();
	if (server_address)
		gtk_entry_set_text(GTK_ENTRY(server_address_entry), server_address);
	gtk_widget_set_usize(server_address_entry, 200, -1);
	gtk_box_pack_start(GTK_BOX(server_hbox), server_address_entry, TRUE, TRUE, 0);
	gtk_widget_show(server_address_entry);

	gtk_widget_show(server_hbox);




	port_hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(configure_vbox), port_hbox, FALSE, FALSE, 0);

	port_label = gtk_label_new("Server Port:");
	gtk_box_pack_start(GTK_BOX(port_hbox), port_label, FALSE, FALSE, 0);
	gtk_widget_show(port_label);

	server_port_entry = gtk_entry_new();
	snprintf (temp, 9, "%d", server_port);
	gtk_entry_set_text (GTK_ENTRY(server_port_entry), temp);
	gtk_widget_set_usize(server_port_entry, 200, -1);
	gtk_box_pack_start(GTK_BOX(port_hbox), server_port_entry, TRUE, TRUE, 0);
	gtk_widget_show(server_port_entry);

	gtk_widget_show(port_hbox);





	index_hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(configure_vbox), index_hbox, FALSE, FALSE, 0);

	index_label = gtk_label_new("Proxy index:");
	gtk_box_pack_start(GTK_BOX(index_hbox), index_label, FALSE, FALSE, 0);
	gtk_widget_show(index_label);

	server_index_entry = gtk_entry_new();
	snprintf (temp, 9, "%d", server_index);
	gtk_entry_set_text (GTK_ENTRY(server_index_entry), temp);
	gtk_widget_set_usize(server_index_entry, 200, -1);
	gtk_box_pack_start(GTK_BOX(index_hbox), server_index_entry, TRUE, TRUE, 0);
	gtk_widget_show(server_index_entry);

	gtk_widget_show(index_hbox);






	buffer_hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(configure_vbox), buffer_hbox, FALSE, FALSE, 0);

	buffer_label = gtk_label_new("Buffer length (ms):");
	gtk_box_pack_start(GTK_BOX(buffer_hbox), buffer_label, FALSE, FALSE, 0);
	gtk_widget_show(buffer_label);

	buffer_entry = gtk_entry_new();
	snprintf (temp, 9, "%d", bufferTime);
	gtk_entry_set_text (GTK_ENTRY(buffer_entry), temp);
	gtk_widget_set_usize(buffer_entry, 200, -1);
	gtk_box_pack_start(GTK_BOX(buffer_hbox), buffer_entry, TRUE, TRUE, 0);
	gtk_widget_show(buffer_entry);

	gtk_widget_show(buffer_hbox);





	configure_separator = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(configure_vbox), configure_separator, FALSE, FALSE, 0);
	gtk_widget_show(configure_separator);

	configure_bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(configure_bbox), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(configure_bbox), 5);
	gtk_box_pack_start(GTK_BOX(configure_vbox), configure_bbox, FALSE, FALSE, 0);

	configure_ok = gtk_button_new_with_label("Ok");
	gtk_signal_connect(GTK_OBJECT(configure_ok), "clicked", GTK_SIGNAL_FUNC(configure_ok_cb), NULL);
	GTK_WIDGET_SET_FLAGS(configure_ok, GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(configure_bbox), configure_ok, TRUE, TRUE, 0);
	gtk_widget_show(configure_ok);
	gtk_widget_grab_default(configure_ok);

	configure_cancel = gtk_button_new_with_label("Cancel");
	gtk_signal_connect_object(GTK_OBJECT(configure_cancel), "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT(configure_win));
	GTK_WIDGET_SET_FLAGS(configure_cancel, GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(configure_bbox), configure_cancel, TRUE, TRUE, 0);
	gtk_widget_show(configure_cancel);
	gtk_widget_show(configure_bbox);
	gtk_widget_show(configure_vbox);
	gtk_widget_show(configure_win);
}
