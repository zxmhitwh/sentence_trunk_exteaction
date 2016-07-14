/**
* @file darts.h  双数组数据结构
*       该结构联合了DAWG因此最终生成的双数组较小
*       darts < ref A compact static double-array keeping character codes >
*       DAWG  < ref http://www.wutka.com/dawg.html >
* 
* @date     
*/

#ifdef WIN32
#define  USING_WIN32_FIRST
#endif

#ifndef _PECKER_DARTS2_H
#define _PECKER_DARTS2_H

#include <cstdio>
#include <iostream>
#include <exception>
#include <new>

using namespace std;

#define DARTS_INT_TO_STR(value)     #value
#define DARTS_LINE_TO_STR(line)     DARTS_INT_TO_STR(line)
#define DARTS_LINE_STR              DARTS_LINE_TO_STR(__LINE__)
#define DARTS_THROW(msg)            throw Darts2::Details::Exception( \
    __FILE__ ":" DARTS_LINE_STR ": exception: " msg)


namespace Darts2 
{

    namespace Details 
    {

        /** 基本类型 */
        typedef char          char_type;
        typedef unsigned char uchar_type;
        typedef int           value_type;
        typedef unsigned int  id_type;


        /**
        * @brief 定义双数组存储单元base.check
        */
        class DoubleArrayUnit
        {
        public:
            /**
            * @brief 双数组存储单元
            */
            DoubleArrayUnit() : unit_()
            {
            }

            /**
            * @brief 判断当前节点是否有叶子节点
            */
            bool has_leaf() const
            { 
                return ((unit_ >> 8) & 1) == 1; 
            }

            /**
            * @brief 获取当前双数组单元值
            */
            value_type value() const
            { 
                return static_cast<value_type>(unit_ & ((1U << 31) - 1)); 
            }

            /**
            * @brief 获取check值
            */
            id_type label() const
            { 
                return unit_ & ((1U << 31) | 0xFF);
            }

            /**
            * @brief 获取base值
            */
            id_type offset() const
            { 
                return (unit_ >> 10) << ((unit_ & (1U << 9)) >> 6);
            }

        private:
            id_type unit_;
        };


        /**
        * @brief 出错管理类
        */
        class Exception : public std::exception
        {
        public:
            /**
            * @brief 显式定义构造函数防止隐式调用该函数
            *
            * @param msg 出错信息
            */
            explicit Exception(const char *msg = NULL) throw() : msg_(msg)
            {
            }

            /**
            * @brief 拷贝构造函数
            *
            * @param rhs 出错对象
            */
            Exception(const Exception &rhs) throw() : msg_(rhs.msg_) 
            {
            }

            /**
            * @brief 析构函数
            */
            virtual ~Exception() throw() 
            {
            }

            virtual const char *what() const throw()
            {
                return (msg_ != NULL) ? msg_ : ""; 
            }

        private:
            const char *msg_;

            /**
            * @brief 在私有区间显式定义对象赋值函数
            *        防止该函数被调用
            */
            Exception &operator=(const Exception &);
        };
    } // namespace Details


    /**
    * @brief 双数组实现类
    */
    template <typename, typename, typename T, typename>
    class DoubleArrayImpl
    {
    public:
        /** 属性类型                        */
        typedef T                  value_type;
        /** 键类型                          */
        typedef Details::char_type key_type;
        /** 结果类型                        */
        typedef value_type         result_type;

        /** 结果对类型 */
        struct result_pair_type
        {
            /** 词条属性 */
            value_type  value;
            /** 词条长度 */
            std::size_t length;
        };

        /**
        * @brief 构造函数
        *        初始化成员变量
        */
        DoubleArrayImpl() : size_(0), array_(NULL), buf_(NULL) 
        {
        }

        /**
        * @brief 析构函数
        *        回收内存
        */
        virtual ~DoubleArrayImpl() 
        { 
            clear(); 
        }

        /**
        * @brief 设定属性值
        *
        * @param result 保存属性地址
        * @param value  属性值
        */
        inline void set_result(value_type *result, value_type value, std::size_t) const;

        /**
        * @brief 设定词条属性长度信息
        *
        * @param result 词条属性长度结构体
        * @param value  词条属性
        * @param length 词条长度
        */
        inline void set_result(result_pair_type *result, value_type value, std::size_t length) const;

        /**
        * @brief 设定双数组array
        * 
        * @param ptr  新的array地址
        * @param size array单元数
        */
        void set_array(const void *ptr, std::size_t size = 0);

        /**
        * @brief 返回双数组array
        *
        * @return 双数组array
        */
        const void *array() const { return array_; }

        /**
        * @brief 释放双数组占用的内存
        */
        void clear();

        /**
        * @brief 返回双数组单元大小
        */
        std::size_t unit_size() const 
        { 
            return sizeof(unit_type);
        }

        /**
        * @brief 返回双数组单元数
        */
        std::size_t size() const 
        { 
            return size_; 
        }

        /**
        * @brief 返回双数组总的内存消耗
        */
        std::size_t total_size() const
        { 
            return unit_size() * size(); 
        }

        /**
        * @brief 返回双数单元数
        */
        std::size_t nonzero_size() const 
        { 
            return size(); 
        }

        /**
        * @brief 构建双数组
        *
        * @param num_keys  词条数
        * @param keys      词条数组
        * @param lengths   词条长度数组
        * @param values    词条属性数组
        * @param progress_func 构建进度打印函数
		// build() constructs a dictionary from given key-value pairs. If `lengths'
		// is NULL, `keys' is handled as an array of zero-terminated strings. If
		// `values' is NULL, the index in `keys' is associated with each key, i.e.
		// the ith key has (i - 1) as its value.
		// Note that the key-value pairs must be arranged in key order and the values
		// must not be negative. Also, if there are duplicate keys, only the first
		// pair will be stored in the resultant dictionary.
		// `progress_func' is a pointer to a callback function. If it is not NULL,
		// it will be called in build() so that the caller can check the progress of
		// dictionary construction. For details, please see the definition of
		// <Darts::Details::progress_func_type>.
		// The return value of build() is 0, and it indicates the success of the
		// operation. Otherwise, build() throws a <Darts::Exception>, which is a
		// derived class of <std::exception>.
		// build() uses another construction algorithm if `values' is not NULL. In
		// this case, Darts-clone uses a Directed Acyclic Word Graph (DAWG) instead
		// of a trie because a DAWG is likely to be more compact than a trie.

        */
        int build(std::size_t num_keys, const key_type * const *keys,
            const std::size_t *lengths = NULL, const value_type *values = NULL,
            int (*progress_func)(std::size_t, std::size_t) = NULL);

        /**
        * @brief 加载双数组
        *
        * @param file_name 双数组存储文件
        * @param mode      打开方式
        * @param offset    文件偏移位置
        * @param size      双数组大小
        *
        * @return 成功返回0，否则返回-1
		// open() reads an array of units from the specified file. And if it goes
		// well, the old array will be freed and replaced with the new array read
		// from the file. `offset' specifies the number of bytes to be skipped before
		// reading an array. `size' specifies the number of bytes to be read from the
		// file. If the `size' is 0, the whole file will be read.
		// open() returns 0 iff the operation succeeds. Otherwise, it returns a
		// non-zero value or throws a <Darts::Exception>. The exception is thrown
		// when and only when a memory allocation fails.

        */
        int open(const char *file_name, const char *mode = "rb",
            std::size_t offset = 0, std::size_t size = 0);

        /**
        * @brief 保存双数组
        *
        * @param file_name 双数组存储文件
        * @param mode      文件打开方式
        * @param offset    保存偏移位置
		// save() writes the array of units into the specified file. `offset'
		// specifies the number of bytes to be skipped before writing the array.
		// open() returns 0 iff the operation succeeds. Otherwise, it returns a
		// non-zero value.

        */
        int save(const char *file_name, const char *mode = "wb",
            std::size_t offset = 0) const;

        /**
        * @brief 确定词条匹配
        * 
        * @param key    待匹配词条
        * @param length 词条长度
        * @param node_pos 在双数组中匹配开始位置
        *
        * @return 词条对应属性
        */

		// The 1st exactMatchSearch() tests whether the given key exists or not, and
		// if it exists, its value and length are set to `result'. Otherwise, the
		// value and the length of `result' are set to -1 and 0 respectively.
		// Note that if `length' is 0, `key' is handled as a zero-terminated string.
		// `node_pos' specifies the start position of matching. This argument enables
		// the combination of exactMatchSearch() and traverse(). For example, if you
		// want to test "xyzA", "xyzBC", and "xyzDE", you can use traverse() to get
		// the node position corresponding to "xyz" and then you can use
		// exactMatchSearch() to test "A", "BC", and "DE" from that position.
		// Note that the length of `result' indicates the length from the `node_pos'.
		// In the above example, the lengths are { 1, 2, 2 }, not { 4, 5, 5 }.
		template <class U>
		void exactMatchSearch(const key_type *key, U &result,
			std::size_t length = 0, std::size_t node_pos = 0) const {
				result = exactMatchSearch<U>(key, length, node_pos);
		}
		// The 2nd exactMatchSearch() returns a result instead of updating the 2nd
		// argument. So, the following exactMatchSearch() has only 3 arguments.
		template <class U>
		inline U exactMatchSearch(const key_type *key, std::size_t length = 0,
			std::size_t node_pos = 0) const;

		// commonPrefixSearch() searches for keys which match a prefix of the given
		// string. If `length' is 0, `key' is handled as a zero-terminated string.
		// The values and the lengths of at most `max_num_results' matched keys are
		// stored in `results'. commonPrefixSearch() returns the number of matched
		// keys. Note that the return value can be larger than `max_num_results' if
		// there are more than `max_num_results' matches. If you want to get all the
		// results, allocate more spaces and call commonPrefixSearch() again.
		// `node_pos' works as well as in exactMatchSearch().

