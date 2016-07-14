#ifndef __MAPFILES_H__
#define __MAPFILES_H__
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace NLP
{
    class MMap 
    {
        public:
            MMap():_fd(-1), _ptr(0), _len(0), _fileName(0) {}
            ~MMap() 
            { 
                reset(); 
            }
            void* mapFile(const char* fileName)
            {
                reset();
                if (fileName) 
                { 
                    _fileName = strdup(fileName);
                    _fd = open(_fileName, O_RDONLY); 
                    if(_fd > 0) 
                    {
                        struct stat st;
                        if (fstat(_fd, &st) == 0)
                        {
                            _len =  st.st_size;
                        }
                    }
                    if(_len > 0)
                    {
                        _ptr = mmap(0, _len, PROT_READ, MAP_PRIVATE, _fd, 0);
                    }
                }
                return _ptr;
            }

            void reset() 
            {
                if(_ptr != NULL && _len > 0) 
                {
                    munmap(_ptr, _len);
                    _ptr = 0;
                    _len = 0;
                }
                if (_fd != -1)
                {
                    close(_fd); 
                    _fd = -1;
                }
                if (_fileName)
                {
                    free (_fileName);
                    _fileName = 0;
                }
            }
            size_t length() const { return _len; }
            void*  getPtr() const { return _ptr; }
        private:
            int    _fd;
            void*  _ptr;
            size_t _len;
            char*  _fileName;
    };
}//namespace

#endif // __MMAP_H__
