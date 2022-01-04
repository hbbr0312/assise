#include "global/global.h"
#include "api/file_indexing.h"

indexing_choice_t g_idx_choice;
bool g_idx_cached;
bool g_idx_has_parallel_lookup;

/*
uint	g_n_devices 		= 5;
uint	g_root_dev		= 1;
uint	g_max_open_files	= 100;
uint	g_block_size_bytes	= 4096;
uint	g_max_blocks_per_operation	 = 800;
uint	g_log_size_blks		= 4000;
uint	g_fs_size_blks		= (512UL * (1 << 10));
uint	g_max_extent_size_blks		 = (1 * (1 << 10));
uint	g_block_reservation_size_blks = (1 * (1 << 10));
uint	g_fd_start 			= 1000000;
*/



void mlfs_setup(void)
{
	//open_file_table = malloc (sizeof(struct file) * g_max_open_files);


}


indexing_choice_t get_indexing_choice(void) {
	// return NONE;
    return HASHFS;
    // const char *env = getenv("MLFS_IDX_STRUCT");

    // if (env != NULL && !strcmp(env, "EXTENT_TREES")) {
    //     printf("%s -> using API per-file extent trees!\n", env);
    //     return EXTENT_TREES;
    // } else if (env != NULL && !strcmp(env, "EXTENT_TREES_TOP_CACHED")) {
    //     printf("%s -> using API per-file extent trees (with top-level caching)!\n", env);
    //     return EXTENT_TREES_TOP_CACHED;
    // } else if (env != NULL && !strcmp(env, "GLOBAL_HASH_TABLE")) {
    //     printf("%s -> using API global hash table!\n", env);
    //     return GLOBAL_HASH_TABLE;
    // } else if (env != NULL && !strcmp(env, "HASHFS")) {
    //     printf("%s -> using API hash fs!\n", env);
    //     return HASHFS;
    // } else if (env != NULL && !strcmp(env, "GLOBAL_HASH_TABLE_COMPACT")) {
    //     printf("%s -> using API global hash table! (compact)\n", env);
    //     if(setenv("IDX_COMPACT", "1", 1)) {
    //         panic("could not make compact!");
    //     }
    //     return GLOBAL_HASH_TABLE;
    // } else if (env != NULL && !strcmp(env, "GLOBAL_CUCKOO_HASH")) {
    //     printf("%s -> using API global CUCKOO hash table!\n", env);
    //     return GLOBAL_CUCKOO_HASH;
    // } else if (env != NULL && !strcmp(env, "GLOBAL_CUCKOO_HASH_COMPACT")) {
    //     printf("%s -> using API global CUCKOO hash table! (compact)\n", env);
    //     if(setenv("IDX_COMPACT", "1", 1)) {
    //         panic("could not make compact!");
    //     }
    //     return GLOBAL_CUCKOO_HASH;
    // } else if (env != NULL && !strcmp(env, "LEVEL_HASH_TABLES")) {
    //     printf("%s -> using API per-file level hashing!\n", env);
    //     return LEVEL_HASH_TABLES;
    // } else if (env != NULL && !strcmp(env, "RADIX_TREES")) {
    //     printf("%s -> using API per-file radix trees!\n", env);
    //     return RADIX_TREES;
    // } else if (env == NULL || !strcmp(env, "") || !strcmp(env, "NONE")){
    //     printf("%s -> using Strata default indexing!\n", env);
    // } else {
    //     panic("Unrecognized indexing structure!");
    // }

    // return NONE;
}

bool get_indexing_is_cached(void) {
	return false;
    // const char *env = getenv("MLFS_IDX_CACHE");

    // if (!env || g_idx_choice != EXTENT_TREES_TOP_CACHED) {
    //     printf("MLFS_IDX_CACHE not set -> disabling caches by default!\n");
    //     return false;
    // }

    // if (g_idx_choice == EXTENT_TREES_TOP_CACHED ||
    //     !strcmp(env, "1") ||
    //     !strcmp(env, "TRUE") || !strcmp(env, "true") ||
    //     !strcmp(env, "YES") || !strcmp(env, "yes")) {
    //     printf("%s -> using API indexing caching!\n", 
    //             env ? env : getenv("MLFS_IDX_STRUCT"));
    //     return true;
    // } 
    
    // printf("%s -> disabling caches!\n", env);
    // return false;
}

// void print_global_idx_stats(bool enable_perf_stats) {
//     if (!enable_perf_stats) return;

//     idx_fns_t *fns;
//     switch(g_idx_choice) {
//         case EXTENT_TREES:
//         case EXTENT_TREES_TOP_CACHED:
//             fns = &extent_tree_fns;
//             break;
//         case RADIX_TREES:
//             fns = &radixtree_fns;
//             break;
//         case LEVEL_HASH_TABLES:
//             fns = &levelhash_fns;
//             break;
//         case GLOBAL_HASH_TABLE:
//             fns = &hash_fns;
//             break;
//         case GLOBAL_CUCKOO_HASH:
//             fns = &cuckoohash_fns;
//             break;
//         default:
//             fprintf(stderr, "global stats method not found\n");
//             return;
//     }

//     if (fns->im_print_global_stats) {
//         fns->im_print_global_stats();
//     }

//     if (fns->im_clean_global_stats) {
//         fns->im_clean_global_stats();
//     } else {
//         fprintf(stderr, "(no clean method for global stats)\n");
//     }
// }

bool get_idx_has_parallel_lookup() {
    // idx_fns_t *fns;
    // switch(g_idx_choice) {
    //     case EXTENT_TREES:
    //     case EXTENT_TREES_TOP_CACHED:
    //         fns = &extent_tree_fns;
    //         break;
    //     case RADIX_TREES:
    //         fns = &radixtree_fns;
    //         break;
    //     case LEVEL_HASH_TABLES:
    //         fns = &levelhash_fns;
    //         break;
    //     case GLOBAL_HASH_TABLE:
    //         fns = &hash_fns;
    //         break;
    //     case GLOBAL_CUCKOO_HASH:
    //         fns = &cuckoohash_fns;
    //         break;
    //     default:
    //         return false;
    // }

    // if (fns->im_lookup_parallel) return true;

    return false;
}

// void add_idx_stats_to_json(bool enable_perf_stats, json_object *root) {
//     if (!enable_perf_stats) return;

//     idx_fns_t *fns;
//     switch(g_idx_choice) {
//         case EXTENT_TREES:
//         case EXTENT_TREES_TOP_CACHED:
//             fns = &extent_tree_fns;
//             break;
//         case RADIX_TREES:
//             fns = &radixtree_fns;
//             break;
//         case LEVEL_HASH_TABLES:
//             fns = &levelhash_fns;
//             break;
//         case GLOBAL_HASH_TABLE:
//             fns = &hash_fns;
//             break;
//         case GLOBAL_CUCKOO_HASH:
//             fns = &cuckoohash_fns;
//             break;
//         default:
//             printf("(no print fn available)\n");
//             return;
//     }

//     if (fns->im_add_global_to_json) {
//         fns->im_add_global_to_json(root);
//     }
// }
