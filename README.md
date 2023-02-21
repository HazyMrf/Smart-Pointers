# Smart-Pointers

## General Info

In some cases, an object created on the heap needs to be shared among 
multiple functions, methods, or other parts of the program. This can 
complicate the management of the object's lifetime and the appropriate 
timing for deallocating the memory it uses.

It would be helpful to use a convenient wrapper that automatically 
allocates memory for an object upon creation and releases the memory when 
the object is no longer needed. This is the purpose of smart pointers.

## Smart Pointers Types

* ```UniquePtr``` ensures exclusive ownership of an object, allowing only one `UniquePtr` to point to it, and is designed to manage a single object's lifetime.
* ```SharedPtr```  provides shared ownership of an object, allowing multiple `SharedPtrs` to point to the same object, and is designed to manage a group of objects' lifetimes.
* ```IntrusivePtr``` provides a non-owning reference to an object managed by a
 `SharedPtr`, allowing a way to check if the object still exists without 
 increasing its reference count, and is designed to avoid circular references between `SharedPtr`.
* ```IntrusivePtr``` is a light version of `SharedPtr`. Read more in `IntrusivePtr` readme.md
