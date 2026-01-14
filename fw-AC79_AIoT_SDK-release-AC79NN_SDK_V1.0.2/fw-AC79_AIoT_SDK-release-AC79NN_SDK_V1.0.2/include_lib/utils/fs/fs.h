#ifndef __FS_H__
#define __FS_H__


#include "generic/typedef.h"
#include "generic/list.h"
#include "generic/ioctl.h"
#include "generic/atomic.h"
#include "system/task.h"
#include "system/malloc.h"
#include "system/sys_time.h"
#include "stdarg.h"
#include "fs_file_name.h"

#define SEEK_SET	0	/* Seek from beginning of file.  */
#define SEEK_CUR	1	/* Seek from current position.  */
#define SEEK_END	2	/* Seek from end of file.  */

#include "sdfile.h"

#define F_ATTR_RO       0x01
#define F_ATTR_ARC      0x02
#define F_ATTR_DIR      0x04
#define F_ATTR_VOL      0x08

#if 1//(VFS_ENABLE == 1)

#ifndef FSELECT_MODE
#define FSELECT_MODE
#define    FSEL_FIRST_FILE     			 0
#define    FSEL_LAST_FILE      			 1
#define    FSEL_NEXT_FILE      			 2
#define    FSEL_PREV_FILE      			 3
#define    FSEL_CURR_FILE      			 4
#define    FSEL_BY_NUMBER      			 5
#define    FSEL_BY_SCLUST      			 6
#define    FSEL_AUTO_FILE      			 7
#define    FSEL_NEXT_FOLDER_FILE       	 8
#define    FSEL_PREV_FOLDER_FILE         9
#define    FSEL_BY_PATH                 10
#endif

#ifndef FCYCLE_MODE
#define FCYCLE_MODE
#define    FCYCLE_LIST			0
#define    FCYCLE_ALL			1
#define    FCYCLE_ONE			2
#define    FCYCLE_FOLDER		3
#define    FCYCLE_RANDOM		4
#define    FCYCLE_MAX			5
#endif

enum {
    FS_IOCTL_GET_FILE_NUM,
    FS_IOCTL_FILE_CHECK,
    FS_IOCTL_GET_ERR_CODE,   //暂不支持
    FS_IOCTL_FREE_CACHE,
    FS_IOCTL_SET_NAME_FILTER,  //设置文件过滤
    FS_IOCTL_GET_FOLDER_INFO,  //获取文件夹序号和文件夹内文件数目
    FS_IOCTL_SET_LFN_BUF,	// 512
    FS_IOCTL_SET_LDN_BUF,	// 512

    FS_IOCTL_SET_EXT_TYPE,  //设置后缀类型
    FS_IOCTL_OPEN_DIR,    //打开目录
    FS_IOCTL_ENTER_DIR,   //进入目录
    FS_IOCTL_EXIT_DIR,    //退出
    FS_IOCTL_GET_DIR_INFO,  //获取目录信息

    FS_IOCTL_GETFILE_BYNAME_INDIR, //由歌曲名称获得歌词

    FS_IOCTL_GET_DISP_INFO, //用于长文件名获取

    FS_IOCTL_MK_DIR, //创建文件夹
    FS_IOCTL_GET_ENCFOLDER_INFO, //获取录音文件信息

    FS_IOCTL_GET_OUTFLASH_ADDR, //获取外置flash实际物理地址（暂时用于手表case,特殊fat系统）
    FS_IOCTL_FLUSH_WBUF, //刷新wbuf

    FS_IOCTL_SAVE_FAT_TABLE, //seek加速处理

    FS_IOCTL_INSERT_FILE, //插入文件
    FS_IOCTL_DIVISION_FILE, //分割文件
    FS_IOCTL_STORE_CLUST_RANG, //存储CLUST_RANG 信息
};


struct vfs_devinfo;
struct vfscan;
struct vfs_operations;



struct vfs_devinfo {
    void *fd;
    u32 sector_size;
    void *private_data;
};

