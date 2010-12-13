# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - RPC Aggregator

    Aggregates RPC calls into MultiCall batches in order to increase
    the speed.

    @copyright: 2006 MoinMoin:AlexanderSchremmer
    @license: GNU GPL, see COPYING for details.
"""

import xmlrpclib
INVALID = object()

class RPCYielder(object):
    """ If you want to have a batchable function, you need
    to inherit from this class and define a method "run" that
    takes exactly one argument.
    This method has to be a generator that yields (func, arg)
    tuples whereas func is the RPC method name (str).
    You can fetch the calls by calling fetch_call(),
    then you have to return the result by calling set_result(res).
    """

    def __init__(self, arg, raise_fault=False):
        self._comm_slot = [INVALID]
        self.raise_fault = raise_fault
        self._gen = self.run(arg)

    def fetch_call(self):
        try:
            next_item = self._gen.next()
        except StopIteration:
            return None
        return next_item

    def set_result(self, result):
        self._comm_slot[0] = result

    def fetch_result(self):
        result = self._comm_slot[0]
        try:
            if result is INVALID:
                return RuntimeError("Invalid state, there is no result to fetch.")
            if self.raise_fault and isinstance(result, xmlrpclib.Fault):
                raise result
            else:
                return result
        finally:
            self._comm_slot[0] = INVALID

    def run(self, arg):
        return NotImplementedError


def scheduler(multicall_func, handler, args, max_calls=10, prepare_multicall_func=None):
    # all generator (or better, RPCYielder) instances
    gens = []
    # those instances that have to be queried in the next step again
    gens_todo = []
    # pending calls, stored as tuples: (generator, (funcname, (args,*)))
    call_list = []

    # instantiate generators
    for arg in args:
        gens.append(handler(arg))
    # schedule generators
    while gens:
        for gen in gens:
            if len(call_list) > max_calls:
                gens_todo.append(gen)
                continue
            call = gen.fetch_call()
            if call is not None:
                call_list.append((gen, call))
                gens_todo.append(gen)
        if call_list:
            if prepare_multicall_func is not None:
                pre_calls = [(RPCYielder(0), x) for x in prepare_multicall_func()]
                call_list = pre_calls + call_list

            m = multicall_func()
            gens_result = [] # generators that will get a result
            for gen, (func, args) in call_list:
                gens_result.append(gen)
                getattr(m, func)(*args) # register call
            result = iter(m()) # execute multicall
            for gen in gens_result:
                try:
                    item = result.next()
                except xmlrpclib.Fault, e:
                    # this exception is reraised by the RPCYielder
                    item = e
                gen.set_result(item)
            call_list = []
        gens = gens_todo
        gens_todo = []


def scheduler_simple(multicall_func, handler, args):
    for arg in args:
        cur_handler = handler(arg)
        while 1:
            call = cur_handler.fetch_call()
            if call is not None:
                func, arg = call
                m = multicall_func()
                getattr(m, func)(arg) # register call
                result = iter(m()) # execute multicall
                try:
                    item = result.next()
                except xmlrpclib.Fault, e:
                    # this exception is reraised by the RPCYielder
                    item = e
                cur_handler.set_result(item)
            else:
                break
