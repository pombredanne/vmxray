/*
** fs_name
** The Sleuth Kit 
**
** Display and manipulate directory entries 
** This file contains generic functions that call the appropriate function
** depending on the file system type.
**
** Brian Carrier [carrier <at> sleuthkit [dot] org]
** Copyright (c) 2006-2008 Brian Carrier.  All Rights reserved
** Copyright (c) 2003-2005 Brian Carrier.  All rights reserved 
**
** TASK
** Copyright (c) 2002 Brian Carrier, @stake Inc.  All rights reserved
**
** TCTUTILs
** Copyright (c) 2001 Brian Carrier.  All rights reserved
**
** This software is distributed under the Common Public License 1.0
*/

/**
 * \file fs_name.c
 * Code to allocate and free the TSK_FS_NAME structures.
 */
#include "tsk_fs_i.h"

char tsk_fs_name_type_str[TSK_FS_NAME_TYPE_STR_MAX][2] =
    { "-", "p", "c", "d", "b", "r",
    "l", "s", "h", "w", "v"
};

/**
 * \internal
 * Allocate a fs_name structure */
TSK_FS_NAME *
tsk_fs_name_alloc(size_t norm_namelen, size_t shrt_namelen)
{
    TSK_FS_NAME *fs_name;
    fs_name = (TSK_FS_NAME *) tsk_malloc(sizeof(*fs_name));
    if (fs_name == NULL)
        return NULL;

    fs_name->name = (char *) tsk_malloc(norm_namelen + 1);
    if (fs_name->name == NULL) {
        free(fs_name);
        return NULL;
    }
    fs_name->name_size = norm_namelen;

    fs_name->flags = 0;

    fs_name->shrt_name_size = shrt_namelen;
    if (shrt_namelen == 0) {
        fs_name->shrt_name = NULL;
    }
    else {
        fs_name->shrt_name = (char *) tsk_malloc(shrt_namelen + 1);
        if (fs_name->shrt_name == NULL) {
            free(fs_name->name);
            free(fs_name);
            return NULL;
        }
    }

    fs_name->type = TSK_FS_NAME_TYPE_UNDEF;
    fs_name->tag = TSK_FS_NAME_TAG;
    return fs_name;
}


/**
 * \internal
 * returns 1 on error
 */
uint8_t
tsk_fs_name_realloc(TSK_FS_NAME * fs_name, size_t namelen)
{
    if ((fs_name == NULL) || (fs_name->tag != TSK_FS_NAME_TAG))
        return 1;

    if (fs_name->name_size >= namelen)
        return 0;

    fs_name->name = (char *) tsk_realloc(fs_name->name, namelen + 1);
    if (fs_name->name == NULL) {
        fs_name->name_size = 0;
        return 1;
    }

    fs_name->type = TSK_FS_NAME_TYPE_UNDEF;
    fs_name->name_size = namelen;

    return 0;
}

/**
 * \internal
 * reset the values in the TSK_FS_NAME structure
 * @param a_fs_name Name structure to reset
 */
void
tsk_fs_name_reset(TSK_FS_NAME * a_fs_name)
{
    if (a_fs_name->name)
        a_fs_name->name[0] = '\0';

    if (a_fs_name->shrt_name)
        a_fs_name->shrt_name[0] = '\0';

    a_fs_name->meta_addr = 0;
    a_fs_name->meta_seq = 0;
    a_fs_name->type = 0;
    a_fs_name->flags = 0;
}

/**
 * \internal
 */
void
tsk_fs_name_free(TSK_FS_NAME * fs_name)
{
    if ((!fs_name) || (fs_name->tag != TSK_FS_NAME_TAG))
        return;

    if (fs_name->name) {
        free(fs_name->name);
        fs_name->name = NULL;
    }
    if (fs_name->shrt_name) {
        free(fs_name->shrt_name);
        fs_name->shrt_name = NULL;
    }

    free(fs_name);
}

/** \internal
 * Copy the contents of a TSK_FS_NAME structure to another
 * structure. 
 * @param a_fs_name_to Destination structure to copy to
 * @param a_fs_name_from Source structure to copy from
 * @returns 1 on error
 */
