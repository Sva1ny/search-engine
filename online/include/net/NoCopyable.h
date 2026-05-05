#ifndef NOCOPYABLE_HPP
#define NOCOPYABLE_HPP

class NoCopyable
{
protected:
    NoCopyable() = default;
    NoCopyable(const NoCopyable &) = delete;
    NoCopyable &operator=(const NoCopyable &) = delete;
};

#endif