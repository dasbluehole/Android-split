/*
** Android ROM image header
*/
#ifndef __ANDROID_IMAGE_HEADER__
#define __ANDROID_IMAGE_HEADER__
#include <stdio.h>
//constants

#define BOOT_MAGIC_SIZE  8
#define BOOT_NAME_SIZE 16
#define BOOT_ARGS_SIZE  512


//Android image header
typedef struct boot_img_hdr
{
    unsigned char magic[BOOT_MAGIC_SIZE];

    unsigned kernel_size;  /* size in bytes */
    unsigned kernel_addr;  /* physical load addr */

    unsigned ramdisk_size; /* size in bytes */
    unsigned ramdisk_addr; /* physical load addr */

    unsigned second_size;  /* size in bytes */
    unsigned second_addr;  /* physical load addr */

    unsigned tags_addr;    /* physical addr for kernel tags */
    unsigned page_size;    /* flash page size we assume */
    unsigned unused[2];    /* future expansion: should be 0 */

    unsigned char name[BOOT_NAME_SIZE]; /* asciiz product name */

    unsigned char cmdline[BOOT_ARGS_SIZE];

    unsigned id[8]; /* timestamp / checksum / sha1 / etc */
}__attribute__((__packed__))boot_img_hdr;

boot_img_hdr *read_header(FILE *fp);
void error_dialog(const char* msg);
void destination_path();
void write_splited_files(FILE *fp);
#endif //__ANDROID_IMAGE_HEADER__
