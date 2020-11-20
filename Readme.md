


# Requirements 

- Install the [Intel Power Gadget Tool](https://software.intel.com/content/dam/develop/external/us/en/documents/Intel-Power-Gadget-v3.7.0.dmg) 
In order to have access to the power measurements of the machine 

- install the python libraries pandas, and tabulate 

# Usage 

Ijoules can be used in 3 different ways
## Basic usage 

``` Python 
from ijoules import IJoules


def foo(): 
    energy_meter= IJoules()
    energy_meter.configure() 
    # ...
    energy_meter.begin() 
    # Code that we want to measure the nergy consumption
    energy_meter.record("first part of the code ")
    # Another measurement 
    energy_meter.end()
    data = meter.report # a dataframe containing all the energy 

```

## Use it as a context 

With the instruction with 

``` Python 

from ijoules import IJoules

def foo() :
    with Ijoules() as energy_meter : 
        # Work that we want to measure 
        energy_meter.record("First part of the work")
        # Second part of the work 
        energy_meter.record("Second part of the work")
        # Second part of the work 

    # This will print the energies as a table 

``` 


## Use it as a decorator 

from ijoules import  measureit

``` Python 

@measureit()
def foo(): 
    # Code that we want to measure 


def main() 
    foo() 
    # This will print the energy of the function as a table 
```

# Example 


| |   pkg_j |   core_j |   dram_j |   timestamp_s |   timestamp_ns | tag         |
|:--:|:--------|:--------:|:--------:|:-------------:|:--------------:|:-----------:|
|  0 | 13.0273 |  11.4529 | 0.378235 |   1.60586e+09 |    3.14977e+07 | worker1     |
|  1 | 12.4085 |  10.8895 | 0.339417 |   1.60586e+09 |    6.30063e+08 | second time |
|  2 |  7.69   |   2.5816 | 1.2774   |   1.60586e+09 |    6.33775e+08 | end         |

