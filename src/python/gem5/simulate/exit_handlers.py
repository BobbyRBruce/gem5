# Copyright (c) 2024 The Regents of the University of California
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from abc import abstractmethod

from m5.event import ExitEvent
from m5.util import fatal

from gem5.utils.override import overrides


class ExitHandler:

    _instance = dict()

    def __new__(cls, exit_event: ExitEvent):
        if not cls._instance:
            for subclass in ExitHandler.__subclasses__():
                cls._instance[subclass] = None
        for exit_object in cls._instance:
            if exit_object.get_type() == exit_event.get_type():
                if cls._instance[exit_object] is None:
                    cls._instance[exit_object] = super().__new__(cls)
                    cls._instance[exit_object]._exit_event = exit_event
                return cls._instance[exit_object]

        fatal(
            f"ExitHandler type '{cls.get_type()}' does not match ExitEvent type '{exit_event.get_type()}'."
        )
        return cls._instance

    def __init__(self, exit_event: ExitEvent):
        pass  # Initialization is handled in __new__

    def process_exit(self) -> bool:
        assert self._exit_event, "ExitHandler has no ExitEvent to handle."
        self._process_exit()
        return self._resume_sim()

    def __str__(self):
        return self.get_type() if self.get_type() else "None"

    # ID's are unique. they are used by the  ExitEvent to identify which
    # ExitHandler to utilize.
    def __eq__(self, other):
        return self.get_type() == other.get_type()

    def __hash__(self):
        return hash(self.get_type())

    # The following methods must be implemented by the ExitHandler subclasses.
    @abstractmethod
    def get_type(cls) -> str:
        raise NotImplementedError("ExitHandler must implement `get_type`.")

    @abstractmethod
    def _process_exit(self) -> None:
        raise NotImplementedError(
            "ExitHandler must implement `_process_exit`."
        )

    @abstractmethod
    def _reenter_sim(self) -> bool:
        raise NotImplementedError("ExitHandler must implement `_resume_sim`.")


class UselessExitHandler(ExitHandler):
    """This class is a Useless ExitHandler: it does nothing of value. It does
    not process
    anything then reenters the simuation.
    """

    def get_type(cls) -> str:
        return "useless"

    def _process_exit(self) -> None:
        pass

    def _reenter_sim(self) -> bool:
        return False


class UserInterruptExit(ExitHandler):
    """This class is an ExitHandler that handles user interrupts (e.g.,
    'cntr-c'). It will print the property "message" from the ExitEventpayload. Prior to exiting the
    simulation.
    """

    @overrides(ExitHandler)
    def _reenter_sim(self) -> bool:
        return True

    @overrides(ExitHandler)
    def get_type(self) -> str:
        return "user-interrupt"

    @overrides(ExitHandler)
    def _process_exit(self) -> None:
        msg = self._exit_event.get_payload().get("message", "<No message>")
        print(f"User interrupt received: {msg}")