		inline size_t exactMatchSearch(const char* sKey)
		{

			result_pair_type resultTemp;
			exactMatchSearch(sKey, resultTemp);
			return resultTemp.value;

		}
		/**
		* @brief 公共前缀词条匹配
		* 
		* @param key               待匹配字符串
		* @param results           保存匹配到的所有词条buffer
		* @param max_num_results   buffer大小
		* @param length            字符串长度
		* @param node_pos          在双数组中匹配开始位置
		*
		* @return 匹配到的词条数
		*/
		template <class U>
		inline std::size_t commonPrefixSearch(const key_type *key, U *results,
			std::size_t max_num_results, std::size_t length = 0,
			std::size_t node_pos = 0) const;



		/**
		* @brief 遍历匹配一个字符串
		*
		* @param key      待匹配字符串
		* @param node_pos 传入匹配开始保存匹配结束位置
        * @param key_pos  字符串匹配开始位置
        * @param length   待匹配长度
        *
        * @return 匹配到词条返回该词条对应的属性
        *         匹配成功字符串返回：        -1
        *         未匹配成功字符串返回：      -2

		// In Darts-clone, a dictionary is a deterministic finite-state automaton
		// (DFA) and traverse() tests transitions on the DFA. The initial state is
		// `node_pos' and traverse() chooses transitions labeled key[key_pos],
		// key[key_pos + 1], ... in order. If there is not a transition labeled
		// key[key_pos + i], traverse() terminates the transitions at that state and
		// returns -2. Otherwise, traverse() ends without a termination and returns
		// -1 or a nonnegative value, -1 indicates that the final state was not an
		// accept state. When a nonnegative value is returned, it is the value
		// associated with the final accept state. That is, traverse() returns the
		// value associated with the given key if it exists. Note that traverse()
		// updates `node_pos' and `key_pos' after each transition.

        */
        value_type traverse(const key_type *key, std::size_t &node_pos,
            std::size_t &key_pos, std::size_t length = 0) const;

    private:
        /** 字符类型                              */
        typedef Details::uchar_type      uchar_type;
        /** 词条id                               */
        typedef Details::id_type         id_type;
        /** 双数组单元类型                       */
        typedef Details::DoubleArrayUnit unit_type;

        /** 双数组存储单元数                    */
        std::size_t                      size_;
        /** 指向双数组存储单元                  */
        const unit_type*                 array_;
        /** 保存双数组buffer */
        unit_type*                       buf_;

        /**
        *@brief 显式定义拷贝构造函数和赋值拷贝构造函数防止被调用
        */
        DoubleArrayImpl(const DoubleArrayImpl &);
        DoubleArrayImpl &operator=(const DoubleArrayImpl &);
    };

    /** 双数组类型, DoubleArray 是一个典型的双数组用法，它使用int作为值类型，适合大多数情况*/
    typedef DoubleArrayImpl<void, void, int, void> DoubleArray;


//上述是接口定义部分，下面是实现部分
////////////////////////////////////////////////////////////////


    template <typename A, typename B, typename T, typename C>
    inline void DoubleArrayImpl<A, B, T, C>::set_result(value_type *result,
        value_type value, std::size_t) const
    {
        *result = value;
    }

    template <typename A, typename B, typename T, typename C>
    inline void DoubleArrayImpl<A, B, T, C>::set_result(result_pair_type *result,
        value_type value, std::size_t length) const
    {
        result->value = value;
        result->length = length;
    }

    template <typename A, typename B, typename T, typename C>
    void DoubleArrayImpl<A, B, T, C>::set_array(const void *ptr,
        std::size_t size)
    {
        clear();

        array_ = static_cast<const unit_type *>(ptr);
        size_ = size;
    }

    template <typename A, typename B, typename T, typename C>
    void DoubleArrayImpl<A, B, T, C>::clear()
    {
        size_ = 0;
        array_ = NULL;
        if (buf_ != NULL)
        {
            delete[] buf_;
            buf_ = NULL;
        }
    }

    template <typename A, typename B, typename T, typename C>
    int DoubleArrayImpl<A, B, T, C>::open(const char *file_name,
        const char *mode, std::size_t offset, std::size_t size)
    {
#ifdef _MSC_VER
        std::FILE *file;
        if (::fopen_s(&file, file_name, mode) != 0)
            return -1;
#else
        std::FILE *file = std::fopen(file_name, mode);
        if (file == NULL)
            return -1;
#endif

        if (size == 0)
        {
            if (std::fseek(file, 0, SEEK_END) != 0)
            {
                std::fclose(file);
                return -1;
            }
            size = std::ftell(file) - offset;
        }

        if (std::fseek(file, offset, SEEK_SET) != 0)
        {
            std::fclose(file);
            return -1;
        }

        size /= unit_size();
        unit_type *buf;
        try
        {
            buf = new unit_type[size];
        }
        catch (const std::bad_alloc &)
        {
            std::fclose(file);
            DARTS_THROW("failed to open double-array: std::bad_alloc");
        }

        if (std::fread(buf, unit_size(), size, file) != size)
        {
            std::fclose(file);
            delete[] buf;
            return -1;
        }
        std::fclose(file);
        clear();
        size_ = size;
        array_ = buf;
        buf_ = buf;

        return 0;
    }

    template <typename A, typename B, typename T, typename C>
    int DoubleArrayImpl<A, B, T, C>::save(const char *file_name,
        const char *mode, std::size_t offset) const
    {
        if (size() == 0)
            return -1;

#ifdef _MSC_VER
        std::FILE *file;
        if (::fopen_s(&file, file_name, mode) != 0 || fseek(file, offset, SEEK_SET) != 0)
            return -1;
#else
        std::FILE *file = std::fopen(file_name, mode);
        if (file == NULL || fseek(file, offset, SEEK_SET) != 0)
            return -1;
#endif
        
        if (std::fwrite(array_, unit_size(), size(), file) != size())
        {
            std::fclose(file);
            return -1;
        }

        std::fclose(file);
        return 0;
    }

	template <typename A, typename B, typename T, typename C>
	template <typename U>
	inline U DoubleArrayImpl<A, B, T, C>::exactMatchSearch(const key_type *key,
		std::size_t length, std::size_t node_pos) const
	{
		U result;
		set_result(&result, static_cast<value_type>(-1), 0);

		unit_type unit = array_[node_pos];
		if (length != 0)
		{
			for (std::size_t i = 0; i < length; ++i)
			{
				node_pos ^= unit.offset() ^ static_cast<uchar_type>(key[i]);
				unit = array_[node_pos];
				if (unit.label() != static_cast<uchar_type>(key[i]))
				{
                    return result;
                }
            }
        }
        else
        {
            for ( ; key[length] != '\0'; ++length)
            {
                node_pos ^= unit.offset() ^ static_cast<uchar_type>(key[length]);
                unit = array_[node_pos];
                if (unit.label() != static_cast<uchar_type>(key[length]))
                {
                    return result;
                }
            }
        }

        if (!unit.has_leaf())
        {
            return result;
        }

        unit = array_[node_pos ^ unit.offset()];
        set_result(&result, static_cast<value_type>(unit.value()), length);

        return result;
    }

	template <typename A, typename B, typename T, typename C>
	template <typename U>
	inline std::size_t DoubleArrayImpl<A, B, T, C>::commonPrefixSearch(
		const key_type *key, U *results, std::size_t max_num_results,
		std::size_t length, std::size_t node_pos) const
	{
		std::size_t num_results = 0;

		unit_type unit = array_[node_pos];
		node_pos ^= unit.offset();
		if (length != 0)
		{
			for (std::size_t i = 0; i < length; ++i)
			{
				node_pos ^= static_cast<uchar_type>(key[i]);
                unit = array_[node_pos];
                if (unit.label() != static_cast<uchar_type>(key[i]))
                {
                    return num_results;
                }

                node_pos ^= unit.offset();
                if (unit.has_leaf())
                {
                    if (num_results < max_num_results)
                    {
                        set_result(&results[num_results], static_cast<value_type>(
                            array_[node_pos].value()), i + 1);
                    }
                    ++num_results;
                }
            }
        }
        else
        {
            for ( ; key[length] != '\0'; ++length)
            {
                node_pos ^= static_cast<uchar_type>(key[length]);
                unit = array_[node_pos];
                if (unit.label() != static_cast<uchar_type>(key[length]))
                {
                    return num_results;
                }

                node_pos ^= unit.offset();
                if (unit.has_leaf())
                {
                    if (num_results < max_num_results)
                    {
                        set_result(&results[num_results], static_cast<value_type>(
                            array_[node_pos].value()), length + 1);
                    }
                    ++num_results;
                }
            }
        }

        return num_results;
    }

	template <typename A, typename B, typename T, typename C>
	inline typename DoubleArrayImpl<A, B, T, C>::value_type
		DoubleArrayImpl<A, B, T, C>::traverse(const key_type *key,
		std::size_t &node_pos, std::size_t &key_pos, std::size_t length) const
	{
		id_type id = static_cast<id_type>(node_pos);
		unit_type unit = array_[id];

		if (length != 0)
        {
            for ( ; key_pos < length; ++key_pos)
            {
                id ^= unit.offset() ^ static_cast<uchar_type>(key[key_pos]);
                unit = array_[id];
                if (unit.label() != static_cast<uchar_type>(key[key_pos]))
                {
                    return static_cast<value_type>(-2);
                }
                node_pos = id;
            }
        }
        else
        {
            for ( ; key[key_pos] != '\0'; ++key_pos)
            {
                id ^= unit.offset() ^ static_cast<uchar_type>(key[key_pos]);
                unit = array_[id];
                if (unit.label() != static_cast<uchar_type>(key[key_pos]))
                {
                    return static_cast<value_type>(-2);
                }
                node_pos = id;
            }
        }

        if (!unit.has_leaf())
        {
            return static_cast<value_type>(-1);
        }

        unit = array_[id ^ unit.offset()];
        return static_cast<value_type>(unit.value());
    }

    namespace Details 
    {

        /**
        * @brief 管理数组内存
        */
        template <typename T>
        class AutoArray
        {
        public:
            /**
            * @brief 构造函数
            *
            * @param array 用于初始化数组地址
            */
            explicit AutoArray(T *array = NULL) : array_(array)
            {
            }

