
#ifndef NFTL_CONF_H
#define NFTL_CONF_H

#define NFTL_ERASED_VALUE 0xff
#define NFTL_SPARE_AREA_LEN (16 * 4)
#define NFTL_TAG_LEN (8)
#define NFTL_TAG_OFFSET (16)
#define NFTL_ENABLE_ECC (true)

/* The page size, in bytes, of the flash used for NFTL storage. */
#ifndef CONFIG_NFTL_PAGE_SIZE
#define NFTL_PAGE_SIZE (2 * 1024)
#else
#define NFTL_PAGE_SIZE CONFIG_NFTL_PAGE_SIZE
#endif

/* page number per block */
#ifndef CONFIG_NFTL_PAGE_PER_BLOCK_NUMS
#define NFTL_PAGE_PER_BLOCK_NUMS 64
#else
#define NFTL_PAGE_PER_BLOCK_NUMS NFTL_CONFIG_PAGE_PER_BLOCK_NUMS
#endif

/* nftl logical partition numbers.
 * TODO: modify below lines if partition numbers changed.
 */
#define NFTL_LOGICAL_PARTITION_NUM 1

/*
 * convert physical block nums to raw block nums,
 * reserve 5% space for bad block(buffer blocks),
 * physical blocks include rawpages+tagpages+bufferblocks
 * raw block nums aligned to 32
 */
#define NFTL_BLOCK_NUMS_PHY2RAW(phy_num) \
    (((phy_num - (phy_num * 5 / 100)) * (NFTL_PAGE_PER_BLOCK_NUMS - 1) / NFTL_PAGE_PER_BLOCK_NUMS) & 0xffffe0)

/* ***************************************************************
 * TODO: Modify below lines if filesystem's partition size changed.
 * Below lines must be the same as nftl user's settings,
 * e.g littefs system/data size (in blocks).
 * ***************************************************************
 */
#define NFTL_PART0_PHY_BLOCK_NUMS (1024)
#define NFTL_PART1_PHY_BLOCK_NUMS (1024)
#define NFTL_PART2_PHY_BLOCK_NUMS (2048)

#define NFTL_PART0_RAW_BLOCK_NUMS \
    NFTL_BLOCK_NUMS_PHY2RAW(NFTL_PART0_PHY_BLOCK_NUMS)
#define NFTL_PART1_RAW_BLOCK_NUMS \
    NFTL_BLOCK_NUMS_PHY2RAW(NFTL_PART1_PHY_BLOCK_NUMS)
#define NFTL_PART2_RAW_BLOCK_NUMS \
    NFTL_BLOCK_NUMS_PHY2RAW(NFTL_PART2_PHY_BLOCK_NUMS)
#define NFTL_TOTAL_RAW_BLOCK_NUMS                            \
    (NFTL_PART0_RAW_BLOCK_NUMS + NFTL_PART1_RAW_BLOCK_NUMS + \
     NFTL_PART2_RAW_BLOCK_NUMS)

/* nftl logical partition blocks */
/* 3 parts: tagpages + rawpages + paddingpages
 * tagpages = physical blocks,
 * rawpages = NFTL_PART_RAW_BLOCK_NUMS * NFTL_PAGE_PER_BLOCK_NUMS
 * paddingpages = (NFTL_PAGE_PER_BLOCK_NUMS-1)
 */
#define NFTL_BLOCK_NUMS_RAW2PHY(raw_num)                                                                                                          \
    ((raw_num * NFTL_PAGE_PER_BLOCK_NUMS + (NFTL_PAGE_PER_BLOCK_NUMS - 2) / (NFTL_PAGE_PER_BLOCK_NUMS - 1) + raw_num * NFTL_PAGE_PER_BLOCK_NUMS + \
      (NFTL_PAGE_PER_BLOCK_NUMS - 1)) /                                                                                                           \
     NFTL_PAGE_PER_BLOCK_NUMS)

/* nftl logical partition 0 blocks, i.e. after 'raw + tagpage' convertion. */
/* 3 parts: tagpages + rawpages + paddingpages
 * tagpages = physical blocks,
 * rawpages = NFTL_PART0_RAW_BLOCK_NUMS * NFTL_PAGE_PER_BLOCK_NUMS
 * paddingpages = (NFTL_PAGE_PER_BLOCK_NUMS-1)
 */
