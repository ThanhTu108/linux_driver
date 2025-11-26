# Atomic Variable
Struct atomic is define: 

```c
typedef struct 
{
    int counter;
} atomic_t;
```

There are 2 tyoe of atomic variable
- Atomic variable operation on integers
- Atomic variable operation on bits

## Atomic variable (Integer)

1. Create atomic variable

There are 2 way to create atomic variable
```c
atomic_t my_atomic_variable;
```
Or
```c
int value = 0;
atomic_t my_atomic_variable = ATOMIC_INIT(value);
```
2. Read atomic
```c
int atomic_read(const atomic_t* v);
```

Ex:
```c
int value;
value = atomic_read(&my_atomic_variable);
```

3. Set atomic
This function safely sets the value of an atomic variable.  
Directly accessing v->counter is not allowed in the Linux kernel.
```c
void atomic_set(atomic_t* v, int i);
```

4. atomic_add
This function is used to adds value to the atomic variable.  
```c
void atomic_add(int i, atomic_t* v);
```

5. atomic_sub
This function atomically subtracts the value from the atomic variable.  
```c
void atomic_sub(int i, atomic_t* v);
```

6. atomic_inc
This function is used to increments the value of atomic variable. (value++)
```c
void atomic_inc(atomic_t *v);
```

7. atomic_dec
This function is used to decrements the value of atomic variable. (value--)
```c
void atomic_dec(atomic_t *v);
```

8. atomic_sub_and_test
This function is used to decrements the value of atomic variable. (value - i) and test the result.  
It's return true if result is zero and false otherwise
```c
atomic_sub_and_test(int i, atomic_t* v)
```

8. atomic_dec_and_test
This function is used to decrements the value of atomic variable. (value--) and test the result.  
It's return true if result is zero and false otherwise
```c
atomic_dec_and_test(atomic_t* v)
```
9. atomic_inc_and_test

10. atomic_add_negative
This function is used to add the value to the atomic variable  
It's return true if result is negative and false otherwise
```c
void atomic_add_negative(int i, atomic_t *v);
```
11. atomic_add_return
This function is used to add the value to the atomic variable  and return new value.
```c
int atomic_add_return(int i, atomic_t* v);
```

## Atomic bitwise
1. Set_bit
```c
void set_bit(int nr, void* addr);
```
This function like: addr |= (1 << nr);
2. Clear_bit
```c
void clear_bit(int nr, void* addr);
```
This function like: addr &= ~(1 << nr);
3. Change_bit
```c
void change_bit(int nr, void *addr);
```
This function like: addr ^= (1 << nr);

4. Test_and_set_bit
This function set the nr bit and return previous value
```c
int test_and_set_bit(int nr, void *addr);
```
5. test_and_clear_bit
6. test_and_change_bit
7. test_bit
This function return value of nr bit
```c
int test_bit(int nr, void *addr);
```
8. find_first_zero_bit
This function return the bit-number of the first zero bit (index)
```c
int find_first_zero_bit(unsigned long *addr, unsigned int size);
```
9. find_first_bit
This function return the bit-number of the first 1 bit (index)
```c
int find_firstbit(unsigned long *addr, unsigned int size);
```

## Non_atomic variable (spinlock, mutex....)
1. Set_bit
```c
void _set_bit(int nr, void* addr);
```
This function like: addr |= (1 << nr);
2. Clear_bit
```c
void _clear_bit(int nr, void* addr);
```
This function like: addr &= ~(1 << nr);
3. Change_bit
```c
void _change_bit(int nr, void *addr);
```
This function like: addr ^= (1 << nr);

4. Test_and_set_bit
This function set the nr bit and return previous value
```c
int _test_and_set_bit(int nr, void *addr);
```
5. _test_and_clear_bit
6. _test_and_change_bit
7. _test_bit
This function return value of nr bit
```c
int _test_bit(int nr, void *addr);
```
8. _find_first_zero_bit
This function return the bit-number of the first zero bit (index)
```c
int _find_first_zero_bit(unsigned long *addr, unsigned int size);
```
9. _find_first_bit
This function return the bit-number of the first 1 bit (index)
```c
int _find_firstbit(unsigned long *addr, unsigned int size);
```