            /**
            * @brief 析构函数
            *        回收内存消耗
            */
            ~AutoArray() 
            { 
                clear(); 
            }

            /**
            * @brief 获取指定位置的数组单元
            *
            * @param id 要获取的数组单元索引
            *
            * @return 数组单元常量引用
            */
            const T &operator[](std::size_t id) const 
            {
                return array_[id];
            }

            /**
            * @brief 获取指定位置的数组单元
            *
            * @param id 要获取的数组单元索引
            *
            * @return 数组单元引用
            */
            T &operator[](std::size_t id)
            { 
                return array_[id]; 
            }

            /**
            * @brief 判断数组是否为空
            *
            * @return 数组为空返回true，否则返回false
            */
            bool empty() const 
            {
                return array_ == NULL;
            }

            /**
            * @brief 回收数组内存
            */
            void clear();

            /**
            * @brief 交换数组array与当前对象的数组
            *
            * @param array 待交换数组
            */
            void swap(AutoArray *array);

            /**
            * @brief 将当前数组buffer设为array
            *
            * @param array 待设定的数组
            */
            void reset(T *array = NULL) 
            {
                AutoArray(array).swap(this); 
            }

        private:
            /** 指向数组缓冲区 */
            T *     array_;

            /**
            * @brief 将拷贝构造函数和赋值拷贝构造
            *        函数定义为私有成员防止被调用
            */
            AutoArray(const AutoArray &);
            AutoArray &operator=(const AutoArray &);
        };

        template <typename T>
        void AutoArray<T>::clear()
        {
            if (array_ != NULL)
            {
                delete[] array_;
                array_ = NULL;
            }
        }

        template <typename T>
        void AutoArray<T>::swap(AutoArray *array)
        {
            T *temp;
            temp = array_;
            array_ = array->array_;
            array->array_ = temp;
        }

        /**
        * @brief 
        */
        template <typename T>
        class AutoPool
        {
        public:
            AutoPool() : buf_(), size_(0), capacity_(0)
            {
            }

            ~AutoPool() 
            {
                clear(); 
            }

            /**
            * @brief 返回指定单元位置的常量引用
            *
            * @param id 指定单元位置
            *
            * @return 指定单元对应的常量引用
            */
            const T &operator[](std::size_t id) const;

            /**
            * @brief 返回指定单元位置的变量引用
            *
            * @param id 指定单元位置
            *
            * @return 指定单元对应的变量引用
            */
            T &operator[](std::size_t id);

            /**
            * @brief 判断当前存储元素是否为空
            *
            * @return 空返回true，否则false
            */
            bool empty() const 
            {
                return size_ == 0;
            }

            /**
            * @brief 返回当前存储的单元数
            *
            * @return 当前存储的单元数
            */
            std::size_t size() const 
            {
                return size_;
            }

            /**
            * @brief 回收内存
            */
            void clear();

            /**
            * @brief 将当前元素添加到缓冲区结尾
            *
            * @param value 待加入的单元
            */
            void push_back(const T &value) 
            { 
                append(value);
            }

            /**
            * @brief 从结尾处移除一个单元
            */
            void pop_back();

            /**
            * @brief 在缓冲区结尾添加一个单元
            *        并使用单元的构造函数初始化
            */
            void append();

            /**
            * @brief 在缓冲区结尾添加一个单元
            */
            void append(const T &value);

            /**
            * @brief 将数组的存储单元数调整为size
            *        a. 若先前的存储单元数大于size，则释放超出size的存储单元
            *        b. 若先前的存储单元数小于size，则添加新的存储单元并使用单元构造函数初始化
            *
            * @param size 调整后的存储单元数
            */
            void resize(std::size_t size);

            /**
            * @brief 将数组的存储单元数调整为size
            *        a. 若先前的存储单元数大于size，则释放超出size的存储单元
            *        b. 若先前的存储单元数小于size，则添加新的存储单元并使用value初始化
            *
            * @param size  调整后的存储单元数
            * @param value 用于初始化新存储单元
            */
            void resize(std::size_t size, const T &value);

            /**
            * @brief 分配存储空间
            *
            * @pram size 待分配存储空间大小
            */
            void reserve(std::size_t size);

        private:
            /** 字符缓冲区         */
            AutoArray<char>     buf_;
            /** 存储的字符数      */
            std::size_t         size_;
            /** 可存储的字符数    */
            std::size_t         capacity_;

            /**
            * @brief 调整数组buffer空间为size
            *        若原数组buffer非空，则将单元拷贝到新单元中
            *
            * @param size 要调整的数组buffer最小大小 
            */
            void resize_buf(std::size_t size);

            /**
            * @brief 将构造函数和拷贝构造函数
            *        定义为私有成员防止被调用
            */
            AutoPool(const AutoPool &);
            AutoPool &operator=(const AutoPool &);
        };

        template <typename T>
        const T &AutoPool<T>::operator[](std::size_t id) const
        {
            return *(reinterpret_cast<const T *>(&buf_[0]) + id);
        }

        template <typename T>
        T &AutoPool<T>::operator[](std::size_t id)
        {
            return *(reinterpret_cast<T *>(&buf_[0]) + id);
        }

        template <typename T>
        void AutoPool<T>::clear()
        {
            resize(0);

            buf_.clear();
            size_ = 0;
            capacity_ = 0;
        }

        template <typename T>
        void AutoPool<T>::pop_back()
        {
            (*this)[--size_].~T();
        }

        template <typename T>
        void AutoPool<T>::append()
        {
            if (size_ == capacity_)
            {
                resize_buf(size_ + 1);
            }

            new(&(*this)[size_++]) T;
        }

        template <typename T>
        void AutoPool<T>::append(const T &value)
        {
            if (size_ == capacity_)
            {
                resize_buf(size_ + 1);
            }

            new(&(*this)[size_++]) T(value);
        }

        template <typename T>
        void AutoPool<T>::resize(std::size_t size)
        {
            while (size_ > size)
            {
                (*this)[--size_].~T();
            }

            if (size > capacity_)
            {
                resize_buf(size);
            }

            while (size_ < size)
            {
                new(&(*this)[size_++]) T;
            }
        }

        template <typename T>
        void AutoPool<T>::resize(std::size_t size, const T &value)
        {
            while (size_ > size)
            {
                (*this)[--size_].~T();
            }

            if (size > capacity_)
            {
                resize_buf(size);
            }

            while (size_ < size)
            {
                new(&(*this)[size_++]) T(value);
            }
        }

        template <typename T>
        void AutoPool<T>::reserve(std::size_t size)
        {
            if (size > capacity_)
            {
                resize_buf(size);
            }
        }

        template <typename T>
        void AutoPool<T>::resize_buf(std::size_t size)
        {
            std::size_t capacity;
            if (size >= capacity_ * 2)
            {
                capacity = size;
            }
            else
            {
                capacity = 1;
                while (capacity < size)
                {
                    capacity <<= 1;
                }
            }

            AutoArray<char> buf;
            try
            {
                buf.reset(new char[sizeof(T) * capacity]);
            }
            catch (const std::bad_alloc &)
            {
                DARTS_THROW("failed to resize pool: std::bad_alloc");
            }

            if (size_ > 0)
            {
                T *src = reinterpret_cast<T *>(&buf_[0]);
                T *dest = reinterpret_cast<T *>(&buf[0]);
                for (std::size_t i = 0; i < size_; ++i)
                {
                    new(&dest[i]) T(src[i]);
                    src[i].~T();
                }
            }

            buf_.swap(&buf);
            capacity_ = capacity;
        }

        /**
        * @brief 栈
        */
        template <typename T>
        class AutoStack
        {
        public:
            /**
            * @brief 构造函数
            */
            AutoStack() : pool_() 
            {
            }

            /**
            * @brief 析构函数
            */
            ~AutoStack() 
            {
                clear();
            }

            /**
            * @brief 获取栈顶元素常量引用
            */
            const T &top() const
            { 
                return pool_[size() - 1]; 
            }

            /**
            * @brief 获取栈顶元素变量引用
            */
            T &top() 
            { 
                return pool_[size() - 1]; 
            }

            /**
            * @brief 判断栈是否为空
            */
            bool empty() const 
            { 
                return pool_.empty(); 
            }

            /**
            * @brief 获取栈中保存元素数
            */
            std::size_t size() const
            { 
                return pool_.size(); 
            }

            /**
            * @brief 将元素压栈
            * 
            * @param value 待压栈元素
            */
            void push(const T &value)
            { 
                pool_.push_back(value); 
            }

            /**
            * @brief 弹出栈顶元素
            */
            void pop() 
            {
                pool_.pop_back();
            }

            /**
            * @brief 回收栈内存
            */
            void clear() 
            { 
                pool_.clear(); 
            }

        private:
            /** brief 保存数据单元buffer */
            AutoPool<T>         pool_;

            /**
            * @brief 显式定义构造函数和赋值拷贝
            *        构造函数为私有成员防止被调用
            */
            AutoStack(const AutoStack &);
            AutoStack &operator=(const AutoStack &);
        };


        /**
        * @brief 位向量
        */
        class BitVector
        {
        public:
            BitVector() : units_(), ranks_(), num_ones_(0), size_(0) 
            {
            }

            ~BitVector() 
            {
                clear(); 
            }

            /**
            * @brief 判断指定的位是否存在
            *
            * @param id 位下标
            *
            * @return 位存在返回true，否则返回false
            */
            bool operator[](std::size_t id) const;

            id_type rank(std::size_t id) const;

            /**
            * @brief 设定指定的位
            *
            * @param id  位下标
            * @param bit 指定的位
            */
            void set(std::size_t id, bool bit);

            /**
            * @brief 判断存储位的向量是否为空
            */
            bool empty() const
            {
                return units_.empty();
            }


            std::size_t num_ones() const
            { 
                return num_ones_;
            }

            /**
            * @brief 位向量单元数
            */
            std::size_t size() const 
            {
                return size_;
            }

            /**
            * @brief 在保存位向量的存储buffer
            *        末尾添加一个单元用于存储位
            */
            void append();

            /**
            * @brief 构建rank数组
            */
            void build();