#define NFTL_PART0_BLOCK_NUMS \
    NFTL_BLOCK_NUMS_RAW2PHY(NFTL_PART0_RAW_BLOCK_NUMS)

/* buffer blocks in nftl partition, used for bad block replacement.
 * It's proposed to be 5% used for bad block.
 */
#define NFTL_PART0_BUFFER_BLOCK_NUMS \
    (NFTL_PART0_PHY_BLOCK_NUMS - NFTL_PART0_BLOCK_NUMS) //((NFTL_PART0_BLOCK_NUMS >> 3) >= 2 ? (NFTL_PART0_BLOCK_NUMS >> 3): 2)

/* nftl logical partition 1 blocks, i.e. after 'raw + tagpage' convertion. */
#define NFTL_PART1_BLOCK_NUMS \
    NFTL_BLOCK_NUMS_RAW2PHY(NFTL_PART1_RAW_BLOCK_NUMS)

#define NFTL_PART1_BUFFER_BLOCK_NUMS \
    (NFTL_PART1_PHY_BLOCK_NUMS - NFTL_PART1_BLOCK_NUMS) //((NFTL_PART1_BLOCK_NUMS >> 3) >= 2 ? (NFTL_PART1_BLOCK_NUMS >> 3): 2)

/* nftl logical partition 2 blocks, i.e. after 'raw + tagpage' convertion. */
#define NFTL_PART2_BLOCK_NUMS \
    NFTL_BLOCK_NUMS_RAW2PHY(NFTL_PART2_RAW_BLOCK_NUMS)

#define NFTL_PART2_BUFFER_BLOCK_NUMS \
    (NFTL_PART2_PHY_BLOCK_NUMS - NFTL_PART2_BLOCK_NUMS) //((NFTL_PART2_BLOCK_NUMS >> 3) >= 2 ? (NFTL_PART2_BLOCK_NUMS >> 3): 2)

/* TODO: modify below line if more logical partition added */
#if NFTL_LOGICAL_PARTITION_NUM == 1
#define NFTL_LOGICAL_TOTAL_BLOCK_NUMS (NFTL_PART0_BLOCK_NUMS)
#define NFTL_BUFFER_TOTAL_BLOCK_NUMS (NFTL_PART0_BUFFER_BLOCK_NUMS)
#elif NFTL_LOGICAL_PARTITION_NUM == 2
#define NFTL_LOGICAL_TOTAL_BLOCK_NUMS (NFTL_PART0_BLOCK_NUMS + NFTL_PART1_BLOCK_NUMS)
#define NFTL_BUFFER_TOTAL_BLOCK_NUMS (NFTL_PART0_BUFFER_BLOCK_NUMS + NFTL_PART1_BUFFER_BLOCK_NUMS)
#elif NFTL_LOGICAL_PARTITION_NUM == 3
#define NFTL_LOGICAL_TOTAL_BLOCK_NUMS (NFTL_PART0_BLOCK_NUMS + NFTL_PART1_BLOCK_NUMS + NFTL_PART2_BLOCK_NUMS)
#define NFTL_BUFFER_TOTAL_BLOCK_NUMS (NFTL_PART0_BUFFER_BLOCK_NUMS + NFTL_PART1_BUFFER_BLOCK_NUMS + NFTL_PART2_BUFFER_BLOCK_NUMS)
#elif
#error Please define more NFTL logical paritions (now 3 but need more)!
#endif

/* ===== physical related definitions ===== */

/* total physical block numbers in FTL partition. */
#define NFTL_WORK_BLOCK_NUMS (NFTL_LOGICAL_TOTAL_BLOCK_NUMS + NFTL_BUFFER_TOTAL_BLOCK_NUMS)

#define NFTL_BLOCK_NUMS NFTL_WORK_BLOCK_NUMS

/* The block size, in bytes, of the flash used for NFTL storage. */
#define NFTL_BLOCK_SIZE (NFTL_PAGE_PER_BLOCK_NUMS * NFTL_PAGE_SIZE)

/* The total nftl storage size. */
#define NFTL_TOTAL_SIZE (NFTL_BLOCK_NUMS * NFTL_BLOCK_SIZE)

#endif /* NFTL_CONF_H */