#define VFS_PART_DIR_MAX 16


struct vfs_partition {
    struct vfs_partition *next;
    u32 offset;
    u32 clust_size;
    u32 total_size;
    u8 fs_attr;
    u8 fs_type;
    char dir[VFS_PART_DIR_MAX];
    void *private_data;
};

struct fiter {
    u32 index;
};

struct ffolder {
    u16 fileStart;
    u16 fileTotal;
};



struct imount {
    int fd;
    const char *path;
    const struct vfs_operations *ops;
    struct vfs_devinfo dev;
    struct vfs_partition part;
    struct list_head entry;
    atomic_t ref;
    OS_MUTEX mutex;
    u8 avaliable;
    u8 part_num;
};

struct vfs_attr {
    u8 attr;
    u32 fsize;
    u32 sclust;
    struct sys_time crt_time;
    struct sys_time wrt_time;
    struct sys_time acc_time;
};

typedef struct {
    struct imount *mt;
    struct vfs_devinfo *dev;
    struct vfs_partition *part;
    void *private_data;
} FILE;


struct vfscan {
    u8 scan_file;
    u8 subpath; //子目录，设置是否只扫描一层
    u8 scan_dir;
    u8 attr;
    u8 cycle_mode;
    char sort;
    char ftype[20 * 3 + 1];
    u16 file_number;
    u16 file_counter;

    u16  dir_totalnumber;          // 文件夹总数
    u16  musicdir_counter;          // 播放文件所在文件夹序号
    u16  fileTotalInDir;           //文件夹下的文件数目

    void *priv;
    struct vfs_devinfo *dev;
    struct vfs_partition *part;
    char  filt_dir[12];
};


struct vfs_operations {
    const char *fs_type;
    int (*mount)(struct imount *, int);
    int (*unmount)(struct imount *);
    int (*format)(struct vfs_devinfo *, struct vfs_partition *);
    int (*fset_vol)(struct vfs_partition *, const char *name);
    int (*fget_free_space)(struct vfs_devinfo *, struct vfs_partition *, u32 *space);
    int (*fopen)(FILE *, const char *path, const char *mode);
    int (*fread)(FILE *, void *buf, u32 len);
    int (*fread_fast)(FILE *, void *buf, u32 len);
    int (*fwrite)(FILE *, void *buf, u32 len);
    int (*fseek)(FILE *, int offset, int);
    int (*fseek_fast)(FILE *, int offset, int);
    int (*flen)(FILE *);
    int (*fpos)(FILE *);
    int (*fcopy)(FILE *, FILE *);
    int (*fget_name)(FILE *, u8 *name, int len);
    int (*fget_path)(FILE *, struct vfscan *, u8 *name, int len, u8 is_relative_path);
    int (*frename)(FILE *, const char *path);
    int (*fclose)(FILE *);
    int (*fdelete)(FILE *);
    int (*fscan)(struct vfscan *, const char *path, u8 max_deepth);
    int (*fscan_interrupt)(struct vfscan *, const char *path, u8 max_deepth, int (*callback)(void));
    void (*fscan_release)(struct vfscan *);
    int (*fsel)(struct vfscan *, int sel_mode, FILE *, int);
    int (*fget_attr)(FILE *, int *attr);
    int (*fset_attr)(FILE *, int attr);
    int (*fget_attrs)(FILE *, struct vfs_attr *);
    int (*fmove)(FILE *file, const char *path_dst, FILE *, int clr_attr, int path_len);
    int (*ioctl)(void *, int cmd, int arg);
    int (*fget_total_space)(struct imount *mt, u32 *space);
};


#define REGISTER_VFS_OPERATIONS(ops) \
	const struct vfs_operations ops SEC_USED(.vfs_operations)


static inline struct vfs_partition *vfs_partition_next(struct vfs_partition *p)
{
    struct vfs_partition *n = (struct vfs_partition *)zalloc(sizeof(*n));

    if (n) {
        p->next = n;
    }
    return n;
}


