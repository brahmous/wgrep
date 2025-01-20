#pragma once

#include <string>
#include <unordered_map>


size_t utf8_char_length(unsigned char c) {
  if ((c & 0x80) == 0) return 1;            // 1-byte character (ASCII)
  else if ((c & 0xE0) == 0xC0) return 2;    // 2-byte character
  else if ((c & 0xF0) == 0xE0) return 3;    // 3-byte character
  else if ((c & 0xF8) == 0xF0) return 4;    // 4-byte character
  throw std::runtime_error("Invalid UTF-8 sequence");
}

class utf8string
{

private:
  std::string mString;
  std::vector<size_t> mUtf8ToByteIndexMap;
  
  size_t mSize = 0;

  friend class utf8stringRef;
  class utf8stringRef {
  private:
    utf8string& mUtf8string;
    const size_t position;
  public:

    utf8stringRef(utf8string& _string, const size_t _position)
      : mUtf8string { _string },
        position{ _position }
    {}

    utf8stringRef& operator=(const std::string& other) {
      size_t oldSize = utf8_char_length(*(mUtf8string.mString.begin()+position));
      size_t newSize = utf8_char_length(other[0]);
      /*
        Check that str only holds one character otherwise throw.
      */
      int diff = newSize - oldSize;

      if (diff < 0)
      {
        std::copy(mUtf8string.mString.begin() + position + oldSize, mUtf8string.mString.end(), mUtf8string.mString.begin() + position + std::abs(diff));
        mUtf8string.mString.resize(mUtf8string.mString.size() - std::abs(diff));
      }
      else if (diff > 0)
      {
        mUtf8string.mString.resize(mUtf8string.mString.size() + std::abs(diff));
        std::copy_backward(mUtf8string.mString.begin() + position + oldSize, mUtf8string.mString.end() - std::abs(diff), mUtf8string.mString.end());
      }
      std::copy(other.begin(), other.end(), mUtf8string.mString.begin() + position);
      return *this;
    }

    utf8stringRef& operator=(const char* str) {
      std::string other(str);
      *this = other;
      return *this;
    }

    bool operator<(const std::string& str) {
      size_t byteIndex = mUtf8string.mUtf8ToByteIndexMap[position];
      return mUtf8string.mString.substr(byteIndex, utf8_char_length(mUtf8string.mString[byteIndex])) < str;
    }

    bool operator<(const char* str) {
      std::string other(str);
      return *this < other;
    }

    bool operator>(const std::string& str) {
      size_t byteIndex = mUtf8string.mUtf8ToByteIndexMap[position];
      return mUtf8string.mString.substr(byteIndex, utf8_char_length(mUtf8string.mString[byteIndex])) > str;
    }

    bool operator>(const char* str) {
      std::string other(str);
      return *this > other;
    }

    bool operator<=(const std::string& str) {
      size_t byteIndex = mUtf8string.mUtf8ToByteIndexMap[position];
      return mUtf8string.mString.substr(byteIndex, utf8_char_length(mUtf8string.mString[byteIndex])) <= str;
    }

    bool operator<=(const char* str) {
      std::string other(str);
      return *this <= other;
    }

    bool operator>=(const std::string& str) {
      size_t byteIndex = mUtf8string.mUtf8ToByteIndexMap[position];
      return mUtf8string.mString.substr(byteIndex, utf8_char_length(mUtf8string.mString[byteIndex])) >= str;
    }

    bool operator>=(const char* str) {
      std::string other(str);
      return *this >= other;
    }

    bool operator==(const std::string& str) {
      size_t byteIndex = mUtf8string.mUtf8ToByteIndexMap[position];
      return mUtf8string.mString.substr(byteIndex, utf8_char_length(mUtf8string.mString[byteIndex])) == str;
    }

    bool operator==(const char* str) {
      std::string other(str);
      return *this == other;
    }

    bool operator!=(const std::string& str) {
      return !(*this == str);
    }

    bool operator!=(const char* str) {
      std::string other(str);
      return *this != other;
    }

  };


  friend struct utf8stringIterator;
  struct utf8stringIterator
  {
    utf8string& mUtf8string;
    size_t index;

    utf8stringIterator(
        utf8string& _string,
        size_t position = 0)
      : mUtf8string{ _string }, index{ position }
    {}

    /**/

    utf8stringRef operator *()
    {
      return utf8stringRef(mUtf8string, index);
    }

    struct utf8stringIterator& operator++()
    {
      index++;
      return *this;
    }

    struct utf8stringIterator& operator--()
    {
      index--;
      return *this;
    }

    struct utf8stringIterator operator+(int offset)
    {
      struct utf8stringIterator it(mUtf8string, index + offset);
      return it;
    }

    struct utf8stringIterator& operator+=(int offset)
    {
      index += offset;
      return *this;
    }

    struct utf8stringIterator operator-(int offset)
    {
      struct utf8stringIterator it(mUtf8string, this->index - offset);
      return it;
    }

    struct utf8stringIterator& operator-=(int offset)
    {
      index -= offset;
      return *this;
    }

