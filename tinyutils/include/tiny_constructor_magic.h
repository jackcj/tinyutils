#ifndef TINY_CONSTRUCTOR_MAGIC_H
#define	TINY_CONSTRUCTOR_MAGIC_H

#define TINY_DISALLOW_COPY_AND_ASSIGN(TypeName)				\
  TypeName(const TypeName&) = delete;						\
  TypeName& operator=(const TypeName&) = delete


#endif // !TINY_CONSTRUCTOR_MAGIC_H
