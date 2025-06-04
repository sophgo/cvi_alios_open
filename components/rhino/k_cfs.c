/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include "k_api.h"

#if (RHINO_CONFIG_SCHED_CFS > 0)
static struct k_rbtree_root_t  cfs_root = RBT_ROOT;

/*
 * Insert @key into rbtree, On success, return 0, else return -1
 */
static void node_insert(struct k_rbtree_root_t *root, cfs_node *node, lr_timer_t key)
{
    struct k_rbtree_node_t **tmp = &(root->rbt_node), *parent = NULL;

    /* Figure out where to put new node */
    while (*tmp) {
        cfs_node *my = k_rbtree_entry(*tmp, cfs_node, rbt_node);
        parent = *tmp;
        if (key <= my->key)
            tmp = &((*tmp)->rbt_left);
        else if (key > my->key)
            tmp = &((*tmp)->rbt_right);
    }

    node->key = key;

    /* Add new node and rebalance tree. */
    k_rbtree_link_node(&node->rbt_node, parent, tmp);
    k_rbtree_insert_color(&node->rbt_node, root);
}

void cfs_node_insert(cfs_node *node, lr_timer_t key)
{
    node_insert(&cfs_root, node, key);
}

void cfs_node_del(cfs_node *node)
{
    k_rbtree_erase(&node->rbt_node, &cfs_root);
}

lr_timer_t cfs_node_min_get(void)
{
    struct k_rbtree_node_t *tmp;
    cfs_node        *node_cfs;

    tmp  = k_rbtree_first(&cfs_root);
    if (tmp == NULL) {
        return 0;
    }

    node_cfs = k_rbtree_entry(tmp, cfs_node, rbt_node);
    return node_cfs->key;
}

ktask_t *cfs_preferred_task_get(void)
{
    struct k_rbtree_node_t *tmp;
    cfs_node        *node_cfs;
    ktask_t         *task;

    tmp  = k_rbtree_first(&cfs_root);
    if (tmp == NULL) {
        return 0;
    }

    node_cfs = k_rbtree_entry(tmp, cfs_node, rbt_node);
    task     = krhino_list_entry(node_cfs, ktask_t, node);

    return task;
}
#endif