            /**
            * @brief 释放用于存储位信息的buffer
            */
            void clear();

        private:
            enum { UNIT_SIZE = sizeof(id_type) * 8 };

            /** 保存位信息           */
            AutoPool<id_type>       units_;

            /** 保存位对应的哈希信息 */
            AutoArray<id_type>      ranks_;

            /** 保存位对应的哈希值 */
            std::size_t             num_ones_;

            /** 保存units_对应的位成员数 */
            std::size_t             size_;

            /**
            * @brief 生成unit对应的哈希值
            *
            * @param unit 待生成哈希值整数
            */
            static id_type pop_count(id_type unit);

            /**
            * @brief 显式定义构造函数和赋值拷贝
            *        构造函数为私有成员防止被调用
            */
            BitVector(const BitVector &);
            BitVector &operator=(const BitVector &);
        };

        inline bool BitVector::operator[](std::size_t id) const
        {
            return (units_[id / UNIT_SIZE] >> (id % UNIT_SIZE) & 1) == 1;
        }

        inline id_type BitVector::rank(std::size_t id) const
        {
            std::size_t unit_id = id / UNIT_SIZE;
            return ranks_[unit_id] + pop_count(units_[unit_id]
            & (~0U >> (UNIT_SIZE - (id % UNIT_SIZE) - 1)));
        }

        inline void BitVector::set(std::size_t id, bool bit)
        {
            if (bit)
            {
                units_[id / UNIT_SIZE] |= 1U << (id % UNIT_SIZE);
            }
            else
            {
                units_[id / UNIT_SIZE] &= ~(1U << (id % UNIT_SIZE));
            }
        }

        inline void BitVector::append()
        {
            if ((size_ % UNIT_SIZE) == 0)
            {
                units_.append(0);
            }

            ++size_;
        }

        inline void BitVector::build()
        {
            try
            {
                ranks_.reset(new id_type[units_.size()]);
            }
            catch (const std::bad_alloc &)
            {
                DARTS_THROW("failed to build rank index: std::bad_alloc");
            }

            num_ones_ = 0;
            for (std::size_t i = 0; i < units_.size(); ++i)
            {
                ranks_[i] = num_ones_;
                num_ones_ += pop_count(units_[i]);
            }
        }

        inline void BitVector::clear()
        {
            units_.clear();
            ranks_.clear();
        }

        inline id_type BitVector::pop_count(id_type unit)
        {
            unit = ((unit & 0xAAAAAAAA) >> 1) + (unit & 0x55555555);
            unit = ((unit & 0xCCCCCCCC) >> 2) + (unit & 0x33333333);
            unit = ((unit >> 4) + unit) & 0x0F0F0F0F;
            unit += unit >> 8;
            unit += unit >> 16;
            return unit & 0xFF;
        }


        /**
        * @brief 字符串集
        *        T -- 字符串属性对应的类型
        */
        template <typename T>
        class Keyset
        {
        public:
            /**
            * @brief 初始化用于构建双数组的字符串集
            *
            * @param num_keys 字符串个数
            * @param keys     字符串
            * @param length   各字符串长度
            * @param values   各字符串对应的value
            */
            Keyset(std::size_t num_keys, const char_type * const *keys,
                const std::size_t *lengths, const T *values):
            num_keys_(num_keys), keys_(keys), lengths_(lengths), values_(values)
            {
            }

            /**
            * @brief 返回字符串个数
            */
            std::size_t num_keys() const 
            { 
                return num_keys_; 
            }

            /**
            * @brief 返回确定索引对应的字符串
            *
            * @param id 字符串索引
            *
            * @return 字符串
            */
            const char_type *keys(std::size_t id) const 
            { 
                return keys_[id]; 
            }

            /**
            * @brief 返回确定字符串中对应的id
            *
            * @param key_id  字符串索引
            * @param char_id 字符索引
            *
            * @return 字符
            */
            uchar_type keys(std::size_t key_id, std::size_t char_id) const;

            /**
            * @brief 判断是否有字符串长度数组
            *
            * @return 有字符串长度数组返回true，否则返回false
            */
            bool has_lengths() const
            { 
                return lengths_ != NULL; 
            }

            /**
            * @brief 返回字符串集中对应字符串的长度
            *
            * @param id 字符串在字符串集中的索引
            *
            * @return 字符串长度
            */
            std::size_t lengths(std::size_t id) const;

            /**
            * @brief 判断是否有字符串属性数组
            *
            * @return 有属性数组返回true，否则返回false
            */
            bool has_values() const 
            {
                return values_ != NULL; 
            }

            /**
            * @brief 返回字符串集中对应字符串的属性
            *
            * @param id 字符串在字符串集中的索引
            *
            * @return 字符串属性
            */
            const value_type values(std::size_t id) const;

        private:
            /** 字符串个数                    */
            std::size_t               num_keys_;
            /** 字符串集                     */
            const char_type * const   *keys_;
            /** 字符串长度                   */
            const std::size_t         *lengths_;
            /** 字符串属性                  */
            const T                   *values_;

            /**
            * @brief 显式声明拷贝构造函数和赋值拷贝构造
            *        函数在private区间以防被调用
            */
            Keyset(const Keyset &);
            Keyset &operator=(const Keyset &);
        };

        template <typename T>
        inline uchar_type Keyset<T>::keys(std::size_t key_id, std::size_t char_id) const
        {
            if (has_lengths() && char_id >= lengths_[key_id])
            {
                return '\0';
            }

            return keys_[key_id][char_id];
        }

        template <typename T>
        inline std::size_t Keyset<T>::lengths(std::size_t id) const
        {
            if (has_lengths())
            {
                return lengths_[id];
            }

            std::size_t length = 0;
            while (keys_[id][length] != '\0')
            {
                ++length;
            }

            return length;
        }

        template <typename T>
        inline const value_type Keyset<T>::values(std::size_t id) const
        {
            if (has_values())
            {
                return static_cast<value_type>(values_[id]);
            }

            return static_cast<value_type>(id);
        }


        /**
        * @brief 有向无环图(Directed Acyclic Word Graph)节点
        */
        class DawgNode
        {
        public:
            /**
            * @brief 构造函数
            *        初始化有向无环图节点
            */
            DawgNode() : child_(0), sibling_(0), label_('\0'),
                is_state_(false), has_sibling_(false) 
            {
            }

            /**
            * @brief 设定子节点
            *
            * @param child 子节点
            */
            void set_child(id_type child) 
            { 
                child_ = child; 
            }

            /**
            * @brief 设定邻居节点
            * 
            * @param sibling 邻居节点
            */
            void set_sibling(id_type sibling) 
            { 
                sibling_ = sibling; 
            }

            /**
            * @brief 设定当前词条对应的属性
            *
            * @param value 当前词条对应的属性
            */
            void set_value(value_type value) 
            { 
                child_ = value; 
            }

            /**
            * @brief 设定当前节点对应的字符
            *
            * @param label 当前节点对应的字符
            */
            void set_label(uchar_type label) 
            { 
                label_ = label; 
            }

            /**
            * @brief 设定当前节点是否对应词条中间状态
            *
            * @param is_state 指示是否对应词条中间状态
            */
            void set_is_state(bool is_state) 
            { 
                is_state_ = is_state; 
            }

            /**
            * @brief 设定当前节点是否有邻居节点
            *
            * @param has_sibling 指示当前节点是否有邻居
            */
            void set_has_sibling(bool has_sibling) 
            { 
                has_sibling_ = has_sibling; 
            }

            /**
            * @brief 返回当前节点的子节点
            */
            id_type child() const
            { 
                return child_; 
            }

            /**
            * @brief 返回当前节点的邻居节点
            */
            id_type sibling() const 
            {
                return sibling_; 
            }

            /**
            * @brief 返回当前词条对应的属性
            */
            value_type value() const 
            { 
                return static_cast<value_type>(child_); 
            }

            /**
            * @brief 返回当前节点对应的字符
            */
            uchar_type label() const 
            { 
                return label_; 
            }

            /**
            * @brief 返回当前节点是否是词条中间状态
            */
            bool is_state() const 
            { 
                return is_state_; 
            }

            /**
            * @brief 返回当前节点是否有邻居节点
            */
            bool has_sibling() const 
            { 
                return has_sibling_; 
            }

            id_type unit() const
            {
                if (label_ == '\0')
                {
                    return (child_ << 1) | (has_sibling_ ? 1 : 0);
                }

                return (child_ << 2) | (is_state_ ? 2 : 0) | (has_sibling_ ? 1 : 0);
            }

        private:
            /** 子节点 */
            id_type     child_;

            /** 邻居节点 */
            id_type     sibling_;

            /** 当前节点对应的字符 */
            uchar_type  label_;

            /** 是否为词条中间状态 */
            bool        is_state_;

            /** 是否有邻居节点 */
            bool        has_sibling_;
        };

        /**
        * @brief 有向无环图(Directed Acyclic Word Graph)单元
        */
        class DawgUnit
        {
        public:
            /**
            * @brief 构造函数
            */
            DawgUnit(id_type unit = 0) : unit_(unit) 
            {
            }

            /**
            * @brief 拷贝构造函数
            */
            DawgUnit(const DawgUnit &unit) : unit_(unit.unit_) 
            {
            }

            /**
            * @brief 赋值函数
            */
            DawgUnit &operator=(id_type unit)
            {
                unit_ = unit;

                return *this; 
            }

            /**
            * @brief 返回DAWG节点
            */
            id_type unit() const 
            {
                return unit_;
            }

            /**
            * @brief 返回节点子节点
            */
            id_type child() const 
            { 
                return unit_ >> 2;
            }

            /**
            * @brief 判断节点是否有邻居节点
            */
            bool has_sibling() const
            { 
                return (unit_ & 1) == 1; 
            }

            /**
            * @brief 获取节点值：包含子节点和是否为中间节点信息
            */
            value_type value() const 
            { 
                return static_cast<value_type>(unit_ >> 1);
            }

            /**
            * @brief 判断当前节点是否为中间节点
            */
            bool is_state() const
            { 
                return (unit_ & 2) == 2;
            }

