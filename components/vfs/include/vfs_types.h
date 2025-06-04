/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef VFS_TYPES_H
#define VFS_TYPES_H

#include <stdint.h>
#include <time.h>

typedef struct {
    time_t actime;  /* time of last access */
    time_t modtime; /* time of last modification */
} vfs_utimbuf_t;

typedef struct {
    uint16_t st_mode;    /* mode of file */
    uint32_t st_size;    /* bytes of file */
    time_t   st_actime;  /* time of last access */
    time_t   st_modtime; /* time of last modification */
} vfs_stat_t;

typedef struct {
    int32_t d_ino;    /* file number */
    uint8_t d_type;   /* type of file */
    char    d_name[]; /* file name */
} vfs_dirent_t;

typedef struct {
    int32_t dd_vfs_fd;
    int32_t dd_rsv;
} vfs_dir_t;

typedef struct {
    int32_t f_type;    /* fs type */
    int32_t f_bsize;   /* optimized transport block size */
    int32_t f_blocks;  /* total blocks */
    int32_t f_bfree;   /* available blocks */
    int32_t f_bavail;  /* number of blocks that non-super users can acquire */
    int32_t f_files;   /* total number of file nodes */
    int32_t f_ffree;   /* available file nodes */
    int32_t f_fsid;    /* fs id */
    int32_t f_namelen; /* max file name length */
} vfs_statfs_t;

typedef void (*vfs_poll_notify_t)(void *fds, void *arg);

typedef struct vfs_file_ops       vfs_file_ops_t;
typedef struct vfs_filesystem_ops vfs_filesystem_ops_t;

union vfs_inode_ops_t {
    const vfs_file_ops_t       *i_ops;
    const vfs_filesystem_ops_t *i_fops;
};

typedef enum {
    VFS_INODE_VALID,    /* node is available*/
    VFS_INODE_INVALID,  /* Node is ready to be deleted, unavailable*/
    VFS_INODE_DETACHED, /* Node is ready to be deleted, and Wait for the operation to complete */
    VFS_INODE_MAX
} vfs_inode_status_t;

typedef struct {
    union vfs_inode_ops_t ops; /* inode operations */

    void    *i_arg;   /* per inode private data */
    char    *i_name;  /* name of inode */
    int32_t  i_flags; /* flags for inode */
    uint8_t  type;    /* type for inode */
    uint8_t  refs;    /* refs for inode */
    void     *lock;   /* lock for inode operations */
    vfs_inode_status_t status; /* valid or invalid status for this inode */
} vfs_inode_t;

typedef struct {
    vfs_inode_t *node;   /* node for file or device */
    void        *f_arg;  /* arguments for file or device */
    uint32_t     offset; /* offset of the file */
#ifdef CONFIG_VFS_LSOPEN
    char         filename[VFS_CONFIG_PATH_MAX];
#endif
    int32_t      redirect_fd; /* the target FD that it's redirected to, optionally used. */
} vfs_file_t;

typedef enum {
    VFS_LIST_TYPE_FS,
    VFS_LIST_TYPE_DEVICE
} vfs_list_type_t;

struct vfs_file_ops {
    int32_t (*open)  (vfs_inode_t *node, vfs_file_t *fp);
    int32_t (*close) (vfs_file_t *fp);
    int32_t (*read)  (vfs_file_t *fp, void *buf, uint32_t nbytes);
    int32_t (*write) (vfs_file_t *fp, const void *buf, uint32_t nbytes);
    int32_t (*ioctl) (vfs_file_t *fp, int32_t cmd, uint32_t arg);
    int32_t (*poll)  (vfs_file_t *fp, int32_t flag, vfs_poll_notify_t notify, void *fds, void *arg);
    uint32_t (*lseek) (vfs_file_t *fp, int64_t off, int32_t whence);
#ifdef AOS_PROCESS_SUPPORT
    void*   (*mmap)(vfs_file_t *fp, void *addr, size_t length, int prot, int flags,
                    int fd, off_t offset);
#endif

};

struct vfs_filesystem_ops {
    int32_t       (*open)      (vfs_file_t *fp, const char *path, int32_t flags);
    int32_t       (*close)     (vfs_file_t *fp);
    int32_t       (*read)      (vfs_file_t *fp, char *buf, uint32_t len);
    int32_t       (*write)     (vfs_file_t *fp, const char *buf, uint32_t len);
    uint32_t      (*lseek)     (vfs_file_t *fp, int64_t off, int32_t whence);
    int32_t       (*sync)      (vfs_file_t *fp);
    int32_t       (*stat)      (vfs_file_t *fp, const char *path, vfs_stat_t *st);
    int32_t       (*fstat)     (vfs_file_t *fp, vfs_stat_t *st);
    int32_t       (*link)      (vfs_file_t *fp, const char *path1, const char *path2);
    int32_t       (*unlink)    (vfs_file_t *fp, const char *path);
    int32_t       (*remove)    (vfs_file_t *fp, const char *path);
    int32_t       (*rename)    (vfs_file_t *fp, const char *oldpath, const char *newpath);
    vfs_dir_t    *(*opendir)   (vfs_file_t *fp, const char *path);
    vfs_dirent_t *(*readdir)   (vfs_file_t *fp, vfs_dir_t *dir);
    int32_t       (*closedir)  (vfs_file_t *fp, vfs_dir_t *dir);
    int32_t       (*mkdir)     (vfs_file_t *fp, const char *path);
    int32_t       (*rmdir)     (vfs_file_t *fp, const char *path);
    void          (*rewinddir) (vfs_file_t *fp, vfs_dir_t *dir);
    int32_t       (*telldir)   (vfs_file_t *fp, vfs_dir_t *dir);
    void          (*seekdir)   (vfs_file_t *fp, vfs_dir_t *dir, int32_t loc);
    int32_t       (*ioctl)     (vfs_file_t *fp, int32_t cmd, uint32_t arg);
    int32_t       (*statfs)    (vfs_file_t *fp, const char *path, vfs_statfs_t *suf);
    int32_t       (*access)    (vfs_file_t *fp, const char *path, int32_t amode);
    int32_t       (*pathconf)  (vfs_file_t *fp, const char *path, int name);
    int32_t       (*fpathconf) (vfs_file_t *fp, int name);
    int32_t       (*utime)     (vfs_file_t *fp, const char *path, const vfs_utimbuf_t *times);
    int32_t       (*truncate)  (vfs_file_t *fp, int64_t length);
};

#if VFS_CONFIG_DEBUG > 0
#define VFS_ERROR printf
#else
#define VFS_ERROR(x, ...)   do {  } while (0)
#endif

#endif /* VFS_TYPES_H */

