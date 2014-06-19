#ifndef HUES_COMMON_H_
#define HUES_COMMON_H_

// Shorthand to disable a class's copy constructor and assignment operator.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(TypeName const&) = delete;    \
    TypeName& operator=(TypeName const&) = delete;

#endif // HUES_COMMON_H_