uint8_t
tsk_fs_name_copy(TSK_FS_NAME * a_fs_name_to,
    const TSK_FS_NAME * a_fs_name_from)
{
    if ((a_fs_name_to == NULL) || (a_fs_name_from == NULL))
        return 1;

    /* If the source has a full name,  copy it */
    if (a_fs_name_from->name) {
        // make sure there is enough space
        if (strlen(a_fs_name_from->name) >= a_fs_name_to->name_size) {
            a_fs_name_to->name_size = strlen(a_fs_name_from->name) + 16;
            a_fs_name_to->name =
                (char *) tsk_realloc(a_fs_name_to->name,
                a_fs_name_to->name_size);
            if (a_fs_name_to->name == NULL)
                return 1;
        }
        strncpy(a_fs_name_to->name, a_fs_name_from->name,
            a_fs_name_to->name_size);
    }
    else {
        if (a_fs_name_to->name_size > 0)
            a_fs_name_to->name[0] = '\0';
        else
            a_fs_name_to->name = NULL;
    }

    // copy the short name, if one exists
    if (a_fs_name_from->shrt_name) {
        if (strlen(a_fs_name_from->shrt_name) >=
            a_fs_name_to->shrt_name_size) {
            a_fs_name_to->shrt_name_size =
                strlen(a_fs_name_from->shrt_name) + 16;
            a_fs_name_to->shrt_name =
                (char *) tsk_realloc(a_fs_name_to->shrt_name,
                a_fs_name_to->shrt_name_size);
            if (a_fs_name_to->shrt_name == NULL)
                return 1;
        }
        strncpy(a_fs_name_to->shrt_name, a_fs_name_from->shrt_name,
            a_fs_name_to->shrt_name_size);
    }
    else {
        if (a_fs_name_to->shrt_name_size > 0)
            a_fs_name_to->shrt_name[0] = '\0';
        else
            a_fs_name_to->shrt_name = NULL;
    }

    a_fs_name_to->meta_addr = a_fs_name_from->meta_addr;
    a_fs_name_to->meta_seq = a_fs_name_from->meta_seq;
    a_fs_name_to->type = a_fs_name_from->type;
    a_fs_name_to->flags = a_fs_name_from->flags;

    return 0;
}





/***********************************************************************
 * Printing functions
 ***********************************************************************/

/**
 * \ingroup fslib
 * Makes the "ls -l" permissions string for a file. 
 *
 * @param a_fs_meta File to be processed
 * @param a_buf [out] Buffer to write results to (must be 12 bytes or longer)
 * @param a_len Length of buffer
 */
uint8_t
tsk_fs_meta_make_ls(TSK_FS_META * a_fs_meta, char *a_buf, size_t a_len)
{
    if (a_len < 12) {
        return 1;
    }

    /* put the default values in */
    strcpy(a_buf, "----------");

    if (a_fs_meta->type < TSK_FS_META_TYPE_STR_MAX)
        a_buf[0] = tsk_fs_meta_type_str[a_fs_meta->type][0];

    /* user perms */
    if (a_fs_meta->mode & TSK_FS_META_MODE_IRUSR)
        a_buf[1] = 'r';
    if (a_fs_meta->mode & TSK_FS_META_MODE_IWUSR)
        a_buf[2] = 'w';
    /* set uid */
    if (a_fs_meta->mode & TSK_FS_META_MODE_ISUID) {
        if (a_fs_meta->mode & TSK_FS_META_MODE_IXUSR)
            a_buf[3] = 's';
        else
            a_buf[3] = 'S';
    }
    else if (a_fs_meta->mode & TSK_FS_META_MODE_IXUSR)
        a_buf[3] = 'x';

    /* group perms */
    if (a_fs_meta->mode & TSK_FS_META_MODE_IRGRP)
        a_buf[4] = 'r';
    if (a_fs_meta->mode & TSK_FS_META_MODE_IWGRP)
        a_buf[5] = 'w';
    /* set gid */
    if (a_fs_meta->mode & TSK_FS_META_MODE_ISGID) {
        if (a_fs_meta->mode & TSK_FS_META_MODE_IXGRP)
            a_buf[6] = 's';
        else
            a_buf[6] = 'S';
    }
    else if (a_fs_meta->mode & TSK_FS_META_MODE_IXGRP)
        a_buf[6] = 'x';

    /* other perms */
    if (a_fs_meta->mode & TSK_FS_META_MODE_IROTH)
        a_buf[7] = 'r';
    if (a_fs_meta->mode & TSK_FS_META_MODE_IWOTH)
        a_buf[8] = 'w';

    /* sticky bit */
    if (a_fs_meta->mode & TSK_FS_META_MODE_ISVTX) {
        if (a_fs_meta->mode & TSK_FS_META_MODE_IXOTH)
            a_buf[9] = 't';
        else
            a_buf[9] = 'T';
    }
    else if (a_fs_meta->mode & TSK_FS_META_MODE_IXOTH)
        a_buf[9] = 'x';
    return 0;
}


