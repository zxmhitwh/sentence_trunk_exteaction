/**
* @file darts.h  ˫�������ݽṹ
*       �ýṹ������DAWG����������ɵ�˫�����С
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

        /** �������� */
        typedef char          char_type;
        typedef unsigned char uchar_type;
        typedef int           value_type;
        typedef unsigned int  id_type;


        /**
        * @brief ����˫����洢��Ԫbase.check
        */
        class DoubleArrayUnit
        {
        public:
            /**
            * @brief ˫����洢��Ԫ
            */
            DoubleArrayUnit() : unit_()
            {
            }

            /**
            * @brief �жϵ�ǰ�ڵ��Ƿ���Ҷ�ӽڵ�
            */
            bool has_leaf() const
            { 
                return ((unit_ >> 8) & 1) == 1; 
            }

            /**
            * @brief ��ȡ��ǰ˫���鵥Ԫֵ
            */
            value_type value() const
            { 
                return static_cast<value_type>(unit_ & ((1U << 31) - 1)); 
            }

            /**
            * @brief ��ȡcheckֵ
            */
            id_type label() const
            { 
                return unit_ & ((1U << 31) | 0xFF);
            }

            /**
            * @brief ��ȡbaseֵ
            */
            id_type offset() const
            { 
                return (unit_ >> 10) << ((unit_ & (1U << 9)) >> 6);
            }

        private:
            id_type unit_;
        };


        /**
        * @brief ���������
        */
        class Exception : public std::exception
        {
        public:
            /**
            * @brief ��ʽ���幹�캯����ֹ��ʽ���øú���
            *
            * @param msg ������Ϣ
            */
            explicit Exception(const char *msg = NULL) throw() : msg_(msg)
            {
            }

            /**
            * @brief �������캯��
            *
            * @param rhs �������
            */
            Exception(const Exception &rhs) throw() : msg_(rhs.msg_) 
            {
            }

            /**
            * @brief ��������
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
            * @brief ��˽��������ʽ�������ֵ����
            *        ��ֹ�ú���������
            */
            Exception &operator=(const Exception &);
        };
    } // namespace Details


    /**
    * @brief ˫����ʵ����
    */
    template <typename, typename, typename T, typename>
    class DoubleArrayImpl
    {
    public:
        /** ��������                        */
        typedef T                  value_type;
        /** ������                          */
        typedef Details::char_type key_type;
        /** �������                        */
        typedef value_type         result_type;

        /** ��������� */
        struct result_pair_type
        {
            /** �������� */
            value_type  value;
            /** �������� */
            std::size_t length;
        };

        /**
        * @brief ���캯��
        *        ��ʼ����Ա����
        */
        DoubleArrayImpl() : size_(0), array_(NULL), buf_(NULL) 
        {
        }

        /**
        * @brief ��������
        *        �����ڴ�
        */
        virtual ~DoubleArrayImpl() 
        { 
            clear(); 
        }

        /**
        * @brief �趨����ֵ
        *
        * @param result �������Ե�ַ
        * @param value  ����ֵ
        */
        inline void set_result(value_type *result, value_type value, std::size_t) const;

        /**
        * @brief �趨�������Գ�����Ϣ
        *
        * @param result �������Գ��Ƚṹ��
        * @param value  ��������
        * @param length ��������
        */
        inline void set_result(result_pair_type *result, value_type value, std::size_t length) const;

        /**
        * @brief �趨˫����array
        * 
        * @param ptr  �µ�array��ַ
        * @param size array��Ԫ��
        */
        void set_array(const void *ptr, std::size_t size = 0);

        /**
        * @brief ����˫����array
        *
        * @return ˫����array
        */
        const void *array() const { return array_; }

        /**
        * @brief �ͷ�˫����ռ�õ��ڴ�
        */
        void clear();

        /**
        * @brief ����˫���鵥Ԫ��С
        */
        std::size_t unit_size() const 
        { 
            return sizeof(unit_type);
        }

        /**
        * @brief ����˫���鵥Ԫ��
        */
        std::size_t size() const 
        { 
            return size_; 
        }

        /**
        * @brief ����˫�����ܵ��ڴ�����
        */
        std::size_t total_size() const
        { 
            return unit_size() * size(); 
        }

        /**
        * @brief ����˫����Ԫ��
        */
        std::size_t nonzero_size() const 
        { 
            return size(); 
        }

        /**
        * @brief ����˫����
        *
        * @param num_keys  ������
        * @param keys      ��������
        * @param lengths   ������������
        * @param values    ������������
        * @param progress_func �������ȴ�ӡ����
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
        * @brief ����˫����
        *
        * @param file_name ˫����洢�ļ�
        * @param mode      �򿪷�ʽ
        * @param offset    �ļ�ƫ��λ��
        * @param size      ˫�����С
        *
        * @return �ɹ�����0�����򷵻�-1
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
        * @brief ����˫����
        *
        * @param file_name ˫����洢�ļ�
        * @param mode      �ļ��򿪷�ʽ
        * @param offset    ����ƫ��λ��
		// save() writes the array of units into the specified file. `offset'
		// specifies the number of bytes to be skipped before writing the array.
		// open() returns 0 iff the operation succeeds. Otherwise, it returns a
		// non-zero value.

        */
        int save(const char *file_name, const char *mode = "wb",
            std::size_t offset = 0) const;

        /**
        * @brief ȷ������ƥ��
        * 
        * @param key    ��ƥ�����
        * @param length ��������
        * @param node_pos ��˫������ƥ�俪ʼλ��
        *
        * @return ������Ӧ����
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
		* @brief ����ǰ׺����ƥ��
		* 
		* @param key               ��ƥ���ַ���
		* @param results           ����ƥ�䵽�����д���buffer
		* @param max_num_results   buffer��С
		* @param length            �ַ�������
		* @param node_pos          ��˫������ƥ�俪ʼλ��
		*
		* @return ƥ�䵽�Ĵ�����
		*/
		template <class U>
		inline std::size_t commonPrefixSearch(const key_type *key, U *results,
			std::size_t max_num_results, std::size_t length = 0,
			std::size_t node_pos = 0) const;



		/**
		* @brief ����ƥ��һ���ַ���
		*
		* @param key      ��ƥ���ַ���
		* @param node_pos ����ƥ�俪ʼ����ƥ�����λ��
        * @param key_pos  �ַ���ƥ�俪ʼλ��
        * @param length   ��ƥ�䳤��
        *
        * @return ƥ�䵽�������ظô�����Ӧ������
        *         ƥ��ɹ��ַ������أ�        -1
        *         δƥ��ɹ��ַ������أ�      -2

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
        /** �ַ�����                              */
        typedef Details::uchar_type      uchar_type;
        /** ����id                               */
        typedef Details::id_type         id_type;
        /** ˫���鵥Ԫ����                       */
        typedef Details::DoubleArrayUnit unit_type;

        /** ˫����洢��Ԫ��                    */
        std::size_t                      size_;
        /** ָ��˫����洢��Ԫ                  */
        const unit_type*                 array_;
        /** ����˫����buffer */
        unit_type*                       buf_;

        /**
        *@brief ��ʽ���忽�����캯���͸�ֵ�������캯����ֹ������
        */
        DoubleArrayImpl(const DoubleArrayImpl &);
        DoubleArrayImpl &operator=(const DoubleArrayImpl &);
    };

    /** ˫��������, DoubleArray ��һ�����͵�˫�����÷�����ʹ��int��Ϊֵ���ͣ��ʺϴ�������*/
    typedef DoubleArrayImpl<void, void, int, void> DoubleArray;


