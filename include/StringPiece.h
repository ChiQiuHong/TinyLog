#pragma once

#include <string>
#include <cstring>
#include <array>

namespace tinylog
{
    class StringPiece
    {
    public:
        constexpr StringPiece()
            : ptr_(NULL),
              length_(0)
        {
        }

        constexpr StringPiece(const char *str)
            : ptr_(str),
              length_(static_cast<int>(strlen(ptr_)))
        {
        }

        StringPiece(const std::string &str)
            : ptr_(str.data()),
              length_(static_cast<int>(str.size()))
        {
        }

        StringPiece(const char *offset, int len)
            : ptr_(offset),
              length_(len)
        {
        }

        const char* data() const { return ptr_; }
        int size() const { return length_; }
        bool empty() const { return length_ == 0; }
        const char *begin() const { return ptr_; }
        const char *end() const { return ptr_ + length_; }

        std::string str() const { return std::string(data(), size()); }

        char operator[](int i) const { return ptr_[i]; }

        void remove_prefix(size_t n)
        {
            ptr_ += n;
            length_ -= n;
        }

        /**
         * Does this StringPiece start with another StringPiece?
         */
        bool startsWith(const StringPiece &other) const
        {
            return size() >= other.size() &&
                subpiece(0, other.size()) == other;
        }

        /**
         * Does this StringPiece end with another StringPiece?
         */
        bool endsWith(const StringPiece& other) const
        {
            return size() >= other.size() &&
                subpiece(size() - other.size()) == other;
        }

        void subtract(size_t n)
        {
            length_ -= n;
        }

        StringPiece subpiece(size_t first, size_t length = std::string::npos) const
        {
            return StringPiece(ptr_ + first, std::min(length, size() - first));
        }

        bool operator==(const StringPiece& x) const
        {
            return ((length_ == x.length_) &&
                    (memcmp(ptr_, x.ptr_, length_) == 0));
        }

        size_t rfind(char c) const
        {
            for (auto i = size(); i-- > 0;)
            {
                if ( (*this)[i] == c)
                    return i;
            }
            return std::string::npos;
        }

    private:
        const char *ptr_;
        int length_;
    };
} // namespace tinylog