static void
tsk_fs_print_time(FILE * hFile, time_t time)
{
    if (time <= 0) {
        tsk_fprintf(hFile, "0000-00-00 00:00:00 (UTC)");
    }
    else {
        struct tm *tmTime = localtime(&time);

        tsk_fprintf(hFile, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d (%s)",
            (int) tmTime->tm_year + 1900,
            (int) tmTime->tm_mon + 1, (int) tmTime->tm_mday,
            tmTime->tm_hour,
            (int) tmTime->tm_min, (int) tmTime->tm_sec,
            tzname[(tmTime->tm_isdst == 0) ? 0 : 1]);
    }
}


/**
 * The only difference with this one is that the time is always
 * 00:00:00, which is applicable for the A-Time in FAT, which does
 * not have a time and if we do it normally it gets messed up because
 * of the timezone conversion
 */
static void
tsk_fs_print_day(FILE * hFile, time_t time)
{
    if (time <= 0) {
        tsk_fprintf(hFile, "0000-00-00 00:00:00 (UTC)");
    }
    else {
        struct tm *tmTime = localtime(&time);

        tsk_fprintf(hFile, "%.4d-%.2d-%.2d 00:00:00 (%s)",
            (int) tmTime->tm_year + 1900,
            (int) tmTime->tm_mon + 1, (int) tmTime->tm_mday,
            tzname[(tmTime->tm_isdst == 0) ? 0 : 1]);
    }
}


/**
* \internal
 * Simple print of dentry type / inode type, deleted, inode, and
 * name
 *
 * fs_attr is used for alternate data streams in NTFS, set to NULL
 * for all other file systems
 *
 * A newline is not printed at the end
 *
 * If path is NULL, then skip else use. it has the full directory name
 *  It needs to end with "/"
 */