//�����ǽӿڶ��岿�֣�������ʵ�ֲ���
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
        * @brief ���������ڴ�
        */
        template <typename T>
        class AutoArray
        {
        public:
            /**
            * @brief ���캯��
            *
            * @param array ���ڳ�ʼ�������ַ
            */
            explicit AutoArray(T *array = NULL) : array_(array)
            {
            }

            /**
            * @brief ��������
            *        �����ڴ�����
            */
            ~AutoArray() 
            { 
                clear(); 
            }

            /**
            * @brief ��ȡָ��λ�õ����鵥Ԫ
            *
            * @param id Ҫ��ȡ�����鵥Ԫ����
            *
            * @return ���鵥Ԫ��������
            */
            const T &operator[](std::size_t id) const 
            {
                return array_[id];
            }

            /**
            * @brief ��ȡָ��λ�õ����鵥Ԫ
            *
            * @param id Ҫ��ȡ�����鵥Ԫ����
            *
            * @return ���鵥Ԫ����
            */
            T &operator[](std::size_t id)
            { 
                return array_[id]; 
            }

            /**
            * @brief �ж������Ƿ�Ϊ��
            *
            * @return ����Ϊ�շ���true�����򷵻�false
            */
            bool empty() const 
            {
                return array_ == NULL;
            }

            /**
            * @brief ���������ڴ�
            */
            void clear();

            /**
            * @brief ��������array�뵱ǰ���������
            *
            * @param array ����������
            */
            void swap(AutoArray *array);

            /**
            * @brief ����ǰ����buffer��Ϊarray
            *
            * @param array ���趨������
            */
            void reset(T *array = NULL) 
            {
                AutoArray(array).swap(this); 
            }

        private:
            /** ָ�����黺���� */
            T *     array_;

            /**
            * @brief ���������캯���͸�ֵ��������
            *        ��������Ϊ˽�г�Ա��ֹ������
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
            * @brief ����ָ����Ԫλ�õĳ�������
            *
            * @param id ָ����Ԫλ��
            *
            * @return ָ����Ԫ��Ӧ�ĳ�������
            */
            const T &operator[](std::size_t id) const;

            /**
            * @brief ����ָ����Ԫλ�õı�������
            *
            * @param id ָ����Ԫλ��
            *
            * @return ָ����Ԫ��Ӧ�ı�������
            */
            T &operator[](std::size_t id);

            /**
            * @brief �жϵ�ǰ�洢Ԫ���Ƿ�Ϊ��
            *
            * @return �շ���true������false
            */
            bool empty() const 
            {
                return size_ == 0;
            }

            /**
            * @brief ���ص�ǰ�洢�ĵ�Ԫ��
            *
            * @return ��ǰ�洢�ĵ�Ԫ��
            */
            std::size_t size() const 
            {
                return size_;
            }

            /**
            * @brief �����ڴ�
            */
            void clear();

            /**
            * @brief ����ǰԪ����ӵ���������β
            *
            * @param value ������ĵ�Ԫ
            */
            void push_back(const T &value) 
            { 
                append(value);
            }

            /**
            * @brief �ӽ�β���Ƴ�һ����Ԫ
            */
            void pop_back();

            /**
            * @brief �ڻ�������β���һ����Ԫ
            *        ��ʹ�õ�Ԫ�Ĺ��캯����ʼ��
            */
            void append();

            /**
            * @brief �ڻ�������β���һ����Ԫ
            */
            void append(const T &value);

            /**
            * @brief ������Ĵ洢��Ԫ������Ϊsize
            *        a. ����ǰ�Ĵ洢��Ԫ������size�����ͷų���size�Ĵ洢��Ԫ
            *        b. ����ǰ�Ĵ洢��Ԫ��С��size��������µĴ洢��Ԫ��ʹ�õ�Ԫ���캯����ʼ��
            *
            * @param size ������Ĵ洢��Ԫ��
            */
            void resize(std::size_t size);

            /**
            * @brief ������Ĵ洢��Ԫ������Ϊsize
            *        a. ����ǰ�Ĵ洢��Ԫ������size�����ͷų���size�Ĵ洢��Ԫ
            *        b. ����ǰ�Ĵ洢��Ԫ��С��size��������µĴ洢��Ԫ��ʹ��value��ʼ��
            *
            * @param size  ������Ĵ洢��Ԫ��
            * @param value ���ڳ�ʼ���´洢��Ԫ
            */
            void resize(std::size_t size, const T &value);

            /**
            * @brief ����洢�ռ�
            *
            * @pram size ������洢�ռ��С
            */
            void reserve(std::size_t size);

        private:
            /** �ַ�������         */
            AutoArray<char>     buf_;
            /** �洢���ַ���      */
            std::size_t         size_;
            /** �ɴ洢���ַ���    */
            std::size_t         capacity_;

            /**
            * @brief ��������buffer�ռ�Ϊsize
            *        ��ԭ����buffer�ǿգ��򽫵�Ԫ�������µ�Ԫ��
            *
            * @param size Ҫ����������buffer��С��С 
            */
            void resize_buf(std::size_t size);

            /**
            * @brief �����캯���Ϳ������캯��
            *        ����Ϊ˽�г�Ա��ֹ������
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
        * @brief ջ
        */
        template <typename T>
        class AutoStack
        {
        public:
            /**
            * @brief ���캯��
            */
            AutoStack() : pool_() 
            {
            }

            /**
            * @brief ��������
            */
            ~AutoStack() 
            {
                clear();
            }

            /**
            * @brief ��ȡջ��Ԫ�س�������
            */
            const T &top() const
            { 
                return pool_[size() - 1]; 
            }

            /**
            * @brief ��ȡջ��Ԫ�ر�������
            */
            T &top() 
            { 
                return pool_[size() - 1]; 
            }

            /**
            * @brief �ж�ջ�Ƿ�Ϊ��
            */
            bool empty() const 
            { 
                return pool_.empty(); 
            }

            /**
            * @brief ��ȡջ�б���Ԫ����
            */
            std::size_t size() const
            { 
                return pool_.size(); 
            }

            /**
            * @brief ��Ԫ��ѹջ
            * 
            * @param value ��ѹջԪ��
            */
            void push(const T &value)
            { 
                pool_.push_back(value); 
            }

            /**
            * @brief ����ջ��Ԫ��
            */
            void pop() 
            {
                pool_.pop_back();
            }

            /**
            * @brief ����ջ�ڴ�
            */
            void clear() 
            { 
                pool_.clear(); 
            }

        private:
            /** brief �������ݵ�Ԫbuffer */
            AutoPool<T>         pool_;

            /**
            * @brief ��ʽ���幹�캯���͸�ֵ����
            *        ���캯��Ϊ˽�г�Ա��ֹ������
            */
            AutoStack(const AutoStack &);
            AutoStack &operator=(const AutoStack &);
        };


        /**
        * @brief λ����
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
            * @brief �ж�ָ����λ�Ƿ����
            *
            * @param id λ�±�
            *
            * @return λ���ڷ���true�����򷵻�false
            */
            bool operator[](std::size_t id) const;

            id_type rank(std::size_t id) const;

            /**
            * @brief �趨ָ����λ
            *
            * @param id  λ�±�
            * @param bit ָ����λ
            */
            void set(std::size_t id, bool bit);

            /**
            * @brief �жϴ洢λ�������Ƿ�Ϊ��
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
            * @brief λ������Ԫ��
            */
            std::size_t size() const 
            {
                return size_;
            }

            /**
            * @brief �ڱ���λ�����Ĵ洢buffer
            *        ĩβ���һ����Ԫ���ڴ洢λ
            */
            void append();

            /**
            * @brief ����rank����
            */
            void build();

            /**
            * @brief �ͷ����ڴ洢λ��Ϣ��buffer
            */
            void clear();

        private:
            enum { UNIT_SIZE = sizeof(id_type) * 8 };

            /** ����λ��Ϣ           */
            AutoPool<id_type>       units_;

            /** ����λ��Ӧ�Ĺ�ϣ��Ϣ */
            AutoArray<id_type>      ranks_;

            /** ����λ��Ӧ�Ĺ�ϣֵ */
            std::size_t             num_ones_;

            /** ����units_��Ӧ��λ��Ա�� */
            std::size_t             size_;

            /**
            * @brief ����unit��Ӧ�Ĺ�ϣֵ
            *
            * @param unit �����ɹ�ϣֵ����
            */
            static id_type pop_count(id_type unit);

            /**
            * @brief ��ʽ���幹�캯���͸�ֵ����
            *        ���캯��Ϊ˽�г�Ա��ֹ������
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
        * @brief �ַ�����
        *        T -- �ַ������Զ�Ӧ������
        */
        template <typename T>
        class Keyset
        {
        public:
            /**
            * @brief ��ʼ�����ڹ���˫������ַ�����
            *
            * @param num_keys �ַ�������
            * @param keys     �ַ���
            * @param length   ���ַ�������
            * @param values   ���ַ�����Ӧ��value
            */
            Keyset(std::size_t num_keys, const char_type * const *keys,
                const std::size_t *lengths, const T *values):
            num_keys_(num_keys), keys_(keys), lengths_(lengths), values_(values)
            {
            }

            /**
            * @brief �����ַ�������
            */
            std::size_t num_keys() const 
            { 
                return num_keys_; 
            }

            /**
            * @brief ����ȷ��������Ӧ���ַ���
            *
            * @param id �ַ�������
            *
            * @return �ַ���
            */
            const char_type *keys(std::size_t id) const 
            { 
                return keys_[id]; 
            }

            /**
            * @brief ����ȷ���ַ����ж�Ӧ��id
            *
            * @param key_id  �ַ�������
            * @param char_id �ַ�����
            *
            * @return �ַ�
            */
            uchar_type keys(std::size_t key_id, std::size_t char_id) const;

            /**
            * @brief �ж��Ƿ����ַ�����������
            *
            * @return ���ַ����������鷵��true�����򷵻�false
            */
            bool has_lengths() const
            { 
                return lengths_ != NULL; 
            }

            /**
            * @brief �����ַ������ж�Ӧ�ַ����ĳ���
            *
            * @param id �ַ������ַ������е�����
            *
            * @return �ַ�������
            */
            std::size_t lengths(std::size_t id) const;

            /**
            * @brief �ж��Ƿ����ַ�����������
            *
            * @return ���������鷵��true�����򷵻�false
            */
            bool has_values() const 
            {
                return values_ != NULL; 
            }

            /**
            * @brief �����ַ������ж�Ӧ�ַ���������
            *
            * @param id �ַ������ַ������е�����
            *
            * @return �ַ�������
            */
            const value_type values(std::size_t id) const;

        private:
            /** �ַ�������                    */
            std::size_t               num_keys_;
            /** �ַ�����                     */
            const char_type * const   *keys_;
            /** �ַ�������                   */
            const std::size_t         *lengths_;
            /** �ַ�������                  */
            const T                   *values_;

            /**
            * @brief ��ʽ�����������캯���͸�ֵ��������
            *        ������private�����Է�������
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
        * @brief �����޻�ͼ(Directed Acyclic Word Graph)�ڵ�
        */
        class DawgNode
        {
        public:
            /**
            * @brief ���캯��
            *        ��ʼ�������޻�ͼ�ڵ�
            */
            DawgNode() : child_(0), sibling_(0), label_('\0'),
                is_state_(false), has_sibling_(false) 
            {
            }

            /**
            * @brief �趨�ӽڵ�
            *
            * @param child �ӽڵ�
            */
            void set_child(id_type child) 
            { 
                child_ = child; 
            }

            /**
            * @brief �趨�ھӽڵ�
            * 
            * @param sibling �ھӽڵ�
            */
            void set_sibling(id_type sibling) 
            { 
                sibling_ = sibling; 
            }

            /**
            * @brief �趨��ǰ������Ӧ������
            *
            * @param value ��ǰ������Ӧ������
            */
            void set_value(value_type value) 
            { 
                child_ = value; 
            }

            /**
            * @brief �趨��ǰ�ڵ��Ӧ���ַ�
            *
            * @param label ��ǰ�ڵ��Ӧ���ַ�
            */
            void set_label(uchar_type label) 
            { 
                label_ = label; 
            }

            /**
            * @brief �趨��ǰ�ڵ��Ƿ��Ӧ�����м�״̬
            *
            * @param is_state ָʾ�Ƿ��Ӧ�����м�״̬
            */
            void set_is_state(bool is_state) 
            { 
                is_state_ = is_state; 
            }

            /**
            * @brief �趨��ǰ�ڵ��Ƿ����ھӽڵ�
            *
            * @param has_sibling ָʾ��ǰ�ڵ��Ƿ����ھ�
            */
            void set_has_sibling(bool has_sibling) 
            { 
                has_sibling_ = has_sibling; 
            }

            /**
            * @brief ���ص�ǰ�ڵ���ӽڵ�
            */
            id_type child() const
            { 
                return child_; 
            }

            /**
            * @brief ���ص�ǰ�ڵ���ھӽڵ�
            */
            id_type sibling() const 
            {
                return sibling_; 
            }

            /**
            * @brief ���ص�ǰ������Ӧ������
            */
            value_type value() const 
            { 
                return static_cast<value_type>(child_); 
            }

            /**
            * @brief ���ص�ǰ�ڵ��Ӧ���ַ�
            */
            uchar_type label() const 
            { 
                return label_; 
            }

            /**
            * @brief ���ص�ǰ�ڵ��Ƿ��Ǵ����м�״̬
            */
            bool is_state() const 
            { 
                return is_state_; 
            }

            /**
            * @brief ���ص�ǰ�ڵ��Ƿ����ھӽڵ�
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
            /** �ӽڵ� */
            id_type     child_;

            /** �ھӽڵ� */
            id_type     sibling_;

            /** ��ǰ�ڵ��Ӧ���ַ� */
            uchar_type  label_;

            /** �Ƿ�Ϊ�����м�״̬ */
            bool        is_state_;

            /** �Ƿ����ھӽڵ� */
            bool        has_sibling_;
        };

        /**
        * @brief �����޻�ͼ(Directed Acyclic Word Graph)��Ԫ
        */
        class DawgUnit
        {
        public:
            /**
            * @brief ���캯��
            */
            DawgUnit(id_type unit = 0) : unit_(unit) 
            {
            }

            /**
            * @brief �������캯��
            */
            DawgUnit(const DawgUnit &unit) : unit_(unit.unit_) 
            {
            }

            /**
            * @brief ��ֵ����
            */
            DawgUnit &operator=(id_type unit)
            {
                unit_ = unit;

                return *this; 
            }

            /**
            * @brief ����DAWG�ڵ�
            */
            id_type unit() const 
            {
                return unit_;
            }

            /**
            * @brief ���ؽڵ��ӽڵ�
            */
            id_type child() const 
            { 
                return unit_ >> 2;
            }

            /**
            * @brief �жϽڵ��Ƿ����ھӽڵ�
            */
            bool has_sibling() const
            { 
                return (unit_ & 1) == 1; 
            }

            /**
            * @brief ��ȡ�ڵ�ֵ�������ӽڵ���Ƿ�Ϊ�м�ڵ���Ϣ
            */
            value_type value() const 
            { 
                return static_cast<value_type>(unit_ >> 1);
            }

            /**
            * @brief �жϵ�ǰ�ڵ��Ƿ�Ϊ�м�ڵ�
            */
            bool is_state() const
            { 
                return (unit_ & 2) == 2;
            }

        private:
            /** ���������޻�ͼ��Ԫ��Ϣ */
            id_type unit_;
        };


        /**
        * @brief �����޻�ͼ(Directed Acyclic Word Graph)������
        *
        * ����ͨ��ʹ��ջ(node_stack_)��������(nodes_)���ϵĲ���
        * ������DAWG�����յ�DAWGͨ��unit_��labels����ʾ��table_��
        * ���ڲ���
        */
        class DawgBuilder
        {
        public:
            /**
            * @brief ���캯��
            *        ��ʼ������Ա����
            */
            DawgBuilder() : nodes_(), units_(), labels_(), is_intersections_(),
                table_(), node_stack_(), recycle_bin_(), num_states_(0)
            {
            }

            /**
            * @brief ��������
            */
            ~DawgBuilder() 
            { 
                clear(); 
            }

            /**
            * @brief ���ظ��ڵ�
            */
            id_type root() const 
            { 
                return 0;
            }

            /**
            * @brief ���ص�ǰ�ڵ��ӽڵ�
            *
            * @param id ��ǰ�ڵ�id
            *
            * @return ��ǰ�ڵ��ӽڵ�
            */
            id_type child(id_type id) const 
            {
                return units_[id].child(); 
            }

            /**
            * @brief ���ص�ǰ�ڵ��ھӽڵ�
            *
            * @param id ��ǰ�ڵ�id
            *
            * @return ��ǰ�ڵ��ھӽڵ㣬���ھӽڵ㷵��0
            */
            id_type sibling(id_type id) const
            { 
                return units_[id].has_sibling() ? id + 1 : 0;
            }

            /**
            * @brief ��ȡ�ڵ�ֵ�������ӽڵ���Ƿ�Ϊ�м�ڵ���Ϣ
            *
            * @param id ��ǰ�ڵ�id
            *
            * @return �ڵ�ֵ
            */
            int value(id_type id) const 
            { 
                return units_[id].value(); 
            }

            /**
            * @brief �жϵ�ǰ�ڵ��Ƿ�ΪҶ�ӽڵ�
            */
            bool is_leaf(id_type id) const 
            { 
                return label(id) == '\0'; 
            }

            /**
            * @brief ��ȡ�ڵ��Ӧ���ַ�
            *
            * @param id �ڵ�id
            *
            * @return �ڵ��Ӧ���ַ�
            */
            uchar_type label(id_type id) const 
            { 
                return labels_[id];
            }

            /**
            * @brief �жϵ�ǰ�ڵ��Ƿ�Ϊ����ڵ�
            *
            * @param id �ڵ�id
            * 
            * @return �Ƿ�Ϊ����ڵ�
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
            * @brief ��ȡ��ǰ����ڵ㱻�������
            *
            * @param id �ڵ�id
            * 
            * @return �������
            */
            std::size_t num_intersections() const
            { 
                return is_intersections_.num_ones(); 
            }

            /**
            * @brief ����units_�е�Ԫ��
            */
            std::size_t size() const 
            { 
                return units_.size();
            }

            /**
            * @brief ��ʼ��DAWG
            */
            void init();

            /**
            * @brief ��nodes_�е�ʣ��ڵ���DAWG��ʽ������units_��labels_��
            *        ������nodes_��node_stack_��recycle_bin_��ռ���ڴ�
            */
            void finish();

            /**
            * @brief ��һ���������뵽DAWG��
            *
            * @param key    ����
            * @param length �ʳ�
            * @param value  ����ֵ
            */
            void insert(const char *key, std::size_t length, value_type value);

            /**
            * @brief ����ڴ�����
            */
            void clear();

            /**
            * @brief ��ӡ�����޻�ͼ
            */
            void printDawg(void);

        private:
            enum { INITIAL_TABLE_SIZE = 1 << 10 };

            /** �洢�����޻�ͼ�ڵ�� */
            AutoPool<DawgNode>      nodes_;

            /** �����޻�ͼ��Ԫ��ÿ����Ԫ����������Ϣ�ӽڵ�|�Ƿ�Ϊ�м�״̬|�Ƿ����ھ� */
            AutoPool<DawgUnit>      units_;

            /** �����ַ� */
            AutoPool<uchar_type>    labels_;

            /** ��¼��ǰ�ڵ��Ƿ������ڵ㹲�� */
            BitVector               is_intersections_;

            /** �洢�����޻�ͼ�ڵ����� */
            AutoPool<id_type>       table_;

            /** ���ڽ������޻�ͼ�ڵ�ѹջ */
            AutoStack<id_type>      node_stack_;

            /** ���ڻ��������޻�ͼ�ڵ� */
            AutoStack<id_type>      recycle_bin_;

            /** ���ڼ�¼״̬�� */
            std::size_t             num_states_;

            /**
            * @brief ��node_stack_�нڵ�id֮�ϵ����нڵ㱣�浽units_��labels_��table_��
            *
            * @param id ��ǰ�ڵ�id
            */
            void flush(id_type id);

            /**
            * @brief ����table_��ϣ���С
            */
            void expand_table();

            /**
            * @brief ��λ��id��table_��ϣ���еĴ洢λ��
            *
            * @param id      Ҫ�洢����ϣ���е���
            * @param hash_id ��ϣ���д洢id�ĵ�Ԫ����
            *
            * @return 0
            */
            id_type find_unit(id_type id, id_type *hash_id) const;

            /**
            * @brief �ж�table_�����Ƿ��Ѿ�����node_id��Ӧ�Ľڵ�
            *        ��ν����ָ���Ǵ���һ���ڵ㼰���ھӾ���node_id��Ӧ�Ľڵ�һ��
            * 
            * @param node_id ��Ҫ��table_���в�ѯ�Ƿ���ڵĽڵ��Ӧ������
            * @param hash_id ����table_�п��Դ洢node_id��Ӧ�Ľڵ�λ��
            *
            * @return �����ڷ���table_���ж�Ӧ����Ƚڵ����������򷵻�0
            */
            id_type find_node(id_type node_id, id_type *hash_id) const;

            /**
            * @brief �ж������ڵ��Ƿ����
            *        �ڵ����ָ���������ڵ��������ھӾ���Ӧ���
            * 
            * @param node_id �ڵ�1����
            * @param unit_id �ڵ�2����
            *
            * @return ��ȷ���true�����򷵻�false
            */
            bool are_equal(id_type node_id, id_type unit_id) const;

            /**
            * @brief ����ڵ��Ӧ�Ĺ�ϣֵ
            *
            * @param id �ڵ�id
            *
            * @return �ڵ��Ӧ�Ĺ�ϣֵ
            */
            id_type hash_unit(id_type id) const;

            /**
            * @brief ����ڵ��Ӧ�Ĺ�ϣֵ
            *
            * @param id �ڵ�id
            *
            * @return �ڵ��Ӧ�Ĺ�ϣֵ
            */
            id_type hash_node(id_type id) const;

            /**
            * @brief �������޻�ͼ�ڵ�������һ���ڵ�
            */
            id_type append_node();

            /**
            * @brief �������޻�ͼ��Ԫ�������һ����Ԫ
            */
            id_type append_unit();

            /**
            * @brief �������޻�ͼ�ڵ���е�һ���ڵ����
            */
            void free_node(id_type id);

            /**
            * @brief ����key��Ӧ�Ĺ�ϣֵ
            *
            * @param key �����ɹ�ϣֵ��
            *
            * @return ���صĹ�ϣֵ
            */
            static id_type hash(id_type key);

            /**
            * @brief ��ʽ���忽�����캯���͸�ֵ�������캯��
            *        Ϊ˽�г�Ա��ֹ������ʱʹ��Ĭ����ʽ����
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

            /* DAWG�ڵ������������ֵΪ0��ʾ�Ӹ��ڵ㿪ʼ���� */
            id_type     id      = 0;

            /* ��ǰҪ�洢���ַ����ַ��������� */
            std::size_t key_pos = 0;

            /* ����DAWGͼ�ҵ���ǰ�ַ������ַ���Ҫ�洢�Ľڵ�λ�� */
            for ( ; key_pos <= length; ++key_pos)
            {
                /* ��ȡ��ǰ�ڵ��ӽڵ����� */
                id_type child_id = nodes_[id].child();
                if (child_id == 0)
                {
                    break;
                }

                /* ��ȡ��ǰҪ�洢������Ӧ�ַ� */
                uchar_type key_label = static_cast<uchar_type>(key[key_pos]);
                if (key_pos < length && key_label == '\0')
                    DARTS_THROW("failed to insert key: invalid null character");

                /* ��ȡ�洢�ڵ�ǰ�ڵ��ӽڵ��ж�Ӧ���ַ� */
                uchar_type unit_label = nodes_[child_id].label();

                if (key_label < unit_label)
                {
                    /* ����ǰ�ַ�С���Ѵ洢�ַ�˵��Ҫ�洢���ַ���˳����ȷ */
                    DARTS_THROW("failed to insert key: wrong key order");
                }
                else if (key_label > unit_label)
                {
                    /* ����ǰ�ַ������Ѵ洢�ַ�˵���µ��ַ���Ҫ�洢 */

                    /* ����ǰ�ڵ��ӽڵ���Ϊ�����ھӽڵ� */
                    nodes_[child_id].set_has_sibling(true);

                    /* ����ǰ�ڵ㼰�����к����ӽڵ�洢��unit_��label_��table_�ȱ���
                    * a. ����unit_���и���Ԫ֮������ӹ�ϵ
                    * b. ����table_���ñ�洢node_���нڵ��Ӧ�ĵ�Ԫ�洢λ��
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
                /* ��ȡ������Ӧ��Ҫ�洢�ַ� */
                uchar_type key_label = static_cast<uchar_type>(
                    (key_pos < length) ? key[key_pos] : '\0');

                /* ��ȡһ���ӽڵ����ڴ洢��ǰ�ַ� */
                id_type child_id = append_node();

                if (nodes_[id].child() == 0)
                {
                    /* ���ӽڵ���Ϊ�м�״̬�ڵ� */
                    nodes_[child_id].set_is_state(true);
                }

                /* �����ڵ���ӽڵ���Ϊ��ǰ�ӽڵ���ھӽڵ� */
                nodes_[child_id].set_sibling(nodes_[id].child());

                /* �趨��ǰ�ӽڵ��Ӧ���ַ� */
                nodes_[child_id].set_label(key_label);

                /* ����ǰ�ӽڵ���Ϊ���ڵ���ӽڵ� */
                nodes_[id].set_child(child_id);

                /* ����ǰ�ӽڵ�ѹջ */
                node_stack_.push(child_id);

                /* ���ӽڵ���Ϊ���ڵ��Ա㴦���ַ��������ַ� */
                id = child_id;
            }

            /* ���������޻�ͼ���� */
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
            id_type     unit_value;  /* ��Ԫֵ */
            uchar_type  label_value; /* �ַ�ֵ */
            bool        bhassibling; /* ���ھ� */
            bool        bstate;      /* �Ƿ�Ϊ�м�״̬ */
            id_type     child_idx;   /* �ӽڵ� */  

            cerr << "�洢��" << endl;
            for (size_t i = 0; i < units_.size(); i++)
            {
                /* ��ӡ�洢������ */
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

                /* ��ӡ�洢�� */
                cerr << " c:" << child_idx << " bh:" << bhassibling << " bs:" << bstate;
            }
            cerr << endl;

            cerr << "�ַ���" << endl;
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

                /* �����ϣ��洢��С����ϣ��洢��Ϊ(table_.size() - table_.size()/4)/table_.size() */
                if (num_states_ >= table_.size() - (table_.size() >> 2))
                {
                    expand_table();
                }

                /* ͳ�Ƶ�ǰ�ڵ㼰���ھ��� */
                id_type num_siblings = 0;
                for (id_type i = node_id; i != 0; i = nodes_[i].sibling())
                {
                    ++num_siblings;
                }

                id_type hash_id;

                /* ���ҵ�ǰ�ڵ���table_�����Ƿ���ڣ������ص�ǰ��Ԫ��Ӧ�洢λ��(hash_id) */
                id_type match_id = find_node(node_id, &hash_id);
                if (match_id != 0)
                {
                    is_intersections_.set(match_id, true);
                }
                else
                {
                    id_type unit_id = 0;

                    /* Ϊ��ǰ�ڵ㼰�������ھӷ���洢�ռ��Ա�洢 */
                    for (id_type i = 0; i < num_siblings; ++i)
                    {
                        unit_id = append_unit();
                    }

                    for (id_type i = node_id; i != 0; i = nodes_[i].sibling())
                    {
                        /* �洢�����޻�ͼ�ڵ� */
                        units_[unit_id] = nodes_[i].unit();

                        /* �洢��ǰ�ڵ��Ӧ���ַ� */
                        labels_[unit_id] = nodes_[i].label();

                        /* �洢��һ����Ԫ */
                        --unit_id;
                    }

                    /* ��¼��ǰ�ڵ���units_���еĴ洢���� */
                    match_id = unit_id + 1;

                    /* ����ǰ��Ԫ�洢���������ڹ�ϣ���� */
                    table_[hash_id] = match_id;

                    /* ��¼״̬�� */
                    ++num_states_;
                }

                /* ���յ�ǰ�ڵ㼰�������ھӽڵ� */
                for (id_type i = node_id, next; i != 0; i = next)
                {
                    next = nodes_[i].sibling();
                    free_node(i);
                }

                /* �趨���ڵ���ӽڵ�Ϊ��ǰ�ڵ� */
                nodes_[node_stack_.top()].set_child(match_id);
            }
            node_stack_.pop();
        }

        inline void DawgBuilder::expand_table()
        {
            /* �����С����1�� */
            std::size_t table_size = table_.size() << 1;

            /* ���֮ǰ�Ĺ�ϣ�������·�����С */
            table_.clear();
            table_.resize(table_size, 0);

            /* ��units_�еĸ���Ԫ�洢���µĹ�ϣ���� */
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
            /* ��is_intersections�����һ����Ԫ */
            is_intersections_.append();

            /* ��units_�����һ����Ԫ */
            units_.append();

            /* ��labels_�����һ����Ԫ */
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
        * @brief ˫���鹹����Ԫ
        */
        class DoubleArrayBuilderUnit
        {
        public:
            /**
            * @brief ���캯��
            */
            DoubleArrayBuilderUnit() : unit_(0) 
            {
            }

            void set_value(value_type value) 
            {
                unit_ = value | (1U << 31); 
            }

            /**
            * @brief �趨LCHECK����ǰCHECK��Ԫ�б�����ַ�
            *        BASE.CHECK��4���ֽڱ�ʾ����һ���ֽ����ڱ���CHECKֵ
            *
            * @param label ������ת����ǰBC�����Ӧ���ַ�
            */
            void set_label(uchar_type label)
            { 
                unit_ = (unit_ & ~0xFFU) | label; 
            }

            /**
            * @brief ��BASE��Ԫ�ĵ�һ��bit�趨Ϊ0��1
            *        ����ָ����ǰ�ڵ��Ƿ�ΪҶ�ڵ�(1)
            *
            * @param has_leaf ָʾ��ǰ��Ԫ�Ƿ�ΪҶ�ڵ�
            *        Ҷ�ڵ�����ָ��TAIL����
            */
            void set_has_leaf(bool has_leaf);

            /**
            * @brief ����BASEֵ
            */
            void set_offset(id_type offset);

        private:
            /** ˫���鹹����Ԫ����BASE|HAS_LEAF|LCHECK����Ϣ */
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
        * @brief ˫���鹹����
        *        a. ���ַ�����ֱ�ӹ���˫����
        *        b. ���ַ������ȹ���ΪDAWGȻ��ת��Ϊ˫����
        */
        class DoubleArrayBuilder
        {
        public:
            /**
            * @brief ���캯��
            *
            * @param progress_func �������ȴ�ӡ����
            */
            explicit DoubleArrayBuilder(int (*progress_func)(
                std::size_t, std::size_t)) : progress_func_(progress_func),
                units_(), extras_(), labels_(), table_(), extras_head_(0)
            {
            }

            /**
            * @brief ��������
            */
            ~DoubleArrayBuilder() 
            {
                clear(); 
            }

            /**
            * @brief ���ַ���������Ϊ˫����
            *
            * @param keyset �ַ�����
            */
            template <typename T>
            void build(const Keyset<T> &keyset);

            /**
            * @brief �������޻�ͼת�������ɵ�˫������Ϣ���浽buf_ptr��
            *
            * @param size_ptr �����޻�ͼת�������ɵ�˫������Ϣ��Ԫ����ָ��
            * @param buf_ptr  ָ�򱣴�˫�����ַ
            */
            void copy(std::size_t *size_ptr, DoubleArrayUnit **buf_ptr) const;

            /**
            * @brief ���չ���˫��������е��м��ڴ�
            */
            void clear();

        private:
            enum { BLOCK_SIZE = 256 };
            enum { NUM_EXTRA_BLOCKS = 16 };
            enum { NUM_EXTRAS = BLOCK_SIZE * NUM_EXTRA_BLOCKS };

            enum { UPPER_MASK = 0xFF << 21 };
            enum { LOWER_MASK = 0xFF };

            /** ˫���鵥Ԫ���� */
            typedef DoubleArrayBuilderUnit      unit_type;
            typedef DoubleArrayBuilderExtraUnit extra_type;

            /**
            * @brief ���ȴ�ӡ��������
            */
            int (* const progress_func_)(std::size_t, std::size_t);


            /** �����������޻�ͼת�������ɵ�˫������Ϣ */
            AutoPool<unit_type>         units_;

            AutoArray<extra_type>       extras_;

            /** ���浱ǰ�ڵ��ӽڵ㼰�ӽڵ������ھӽڵ��Ӧ���ַ� */
            AutoPool<uchar_type>        labels_;

            /** ��ϣ����˫����ڵ�� */
            AutoArray<id_type>          table_;

            id_type                     extras_head_;


            /**
            * @brief ���ص�Ԫ��
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
            * @brief ʹ���ַ���������DAWG
            *
            * @param keyset �ַ�����
            * @param dawg_builder DAWG��������
            */
            template <typename T>
            void build_dawg(const Keyset<T> &keyset, DawgBuilder *dawg_builder);

            /**
            * @brief ʹ��DAWG����˫����
            *
            * @param dawg DAWGͼ
            */
            void build_from_dawg(const DawgBuilder &dawg);

            /**
            * @brief �ݹ麯�����ڵݹ鹹��DAWG��Ӧ��˫����
            *        ͨ���ݹ���ԣ��ú�������شﵽ�˱�ʾ
            *        DAWGѹ����˼��
            *
            * @param dawg DAWGͼ
            * @param dawg_id DAWGͼ����
            * @param dic_id  ˫��������
            */
            void build_from_dawg(const DawgBuilder &dawg,
                id_type dawg_id, id_type dic_id);

            /**
            * @brief ��DAWGͼ��ǰ�ڵ㼰�������ھӽڵ㱣�浽˫������
            *        ͬʱ����˫���鵱ǰ�ڵ��offsetֵ
            *
            * @param dawg     DAWGͼ
            * @param dawg_id  DAWGͼ�ڵ�����
            * @param dic_id   ˫����ڵ�����
            *
            * @return ˫���鵱ǰ�ڵ��offsetֵ
            */
            id_type arrange_from_dawg(const DawgBuilder &dawg,
                id_type dawg_id, id_type dic_id);

            /**
            * @brief ֱ��ʹ���������ַ���������˫����
            *
            * @param keyset �������ַ�����
            */
            template <typename T>
            void build_from_keyset(const Keyset<T> &keyset);

            /**
            * @brief �ݹ麯�����ڵݹ鹹���ַ������϶�Ӧ��˫����
            *
            * @param keyset �ַ�������
            * @param begin  �ַ�����ʼ����
            * @param end    �ַ�����������
            * @param depth  �ַ����ַ����
            * @param dic_id ˫����ڵ�����
            */
            template <typename T>
            void build_from_keyset(const Keyset<T> &keyset, std::size_t begin,
                std::size_t end, std::size_t depth, id_type dic_id);

            /**
            * @brief ���ַ������ϵ�ǰ�ַ���������ͬһ��ȵ��ַ����浽˫������
            *        ͬʱ����˫���鵱ǰ�ڵ��offsetֵ
            *
            * @param keyset   �ַ�����
            * @param begin    �ַ�����ʼ����
            * @param end      �ַ�����������
            * @param depth    �ַ����ַ����
            * @param dic_id   ˫����ڵ�����
            *
            * @return ˫���鵱ǰ�ڵ��offsetֵ
            */
            template <typename T>
            id_type arrange_from_keyset(const Keyset<T> &keyset, std::size_t begin,
                std::size_t end, std::size_t depth, id_type dic_id);

            /**
            * @brief ����˫���鵱ǰ�ڵ��Ӧ����offsetֵ
            *
            * @param ˫���鵱ǰ�ڵ��offsetֵ
            */
            id_type find_valid_offset(id_type id) const;

            /**
            * @brief �ж�˫���鵱ǰ�ڵ��Ӧ��ƫ�����Ƿ����
            *
            * @param id     ˫���鵱ǰ�ڵ�
            * @param offset ˫���鵱ǰ�ڵ��Ӧ��ƫ����
            *
            * @return ������true�����򷵻�false
            */
            bool is_valid_offset(id_type id, id_type offset) const;

            void reserve_id(id_type id);

            void expand_units();

            void fix_all_blocks();
            void fix_block(id_type block_id);

            /**
            * @brief ��ʽ���忽�����캯���͸�ֵ����
            *        ���캯��Ϊ˽�г�Ա��ֹ������
            */
            DoubleArrayBuilder(const DoubleArrayBuilder &);
            DoubleArrayBuilder &operator=(const DoubleArrayBuilder &);
        };

        template <typename T>
        void DoubleArrayBuilder::build(const Keyset<T> &keyset)
        {
            Details::DawgBuilder dawg_builder;

            /* ���������޻�ͼ */
            build_dawg(keyset, &dawg_builder);

            /* �������޻�ͼת��Ϊ˫���� */
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
                /* ����ǰ�ַ�������������Ա��浽DAWG�� */
                dawg_builder->insert(keyset.keys(i), keyset.lengths(i),
                    keyset.values(i));

                if (progress_func_ != NULL)
                {
                    progress_func_(i + 1, keyset.num_keys() + 1);
                }
            }

            /* ��nodes_��ʣ��Ľڵ㱣�浽unit_�ȵ�Ԫ�й������յĵ�DAWG */
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

            /* ��ȡDAWG�е�ǰ�ڵ��ӽڵ㼰�ӽڵ������ھӽڵ��Ӧ���ַ� */
            id_type dawg_child_id = dawg.child(dawg_id);
            while (dawg_child_id != 0)
            {
                labels_.append(dawg.label(dawg_child_id));
                dawg_child_id = dawg.sibling(dawg_child_id);
            }

            /* �ҵ��洢��ǰ�ڵ������ӽڵ�ĺ���offset */
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
        /* ��ʼ���ַ����������Լ��� */
        Details::Keyset<value_type> keyset(num_keys, keys, lengths, values);

        /* ��ʼ��˫���鹹������ */
        Details::DoubleArrayBuilder builder(progress_func);

        /* ����˫���� */
        builder.build(keyset);

        /* ���湹����ϵ�˫���� */
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