static inline void vfs_partition_free(struct vfs_partition *p)
{
    struct vfs_partition *n = p->next;

    while (n) {
        p = n->next;
        free(n);
        n = p;
    }
}

/**
 * @brief 挂载设备虚拟文件系统
 *
 * @param dev_name 设备名称
 * @param path 挂载点
 * @param fs_type 文件系统类型(支持"fat" "devfs" "ramfs" "sdfile")
 * @param cache_num  文件系统缓存
 * @param dev_arg 设备参数指针
 * @param reserve_spsace 文件系统后面到flash末尾保留多少空间
 *
 * @return 指向挂载点结构体的指针
 * @return NULL 挂载失败
 */
struct imount *mount(const char *dev_name, const char *path, const char *fs_type,
                     int cache_num, void *dev_arg);   //挂载

/**
 * @brief 卸载设备虚拟文件系统
 *
 * @param path 卸载路径
 *
 * @return 0: 卸载成功
 * @return other: 卸载失败
 */
int unmount(const char *path);

/**
 * @brief 格式化驱动器
 *
 * @param path 需要格式化的路径
 * @param fs_type 文件系统类型
 * @param clust_size 簇大小,簇为0时默认为卡本身簇大小
 *
 * @return 0: 格式化成功
 * @return other: 格式化失败
 */
int f_format(const char *path, const char *fs_type, u32 clust_size);

/**
 * @brief 刷新文件缓存数据（一般使用在fclose后某些数据还没有及时写进SD卡，需要主动刷新一下缓存区）
 *
 * @param path 需要刷新的路径
 *
 * @return 0: 刷新成功
 * @return other: 刷新失败
 */
int f_free_cache(const char *path);

/**
 * @brief 打开文件, 获取指向文件流的文件指针
 *
 * @param path 文件路径
 * @param mode 打开模式
 *
 * @return 指向文件流的文件指针
 * @return NULL: 打开失败
 * @note: fopen自动打开、创建文件夹和文件,打开模式只支持"r" "rb" "r+" "w" "w+"
 * 1. 设备路径+文件，其中文件传入格式:"music/test/1/2/3/pk*.wav"  "JL_REC/AC69****.wav"  "JL_REC/AC690000.wav"
 * 2. 文件名带*号，带多少个*表示多少个可变数字，最多为8+3的大小，如表示可变数字名称变为XXX0001,XXXX002这样得格式，不带*号则只创建一个文件，写覆盖。
 * 3. 文件名长度超过8个字节的需要用长文件名打开
 */
FILE *fopen(const char *path, const char *mode);

/**
 * @brief 从文件中读取数据
 *
 * @param[out] buf 保存读取到的数据
 * @param size 每次读取字节数
 * @param count 总共读取次数
 * @param file 指向文件流的文件指针
 *
 * @return 成功读取到的数据的字节长度
 */
int fread(void *buf, u32 size, u32 count, FILE *file);

/**
 * @brief 写入数据到文件中
 * @param[in] buf 需要写入的数据
 * @param size 每次写入字节数
 * @param count 总共写入次数
 * @param file 指向文件流的文件指针
 *
 * @return 成功写入的数据的字节长度
 */
int fwrite(void *buf, u32 size, u32 count, FILE *file);

/**
 * @brief 设置文件指针的位置
 *
 * @param file 指向文件流的文件指针
 * @param offset 偏移量
 * @param orig 偏移的基准位置
 *
 * @return 成功返回0
 */
int fseek(FILE *file, int offset, int orig);

/**
 * @brief 快速设置文件指针的位置
 *
 * @param file 指向文件流的文件指针
 * @param offset 偏移量
 * @param orig 偏移的基准位置
 * @note 一般手表case使用,去除互斥,设置ram里面跑
 * @return 成功返回0
 */
int fseek_fast(FILE *file, int offset, int orig);// 快速seek

