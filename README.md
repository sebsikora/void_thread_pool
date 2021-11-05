# void_thread_pool.hpp

© 2021 Dr Sebastien Sikora.

[seb.nf.sikora@protonmail.com](mailto:seb.nf.sikora@protonmail.com)

Updated 05/11/2021.

What is it?
-------------------------
void_thread_pool is an ultra-simple header-only thread pool implementation for running void() functions in multiple worker threads.
It is a synthesis of the brilliantly helpful code snippets and examples by users 'phd-ap-ece' and 'pio' [here](https://stackoverflow.com/questions/15752659/thread-pooling-in-c11).
Useful supplementary information found [here](https://stackoverflow.com/questions/10673585/start-thread-with-member-function).
Explanation of the syntax `[this]() { ... }` for the predicate second argument to `condition_variable.wait()` found [here](https://stackoverflow.com/questions/39565218/c-condition-variable-wait-for-predicate-in-my-class-stdthread-unresolved-o).

License:
-------------------------
![Open - Mit License Logo Svg @clipartmax.com](https://www.clipartmax.com/png/small/140-1401362_open-mit-license-logo-svg.png)
<br/><br/>
void_thread_pool.hpp is distributed under the terms of the MIT license.
Learn about the MIT license [here](https://choosealicense.com/licenses/mit/)
