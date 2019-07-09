/**
 * @file Noncopyable.h
 * @brief
 * 不可拷贝对象
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#ifndef __NDSL_UTILS_NONCOPYABLE_H__
#define __NDSL_UTILS_NONCOPYABLE_H__

namespace ndsl {
namespace utils {

class noncopyable
{
  protected:
    noncopyable() = default;
    ~noncopyable() = default;

  private:
    // 删除复制构造函数
    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;
};

class nonmoveable
{
  public:
    nonmoveable() = default;
    ~nonmoveable() = default;

  private:
    // 删除移动构造函数
    nonmoveable(nonmoveable &&) = delete;
    nonmoveable &operator=(nonmoveable &&) = delete;
};

} // namespace utils
} // namespace ndsl

#endif // __NDSL_UTILS_NONCOPYABLE_H__