/**
 * @brief 从文件中快速读取数据
 *
 * @param[out] buf 保存读取到的数据
 * @param size 每次读取字节数
 * @param count 总共读取次数
 * @param file 指向文件流的文件指针
 * @note 一般手表case使用,去除互斥,设置ram里面跑
 * @return 成功读取到的数据的字节长度
 */
int fread_fast(void *buf, u32 size, u32 count, FILE *file);//快速read

/**
 * @brief 获取文件的大小
 *
 * @param file 指向文件流的文件指针
 *
 * @return 文件大小(负值表示获取失败)
 */
int flen(FILE *file);

/**
 * @brief 获取文件指针的位置
 *
 * @param file 指向文件流的文件指针
 *
 * @return 文件指针的位置
 */
int ftell(FILE *file);

/**
 * @brief 获取文件名(不包含目录)
 *
 * @param file 指向文件流的文件指针
 * @param name 保存文件名的buffer
 * @param len buffer的长度
 *
 * @return 文件名的长度(大于0)
 */
int fget_name(FILE *file, u8 *name, int len);

/**
 * @brief 重命名文件
 *
 * @param file 指向文件流的文件指针
 * @param path 文件的新文件名，不需要加目录路径
 *
 * @return 1: 成功
 * @return other: 失败
 */
int frename(FILE *file, const char *fname);

/**
 * @brief 关闭文件流
 *
 * @param file 指向文件流的文件指针
 *
 * @return 0: 关闭成功
 */
int fclose(FILE *file);

/**
 * @brief 删除文件,操作成功后会自动关闭文件
 *
 * @param file 指向文件流的文件指针
 *
 * @return 0: 删除成功
 * @return other: 删除失败
 */
int fdelete(FILE *file);

/**
 * @brief 根据文件路径删除文件
 *
 * @param fname 文件路径
 *
 * @return 0: 删除成功
 * @return other: 删除失败
 */
int fdelete_by_name(const char *fname);

/**
 * @brief 获取剩余空间
 *
 * @param path 设备路径
 * @param space 保存剩余空间的大小
 *
 * @return 0: 获取成功
 * @return other: 获取失败
 */
int fget_free_space(const char *path, u32 *space);

/**
 * @brief 获取存储设备的物理总空间(注意是物理大小，并不是分区的逻辑空间大小)
 *
 * @param path 根目录
 * @param space 保存总空间的大小
 *
 * @return 0: 获取成功
 * @return other: 获取失败
 */
int fget_physical_total_space(const char *path, u32 *space);

/**
 * @brief 获取当前文件相对路径和绝对路径
 *
 * @param file 指向文件流的文件指针
 * @param fscan 文件扫描结构体句柄
 * @param name 保存文件路径的buffer
 * @param len buffer的长度
 * @param is_relative_path 是否要获取相对路径
 *
 * @return 文件名的长度(大于0)
 */
int fget_path(FILE *file, struct vfscan *fscan, u8 *name, int len, u8 is_relative_path);

/**
 * @brief 文件扫描
 *
 * @param path 扫描路径
 * @param arg 扫描参数设置
 * @param max_deepth 扫描目录层数
 *
 * @return 虚拟文件扫描结构体句柄
 * @return NULL: 扫描失败
 * @note arg:
 * @note -t  文件类型
 * @note -r  包含子目录
 * @note -d  扫描文件夹
 * @note -a  文件属性 r: 读， /: 非
 * @note -s  排序方式， t:按时间排序， n:按文件号排序
 */
struct vfscan *fscan(const char *path, const char *arg, u8 max_deepth); //扫描接口，参数配置如上。

/**
 * @brief 可打断的文件扫描
 *
 * @param path 扫描路径
 * @param arg 扫描参数设置
 * @param max_deepth 扫描目录层数，最大为9
 * @param callback 每扫描完一个目录后回调一次
 *
 * @return 虚拟文件扫描结构体句柄
 * @return NULL: 扫描失败
 * @note arg:
 * @note -t  文件类型
 * @note -r  包含子目录
 * @note -d  扫描文件夹
 * @note -a  文件属性 r: 读， /: 非
 * @note -s  排序方式， t:按时间排序， n:按文件号排序
 */