    bool operator==(struct utf8stringIterator& other)
    {
      return &other.mUtf8string == &mUtf8string && other.index == index;
    }

    bool operator!=(struct utf8stringIterator& other)
    {
      return !(*this == other);
    }

    bool operator<(struct utf8stringIterator& other)
    {
      return &other.mUtf8string == &mUtf8string && index < other.index;
    }

    bool operator<=(struct utf8stringIterator& other)
    {
      return &other.mUtf8string == &mUtf8string && index <= other.index;
    }

    bool operator>(struct utf8stringIterator& other)
    {
      return &other.mUtf8string == &mUtf8string && index > other.index;
    }

    bool operator>=(struct utf8stringIterator& other)
    {
      return &other.mUtf8string == &mUtf8string && index >= other.index;
    }

  };

  void indexString(std::string str) {
    size_t byte_index{ 0 };
    size_t utf8char_index{ 0 };

    while (byte_index < mString.size())
    {

      mUtf8ToByteIndexMap.push_back(byte_index);

      size_t nextCharByteIndex = byte_index + utf8_char_length(mString[byte_index]);

      if (nextCharByteIndex > mString.size())
      {
        throw std::runtime_error("malformed utf8 string!");
      }

      byte_index = nextCharByteIndex;
      utf8char_index += 1;
      mSize++;
    }

    mUtf8ToByteIndexMap.push_back(byte_index);
  }

public:

  using iterator = struct utf8stringIterator;

  utf8string() : mString{""}
  {};

  utf8string(const std::string& string) : mString{ string }
  {
    indexString(mString);
  };

  utf8string(const char * string) : mString{ string }
  {
    indexString(mString);
  };

  /*Copy and move constructor*/

  utf8string(const utf8string& other) noexcept
  {

    if (!mString.empty()) {
      mString.clear();
    }
    mString.resize(other.mString.size());
    /*TODO: Try to use my custom iterator instead of std::string iterator.*/    
    std::copy(other.mString.begin(), other.mString.end(), mString.begin());
    Reload();
  }

  utf8string(utf8string&& other) noexcept 
  {
    std::cout << "Move constructor\n";
    mString = std::move(other.mString);
    /*TODO: Think about moving the index vector also without recomputing*/
    Reload();
  }

  utf8string& operator=(const utf8string& other) noexcept
  {
    if (!mString.empty()) {
      mString.clear();
    }
    mString.resize(other.mString.size());
    /*TODO: Try to use my custom iterator instead of std::string iterator.*/
    std::copy(other.mString.begin(), other.mString.end(), mString.begin());
    Reload();
  }
  utf8string& operator=(utf8string&& other) noexcept
  {
    std::cout << "move assignment\n";
    mString = std::move(other.mString);
    /*TODO: Think about moving the index vector also without recomputing*/
    Reload();
  }

  ~utf8string() = default;


  utf8stringIterator Begin() {
    return utf8stringIterator(*this);
  }

  utf8stringIterator End() {
    return utf8stringIterator(*this, mUtf8ToByteIndexMap.size());
  }

  /*Operators and Substring*/
  
  utf8stringRef operator[](size_t index)
  {
    return utf8stringRef(*this, index);
  }
  
  utf8string operator+(const utf8string& other)
  {
    std::string newString;
    newString.resize(mString.size()+other.mString.size());
    std::copy(mString.begin(), mString.end(), newString.begin());
    std::copy(other.mString.begin(), other.mString.end(), newString.begin()+mString.size());
    return newString;
  }

  utf8string& operator+=(const utf8string& other)
  {

    std::size_t size = mString.size();
    mString.resize(mString.size() + other.mString.size());
    std::copy(other.mString.begin(), other.mString.end(), mString.begin() + size);

    return *this;
  }

  bool operator==(const utf8string& other)
  {
    return mString == other.mString;
  }

  bool operator!=(const utf8string& other)
  {
    return !(*this == other);
  }

  bool operator<(const utf8string& other)
  {
    return mString < other.mString;
  }

  bool operator<=(const utf8string& other)
  {
    return mString <= other.mString;
  }

  bool operator>(const utf8string& other)
  {
    return mString > other.mString;
  }

  bool operator>=(const utf8string& other)
  {
    return mString >= other.mString;
  }

  const std::string& Str() const {
    return mString;
  }

  std::string& Str() {
    return mString;
  }

  const size_t Size() const {
    return mSize;
  }

  size_t Size() {
    return mSize;
  }

  const bool Empty() const {
    return mString.size() == 0;
  }

  bool Empty() {
    return mString.size() == 0;
  }

  void Reload() {
    if (!mUtf8ToByteIndexMap.empty())
    {
      mUtf8ToByteIndexMap.erase(mUtf8ToByteIndexMap.begin(), mUtf8ToByteIndexMap.end());
    }
    indexString(mString);
  }

};

std::ostream& operator<<(std::ostream& out, const utf8string& string) {
  out << string.Str();
  return out;
}

std::istream& operator>>(std::istream& in, utf8string& string) {
  in >> string.Str();
  string.Reload();
  return in;
}
