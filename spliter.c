#include <gtk/gtk.h>
#include "android_image.h"
#include <stdio.h>

GtkEntry *entr;
GtkTextView *view;
GtkButton *spbtn;
GtkTextBuffer *buffer;
GtkTextIter iter;
extern unsigned int PAGE_SIZE;
extern unsigned int KERNEL_SIZE;
extern unsigned int RAMDISK_SIZE;
extern unsigned int SECOND_SIZE;
FILE *fp=NULL;
boot_img_hdr *bh;
unsigned char *dest_path=NULL;
int main(int argc, char *argv[])
{
    GtkBuilder      *builder=NULL; 
    GtkWidget       *window=NULL;

    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "spliter.glade", NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
    if(!window)
	{
		printf("No window created\n");
		gtk_main_quit();
	}
	gtk_builder_connect_signals(builder, NULL);
	entr=(GtkEntry*)GTK_WIDGET(gtk_builder_get_object(builder, "filenameentry"));
	if(!entr)
		printf("text entry not found\n"); 
	view=(GtkTextView *)GTK_WIDGET(gtk_builder_get_object(builder, "Infoview"));
	if(!view)
		printf("Infoview not found\n");
	spbtn = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder,"Splitbtn"));
	if(!spbtn)
		printf("Split button not found\n");
	buffer=gtk_text_view_get_buffer (GTK_TEXT_VIEW(view));
	//gtk_text_view_set_buffer (GTK_TEXT_VIEW(view), buffer);
	if(!buffer)
		printf("No buffer for text view\n");
	gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
	//gtk_text_buffer_set_text (buffer,"hello",-1);
	g_object_unref(builder);

    gtk_widget_show(window);                
    gtk_main();

    return 0;
}

// called when window is closed
void on_window1_destroy()
{
    //printf("Window destroy event\n");
    if(bh)
		free(bh);
	if(fp)
		fclose(fp);
	if(dest_path)
		free(dest_path);
    gtk_main_quit();
}
void on_openbtn_clicked()
{
	//printf("Open btn clicked\n");
	GtkWidget *dialog=NULL;
	GtkFileChooserAction action=GTK_FILE_CHOOSER_ACTION_OPEN;
	gint res;
	dialog = gtk_file_chooser_dialog_new ("Open ROM image File",
                                      NULL,
                                      action,
                                      ("_Cancel"),
                                      GTK_RESPONSE_CANCEL,
                                      ("_Open"),
                                      GTK_RESPONSE_ACCEPT,
                                      NULL);
	res = gtk_dialog_run (GTK_DIALOG (dialog));
	if(res == GTK_RESPONSE_ACCEPT)
  	{
    		char *filename;
    		GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
    		filename = gtk_file_chooser_get_filename (chooser);
    		//printf("Selected file %s\n",filename);
		//GtkEntry *entr= GTK_WIDGET(gtk_builder_get_object(NULL,"filenameentry"));
		//gtk_entry_buffer_set_max_length (filenameentry,1024);
		gtk_entry_set_text(entr,filename);
		gtk_text_buffer_insert(buffer, &iter, "Opening file: ", -1);
		gtk_text_buffer_insert(buffer, &iter, filename, -1);
		fp=fopen(filename,"rb");
		if(fp==NULL)
		{
			error_dialog("Unable to open file");
			//gtk_main_quit();
			return;
		}
		
		g_free (filename);
	}
	gtk_widget_destroy (dialog);
	bh=read_header(fp);
	if(bh==NULL)
	{
		error_dialog("Unable to read header ");
		//gtk_main_quit();
		return;
	}
	char *buff=NULL;
	buff = (char*)malloc(1024*sizeof(char));
	if(!buff)
		error_dialog("Unable to allocate buffer for sprintf()\n");
	
	sprintf(buff, \
	"page size = %8d bytes\nkernel size = %8d\nkernel address = 0x%08x\nramdisk size = %8d\n ramdisk addr = 0x%08x\nsecond size = %8d\nsecond load address = 0x%08x\n", \
	 bh->page_size,bh->kernel_size,bh->kernel_addr,bh->ramdisk_size,bh->ramdisk_addr,bh->second_size,bh->second_addr);
	 gtk_text_buffer_insert(buffer,&iter,buff,-1);
	 PAGE_SIZE = bh->page_size;
	 KERNEL_SIZE = bh->kernel_size;
	 RAMDISK_SIZE = bh->ramdisk_size;
	 SECOND_SIZE = bh->second_size;
	if(buff)
		free(buff);
	 // change sensitivity of button as everything is alright ... is it??? :P
	 gtk_widget_set_sensitive ((GtkWidget*)spbtn, TRUE);
}
void on_exitbtn_clicked()
{
	//printf("Exit btn clicked\n");
	//gtk_main_quit();
	on_window1_destroy();
}
void on_Splitbtn_clicked()
{
	//printf("Split btn clicked\n");
	// here we have to open 3 files for write
	// one each for kernel, ramdisk and dtb
	// we will read the offset where each individual pieces are present 
	// and extract them.
	destination_path();
	//printf("dest_path : %s\n",dest_path);
	write_splited_files(fp);
	gtk_text_buffer_insert(buffer,&iter,"Extraction complete\n",-1);
}