struct vfscan *fscan_interrupt(const char *path, const char *arg, u8 max_deepth, int (*callback)(void));

/**
 * @brief 文件扫描(进入指定子目录，只扫此目录下文件信息)
 *
 * @param fs 文件扫描结构体句柄
 * @param path 指定目录的相对路径
 *
 * @return 虚拟文件扫描结构体句柄
 * @return NULL: 扫描失败
 */
struct vfscan *fscan_enterdir(struct vfscan *fs, const char *path);

/**
 * @brief 返回上一层的文件扫描目录
 *
 * @param fs 文件扫描结构体句柄
 *
 * @return 虚拟文件扫描结构体句柄
 * @return NULL: 扫描失败
 */
struct vfscan *fscan_exitdir(struct vfscan *fs);

/**
 * @brief 释放文件扫描结构体句柄
 *
 * @param fs 文件扫描结构体句柄
 */
void fscan_release(struct vfscan *fs);

/**
 * @brief 选择文件
 *
 * @param fs 文件扫描结构体句柄
 * @param selt_mode 选择模式(支持按簇号、序号、路径选择)
 * @param arg 选择参数,默认0
 *
 * @return 指向文件流的文件指针
 * @return NULL: 选择操作失败
 */
FILE *fselect(struct vfscan *fs, int selt_mode, int arg); //选择指定文件

/**
 * @brief 检查目录是否存在
 *
 * @param dir 目录路径
 *
 * @return 1: 存在
 * @return 0: 不存在
 */
int fdir_exist(const char *dir); //check 目录是否存在

/**
 * @brief 获取文件的文件属性
 *
 * @param file 指向文件流的文件指针
 * @param attr 保存操作成功后文件的文件属性信息
 *
 * @return 0: 获取成功
 * @return other: 获取失败
 */
int fget_attr(FILE *file, int *attr); //获取文件属性

/**
 * @brief 设置文件的文件属性
 *
 * @param file 指向文件流的文件指针
 * @param attr 文件属性码
 *
 * @return 0: 设置成功
 * @return other: 设置失败
 */
int fset_attr(FILE *file, int attr); //设置文件属性

/**
 * @brief 获取文件的详细信息
 *
 * @param file 指向文件流的文件指针
 * @param attr 保存操作成功后文件的详细信息
 *
 * @return 0: 获取成功
 * @return other: 获取失败
 */
int fget_attrs(FILE *file, struct vfs_attr *attr);//获得文件相关信息如属性、簇号、大小等

/**
 * @brief 获取路径对应的分区
 *
 * @param path 分区路径
 *
 * @return 指向分区结构体的指针
 * @return NULL: 获取失败
 */
struct vfs_partition *fget_partition(const char *path);//获得分区part

/**
 * @brief 设置路径对应的卷标的名称
 *
 * @param path 卷标路径
 * @param name 卷标名称
 *
 * @return 0: 设置成功
 * @return other: 设置失败
 */
int fset_vol(const char *path, const char *name);//设置卷标

/**
 * @brief 移动文件
 *
 * @param file 指向文件流的文件指针
 * @param path_dst 目标路径，注意如果不需要更改原文件名，路径不需要填写文件名，如/aa/即可，根目录前面不能加分区路径
 * @param newFile 保存操作成功后文件在新路径的文件流指针
 * @param clr_attr 是否清除原有文件的文件属性
 * @param path_len 目标路径长度
 *
 * @return 0: 移动成功(自动关闭旧文件流)
 * @return other: 移动失败
 */
int fmove(FILE *file, const char *path_dst, FILE **newFile, int clr_attr, int path_len);

/**
 * @brief 检查文件
 *
 * @param file 指向文件流的文件指针
 *
 * @return 文件错误码
 */
int fcheck(FILE *file);//暂不支持

/**
 * @brief 获取文件系统的错误代码
 *
 * @param path 文件路径
 *
 * @return 文件错误码
 */