void
tsk_fs_name_print(FILE * hFile, const TSK_FS_FILE * fs_file,
    const char *a_path, TSK_FS_INFO * fs, const TSK_FS_ATTR * fs_attr,
    uint8_t print_path)
{
    /* type of file - based on dentry type */
    if (fs_file->name->type < TSK_FS_NAME_TYPE_STR_MAX)
        tsk_fprintf(hFile, "%s/",
            tsk_fs_name_type_str[fs_file->name->type]);
    else
        tsk_fprintf(hFile, "-/");

    /* type of file - based on inode type: we want letters though for
     * regular files so we use the dent_str though */
    if (fs_file->meta) {
        /* 
         * An NTFS directory can have a Data stream, in which
         * case it would be printed with modes of a
         * directory, although it is really a file
         * So, to avoid confusion we will set the modes
         * to a file so it is printed that way.  The
         * entry for the directory itself will still be
         * printed as a directory
         */
        if ((fs_attr) && (fs_attr->type == TSK_FS_ATTR_TYPE_NTFS_DATA) &&
            (fs_file->meta->type == TSK_FS_META_TYPE_DIR)) {
            tsk_fprintf(hFile, "r ");
        }
        else {
            if (fs_file->meta->type < TSK_FS_META_TYPE_STR_MAX)
                tsk_fprintf(hFile, "%s ",
                    tsk_fs_meta_type_str[fs_file->meta->type]);
            else
                tsk_fprintf(hFile, "- ");
        }
    }
    else {
        tsk_fprintf(hFile, "- ");
    }


    /* print a * if it is deleted */
    if (fs_file->name->flags & TSK_FS_NAME_FLAG_UNALLOC)
        tsk_fprintf(hFile, "* ");

    tsk_fprintf(hFile, "%" PRIuINUM "", fs_file->name->meta_addr);

    /* print the id and type if we have fs_attr (NTFS) */
    if (fs_attr)
        tsk_fprintf(hFile, "-%" PRIu32 "-%" PRIu16 "", fs_attr->type,
            fs_attr->id);

    tsk_fprintf(hFile, "%s:\t",
        ((fs_file->meta) && (fs_file->meta->flags & TSK_FS_META_FLAG_ALLOC)
            && (fs_file->
                name->flags & TSK_FS_NAME_FLAG_UNALLOC)) ? "(realloc)" :
        "");

    if ((print_path) && (a_path != NULL))
        tsk_fprintf(hFile, "%s", a_path);

    tsk_fprintf(hFile, "%s", fs_file->name->name);

/*  This will add the short name in parentheses
    if (fs_file->name->shrt_name != NULL && fs_file->name->shrt_name[0] != '\0')
	tsk_fprintf(hFile, " (%s)", fs_file->name->shrt_name);
*/

    /* print the data stream name if we the non-data NTFS stream */
    if (fs_attr) {
        if (((fs_attr->type == TSK_FS_ATTR_TYPE_NTFS_DATA) &&
                (strcmp(fs_attr->name, "$Data") != 0)) ||
            ((fs_attr->type == TSK_FS_ATTR_TYPE_NTFS_IDXROOT) &&
                (strcmp(fs_attr->name, "$I30") != 0)))
            tsk_fprintf(hFile, ":%s", fs_attr->name);
    }

    return;
}

/**
 * \internal
 * Print contents of  fs_name entry format like ls -l
**
** All elements are tab delimited 
**
** If path is NULL, then skip else use. it has the full directory name
**  It needs to end with "/"
*/
void
tsk_fs_name_print_long(FILE * hFile, const TSK_FS_FILE * fs_file,
    const char *a_path, TSK_FS_INFO * fs, const TSK_FS_ATTR * fs_attr,
    uint8_t print_path)
{
    tsk_fs_name_print(hFile, fs_file, a_path, fs, fs_attr, print_path);

    if ((fs == NULL) || (fs_file->meta == NULL)) {

        tsk_fprintf(hFile, "\t");
        tsk_fs_print_time(hFile, 0);    // mtime
        tsk_fprintf(hFile, "\t");
        tsk_fs_print_time(hFile, 0);    // atime
        tsk_fprintf(hFile, "\t");
        tsk_fs_print_time(hFile, 0);    // ctime
        tsk_fprintf(hFile, "\t");
        tsk_fs_print_time(hFile, 0);    // crtime

        // size, uid, gid
        tsk_fprintf(hFile, "\t0\t0\t0\n");
    }
    else {

        /* MAC times */
        tsk_fprintf(hFile, "\t");
        tsk_fs_print_time(hFile, fs_file->meta->mtime);

        tsk_fprintf(hFile, "\t");
        /* FAT only gives the day of last access */
        if (TSK_FS_TYPE_ISFAT(fs->ftype))
            tsk_fs_print_day(hFile, fs_file->meta->atime);
        else
            tsk_fs_print_time(hFile, fs_file->meta->atime);

        tsk_fprintf(hFile, "\t");
        tsk_fs_print_time(hFile, fs_file->meta->ctime);

        tsk_fprintf(hFile, "\t");
        tsk_fs_print_time(hFile, fs_file->meta->crtime);

        /* use the stream size if one was given */
        if (fs_attr)
            tsk_fprintf(hFile, "\t%" PRIuOFF, fs_attr->size);
        else
            tsk_fprintf(hFile, "\t%" PRIuOFF, fs_file->meta->size);

        tsk_fprintf(hFile, "\t%" PRIuGID "\t%" PRIuUID "\n",
            fs_file->meta->gid, fs_file->meta->uid);
    }

    return;
}