        private:
            /** 保存有向无环图单元信息 */
            id_type unit_;
        };


        /**
        * @brief 有向无环图(Directed Acyclic Word Graph)构建类
        *
        * 该类通过使用栈(node_stack_)数组链表(nodes_)相结合的策略
        * 来构建DAWG，最终的DAWG通过unit_和labels来表示，table_表
        * 用于查找
        */
        class DawgBuilder
        {
        public:
            /**
            * @brief 构造函数
            *        初始化各成员变量
            */
            DawgBuilder() : nodes_(), units_(), labels_(), is_intersections_(),
                table_(), node_stack_(), recycle_bin_(), num_states_(0)
            {
            }

            /**
            * @brief 析构函数
            */
            ~DawgBuilder() 
            { 
                clear(); 
            }

            /**
            * @brief 返回根节点
            */
            id_type root() const 
            { 
                return 0;
            }

            /**
            * @brief 返回当前节点子节点
            *
            * @param id 当前节点id
            *
            * @return 当前节点子节点
            */
            id_type child(id_type id) const 
            {
                return units_[id].child(); 
            }

            /**
            * @brief 返回当前节点邻居节点
            *
            * @param id 当前节点id
            *
            * @return 当前节点邻居节点，无邻居节点返回0
            */
            id_type sibling(id_type id) const
            { 
                return units_[id].has_sibling() ? id + 1 : 0;
            }

            /**
            * @brief 获取节点值：包含子节点和是否为中间节点信息
            *
            * @param id 当前节点id
            *
            * @return 节点值
            */
            int value(id_type id) const 
            { 
                return units_[id].value(); 
            }

            /**
            * @brief 判断当前节点是否为叶子节点
            */
            bool is_leaf(id_type id) const 
            { 
                return label(id) == '\0'; 
            }

            /**
            * @brief 获取节点对应的字符
            *
            * @param id 节点id
            *
            * @return 节点对应的字符
            */
            uchar_type label(id_type id) const 
            { 
                return labels_[id];
            }

            /**
            * @brief 判断当前节点是否为共享节点
            *
            * @param id 节点id
            * 
            * @return 是否为共享节点
            */
            bool is_intersection(id_type id) const 
            {
                return is_intersections_[id]; 
            }


            id_type intersection_id(id_type id) const
            { 
                return is_intersections_.rank(id) - 1; 
            }

            /**
            * @brief 获取当前共享节点被共享次数
            *
            * @param id 节点id
            * 
            * @return 共享次数
            */
            std::size_t num_intersections() const
            { 
                return is_intersections_.num_ones(); 
            }

            /**
            * @brief 返回units_中单元数
            */
            std::size_t size() const 
            { 
                return units_.size();
            }

            /**
            * @brief 初始化DAWG
            */
            void init();

            /**
            * @brief 将nodes_中的剩余节点以DAWG形式构建到units_和labels_中
            *        并回收nodes_、node_stack_、recycle_bin_所占的内存
            */
            void finish();

            /**
            * @brief 将一个词条加入到DAWG中
            *
            * @param key    词条
            * @param length 词长
            * @param value  属性值
            */
            void insert(const char *key, std::size_t length, value_type value);

            /**
            * @brief 清空内存消耗
            */
            void clear();

            /**
            * @brief 打印有向无环图
            */
            void printDawg(void);

        private:
            enum { INITIAL_TABLE_SIZE = 1 << 10 };

            /** 存储有向无环图节点表 */
            AutoPool<DawgNode>      nodes_;

            /** 有向无环图单元表：每个单元包含三个信息子节点|是否为中间状态|是否有邻居 */
            AutoPool<DawgUnit>      units_;

            /** 保存字符 */
            AutoPool<uchar_type>    labels_;

            /** 记录当前节点是否被其他节点共享 */
            BitVector               is_intersections_;

            /** 存储有向无环图节点索引 */
            AutoPool<id_type>       table_;

            /** 用于将有向无环图节点压栈 */
            AutoStack<id_type>      node_stack_;

            /** 用于回收有向无环图节点 */
            AutoStack<id_type>      recycle_bin_;

            /** 用于记录状态数 */
            std::size_t             num_states_;

            /**
            * @brief 将node_stack_中节点id之上的所有节点保存到units_、labels_和table_中
            *
            * @param id 当前节点id
            */
            void flush(id_type id);

            /**
            * @brief 扩充table_哈希表大小
            */
            void expand_table();

            /**
            * @brief 定位数id在table_哈希表中的存储位置
            *
            * @param id      要存储到哈希表中的数
            * @param hash_id 哈希表中存储id的单元索引
            *
            * @return 0
            */
            id_type find_unit(id_type id, id_type *hash_id) const;

            /**
            * @brief 判断table_表中是否已经存在node_id对应的节点
            *        所谓存在指的是存在一个节点及其邻居均与node_id对应的节点一致
            * 
            * @param node_id 需要在table_表中查询是否存在的节点对应的索引
            * @param hash_id 保存table_中可以存储node_id对应的节点位置
            *
            * @return 若存在返回table_表中对应的相等节点索引，否则返回0
            */
            id_type find_node(id_type node_id, id_type *hash_id) const;

            /**
            * @brief 判断两个节点是否相等
            *        节点相等指的是两个节点自身及其邻居均对应相等
            * 
            * @param node_id 节点1索引
            * @param unit_id 节点2索引
            *
            * @return 相等返回true，否则返回false
            */
            bool are_equal(id_type node_id, id_type unit_id) const;

            /**
            * @brief 计算节点对应的哈希值
            *
            * @param id 节点id
            *
            * @return 节点对应的哈希值
            */
            id_type hash_unit(id_type id) const;

            /**
            * @brief 计算节点对应的哈希值
            *
            * @param id 节点id
            *
            * @return 节点对应的哈希值
            */
            id_type hash_node(id_type id) const;

            /**
            * @brief 在有向无环图节点表中添加一个节点
            */
            id_type append_node();

            /**
            * @brief 在有向无环图单元表中添加一个单元
            */
            id_type append_unit();

            /**
            * @brief 将有向无环图节点表中的一个节点回收
            */
            void free_node(id_type id);

            /**
            * @brief 计算key对应的哈希值
            *
            * @param key 带生成哈希值数
            *
            * @return 返回的哈希值
            */
            static id_type hash(id_type key);

            /**
            * @brief 显式定义拷贝构造函数和赋值拷贝构造函数
            *        为私有成员防止被调用时使用默认形式出错
            */
            DawgBuilder(const DawgBuilder &);
            DawgBuilder &operator=(const DawgBuilder &);
        };

        inline void DawgBuilder::init()
        {
            table_.resize(INITIAL_TABLE_SIZE, 0);

            append_node();
            append_unit();

            num_states_ = 1;

            nodes_[0].set_label(0xFF);
            node_stack_.push(0);
        }

        inline void DawgBuilder::finish()
        {
            flush(0);

            units_[0] = nodes_[0].unit();
            labels_[0] = nodes_[0].label();

            nodes_.clear();
            table_.clear();
            node_stack_.clear();
            recycle_bin_.clear();

            is_intersections_.build();
        }

        inline void DawgBuilder::insert(const char *key, std::size_t length,
            value_type value)
        {
            if (value < 0)
            {
                DARTS_THROW("failed to insert key: negative value");
            }
            else if (length == 0)
            {
                DARTS_THROW("failed to insert key: zero-length key");
            }

            /* DAWG节点遍历索引：初值为0表示从根节点开始遍历 */
            id_type     id      = 0;

            /* 当前要存储的字符串字符遍历索引 */
            std::size_t key_pos = 0;

            /* 遍历DAWG图找到当前字符串中字符需要存储的节点位置 */
            for ( ; key_pos <= length; ++key_pos)
            {
                /* 获取当前节点子节点索引 */
                id_type child_id = nodes_[id].child();
                if (child_id == 0)
                {
                    break;
                }

                /* 获取当前要存储词条对应字符 */
                uchar_type key_label = static_cast<uchar_type>(key[key_pos]);
                if (key_pos < length && key_label == '\0')
                    DARTS_THROW("failed to insert key: invalid null character");

                /* 获取存储在当前节点子节点中对应的字符 */
                uchar_type unit_label = nodes_[child_id].label();

                if (key_label < unit_label)
                {
                    /* 若当前字符小于已存储字符说明要存储的字符串顺序不正确 */
                    DARTS_THROW("failed to insert key: wrong key order");
                }
                else if (key_label > unit_label)
                {
                    /* 若当前字符大于已存储字符说明新的字符需要存储 */

                    /* 将当前节点子节点设为存在邻居节点 */
                    nodes_[child_id].set_has_sibling(true);

                    /* 将当前节点及其所有后续子节点存储到unit_、label_、table_等表中
                    * a. 构建unit_表中各单元之间的链接关系
                    * b. 构建table_表，该表存储node_表中节点对应的单元存储位置
                    */
                    flush(child_id);

                    break;
                }

                id = child_id;
            }

            if (key_pos > length)
            {
                return;
            }

            for ( ; key_pos <= length; ++key_pos)
            {
                /* 获取词条对应的要存储字符 */
                uchar_type key_label = static_cast<uchar_type>(
                    (key_pos < length) ? key[key_pos] : '\0');

                /* 获取一个子节点用于存储当前字符 */
                id_type child_id = append_node();

                if (nodes_[id].child() == 0)
                {
                    /* 将子节点设为中间状态节点 */
                    nodes_[child_id].set_is_state(true);
                }

                /* 将父节点的子节点设为当前子节点的邻居节点 */
                nodes_[child_id].set_sibling(nodes_[id].child());

                /* 设定当前子节点对应的字符 */
                nodes_[child_id].set_label(key_label);

                /* 将当前子节点设为父节点的子节点 */
                nodes_[id].set_child(child_id);

                /* 将当前子节点压栈 */
                node_stack_.push(child_id);

                /* 将子节点设为父节点以便处理字符串后续字符 */
                id = child_id;
            }

            /* 保存有向无环图属性 */
            nodes_[id].set_value(value);
        }

