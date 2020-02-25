#! /usr/bin/env python3
import random


def main():
    n_task = 8
    rand_task = random.choice(range(n_task)) + 1
    print(rand_task)


if __name__ == "__main__":
    main()