/**
 * \internal
 *
** Print output in the format that mactime reads.
** This allows the deleted files to be inserted to get a better
** picture of what happened
**
** Prepend a_path when printing full file name
**  dir needs to end with "/" 
**
** prepend *prefix to path as the mounting point that the original
** grave-robber was run on
**
** If the flags in the fs_file->meta structure are set to FS_FLAG_ALLOC
** then it is assumed that the inode has been reallocated and the
** contents are not displayed
**
** fs is not required (only used for block size).  
*/
void
tsk_fs_name_print_mac(FILE * hFile, const TSK_FS_FILE * fs_file,
    const char *a_path, TSK_FS_INFO * fs, const TSK_FS_ATTR * fs_attr,
    const char *prefix)
{
    char ls[12];

    if ((!hFile) || (!fs_file))
        return;

    /* md5 */
    tsk_fprintf(hFile, "0|");

    /* file name */
    tsk_fprintf(hFile, "%s%s%s", prefix, a_path, fs_file->name->name);

    /* print the data stream name if it exists and is not the default NTFS */
    if ((fs_attr) && (((fs_attr->type == TSK_FS_ATTR_TYPE_NTFS_DATA) &&
                (strcmp(fs_attr->name, "$Data") != 0)) ||
            ((fs_attr->type == TSK_FS_ATTR_TYPE_NTFS_IDXROOT) &&
                (strcmp(fs_attr->name, "$I30") != 0))))
        tsk_fprintf(hFile, ":%s", fs_attr->name);

    if ((fs_file->meta)
        && (fs_file->meta->type == TSK_FS_META_TYPE_LNK)
        && (fs_file->meta->link)) {
        tsk_fprintf(hFile, " -> %s", fs_file->meta->link);
    }

    /* if filename is deleted add a comment and if the inode is now
     * allocated, then add realloc comment */
    if (fs_file->name->flags & TSK_FS_NAME_FLAG_UNALLOC)
        tsk_fprintf(hFile, " (deleted%s)", ((fs_file->meta)
                && (fs_file->
                    meta->flags & TSK_FS_META_FLAG_ALLOC)) ? "-realloc" :
            "");

    /* inode */
    tsk_fprintf(hFile, "|%" PRIuINUM, fs_file->name->meta_addr);
    if (fs_attr)
        tsk_fprintf(hFile, "-%" PRIu32 "-%" PRIu16 "", fs_attr->type,
            fs_attr->id);

    tsk_fprintf(hFile, "|");

    /* TYPE as specified in the directory entry 
     */
    if (fs_file->name->type < TSK_FS_NAME_TYPE_STR_MAX)
        tsk_fprintf(hFile, "%s/",
            tsk_fs_name_type_str[fs_file->name->type]);
    else
        tsk_fprintf(hFile, "-/");

    if (!fs_file->meta) {
        tsk_fprintf(hFile, "----------|0|0|0|0|0|0|0\n");
    }
    else {

        /* mode as string */
        tsk_fs_meta_make_ls(fs_file->meta, ls, sizeof(ls));
        tsk_fprintf(hFile, "%s|", ls);

        /* uid, gid */
        tsk_fprintf(hFile, "%" PRIuUID "|%" PRIuGID "|",
            fs_file->meta->uid, fs_file->meta->gid);

        /* size - use data stream if we have it */
        if (fs_attr)
            tsk_fprintf(hFile, "%" PRIuOFF "|", fs_attr->size);
        else
            tsk_fprintf(hFile, "%" PRIuOFF "|", fs_file->meta->size);

        /* atime, mtime, ctime, crtime */
        tsk_fprintf(hFile,
            "%" PRIu32 "|%" PRIu32 "|%" PRIu32 "|%" PRIu32 "\n",
            (uint32_t) fs_file->meta->atime,
            (uint32_t) fs_file->meta->mtime,
            (uint32_t) fs_file->meta->ctime,
            (uint32_t) fs_file->meta->crtime);
    }
}