int fget_err_code(const char *path); //暂不支持

/**
 * @brief 拼接文件名(目录+文件名) (特别用于中文名文件的路径拼接)
 *
 * @param result 保存操作成功后的文件路径的字符串
 * @param path 目录路径
 * @param fname 文件名
 * @param len fname的长度
 * @param is_dir 拼接的是否为文件夹
 * @param is_dir 拼接的是否为长文件名
 *
 * @return 文件名的长度(大于0)
 */
int fname_to_path(char *result, const char *path, const char *fname, int len, u8 is_dir, u8 is_lfn);

int fset_name_filter(const char *path, void *name_filter);//暂不支持

/* --------------------------------------------------------------------------*/
/**
* @brief 获取文件夹信息
*
* @param fs vfscan 结构句柄
* @param arg 文件夹信息结构句柄
*
* @return 无意义
*/
/* ----------------------------------------------------------------------------*/
int fget_folder(struct vfscan *fs, struct ffolder *arg); //获取文件夹序号和文件夹内文件数目

/* --------------------------------------------------------------------------*/
/**
 * @brief  设置长文件名Buf (现已不常用)
 *
 * @param fs vfscan 句柄
 * @param arg buf
 *
 * @return 0成功， 非0错误。
 */
/* ----------------------------------------------------------------------------*/
int fset_lfn_buf(struct vfscan *fs, void *arg);//设置长文件名buf

/* --------------------------------------------------------------------------*/
/**
 * @brief  设置长文件夹名Buf (现已不常用)
 *
 * @param fs vfscan 句柄
 * @param arg buf
 *
 * @return 0成功， 非0错误。
 */
/* ----------------------------------------------------------------------------*/
int fset_ldn_buf(struct vfscan *fs, void *arg);//设置长文件夹名buf

/* --------------------------------------------------------------------------*/
/**
 * @brief  设置后缀名过滤（现已不常用）
 *
 * @param path 根路径
 * @param ext_type 后缀名
 *
 * @return 0成功， 非0错误。
 */
/* ----------------------------------------------------------------------------*/
int fset_ext_type(const char *path, void *ext_type);//设置后缀类型

/* --------------------------------------------------------------------------*/
/**
 * @brief  文件浏览使用，打开目录
 *
 * @param path 路径
 * @param pp_file 文件句柄
 * @param dir_dj 目录信息句柄
 *
 * @return 无意义
 */
/* ----------------------------------------------------------------------------*/
int fopen_dir_info(const char *path, FILE **pp_file, void *dir_dj); //打开目录

/* --------------------------------------------------------------------------*/
/**
 * @brief  文件浏览使用，进入目录
 *
 * @param file 文件句柄
 * @param dir_dj 目录信息句柄
 *
 * @return  目录项总数
 */
/* ----------------------------------------------------------------------------*/
int fenter_dir_info(FILE *file, void *dir_dj); //进入目录

/* --------------------------------------------------------------------------*/
/**
 * @brief  文件浏览使用，退出目录
 *
 * @param file  文件句柄
 *
 * @return 目录项总数
 */
/* ----------------------------------------------------------------------------*/
int fexit_dir_info(FILE *file); //退出

/* --------------------------------------------------------------------------*/
/**
 * @brief 文件浏览使用，获取目录信息
 *
 * @param file 文件句柄
 * @param start_num 起始位置
 * @param total_num 获取目录个数
 * @param buf_info 目录信息句柄
 *
 * @return  获取的目录数
 */
/* ----------------------------------------------------------------------------*/
int fget_dir_info(FILE *file, u32 start_num, u32 total_num, void *buf_info); ////获取目录信息

/* --------------------------------------------------------------------------*/
/**
 * @brief 存储文件簇信息
 *
 * @param file 文件句柄
 *
 * @note 一般手表case使用, 用于fget_fat_outflash_addr()之前调用，节省fget_fat_outflash_addr()时间
 * @return 0成功， 非0错误。
 */
