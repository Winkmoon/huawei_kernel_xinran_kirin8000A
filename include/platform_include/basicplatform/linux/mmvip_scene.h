#ifndef __MMVIP_SCENE_H
#define __MMVIP_SCENE_H

#include <linux/swap.h>
#include <linux/page-isolation.h>
#include <linux/mmzone.h> /* for clear pcp_vip: struct per_cpu_pages */
#include <linux/gfp.h>
#include <trace/events/kmem.h>

#define MMVIP_CAM_VAL     (0xfffffff)
#define MAX_CAM_NAME_P    (4)
#define FPI_NONE          ((__force fpi_t)0)

#ifdef CONFIG_MM_VIP
#define is_migrate_vip(migratetype) unlikely((migratetype) == MIGRATE_VIP)
#define is_migrate_vip_page(_page) (get_pageblock_migratetype(_page) == MIGRATE_VIP)
extern atomic_t mmvip_page_num;
extern int mm_vip_pool_pages;
/* Defined in page_alloc.c  */
extern inline bool free_is_enough_for_vip(struct page *page);
#endif

struct cmp_name {
	const char *name;
};

bool is_mm_vip_scene(void);

/* alloc using. only used in page_alloc.c */
void set_gfp_vip_slowpath(gfp_t *gfp_mask);

/* reclaim using. only used in page_alloc.c */
void set_migrate_vip(struct page *page, int *migratetype, unsigned int order);

void add_pcppages_count(struct per_cpu_pages *pcp, int migratetype, int count);
void sub_pcppages_count(struct per_cpu_pages *pcp, int migratetype);
void free_pcppages_vip(struct page *page, struct zone *zone,
			struct per_cpu_pages *pcp, int migratetype);

static inline void add_vip_page_num(int migratetype, unsigned int order, struct page *page)
{
	if (migratetype == MIGRATE_VIP) {
		atomic_add(1 << order, &mmvip_page_num);
		SetPageVip(page);
	}
}

static inline void sub_vip_page_num(unsigned int order, struct page *page)
{
	if (PageVip(page)) {
		atomic_sub(1 << order, &mmvip_page_num);
		ClearPageVip(page);
	}
}

static inline void change_vip_page_num(int migratetype, unsigned int order, struct page *page)
{
	/* change from PageVIP to migratetype */
	if (migratetype == MIGRATE_VIP || PageVip(page)) {
		if (migratetype != MIGRATE_VIP) {  // means: migratetype != VIP && PageVip(page)
			atomic_sub(1 << order, &mmvip_page_num);
			ClearPageVip(page);
		} else if (!PageVip(page)) {  // means: migratetype == VIP && !PageVip(page)
			atomic_add(1 << order, &mmvip_page_num);
			SetPageVip(page);
		}
	}
}

static inline int get_vip_page_num()
{
	return atomic_read(&mmvip_page_num);
}

static inline void change_migrate_type_vip(struct page* page, int *migratetype)
{
	if (mm_vip_pool_pages == 0 && *migratetype == MIGRATE_VIP) {
		*migratetype = MIGRATE_MOVABLE;
		set_pageblock_migratetype(page, *migratetype);
	}
}

#endif /* __MMVIP_SCENE_H__ */
