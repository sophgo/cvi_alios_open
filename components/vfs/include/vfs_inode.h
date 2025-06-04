/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef VFS_INODE_H
#define VFS_INODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vfs_types.h"

enum {
    VFS_TYPE_NOT_INIT,
    VFS_TYPE_CHAR_DEV,
    VFS_TYPE_BLOCK_DEV,
    VFS_TYPE_FS_DEV
};

#define INODE_IS_TYPE(node, t) ((node)->type == (t))

#define INODE_IS_CHAR(node)  INODE_IS_TYPE(node, VFS_TYPE_CHAR_DEV)
#define INODE_IS_BLOCK(node) INODE_IS_TYPE(node, VFS_TYPE_BLOCK_DEV)
#define INODE_IS_FS(node)    INODE_IS_TYPE(node, VFS_TYPE_FS_DEV)

#define INODE_GET_TYPE(node) ((node)->type)
#define INODE_SET_TYPE(node, t) do { (node)->type = (t); } while(0)

#define INODE_SET_CHAR(node)  INODE_SET_TYPE(node, VFS_TYPE_CHAR_DEV)
#define INODE_SET_BLOCK(node) INODE_SET_TYPE(node, VFS_TYPE_BLOCK_DEV)
#define INODE_SET_FS(node)    INODE_SET_TYPE(node, VFS_TYPE_FS_DEV)

/**
 * @brief Initialize inode
 *
 * @return 0 on success, negative error on failure
 *
 */
int32_t vfs_inode_init(void);

/**
 * @brief Alloc a free inode
 *
 * @return the index of inode, VFS_ERR_NOMEM on failure
 *
 */
int32_t vfs_inode_alloc(void);

/**
 * @brief Delete a inode
 *
 * @param[in] node pointer to the inode to delete
 *
 * @return 0 on success, negative error on failure
 *
 */
int32_t vfs_inode_del(vfs_inode_t *node);

/**
 * @brief Open the inode by path
 *
 * @param[in] path the path of the inode reference
 *
 * @return the pointer of the inode, NULL on failure or not found
 *
 */
vfs_inode_t *vfs_inode_open(const char *path);

/**
 * @brief Get the inode pointer by fd
 *
 * @param[in]  fd     the file descriptor
 * @param[out] p_node the pointer of the inode pointer
 *
 * @return 0 on success, negative error on failure
 *
 */
int32_t vfs_inode_ptr_get(int32_t fd, vfs_inode_t **p_node);

/**
 * @brief Get the available inode count
 *
 * @return the count of the available inodes
 *
 */
int32_t vfs_inode_avail_count(void);

/**
 * @brief Add the inode refence count
 *
 * @param[in] node the pointer of the inode
 *
 * @return none
 *
 */
void vfs_inode_ref(vfs_inode_t *node);

/**
 * @brief Dec the inode refence count
 *
 * @param[in] node the pointer of the inode
 *
 * @return none
 *
 */
void vfs_inode_unref(vfs_inode_t *node);

/**
 * @brief Check whether the inode is busy or not
 *
 * @param[in] node the pointer of the inode
 *
 * @return 1 on busy, 0 on free
 *
 */
int32_t vfs_inode_busy(vfs_inode_t *node);

/**
 * @brief Reserve an inode
 *
 * @param[in] path   the path of the inode reference
 * @param[in] p_node pointer to the inode pointer
 *
 * @return 0 on success, negative error on failure
 *
 */
int32_t vfs_inode_reserve(const char *path, vfs_inode_t **p_node);

/**
 * @brief Release an inode
 *
 * @param[in] path the path of the inode reference
 *
 * @return 0 on success, negative error on failure
 *
 */
int32_t vfs_inode_release(const char *path);

/**
 * @brief list all inode with the type
 *
 * @param[in] type the type of inode
 *
 * @return 0 on success, negative error on failure
 *
 */
int32_t vfs_inode_list(vfs_list_type_t type);

/**
 * @brief get FS nodes name
 *
 * @param[in] path the parent path of inodes
 *
 * @param[out] names FS nodes name as request
 *
 * @param[out] size  names count
 *
 * @return 0 on success, negative error on failure
 *
 */
int vfs_inode_get_names(const char *path, char names[][64], uint32_t* size);

/**
 * @brief only used by FS node to mark deatched state, so it can
 *        umount itself after it ceases to be busy.
 */
int32_t vfs_inode_detach(vfs_inode_t *node);

#define inode_init vfs_inode_init
#define inode_alloc vfs_inode_alloc
#define inode_del vfs_inode_del
#define inode_open vfs_inode_open
#define inode_ptr_get vfs_inode_ptr_get
#define inode_avail_count vfs_inode_avail_count

#ifdef __cplusplus
}
#endif

#endif /* VFS_INODE_H */
