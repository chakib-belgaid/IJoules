from ijoules import IJoules, measureit
import time
from tabulate import tabulate


@measureit()
def worker1():
    s = 0
    for i in range(10**7):
        s += i


def worker():
    s = 0
    for i in range(10**7):
        s += i


def usecase1():
    print("basic usage")
    meter = IJoules()
    meter.configure()
    domains = meter.avaialable_domaines
    print(
        f"The available domains that we can measure in this machine are {domains}")
    meter.begin()
    worker()
    meter.record("worker1")
    worker()
    meter.record("second time")
    time.sleep(2)
    meter.end()
    meter.destroy()
    print(tabulate(meter.report,
                   headers='keys', tablefmt='fancy_grid'))


def usecase2():
    print("using IJoules as a context")
    with IJoules() as meter:
        worker()
        meter.record("worker1")
        worker()
        meter.record("second time")
        time.sleep(2)


def usecase3():
    print("as a decorator")
    worker1()


if __name__ == "__main__":
    usecase1()
    usecase2()
    usecase3()