/* ----------------------------------------------------------------------------*/
int fstore_clust_rang(FILE *file);

/* --------------------------------------------------------------------------*/
/**
 * @brief 获取外置flash实际物理地址
 *
 * @param file 文件句柄
 * @param name sdfile格式文件名
 * @param buf_info 存储相关信息结构buf指针
 * @param buf_len buf 长度
 *
 * @note 一般手表case使用
 * @return  0表示buf不够 大于 0 表示存储多少个信息结构，其他 错误
 */
/* ----------------------------------------------------------------------------*/
int fget_fat_outflash_addr(FILE *file, char *name, void *buf_info, int buf_len); //获取外置flash实际物理地址

/* --------------------------------------------------------------------------*/
/**
 * @brief  文件浏览使用，由歌曲名称获取歌词
 *
 * @param file 歌曲文件句柄
 * @param newFile 歌词文件句柄
 * @param ext_name 后缀名称
 *
 * @return 0成功， 非0错误。
 */
/* ----------------------------------------------------------------------------*/
int fget_file_byname_indir(FILE *file, FILE **newFile, void *ext_name); //由歌曲名称获得歌词

/* --------------------------------------------------------------------------*/
/**
 * @brief  获取长文件名和长文件夹名信息（现在不常使用）
 *
 * @param file 歌曲文件句柄
 * @param arg 长文件相关信息结构指针
 *
 * @note 需要先设置长文件名或者长文件夹名buf进去
 * @return 0成功， 非0错误。
 */
/* ----------------------------------------------------------------------------*/
int fget_disp_info(FILE *file, void *arg); //用于长文件名获取

/* --------------------------------------------------------------------------*/
/**
 * @brief  创建目录
 *
 * @param path 路径
 * @param folder 文件夹名称,不需要 /
 * @param mode 目录属性（1 设置为隐藏属性， 0 不设置 ）
 *
 * @return   0成功，非0不成功
 */
/* ----------------------------------------------------------------------------*/
int fmk_dir(const char *path, char *folder, u8 mode); //创建目录

/* --------------------------------------------------------------------------*/
/**
 * @brief 获取录音文件信息(现在不常用)
 *
 * @param path 路径
 * @param folder 文件夹名称
 * @param ext 文件名后缀
 * @param last_num 可变数字最大数字
 * @param total_num 文件总数
 *
 * @return 0成功，非0不成功
 */
/* ----------------------------------------------------------------------------*/
int fget_encfolder_info(const char *path, char *folder, char *ext, u32 *last_num, u32 *total_num); //获取录音文件信息

/* --------------------------------------------------------------------------*/
/**
 * @brief  截取path中根目录之后的文件名
 *
 * @param path 路径
 *
 * @return 文件名
 */
/* ----------------------------------------------------------------------------*/
const char *fname_after_root(const char *path); // 截取path中根目录之后的文件名

/* --------------------------------------------------------------------------*/
/**
 * @brief 获取文件系统类型
 *
 * @param path 路径
 *
 * @return  文件系统类型
 */
/* ----------------------------------------------------------------------------*/
const char *fget_fs_type(const char *path);	// 获取文件系统类型

/* --------------------------------------------------------------------------*/
/**
 * @brief 判断文件路径是否带有unicode编码
 *
 * @param path 路径
 * @param len 路径长度
 *
 * @return 是否带unicode
 */
/* ----------------------------------------------------------------------------*/
int fget_name_type(char *path, int len); //判断是不是unicode码

/* --------------------------------------------------------------------------*/
/**
 * @brief  录音获取最后序号
 *
 * @return 最后序号
 */
/* ----------------------------------------------------------------------------*/
int get_last_num(void); //录音获取最后序号。

/* --------------------------------------------------------------------------*/
/**
 * @brief 设置断点参数
 *
 * @param clust 记录的簇号
 * @param fsize 记录的文件大小
 * @param flag 文件是否存在标志
 *
 * @note 1.接口调用在扫描前
 *       2.使用完需要put_bp_info对应释放buf
 */
