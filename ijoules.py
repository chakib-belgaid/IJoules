import functools
import os
import pathlib
import socket
import subprocess
import time
from os import error, read, strerror

import atexit

import pandas as pd
from tabulate import tabulate

# Standard loopback interface address (localhost)
SERVERS = []


class IJoules(object):
    HOST = ('0.0.0.0', 3310)

    def __init__(self,):
        self._server_path = os.path.join(pathlib.Path(
            __file__).parent.absolute(), "ijoules-server")
        self._energies = {}
        # self.configure()

    def configure(self):
        """
        Launch the IJoules server and connect the client socket to the server
        """
        self._server = subprocess.Popen(self._server_path)
        SERVERS.append((self._server))
        time.sleep(0.5)
        self._client = socket.socket(
            socket.AF_INET, socket.SOCK_STREAM)
        self._client.connect(self.HOST)
        self.get_available_domains()

    def get_available_domains(self):
        """
        return a list of all available domains in the current machine
        """
        self._client.send(b'list_available')
        response = self._client.recv(1024).decode('ascii')
        self.avaialable_domaines = [
            i.split(":")[0] for i in response.split(",")
            if i.split(":")[1] == "1"]
        return self.avaialable_domaines

    def get_energy(self):
        """
        return the energy consumption of all active devices + time stamp since the epoch

        Units : energy Joules
        Timestamp : in Nano Seconds
        """
        self._client.send(b'all')
        response = self._client.recv(1024).decode('ascii').strip()
        response = {x.split(":")[0]: float(x.split(":")[1])
                    for x in response.split(" ")}
        # response["timestamp"] = response["timestamp_ns"]/10**9
        return response

    def begin(self):
        """
        start recording 
        """
        # self._client.send(b'start')
        # self._client.recv(1024)
        self.get_energy()
        # intial_measures = {x.split(":")[0]: float(x.split(":")[1])
        #                    for x in response.split(" ")}
        # intial_measures.update({x: 0 for x in self.avaialable_domaines})
        # intial_measures.update({"tag": ""})

        self._energies = [self.get_energy()]

    def record(self, tag="tag"):
        """ 
        add a step into recording 
        """
        x = self.get_energy()

        x["tag"] = tag
        self._energies.append(x)
        # self._client.send(b'start')

        # self._client.recv(1024)

    def end(self, tag="end"):
        """
        end recording phase 
        """
        x = self.get_energy()
        x["tag"] = tag
        self._energies.append(x)

    @property
    def report(self):
        """
        transform the energy consumption into a Dataframe
        """
        df = pd.DataFrame(self._energies)

        df["duration_s"] = df.apply(lambda row: float(
            row["timestamp_s"])+float(row["timestamp_ns"])/10**9, axis=1)

        tags = df["tag"]
        df = df.drop(["tag", "timestamp_s", "timestamp_ns"], axis=1).diff()
        df["tag"] = tags
        return df.drop(0)

    def destroy(self):
        """
        Cleanup the mess
        - stop the server
        - clean any temporary files
        """
        self._client.send(b'exit')
        self._client.close()

    def __enter__(self):
        self._context_meter = IJoules()
        self._context_meter.configure()
        self._context_meter.begin()
        return self._context_meter

    def __exit__(self, exc_type, exc_value, exc_traceback):
        self._context_meter.end()
        self._context_meter.destroy()
        print(tabulate(self._context_meter.report,
                       headers='keys', tablefmt='fancy_grid'))


def measureit(func=None):

    def decorator_measure_energy(func):

        @functools.wraps(func)
        def wrapper_measure(*args, **kwargs):
            meter = IJoules()
            meter.configure()
            meter.begin()
            val = func(*args, **kwargs)
            meter.end(func.__name__)
            meter.destroy()
            print(tabulate(meter.report,
                           headers='keys', tablefmt='fancy_grid'))
            return val
        return wrapper_measure

    if func is None:
        # to ensure the working system when you call it with parameters or without parameters
        return decorator_measure_energy
    else:
        return decorator_measure_energy(func)


@atexit.register
def goodbye():
    for server in SERVERS:
        try:
            server.terminate()
        except error:
            pass
