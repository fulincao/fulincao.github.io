

# 因子和
def sum_factor(n):
    sum = 1
    i = 2
    tmp = n
    while i <= n:
        p = i
        while n % i ==0:
            n = n // i
            p *= i
        sum *=  (1 - p) / (1 - i)
        i += 1
    return sum


if __name__ == "__main__":
    print(6, sum_factor(6))
    print(11, sum_factor(11))
    print(24, sum_factor(24))