        inline void DawgBuilder::clear()
        {
            nodes_.clear();
            units_.clear();
            labels_.clear();
            is_intersections_.clear();
            table_.clear();
            node_stack_.clear();
            recycle_bin_.clear();
            num_states_ = 0;
        }

        inline void DawgBuilder::printDawg(void)
        {
            id_type     unit_value;  /* 单元值 */
            uchar_type  label_value; /* 字符值 */
            bool        bhassibling; /* 有邻居 */
            bool        bstate;      /* 是否为中间状态 */
            id_type     child_idx;   /* 子节点 */  

            cerr << "存储表" << endl;
            for (size_t i = 0; i < units_.size(); i++)
            {
                /* 打印存储表索引 */
                cerr << "\t" << i;
            }
            cerr << endl;

            for (size_t i = 0; i < units_.size(); i++)
            {
                unit_value  = units_[i].unit();

                if (labels_[i] == '\0')
                {
                    child_idx   = unit_value >> 1;
                    bhassibling = unit_value & 1;
                    bstate      = false;
                }
                else
                {
                    child_idx   = unit_value >> 2;
                    bhassibling = unit_value & 1;
                    bstate      = unit_value & 2;
                }

                /* 打印存储表 */
                cerr << " c:" << child_idx << " bh:" << bhassibling << " bs:" << bstate;
            }
            cerr << endl;

            cerr << "字符表" << endl;
            for (size_t i = 0; i < labels_.size(); i++)
            {
                label_value = labels_[i];
                if (label_value == '\0')
                {
                    label_value = '0';
                }

                cerr << "\t" << label_value;
            }
            cerr << endl;
        }

        inline void DawgBuilder::flush(id_type id)
        {
            while (node_stack_.top() != id)
            {
                id_type node_id = node_stack_.top();
                node_stack_.pop();

                /* 扩充哈希表存储大小：哈希表存储率为(table_.size() - table_.size()/4)/table_.size() */
                if (num_states_ >= table_.size() - (table_.size() >> 2))
                {
                    expand_table();
                }

                /* 统计当前节点及其邻居数 */
                id_type num_siblings = 0;
                for (id_type i = node_id; i != 0; i = nodes_[i].sibling())
                {
                    ++num_siblings;
                }

                id_type hash_id;

                /* 查找当前节点在table_表中是否存在，并返回当前单元对应存储位置(hash_id) */
                id_type match_id = find_node(node_id, &hash_id);
                if (match_id != 0)
                {
                    is_intersections_.set(match_id, true);
                }
                else
                {
                    id_type unit_id = 0;

                    /* 为当前节点及其所有邻居分配存储空间以便存储 */
                    for (id_type i = 0; i < num_siblings; ++i)
                    {
                        unit_id = append_unit();
                    }

                    for (id_type i = node_id; i != 0; i = nodes_[i].sibling())
                    {
                        /* 存储有向无环图节点 */
                        units_[unit_id] = nodes_[i].unit();

                        /* 存储当前节点对应的字符 */
                        labels_[unit_id] = nodes_[i].label();

                        /* 存储下一个单元 */
                        --unit_id;
                    }

                    /* 记录当前节点在units_表中的存储索引 */
                    match_id = unit_id + 1;

                    /* 将当前单元存储索引保存在哈希表中 */
                    table_[hash_id] = match_id;

                    /* 记录状态数 */
                    ++num_states_;
                }

                /* 回收当前节点及其所有邻居节点 */
                for (id_type i = node_id, next; i != 0; i = next)
                {
                    next = nodes_[i].sibling();
                    free_node(i);
                }

                /* 设定父节点的子节点为当前节点 */
                nodes_[node_stack_.top()].set_child(match_id);
            }
            node_stack_.pop();
        }

        inline void DawgBuilder::expand_table()
        {
            /* 将表大小扩充1倍 */
            std::size_t table_size = table_.size() << 1;

            /* 清空之前的哈希表，并重新分配表大小 */
            table_.clear();
            table_.resize(table_size, 0);

            /* 将units_中的各单元存储到新的哈希表中 */
            for (std::size_t i = 1; i < units_.size(); ++i)
            {
                id_type id = static_cast<id_type>(i);
                if (labels_[id] == '\0' || units_[id].is_state())
                {
                    id_type hash_id;
                    find_unit(id, &hash_id);
                    table_[hash_id] = id;
                }
            }
        }

        inline id_type DawgBuilder::find_unit(id_type id, id_type *hash_id) const
        {
            *hash_id = hash_unit(id) % table_.size();
            for ( ; ; *hash_id = (*hash_id + 1) % table_.size())
            {
                id_type unit_id = table_[*hash_id];
                if (unit_id == 0)
                {
                    break;
                }
            }
            return 0;
        }

        inline id_type DawgBuilder::find_node(id_type node_id, id_type *hash_id) const
        {
            *hash_id = hash_node(node_id) % table_.size();
            for ( ; ; *hash_id = (*hash_id + 1) % table_.size())
            {
                id_type unit_id = table_[*hash_id];
                if (unit_id == 0)
                {
                    break;
                }

                if (are_equal(node_id, unit_id))
                {
                    return unit_id;
                }
            }
            return 0;
        }

        inline bool DawgBuilder::are_equal(id_type node_id, id_type unit_id) const
        {
            for (id_type i = nodes_[node_id].sibling(); i != 0;
                i = nodes_[i].sibling())
            {
                if (units_[unit_id].has_sibling() == false)
                {
                    return false;
                }

                ++unit_id;
            }

            if (units_[unit_id].has_sibling() == true)
            {
                return false;
            }

            for (id_type i = node_id; i != 0; i = nodes_[i].sibling(), --unit_id)
            {
                if (nodes_[i].unit() != units_[unit_id].unit() ||
                    nodes_[i].label() != labels_[unit_id])
                {
                    return false;
                }
            }

            return true;
        }

        inline id_type DawgBuilder::hash_unit(id_type id) const
        {
            id_type hash_value = 0;
            for ( ; id != 0; ++id)
            {
                id_type unit = units_[id].unit();
                uchar_type label = labels_[id];
                hash_value ^= hash((label << 24) ^ unit);

                if (units_[id].has_sibling() == false)
                {
                    break;
                }
            }
            return hash_value;
        }

        inline id_type DawgBuilder::hash_node(id_type id) const
        {
            id_type hash_value = 0;
            for ( ; id != 0; id = nodes_[id].sibling())
            {
                id_type unit = nodes_[id].unit();
                uchar_type label = nodes_[id].label();
                hash_value ^= hash((label << 24) ^ unit);
            }
            return hash_value;
        }

        inline id_type DawgBuilder::append_unit()
        {
            /* 在is_intersections中添加一个单元 */
            is_intersections_.append();

            /* 在units_中添加一个单元 */
            units_.append();

            /* 在labels_中添加一个单元 */
            labels_.append();

            return static_cast<id_type>(is_intersections_.size() - 1);
        }

        inline id_type DawgBuilder::append_node()
        {
            id_type id;
            if (recycle_bin_.empty())
            {
                id = static_cast<id_type>(nodes_.size());
                nodes_.append();
            }
            else
            {
                id = recycle_bin_.top();
                nodes_[id] = DawgNode();
                recycle_bin_.pop();
            }
            return id;
        }

        inline void DawgBuilder::free_node(id_type id)
        {
            recycle_bin_.push(id);
        }

        // 32-bit mix function.
        // http://www.concentric.net/~Ttwang/tech/inthash.htm
        inline id_type DawgBuilder::hash(id_type key)
        {
            key = ~key + (key << 15);  // key = (key << 15) - key - 1;
            key = key ^ (key >> 12);
            key = key + (key << 2);
            key = key ^ (key >> 4);
            key = key * 2057;  // key = (key + (key << 3)) + (key << 11);
            key = key ^ (key >> 16);
            return key;
        }


        /**
        * @brief 双数组构建单元
        */
        class DoubleArrayBuilderUnit
        {
        public:
            /**
            * @brief 构造函数
            */
            DoubleArrayBuilderUnit() : unit_(0) 
            {
            }

            void set_value(value_type value) 
            {
                unit_ = value | (1U << 31); 
            }

            /**
            * @brief 设定LCHECK即当前CHECK单元中保存的字符
            *        BASE.CHECK用4个字节表示，第一个字节用于保存CHECK值
            *
            * @param label 保存跳转到当前BC数组对应的字符
            */
            void set_label(uchar_type label)
            { 
                unit_ = (unit_ & ~0xFFU) | label; 
            }

            /**
            * @brief 将BASE单元的第一个bit设定为0或1
            *        用于指定当前节点是否为叶节点(1)
            *
            * @param has_leaf 指示当前单元是否为叶节点
            *        叶节点用于指向TAIL数组
            */
            void set_has_leaf(bool has_leaf);

            /**
            * @brief 设置BASE值
            */
            void set_offset(id_type offset);

        private:
            /** 双数组构建单元，含BASE|HAS_LEAF|LCHECK等信息 */
            id_type unit_;
        };
        
        inline void DoubleArrayBuilderUnit::set_has_leaf(bool has_leaf)
        {
            if (has_leaf)
            {
                unit_ |= 1U << 8;
            }
            else
            {
                unit_ &= ~(1U << 8);
            }
        }

        inline void DoubleArrayBuilderUnit::set_offset(id_type offset)
        {
            if (offset >= 1U << 29)
            {
                DARTS_THROW("failed to modify unit: too large offset");
            }

            unit_ &= (1U << 31) | (1U << 8) | 0xFF;
            if (offset < 1U << 21)
            {
                unit_ |= (offset << 10);
            }
            else
            {
                unit_ |= (offset << 2) | (1U << 9);
            }
        }

        //
        // Extra unit of double-array builder.
        //
        class DoubleArrayBuilderExtraUnit
        {
        public:
            DoubleArrayBuilderExtraUnit() : prev_(0), next_(0),
                is_fixed_(false), is_used_(false) {}

            void set_prev(id_type prev) { prev_ = prev; }
            void set_next(id_type next) { next_ = next; }
            void set_is_fixed(bool is_fixed) { is_fixed_ = is_fixed; }
            void set_is_used(bool is_used) { is_used_ = is_used; }

