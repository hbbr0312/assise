#ifndef _FS_H_
#define _FS_H_

#include "global/global.h"
#include "global/types.h"
#include "global/defs.h"
#include "global/mem.h"
#include "global/util.h"
#include "global/ncx_slab.h"
#include "ds/uthash.h"
#include "ds/rbtree.h"
#include "filesystem/shared.h"
#include "cache_stats.h"
#include "concurrency/synchronization.h"
#include "distributed/rpc_interface.h"
#include "experimental/leases.h"

#ifdef __cplusplus
extern "C" {
#endif

// iangneal: for API init
extern mem_man_fns_t strata_mem_man;
extern callback_fns_t strata_callbacks;
extern idx_spec_t strata_idx_spec;

// libmlfs Disk layout:
// [ boot block | sb block | inode blocks | free bitmap | data blocks | log blocks ]
// [ inode block | free bitmap | data blocks | log blocks ] is a block group.
// If data blocks is full, then file system will allocate a new block group.
// Block group expension is not implemented yet.

typedef struct mlfs_kernfs_stats {
	uint64_t digest_time_tsc;
	uint64_t path_search_tsc;
    uint64_t path_search_size;
	uint64_t path_search_nr;
	uint64_t path_storage_nr;
	uint64_t path_storage_tsc;
	uint64_t replay_time_tsc;
	uint64_t apply_time_tsc;
	uint64_t digest_dir_tsc;
	uint64_t digest_inode_tsc;
	uint64_t digest_file_tsc;
	uint64_t persist_time_tsc;
	uint64_t n_digest;
	uint64_t n_digest_skipped;
	uint64_t total_migrated_mb;
	uint64_t extent_getblk_tsc; // added
#ifdef MLFS_LEASE
	uint64_t lease_rpc_local_nr;
	uint64_t lease_rpc_remote_nr;
	uint64_t lease_contention_nr;
	uint64_t lease_migration_nr;
#endif
    // block allocator
    uint64_t balloc_tsc;
    uint64_t balloc_nblk;
    uint64_t balloc_nr;
    uint64_t balloc_meta_nr;
    // undo log
    uint64_t undo_tsc;
    uint64_t undo_nr;

    // Indexing cache rates
    cache_stats_t cache_stats;
} kernfs_stats_t;

extern struct disk_superblock disk_sb[g_n_devices + 1];
extern struct super_block *sb[g_n_devices + 1];
extern kernfs_stats_t g_perf_stats;
extern uint8_t enable_perf_stats;

// Inodes per block.
#define IPB           (g_block_size_bytes / sizeof(struct dinode))

struct mlfs_range_node *mlfs_alloc_blocknode(struct super_block *sb);
struct mlfs_range_node *mlfs_alloc_inode_node(struct super_block *sb);

extern pthread_spinlock_t icache_spinlock;
extern pthread_spinlock_t dcache_spinlock;

extern struct dirent_block *dirent_hash[g_n_devices + 1];
extern struct inode *inode_hash;

static inline struct inode *icache_find(uint32_t inum)
{
	struct inode *inode;

	pthread_spin_lock(&icache_spinlock);

	HASH_FIND(hash_handle, inode_hash, &inum,
        		sizeof(uint32_t), inode);

	pthread_spin_unlock(&icache_spinlock);

	return inode;
}

static inline void init_api_idx_struct(uint8_t dev, struct inode *inode) {
    // iangneal: indexing API init.
    if (IDXAPI_IS_PER_FILE() && inode->itype == T_FILE) {
        static bool notify = false;

        if (!notify) {
            printf("Init API extent trees!!!\n");
            notify = true;
        }

        // paddr_range_t direct_extents = {
        //     .pr_start      = get_inode_block(dev, inode->inum),
        //     .pr_blk_offset = (off_t)((sizeof(struct dinode) * (inode->inum % IPB)) + 64),
        //     .pr_nbytes     = 64
        // };

        // idx_struct_t *tmp = (idx_struct_t*)mlfs_zalloc(sizeof(*inode->ext_idx));
        // int init_err;

        // switch(g_idx_choice) {
        //     case EXTENT_TREES_TOP_CACHED:
        //         g_idx_cached = true;
        //     case EXTENT_TREES:
        //         init_err = extent_tree_fns.im_init_prealloc(&strata_idx_spec,
        //                                                     &direct_extents,
        //                                                     tmp);
        //         break;
        //     case LEVEL_HASH_TABLES:
        //         init_err = levelhash_fns.im_init_prealloc(&strata_idx_spec,
        //                                                   &direct_extents,
        //                                                   tmp);
        //         break;
        //     case RADIX_TREES:
        //         init_err = radixtree_fns.im_init_prealloc(&strata_idx_spec,
        //                                                   &direct_extents,
        //                                                   tmp);
        //         break;
        //     default:
        //         panic("Invalid choice!!!\n");
        // }

        // FN(tmp, im_set_caching, tmp, g_idx_cached);

        // if (init_err) {
        //     fprintf(stderr, "Error in extent tree API init: %d\n", init_err);
        //     panic("Could not initialize API per-inode structure!\n");
        // }

        // if (tmp->idx_fns->im_set_stats) {
        //     FN(tmp, im_set_stats, tmp, enable_perf_stats);
        // }

        // inode->ext_idx = tmp;
    }
}

static inline struct inode *icache_alloc_add(uint32_t inum)
{
	struct inode *inode;

#ifdef __cplusplus
	inode = static_cast<struct inode *>(mlfs_zalloc(sizeof(*inode)));
#else
	inode = mlfs_zalloc(sizeof(*inode));
#endif

	if (!inode)
		panic("Fail to allocate inode\n");

	inode->inum = inum;
	inode->i_ref = 1;
	inode->flags = 0;
	inode->i_dirty_dblock = RB_ROOT;
	inode->_dinode = (struct dinode *)inode;

	//FIXME: LOCKO
	//pthread_rwlockattr_t rwlattr;
	//pthread_rwlockattr_setpshared(&rwlattr, PTHREAD_PROCESS_SHARED);
	//pthread_rwlock_init(&inode->de_cache_rwlock, &rwlattr);

	inode->i_sb = sb;

	pthread_mutex_init(&inode->i_mutex, NULL);

	INIT_LIST_HEAD(&inode->i_slru_head);

	pthread_spin_lock(&icache_spinlock);

	HASH_ADD(hash_handle, inode_hash, inum,
	 		sizeof(uint32_t), inode);

	pthread_spin_unlock(&icache_spinlock);

	return inode;
}

static inline struct inode *icache_add(struct inode *inode)
{
	uint32_t inum = inode->inum;

	pthread_mutex_init(&inode->i_mutex, NULL);
	
	pthread_spin_lock(&icache_spinlock);

	HASH_ADD(hash_handle, inode_hash, inum,
	 		sizeof(uint32_t), inode);

	pthread_spin_unlock(&icache_spinlock);

	return inode;
}

static inline int icache_del(struct inode *ip)
{
	pthread_spin_lock(&icache_spinlock);

	HASH_DELETE(hash_handle, inode_hash, ip);

	pthread_spin_unlock(&icache_spinlock);

	return 0;
}

//forward declaration
struct fs_stat;

//APIs
#ifdef USE_SLAB
void mlfs_slab_init(uint64_t pool_size);
#endif
void read_superblock(uint8_t dev);
void read_root_inode();
int read_ondisk_inode(uint32_t inum, struct dinode *dip);
int write_ondisk_inode(struct inode *ip);
#ifdef DISTRIBUTED
void signal_callback(struct app_context *msg);
void persist_replicated_logs(int dev, addr_t n_log_blk);
void update_remote_ondisk_inode(uint8_t node_id, struct inode *ip);
#endif
struct inode* ialloc(uint8_t, uint32_t);
struct inode* idup(struct inode*);
void cache_init(uint8_t dev);
void ilock(struct inode*);
void iput(struct inode*);
void iunlock(struct inode*);
void iunlockput(struct inode*);
void iupdate(struct inode*);
struct inode* namei(char*);
struct inode* nameiparent(char*, char*);
addr_t readi(struct inode*, char*, offset_t, addr_t);
void stati(struct inode*, struct fs_stat*);
int bmap(uint8_t mode, struct inode *ip, offset_t offset, addr_t *block_no);
void itrunc(struct inode*);
struct inode* iget(uint32_t inum);
int mlfs_mark_inode_dirty(int id, struct inode *inode);
int persist_dirty_dirent_block(struct inode *inode);
int persist_dirty_object(void);
int digest_file(uint8_t from_dev, uint8_t to_dev, int libfs_id, uint32_t file_inum, 
		offset_t offset, uint32_t length, addr_t blknr);
int reserve_log(struct peer_id *peer);
void show_storage_stats(void);

//APIs for debugging.
uint32_t dbg_get_iblkno(uint32_t inum);
void dbg_dump_inode(uint8_t dev, uint32_t inum);
void dbg_check_inode(void *data);
void dbg_check_dir(void *data);
void dbg_dump_dir(uint8_t dev, uint32_t inum);
struct inode* dbg_dir_lookup(struct inode *dir_inode,
		char *name, uint32_t *poff);
void dbg_path_walk(char *path);

#if MLFS_LEASE

#define LPB           (g_block_size_bytes / sizeof(mlfs_lease_t))

// Block containing inode i
static inline addr_t get_lease_block(uint8_t dev, uint32_t inum)
{
	return (inum / LPB) + disk_sb[dev].lease_start;
}

#endif

// Block containing inode i
static inline addr_t get_inode_block(uint8_t dev, uint32_t inum)
{
	return (inum / IPB) + disk_sb[dev].inode_start;
}

// Bitmap bits per block
#define BPB           (g_block_size_bytes*8)

// Block of free map containing bit for block b
#define BBLOCK(b, disk_sb) (b/BPB + disk_sb.bmap_start)

#ifdef __cplusplus
}
#endif

#endif
