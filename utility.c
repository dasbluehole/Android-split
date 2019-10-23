#include "android_image.h"
#include <gtk/gtk.h>
#include <stdio.h>
// Global variables to be filled.
unsigned char BOOT_MAGIC[] = "ANDROID!";
unsigned int PAGE_SIZE = 0;
unsigned int KERNEL_SIZE = 0;
unsigned int RAMDISK_SIZE = 0;
unsigned int SECOND_SIZE = 0;
extern unsigned char *dest_path;
extern GtkTextBuffer *buffer;
extern GtkTextIter iter;
void error_dialog(const char* msg)
{
	GtkWidget *dialog;
		dialog = gtk_message_dialog_new(NULL, \
            GTK_DIALOG_DESTROY_WITH_PARENT, \
            GTK_MESSAGE_ERROR, \
            GTK_BUTTONS_OK, \
            msg);
		gtk_window_set_title(GTK_WINDOW(dialog), "ERROR");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
}
boot_img_hdr *read_header(FILE *fp)
{
	if(!fp)
	{
		error_dialog("File not Opened");
		return NULL;
	}
	boot_img_hdr *bih=NULL;
	bih=(boot_img_hdr *)malloc(sizeof(boot_img_hdr));
	if(!bih)
	{
		error_dialog("Memory allocation error. Unable to allocate memory for boot image header");
		return NULL;
	}
	if(fread(bih,sizeof(boot_img_hdr),1,fp)!=1)
	{
		error_dialog("Unable to read file");
		return NULL;
	}
	char str[1024]="";
	strncpy(str,(const char *)bih->magic,8);
	if(strcmp((const char *)BOOT_MAGIC,str)!=0)
	{
		error_dialog("Bad Magic!!!\n Wrong header or incorrect file");
		// in future show hex dump on a widget
		return NULL;
	}
	return(bih);
}

void destination_path()
{
	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new ("Target Path",
                                      NULL,
                                      GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                      NULL);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *path;

		path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		dest_path=(unsigned char*)malloc(1024*sizeof(char));
		//printf("Choosen path = %s\n",path);
		strcpy((char *)dest_path,path);
		g_free (path);
	}
	
	gtk_widget_destroy (dialog);
}
void dump_file(FILE *dest, FILE *src, int nPgs)
{
	unsigned char *buf =(unsigned char*)malloc(PAGE_SIZE);
	if(!buf)
	{
		printf("Error: Unable to allocate buffer of size %d\n Exiting...\n",PAGE_SIZE);
		if(dest)
			fclose(dest);
		if(src)
			fclose(src);		
		exit(0);
	} 
	while(nPgs)
	{
		if(!fread(buf,PAGE_SIZE,1,src))
		{
			printf("Error: reading file\n");
			return;
		}
		fwrite(buf,PAGE_SIZE,1,dest);
		nPgs--;
	}
	
}
void write_splited_files(FILE *fp)
{
	// we will attempt to split the input file in to constituents
	//though header is a structure it is given one page on disk
	// so skip 1 page from the begining to get the kernel.
	rewind(fp);
	fseek(fp,PAGE_SIZE,SEEK_SET); //skipped header
	// dump kernel
	char *kpath=NULL;
	kpath=malloc(1024*sizeof(char));
	if(!kpath)
	{
		printf("Unable to allocate memory for destination path for kernel image\n");
		return;
	}
	strcpy(kpath,(const char *)dest_path);
	strcat(kpath,"/kernel_boot.img");
	FILE *kfp=fopen(kpath,"wb");
	if(!kfp)
	{
		printf("Error: Writing kernel image\n");
		free(kpath);
		return;
	}
	//calculate size in pages
	int kp = (int)((KERNEL_SIZE + PAGE_SIZE - 1) / PAGE_SIZE); // kernel pages
	int rp = (int)((RAMDISK_SIZE + PAGE_SIZE - 1) / PAGE_SIZE);// ramdisk pages
	int dp = (int)((SECOND_SIZE + PAGE_SIZE - 1) / PAGE_SIZE); //DTB pages
	dump_file(kfp,fp,kp);
	fclose(kfp);
	free(kpath);
	gtk_text_buffer_insert(buffer,&iter,"[kernel_boot.img] Written\n",-1);
	//printf("[kernel_boot.img] Written\n");
	char *rpath=malloc(1024*sizeof(char));
	if(!rpath)
	{
		printf("Unable to allocate memory for destination path for ramdisk image\n");
		return;
	}
	strcpy(rpath,(const char *)dest_path);
	strcat(rpath,"/ramdisk_boot.img");
	rewind(fp);
	fseek(fp,PAGE_SIZE*(1+kp),SEEK_SET); //skipped header+kernel
	FILE *rfp=fopen(rpath,"wb");
	if(!rfp)
	{
		printf("Error: Writing ramdisk image\n");
		free(rpath);
		return;
	}
	dump_file(rfp,fp,rp);
	fclose(rfp);
	free(rpath);
	gtk_text_buffer_insert(buffer,&iter,"[ramdisk_boot.img] Written\n",-1);
	char *dpath=malloc(1024*sizeof(char));
	if(!dpath)
	{
		printf("Unable to allocate memory for destination path for dtb image\n");
		return;
	}
	strcpy(dpath,(const char *)dest_path);
	strcat(dpath,"/dtb_boot.img"); 
	rewind(fp);
	fseek(fp,PAGE_SIZE*(1+kp+rp),SEEK_SET); //skipped header+kernel
	FILE *dfp=fopen(dpath,"wb");
	if(!dfp)
	{
		printf("Error: Writing dtb image\n");
		free(dpath);
		return;
	}
	dump_file(dfp,fp,dp);
	fclose(dfp);
	free(dpath);
	gtk_text_buffer_insert(buffer,&iter,"[dtb_boot.img] Written\n",-1);
}