            id_type prev() const { return prev_; }
            id_type next() const { return next_; }
            bool is_fixed() const { return is_fixed_; }
            bool is_used() const { return is_used_; }

        private:
            id_type prev_;
            id_type next_;
            bool is_fixed_;
            bool is_used_;

            // Copyable.
        };

        /**
        * @brief 双数组构建类
        *        a. 从字符串集直接构建双数组
        *        b. 将字符串集先构建为DAWG然后转换为双数组
        */
        class DoubleArrayBuilder
        {
        public:
            /**
            * @brief 构造函数
            *
            * @param progress_func 构建进度打印函数
            */
            explicit DoubleArrayBuilder(int (*progress_func)(
                std::size_t, std::size_t)) : progress_func_(progress_func),
                units_(), extras_(), labels_(), table_(), extras_head_(0)
            {
            }

            /**
            * @brief 析构函数
            */
            ~DoubleArrayBuilder() 
            {
                clear(); 
            }

            /**
            * @brief 将字符串集构建为双数组
            *
            * @param keyset 字符串集
            */
            template <typename T>
            void build(const Keyset<T> &keyset);

            /**
            * @brief 将有向无环图转换后生成的双数组信息保存到buf_ptr中
            *
            * @param size_ptr 有向无环图转换后生成的双数组信息单元个数指针
            * @param buf_ptr  指向保存双数组地址
            */
            void copy(std::size_t *size_ptr, DoubleArrayUnit **buf_ptr) const;

            /**
            * @brief 回收构建双数组过程中的中间内存
            */
            void clear();

        private:
            enum { BLOCK_SIZE = 256 };
            enum { NUM_EXTRA_BLOCKS = 16 };
            enum { NUM_EXTRAS = BLOCK_SIZE * NUM_EXTRA_BLOCKS };

            enum { UPPER_MASK = 0xFF << 21 };
            enum { LOWER_MASK = 0xFF };

            /** 双数组单元类型 */
            typedef DoubleArrayBuilderUnit      unit_type;
            typedef DoubleArrayBuilderExtraUnit extra_type;

            /**
            * @brief 进度打印函数类型
            */
            int (* const progress_func_)(std::size_t, std::size_t);


            /** 保存由有向无环图转换后生成的双数组信息 */
            AutoPool<unit_type>         units_;

            AutoArray<extra_type>       extras_;

            /** 保存当前节点子节点及子节点所有邻居节点对应的字符 */
            AutoPool<uchar_type>        labels_;

            /** 哈希表保存双数组节点表 */
            AutoArray<id_type>          table_;

            id_type                     extras_head_;


            /**
            * @brief 返回单元数
            */
            std::size_t num_blocks() const 
            { 
                return units_.size() / BLOCK_SIZE; 
            }

            const extra_type &extras(id_type id) const
            { 
                return extras_[id % NUM_EXTRAS];
            }

            extra_type &extras(id_type id)
            { 
                return extras_[id % NUM_EXTRAS];
            }

            /**
            * @brief 使用字符串集构建DAWG
            *
            * @param keyset 字符串集
            * @param dawg_builder DAWG构建对象
            */
            template <typename T>
            void build_dawg(const Keyset<T> &keyset, DawgBuilder *dawg_builder);

            /**
            * @brief 使用DAWG构建双数组
            *
            * @param dawg DAWG图
            */
            void build_from_dawg(const DawgBuilder &dawg);

            /**
            * @brief 递归函数用于递归构建DAWG对应的双数组
            *        通过递归策略，该函数巧妙地达到了表示
            *        DAWG压缩的思想
            *
            * @param dawg DAWG图
            * @param dawg_id DAWG图索引
            * @param dic_id  双数组索引
            */
            void build_from_dawg(const DawgBuilder &dawg,
                id_type dawg_id, id_type dic_id);

            /**
            * @brief 将DAWG图当前节点及其所有邻居节点保存到双数组中
            *        同时返回双数组当前节点的offset值
            *
            * @param dawg     DAWG图
            * @param dawg_id  DAWG图节点索引
            * @param dic_id   双数组节点索引
            *
            * @return 双数组当前节点的offset值
            */
            id_type arrange_from_dawg(const DawgBuilder &dawg,
                id_type dawg_id, id_type dic_id);

            /**
            * @brief 直接使用排序后的字符串集构建双数组
            *
            * @param keyset 排序后的字符串集
            */
            template <typename T>
            void build_from_keyset(const Keyset<T> &keyset);

            /**
            * @brief 递归函数用于递归构建字符串集合对应的双数组
            *
            * @param keyset 字符串集合
            * @param begin  字符串开始索引
            * @param end    字符串结束索引
            * @param depth  字符串字符深度
            * @param dic_id 双数组节点索引
            */
            template <typename T>
            void build_from_keyset(const Keyset<T> &keyset, std::size_t begin,
                std::size_t end, std::size_t depth, id_type dic_id);

            /**
            * @brief 将字符串集合当前字符及其所有同一深度的字符保存到双数组中
            *        同时返回双数组当前节点的offset值
            *
            * @param keyset   字符串集
            * @param begin    字符串开始索引
            * @param end      字符串结束索引
            * @param depth    字符串字符深度
            * @param dic_id   双数组节点索引
            *
            * @return 双数组当前节点的offset值
            */
            template <typename T>
            id_type arrange_from_keyset(const Keyset<T> &keyset, std::size_t begin,
                std::size_t end, std::size_t depth, id_type dic_id);

            /**
            * @brief 搜索双数组当前节点对应合理offset值
            *
            * @param 双数组当前节点的offset值
            */
            id_type find_valid_offset(id_type id) const;

            /**
            * @brief 判断双数组当前节点对应的偏移量是否合理
            *
            * @param id     双数组当前节点
            * @param offset 双数组当前节点对应的偏移量
            *
            * @return 合理返回true，否则返回false
            */
            bool is_valid_offset(id_type id, id_type offset) const;

            void reserve_id(id_type id);

            void expand_units();

            void fix_all_blocks();
            void fix_block(id_type block_id);

            /**
            * @brief 显式定义拷贝构造函数和赋值拷贝
            *        构造函数为私有成员防止被调用
            */
            DoubleArrayBuilder(const DoubleArrayBuilder &);
            DoubleArrayBuilder &operator=(const DoubleArrayBuilder &);
        };

        template <typename T>
        void DoubleArrayBuilder::build(const Keyset<T> &keyset)
        {
            Details::DawgBuilder dawg_builder;

            /* 构建有向无环图 */
            build_dawg(keyset, &dawg_builder);

            /* 将有向无环图转换为双数组 */
            build_from_dawg(dawg_builder);

            dawg_builder.clear();
        }

        inline void DoubleArrayBuilder::copy(std::size_t *size_ptr,
            DoubleArrayUnit **buf_ptr) const
        {
            if (size_ptr != NULL)
            {
                *size_ptr = units_.size();
            }

            if (buf_ptr != NULL)
            {
                *buf_ptr = new DoubleArrayUnit[units_.size()];
                unit_type *units = reinterpret_cast<unit_type *>(*buf_ptr);
                for (std::size_t i = 0; i < units_.size(); ++i)
                {
                    units[i] = units_[i];
                }
            }
        }

        inline void DoubleArrayBuilder::clear()
        {
            units_.clear();
            extras_.clear();
            labels_.clear();
            table_.clear();
            extras_head_ = 0;
        }


        template <typename T>
        void DoubleArrayBuilder::build_dawg(const Keyset<T> &keyset,
            DawgBuilder *dawg_builder)
        {
            dawg_builder->init();
            for (std::size_t i = 0; i < keyset.num_keys(); ++i)
            {
                /* 将当前字符串及其相关属性保存到DAWG中 */
                dawg_builder->insert(keyset.keys(i), keyset.lengths(i),
                    keyset.values(i));

                if (progress_func_ != NULL)
                {
                    progress_func_(i + 1, keyset.num_keys() + 1);
                }
            }

            /* 将nodes_中剩余的节点保存到unit_等单元中构建最终的的DAWG */
            dawg_builder->finish();
        }

        inline void DoubleArrayBuilder::build_from_dawg(const DawgBuilder &dawg)
        {
            std::size_t num_units = 1;
            while (num_units < dawg.size())
            {
                num_units <<= 1;
            }
            units_.reserve(num_units);

            table_.reset(new id_type[dawg.num_intersections()]);
            for (std::size_t i = 0; i < dawg.num_intersections(); ++i)
            {
                table_[i] = 0;
            }

            extras_.reset(new extra_type[NUM_EXTRAS]);

            reserve_id(0);
            extras(0).set_is_used(true);
            units_[0].set_offset(1);
            units_[0].set_label('\0');

            if (dawg.child(dawg.root()) != 0)
            {
                build_from_dawg(dawg, dawg.root(), 0);
            }

            fix_all_blocks();

            extras_.clear();
            labels_.clear();
            table_.clear();
        }

        inline void DoubleArrayBuilder::build_from_dawg(const DawgBuilder &dawg,
            id_type dawg_id, id_type dic_id)
        {
            id_type dawg_child_id = dawg.child(dawg_id);
            if (dawg.is_intersection(dawg_child_id))
            {
                id_type intersection_id = dawg.intersection_id(dawg_child_id);
                id_type offset = table_[intersection_id];
                if (offset != 0)
                {
                    offset ^= dic_id;
                    if (!(offset & UPPER_MASK) || !(offset & LOWER_MASK))
                    {
                        if (dawg.is_leaf(dawg_child_id))
                        {
                            units_[dic_id].set_has_leaf(true);
                        }
                        units_[dic_id].set_offset(offset);
                        return;
                    }
                }
            }

            id_type offset = arrange_from_dawg(dawg, dawg_id, dic_id);
            if (dawg.is_intersection(dawg_child_id))
            {
                table_[dawg.intersection_id(dawg_child_id)] = offset;
            }

            do
            {
                uchar_type child_label = dawg.label(dawg_child_id);
                id_type dic_child_id = offset ^ child_label;
                if (child_label != '\0')
                {
                    build_from_dawg(dawg, dawg_child_id, dic_child_id);
                }
                dawg_child_id = dawg.sibling(dawg_child_id);
            } while (dawg_child_id != 0);
        }

