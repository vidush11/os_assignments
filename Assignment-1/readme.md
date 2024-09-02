<h1>This is the first assignment of OS (2024).</h1>

<h2>In this we have made the with- bonus assignment.</h2>

Contributors-

<p>Firstly we wrote the code in loader.c. </p>
<ol>
<li>
  We opened the file fib.out by using open() in c and checked for errors if it returned -1
</li>

<li>
    We then loaded the whole file into memory by using malloc first for allocating space and then read()
</li>

<li>
  After it we found the address of phdr table by reading the offest in ELF header, and also the number of entries.
</li>

<li>
  We moved to the offset by using lseek and then started to iterate through all the segments
</li>
<li>
  Once a segment which was of the type PT_LOAD and contained the address of _start found earlier by checking it in elf header was found, we loaded that segment into memory. We did this by allocating space through mmap and then copying the data through memcopy
  
</li>

<li>
  After it we casted the virtual address thus created by mmap of _start into a suitable function pointer and then ran it to get the desired return value of fib(40)
</li>
  
  
</ol>
