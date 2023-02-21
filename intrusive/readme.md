# IntrusivePtr

### Что это?
`IntrusivePtr` is a smart pointer that is similar in semantics to `SharedPtr`,
but without the ability to create a `WeakPtr` to the pointer. However, as you
will see, the implementation of this class is much simpler than that of `SharedPtr`.
This is achieved by restricting the user-defined type to satisfy the following condition:
1. The user-defined type must have a reference count inside it (hence the name "intrusive" pointer: the reference count is located directly in the object).
2. There is a method `IncRef()` which increments the internal reference count.
3. There is a method `DecRef()` which decrements the internal reference count; when the count reaches zero, the object is automatically destroyed.
4. There is a method `RefCount()` which returns the current value of the reference count.


### Why should we use IntrusivePtr?
Due to the stricter requirements on the user-defined type compared to `SharedPtr`
and the absence of `WeakPtr`, `IntrusivePtr` is implemented much simpler and 
more efficiently. The convenient abstraction with an external reference count 
makes it easy to use `IntrusivePtr` for non-trivial lifetimes. 
Most of the uses of std::shared_ptr in your code can actually be replaced with the lighter `IntrusivePtr`.
You can also check `boost` library for `IntrusivePtr`.