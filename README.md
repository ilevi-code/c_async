# C async

This project is for me to test an idea - switch stacks in C to create a generator like experience.  
Based on this primitive, I can create coroutines, and an event loop to manage them all.

## Status

[+] Generators  
[+] Event loop  
[+] Futures

## How does this work?

Generators have their own call stack, think of the following example:

```py
def foo():
    yield 1
    print("hi")
    yiled 2

def main():
    for i in foo():
        print(i)
```
When foo calls `print`, it has no effect on the call stack of `main`, and vice-versa.
To allow suck primitives in c, the library creates a separate stack for each coroutine.
    
```
+-------------------+  next()  +----------------+
|                   |   -->    |                |
|       main        | yield()  |       foo      |
|                   |   <--    |                |
+-------------------+          +----------------+ 
| printf("%d\n", i) |          | printf("hi\n") | 
+-------------------+          +----------------+
```
