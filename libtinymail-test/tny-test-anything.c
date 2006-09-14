#include <tny-gtk-text-buffer-stream.h>

#include <string.h>

#include <tny-stream-test.h>

/* We are going to test the camel implementation */
#include <tny-stream-camel.h>
#include <tny-camel-stream.h>
#include <camel/camel-stream-mem.h>
#include <camel/camel-data-wrapper.h>
#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-folder-summary.h>
#include <tny-test-stream.h>
#include <tny-fs-stream.h>

static TnyStream *cmstream = NULL, *fstream, *tbstream = NULL, *source = NULL;


int main (int argc , char **argv)
{
	GtkTextBuffer *buffer;
	GtkTextView *view;
    	gchar *tmpl = g_strdup ("/tmp/tinymail-stream-test.XXXXXX");
	gint filed = g_mkstemp (tmpl);
    
 	if (filed == -1)
		perror ("Creating temporary file");
    
    	g_free (tmpl);
    
   	gtk_init (&argc, &argv);
	g_thread_init (NULL);
    
    	view = GTK_TEXT_VIEW (gtk_text_view_new ());
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
  
	CamelInternetAddress *addr = camel_internet_address_new ();
	CamelStream *stream = camel_stream_mem_new ();

	camel_object_unref (CAMEL_OBJECT (addr));

	/* These are the streams that is being tested */
	tbstream = tny_gtk_text_buffer_stream_new (buffer);
	source = TNY_STREAM (tny_test_stream_new ());
	fstream = tny_fs_stream_new (filed);
    
     TnyStream *streams [1] = { fstream };
    int te=0;
    
    
    for (te=0; te<1; te++)
    {
	TnyStream *iface = streams[te];
	gchar *buffer = (gchar*) malloc (sizeof (gchar) * 42);
	gint n=0;
	/* 21 times the answer to all questions */
	const gchar *ret = "424242424242424242424242424242424242424242";

	tny_stream_reset (source);
	tny_stream_reset (iface);

	tny_stream_write_to_stream (source, iface);

	tny_stream_reset (iface);
	tny_stream_read (iface, buffer, strlen (ret));

	buffer[strlen(ret)] = '\0';



	tny_stream_reset (iface);
	tny_stream_reset (source);

	while (!tny_stream_eos (iface))
	{
		gchar buf[2];
		tny_stream_read (iface, buf, 2);
		printf ("[[%s]]\n", buf);
	    	n++;
	}
printf ("n=%d\n", n);

	g_free (buffer);
	
	g_object_unref (G_OBJECT (iface));
   }
}
