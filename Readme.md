


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
    data = energy_meter.report # a dataframe containing all the energy 

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



|    |   pkg_j |   core_j |   dram_j |   duration_s | Tag         |
|:--:|:-------:|:--------:|:--------:|:------------:|:-----------:|
|  1 | 13.5483 | 11.4319  | 0.408875 |     0.610369 | worker1     |
|  2 | 13.5273 | 11.413   | 0.367126 |     0.60813  | second time |
|  3 | 10.4964 |  3.47644 | 1.36255  |     2.00226  | end         |

`IJoules` allow you to measure different parts of your **CPU** based on their availablity. 
To know the available parts that can be measured you can call the methdoe `get_available_domains`

``` python

energy_meter= IJoules()
energy_meter.configure()
energy_meter.get_available_domains()
```

The possible domains (CPU parts) are : 

- core : The energy consumption of the **core units** -in *Joules*-
- dram : The energy consumption of **DRAM** -in *Joules*-
- gpu : The energy consumption of intergated **GPU** -in *Joules*-
- pkg : The whole energy consumption of the whole **CPU** (Core+GPU+other parts) -in *Joules*-

The figure below will illustrate more those parts:

![CPU domains](https://blog.chih.me/images/power-planes.jpg)

References :
- https://blog.chih.me/read-cpu-power-with-RAPL.html
- https://software.intel.com/content/www/us/en/develop/articles/intel-power-gadget.html

