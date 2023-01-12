函数原型在Linux内核中：include/linux/of.h

1、通过节点名字查找指定的节点

struct device_node *of_find_node_by_name(struct device_node *from, const char *name);
from：开始查找的节点，如果为 NULL 表示从根节点开始查找整个设备树。

type：要查找的节点名字

返回值：找到的节点，如果为 NULL 表示查找失败

2、通过 device_type 属性查找指定的节点

struct device_node *of_find_node_by_type(struct device_node *from, const char *type)
from：开始查找的节点，如果为 NULL 表示从根节点开始查找整个设备树。

type：要查找的节点对应的 type 字符串，也就是 device_type 属性值。

返回值：找到的节点，如果为 NULL 表示查找失败。

3、根据 device_type 和 compatible 这两个属性查找指定的节点

struct device_node *of_find_compatible_node(struct device_node *from, const char *type, const char *compatible)
from：开始查找的节点，如果为 NULL 表示从根节点开始查找整个设备树。

type：要查找的节点对应的 type 字符串，也就是 device_type 属性值，可以为 NULL，表示 忽略掉 device_type 属性。

compatible：要查找的节点所对应的 compatible 属性列表。

返回值：找到的节点，如果为 NULL 表示查找失败

4、通过 of_device_id 匹配表来查找指定的节点

struct device_node *of_find_matching_node_and_match(struct device_node *from, const struct of_device_id *matches, const struct of_device_id **match)
from：开始查找的节点，如果为 NULL 表示从根节点开始查找整个设备树。

matches：of_device_id 匹配表，也就是在此匹配表里面查找节点。

match：找到的匹配的 of_device_id。

返回值：找到的节点，如果为 NULL 表示查找失败

5、通过路径来查找指定的节点

inline struct device_node *of_find_node_by_path(const char *path)
path:带有全路径的节点名，可以使用节点的别名，比如“/led”就是 led 这个 节点的全路径,意思是根节点下的子节点led。

返回值：找到的节点

查找父子节点：

6、获取指定节点的父节点(如果有父节点的话) (儿子找爸爸)

struct device_node *of_get_parent(const struct device_node *node)
node:要查找的父节点的节点。

返回值：找到的父节点

7、用迭代的方式查找子节点(爸爸找儿子)

struct device_node *of_get_next_child(const struct device_node *node, struct device_node *prev)
node:父节点

prev:前一个子节点，也就是从哪一个子节点开始迭代的查找下一个子节点。可以设置为NULL，表示从第一个子节点开始。

提取属性值的of函数：

节点的属性信息里面保存了驱动所需要的内容，因此对于属性值的提取非常重要，Linux 内核中使用结构体 property 表示属性，此结构体同样定义在文件 include/linux/of.h 中，内容如下：

struct property {
 
    char *name;  //属性名
 
    int length;   //属性长度
 
    void *value;   //属性值
 
    struct property *next;  //下一个属性
 
    unsigned long _flags;
 
    unsigned int unique_id;
 
    struct bin_attribute attr;
 
};

8、查找指定的属性

property *of_find_property(const struct device_node *np, const char *name, int *lenp)
np:设备节点

name:属性名

lenp：属性值的字节数，也就是长度

返回值:找到的属性

9、获取属性中元素的数量，比如 reg 属性值是一个数组，那么使用此函数可以获取到这个数组的大小。

int of_property_count_elems_of_size(const struct device_node *np, const char *propname,int elem_size)
np:设备节点

propname：需要统计元素数量的属性名字。

elem_size: 元素长度

10、从属性中获取指定标号的 u32 类型数据值，比如某个属性有多个 u32 类型的值，那么就可以使用此函数来获取指定标号的数据值

int of_property_read_u32_index(const struct device_node *np, const char *propname, u32 index, u32 *out_value)
Np：设备节点。

propname： 要读取的属性名字。

index：要读取的值标号。

Out_value：读取到的值

返回值：0 读取成功，负值，读取失败，-EINVAL 表示属性不存在，-ENODATA 表示没有要读取的数据，-EOVERFLOW 表示属性值列表太小。

11、这 4 个函数分别是读取属性中 u8、u16、u32 和 u64 类型的数组数据，比如大多数的 reg 属性都是数组数据，可以使用这 4 个函数一次读取出 reg 属性中的所有数据。

int of_property_read_u8_array(const struct device_node *np,const char *propname, u8 *out_values, size_t sz);
 
 
 
int of_property_read_u16_array(const struct device_node *np,const char *propname, u16 *out_values, size_t sz);
 
 
 
int of_property_read_u32_array(const struct device_node *np,const char *propname,u32 *out_values,size_t sz);
 
 
 
int of_property_read_u64_array(const struct device_node *np,const char *propname,u64 *out_values,size_t sz);
np：设备节点。

propname： 要读取的属性名字。

Out_values：读取到的数组值，分别为 u8、u16、u32 和 u64。

sz：要读取的数组元素数量。

返回值：0，读取成功，负值，读取失败，-EINVAL 表示属性不存在，-ENODATA 表示没

有要读取的数据，-EOVERFLOW 表示属性值列表太小。

12、有些属性只有一个整形值，这四个函数就是用于读取这种只有一个整形值的属性，分别用于读取 u8、u16、u32 和 u64 类型属性值

int of_property_read_u8(const struct device_node *np,const char *propname, u8 *out_values);
 
 
 
int of_property_read_u16(const struct device_node *np,const char *propname, u16 *out_values);
 
 
 
int of_property_read_u32(const struct device_node *np,const char *propname, u32 *out_values);
np：设备节点。

propname： 要读取的属性名字。

Out_values：读取到的值，分别为 u8、u16、u32

返回值：0，读取成功，负值，读取失败，-EINVAL 表示属性不存在，-ENODATA 表示没有要读取的数据，-EOVERFLOW 表示属性值列表太小。

13、读取属性中字符串值

int of_property_read_string(struct device_node *np, const char *propname, const char **out_string)
np：设备节点。

propname： 要读取的属性名字。

string：读取到的字符串值。

返回值：0，读取成功，负值，读取失败。

14、获取#address-cells 属性值

int of_n_addr_cells(struct device_node *np)
np：设备节点。

返回值：获取到的#address-cells 属性值。

15、获取#size-cells 属性值

int of_n_size_cells(struct device_node *np)
np：设备节点。

返回值：获取到的#size-cells 属性值。

16、查看节点的 compatible 属性是否有包含 compat 指定的字符串，也就是检查设备节点的兼容性。

int of_device_is_compatible(const struct device_node *device, const char *compat)
device：设备节点。

compat：要查看的字符串。

返回值：0，节点的 compatible 属性中包含 compat 指定的字符串；其他值，节点的 compatible 属性中不包含 compat 指定的字符串。

17、获取地址相关属性，主要是“reg”或者“assigned-addresses”属性值

const __be32 *of_get_address(struct device_node *dev, int index, u64 *size, unsigned int *flags)
device_node：设备节点。

index：要读取的地址标号。

size：地址长度。

flags：参数，比如 IORESOURCE_IO、IORESOURCE_MEM 等

返回值：读取到的地址数据首地址，为 NULL 的话表示读取失败。

18、将从设备树读取到的地址转换为物理地址

u64 of_translate_address(struct device_node *dev, const __be32 *in_addr)
device_node：设备节点。

in_addr：要转换的地址。

返回值：得到的物理地址，如果为 OF_BAD_ADDR 的话表示转换失败。