        inline id_type DoubleArrayBuilder::arrange_from_dawg(const DawgBuilder &dawg,
            id_type dawg_id, id_type dic_id)
        {
            labels_.resize(0);

            /* 获取DAWG中当前节点子节点及子节点所有邻居节点对应的字符 */
            id_type dawg_child_id = dawg.child(dawg_id);
            while (dawg_child_id != 0)
            {
                labels_.append(dawg.label(dawg_child_id));
                dawg_child_id = dawg.sibling(dawg_child_id);
            }

            /* 找到存储当前节点所有子节点的合理offset */
            id_type offset = find_valid_offset(dic_id);
            units_[dic_id].set_offset(dic_id ^ offset);

            dawg_child_id = dawg.child(dawg_id);
            for (std::size_t i = 0; i < labels_.size(); ++i)
            {
                id_type dic_child_id = offset ^ labels_[i];
                reserve_id(dic_child_id);

                if (dawg.is_leaf(dawg_child_id))
                {
                    units_[dic_id].set_has_leaf(true);
                    units_[dic_child_id].set_value(dawg.value(dawg_child_id));
                }
                else
                {
                    units_[dic_child_id].set_label(labels_[i]);
                }

                dawg_child_id = dawg.sibling(dawg_child_id);
            }
            extras(offset).set_is_used(true);

            return offset;
        }


        template <typename T>
        void DoubleArrayBuilder::build_from_keyset(const Keyset<T> &keyset)
        {
            std::size_t num_units = 1;
            while (num_units < keyset.num_keys())
            {
                num_units <<= 1;
            }

            units_.reserve(num_units);

            extras_.reset(new extra_type[NUM_EXTRAS]);

            reserve_id(0);
            extras(0).set_is_used(true);
            units_[0].set_offset(1);
            units_[0].set_label('\0');

            if (keyset.num_keys() > 0)
            {
                build_from_keyset(keyset, 0, keyset.num_keys(), 0, 0);
            }

            fix_all_blocks();

            extras_.clear();
            labels_.clear();
        }

        template <typename T>
        void DoubleArrayBuilder::build_from_keyset(const Keyset<T> &keyset,
            std::size_t begin, std::size_t end, std::size_t depth, id_type dic_id)
        {
            id_type offset = arrange_from_keyset(keyset, begin, end, depth, dic_id);

            while (begin < end)
            {
                if (keyset.keys(begin, depth) != '\0')
                {
                    break;
                }

                ++begin;
            }

            if (begin == end)
            {
                return;
            }

            std::size_t last_begin = begin;
            uchar_type last_label = keyset.keys(begin, depth);
            while (++begin < end)
            {
                uchar_type label = keyset.keys(begin, depth);
                if (label != last_label)
                {
                    build_from_keyset(keyset, last_begin, begin,
                        depth + 1, offset ^ last_label);
                    last_begin = begin;
                    last_label = keyset.keys(begin, depth);
                }
            }
            build_from_keyset(keyset, last_begin, end, depth + 1, offset ^ last_label);
        }

        template <typename T>
        id_type DoubleArrayBuilder::arrange_from_keyset(const Keyset<T> &keyset,
            std::size_t begin, std::size_t end, std::size_t depth, id_type dic_id)
        {
            labels_.resize(0);

            value_type value = -1;
            for (std::size_t i = begin; i < end; ++i)
            {
                uchar_type label = keyset.keys(i, depth);
                if (label == '\0')
                {
                    if (keyset.has_lengths() && depth < keyset.lengths(i))
                    {
                        DARTS_THROW("failed to build double-array: "
                            "invalid null character");
                    }
                    else if (keyset.values(i) < 0)
                    {
                        DARTS_THROW("failed to build double-array: negative value");
                    }

                    if (value == -1)
                    {
                        value = keyset.values(i);
                    }

                    if (progress_func_ != NULL)
                    {
                        progress_func_(i + 1, keyset.num_keys());
                    }
                }

                if (labels_.empty())
                {
                    labels_.append(label);
                }
                else if (label != labels_[labels_.size() - 1])
                {
                    if (label < labels_[labels_.size() - 1])
                    {
                        DARTS_THROW("failed to build double-array: wrong key order");
                    }

                    labels_.append(label);
                }
            }

            id_type offset = find_valid_offset(dic_id);
            units_[dic_id].set_offset(dic_id ^ offset);

            for (std::size_t i = 0; i < labels_.size(); ++i)
            {
                id_type dic_child_id = offset ^ labels_[i];
                reserve_id(dic_child_id);
                if (labels_[i] == '\0')
                {
                    units_[dic_id].set_has_leaf(true);
                    units_[dic_child_id].set_value(value);
                }
                else
                {
                    units_[dic_child_id].set_label(labels_[i]);
                }
            }
            extras(offset).set_is_used(true);

            return offset;
        }

        inline id_type DoubleArrayBuilder::find_valid_offset(id_type id) const
        {
            if (extras_head_ >= units_.size())
            {
                return units_.size() | (id & LOWER_MASK);
            }

            id_type unfixed_id = extras_head_;
            do
            {
                id_type offset = unfixed_id ^ labels_[0];
                if (is_valid_offset(id, offset))
                {
                    return offset;
                }
                unfixed_id = extras(unfixed_id).next();
            } while (unfixed_id != extras_head_);

            return units_.size() | (id & LOWER_MASK);
        }

        inline bool DoubleArrayBuilder::is_valid_offset(id_type id,
            id_type offset) const
        {
            if (extras(offset).is_used())
            {
                return false;
            }

            id_type rel_offset = id ^ offset;
            if ((rel_offset & LOWER_MASK) && (rel_offset & UPPER_MASK))
            {
                return false;
            }

            for (std::size_t i = 1; i < labels_.size(); ++i)
            {
                if (extras(offset ^ labels_[i]).is_fixed())
                {
                    return false;
                }
            }

            return true;
        }

        inline void DoubleArrayBuilder::reserve_id(id_type id)
        {
            if (id >= units_.size())
            {
                expand_units();
            }

            if (id == extras_head_)
            {
                extras_head_ = extras(id).next();
                if (extras_head_ == id)
                {
                    extras_head_ = units_.size();
                }
            }
            extras(extras(id).prev()).set_next(extras(id).next());
            extras(extras(id).next()).set_prev(extras(id).prev());
            extras(id).set_is_fixed(true);
        }

        inline void DoubleArrayBuilder::expand_units()
        {
            id_type src_num_units = units_.size();
            id_type src_num_blocks = num_blocks();

            id_type dest_num_units = src_num_units + BLOCK_SIZE;
            id_type dest_num_blocks = src_num_blocks + 1;

            if (dest_num_blocks > NUM_EXTRA_BLOCKS)
            {
                fix_block(src_num_blocks - NUM_EXTRA_BLOCKS);
            }

            units_.resize(dest_num_units);

            if (dest_num_blocks > NUM_EXTRA_BLOCKS)
            {
                for (std::size_t id = src_num_units; id < dest_num_units; ++id)
                {
                    extras(id).set_is_used(false);
                    extras(id).set_is_fixed(false);
                }
            }

            for (id_type i = src_num_units + 1; i < dest_num_units; ++i)
            {
                extras(i - 1).set_next(i);
                extras(i).set_prev(i - 1);
            }

            extras(src_num_units).set_prev(dest_num_units - 1);
            extras(dest_num_units - 1).set_next(src_num_units);

            extras(src_num_units).set_prev(extras(extras_head_).prev());
            extras(dest_num_units - 1).set_next(extras_head_);

            extras(extras(extras_head_).prev()).set_next(src_num_units);
            extras(extras_head_).set_prev(dest_num_units - 1);
        }

        inline void DoubleArrayBuilder::fix_all_blocks()
        {
            id_type begin = 0;
            if (num_blocks() > NUM_EXTRA_BLOCKS)
            {
                begin = num_blocks() - NUM_EXTRA_BLOCKS;
            }

            id_type end = num_blocks();

            for (id_type block_id = begin; block_id != end; ++block_id)
            {
                fix_block(block_id);
            }
        }

        inline void DoubleArrayBuilder::fix_block(id_type block_id)
        {
            id_type begin = block_id * BLOCK_SIZE;
            id_type end = begin + BLOCK_SIZE;

            id_type unused_offset = 0;
            for (id_type offset = begin; offset != end; ++offset)
            {
                if (!extras(offset).is_used())
                {
                    unused_offset = offset;
                    break;
                }
            }

            for (id_type id = begin; id != end; ++id)
            {
                if (!extras(id).is_fixed())
                {
                    reserve_id(id);
                    units_[id].set_label(static_cast<uchar_type>(id ^ unused_offset));
                }
            }
        }
    }  // namespace Details


    template <typename A, typename B, typename T, typename C>
    int DoubleArrayImpl<A, B, T, C>::build(std::size_t num_keys,
        const key_type * const *keys, const std::size_t *lengths,
        const value_type *values, int (*progress_func)(std::size_t, std::size_t))
    {
        /* 初始化字符串及其属性集合 */
        Details::Keyset<value_type> keyset(num_keys, keys, lengths, values);

        /* 初始化双数组构建对象 */
        Details::DoubleArrayBuilder builder(progress_func);

        /* 构建双数组 */
        builder.build(keyset);

        /* 保存构建完毕的双数组 */
        std::size_t size = 0;
        unit_type *buf = NULL;
        builder.copy(&size, &buf);

        clear();

        size_ = size;
        array_ = buf;
        buf_ = buf;

        if (progress_func != NULL)
        {
            progress_func(num_keys + 1, num_keys + 1);
        }

        return 0;
    }

}  // namespace Darts

#undef DARTS_INT_TO_STR
#undef DARTS_LINE_TO_STR
#undef DARTS_LINE_STR
#undef DARTS_THROW

#endif  /* _PECKER_DARTS_H */

