"""
    test_3b.py\n
    This is a Python 3 replica of test program 3b for Xplice, and it is only for crude runtime benchmarks.
"""

def fib(n: int) -> int:
    if n == 0 or n == 1:
        return 1

    return fib(n - 1) + fib(n - 2)

if __name__ == '__main__':
    answer = fib(29)

    if answer > 832040:
        exit(0)
    else:
        exit(1)