/* ----------------------------------------------------------------------------*/
void set_bp_info(u32 clust, u32 fsize, u32 *flag); //扫描前设置断点参数，需要put_bp_info对应释放buf.
/* --------------------------------------------------------------------------*/
/**
 * @brief  释放内存
 */
/* ----------------------------------------------------------------------------*/
void put_bp_info(void);

/* --------------------------------------------------------------------------*/
/**
 * @brief 优化扫盘速度
 *
 * @param enable 开关
 * @note 1.目的是是否去除获取文件夹内所有文件功能，默认enable 是1 获取数目，置0不获取，所以不需要切换文件夹操作的功能，可置0 关闭
 *       2.在扫描前调用接口
 */
/* ----------------------------------------------------------------------------*/
void ff_set_FileInDir_enable(u8 enable); // 优化文件打开速度，如果不需要切换文件夹的操作，可置0关闭

/* --------------------------------------------------------------------------*/
/**
 * @brief 设置目录项基点信息（用于加速）
 *
 * @param buf 存储基点buf (长度 10 * n)
 * @param n 基点数目
 * @note 1.加速序号选择文件，明显效果体现在上一曲加速
 *       2.注意buf使用
 */
/* ----------------------------------------------------------------------------*/
void ff_set_DirBaseInfo(void *buf, u16 n);

/* --------------------------------------------------------------------------*/
/**
 * @brief 设置文件的创建时间
 *
 * @param year 年
 * @param month 月
 * @param day 日
 * @param hour 小时
 * @param minute 分钟
 * @param second 秒
 */
/* ----------------------------------------------------------------------------*/
void fat_set_datetime_info(u16 year, u8 month, u8 day, u8 hour, u8 minute, u8 second);

/* --------------------------------------------------------------------------*/
/**
 * @brief 隐藏属性文件是否过滤
 *
 * @param flag 置1 为过滤
 */
/* ----------------------------------------------------------------------------*/
void hidden_file(u8 flag);

/* --------------------------------------------------------------------------*/
/**
 * @brief 是否保存预申请长度
 *
 * @param enable 1保存，0 不保存
 */
/* ----------------------------------------------------------------------------*/
void fat_save_already_size_enable(char enable);

/* --------------------------------------------------------------------------*/
/**
 * @brief 设置预申请簇数目
 *
 * @param num 数目（1-0x1fe）
 */
/* ----------------------------------------------------------------------------*/
void fat_set_pre_create_chain_num(u16 num);

/* --------------------------------------------------------------------------*/
/**
 * @brief 存储文件簇信息
 *
 * @param file 文件句柄
 * @param btr  buf 长度
 * @param buf  buf指针
 *
 * @note seek加速，4字节对齐
 * @return 0成功，非0不成功
 */
/* ----------------------------------------------------------------------------*/
int fsave_fat_table(FILE *file, u16 btr, u8 *buf);

/* --------------------------------------------------------------------------*/
/**
 * @brief  刷新文件系统缓存buf
 *
 * @param path 设备路径
 *
 * @return 0成功，非0不成功
 */
/* ----------------------------------------------------------------------------*/
int f_flush_wbuf(const char *path);

/* --------------------------------------------------------------------------*/
/**
 * @brief 插入文件
 *
 * @param file 源文件
 * @param i_file 需要插入的文件
 * @param fptr 源文件被插入的位置
 *
 * @return 0成功，非0不成功
 */
/* ----------------------------------------------------------------------------*/
int finsert_file(FILE *file, FILE *i_file, u32 fptr);

/* --------------------------------------------------------------------------*/
/**
 * @brief  分割文件
 *
 * @param file 源文件
 * @param file_name 分割后第二个文件文件名
 * @param fptr 源文件被分割位置
 *
 * @return 0成功，非0不成功
 */
/* ----------------------------------------------------------------------------*/
int fdicvision_file(FILE *file, char *file_name, u32 fptr);

#endif  /* VFS_ENABLE */

#endif  /* __FS_H__ */

