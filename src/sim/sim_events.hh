/*
 * Copyright (c) 2013 ARM Limited
 * All rights reserved
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2002-2005 The Regents of The University of Michigan
 * Copyright (c) 2013 Advanced Micro Devices, Inc.
 * Copyright (c) 2013 Mark D. Hill and David A. Wood
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __SIM_SIM_EVENTS_HH__
#define __SIM_SIM_EVENTS_HH__

#include "sim/global_event.hh"
#include "sim/serialize.hh"

namespace gem5
{


/**
 * `ExitEvent` is the base for all Exit events. Exit events are events
 * which exit the simulation loop. The `ExitEvent` class is the most basic and
 * is used to exit the simulation loop immediately, with no further automatic
 * action or special data.
 *
 * In order to support backwards compatibility, the `ExitEvent` class inherits
 * from the `GlobalSimLoopExitEvent` class. Prior to the introduction of the
 * `ExitEvent` class the `GlobalSimLoopExitEvent` class was used to pass
 * information about the exit exit of the loop from the CPP code to the Python.
 *
 */
class ExitEvent : public GlobalEvent
{
  private:

    const uint64_t type_id;

    // Right now i'm just making this string to string but it could be better.
    const std::map<std::string, std::string> payload;

  public:

    // Used for backwards compatibility with GlobalSimLoopExitEvent. The
    // GlobalSimLoopExitEvent class does not have an ID set, so we need to
    // communicate to use the ExitEvent class as one would  have used
    // GlobalSimLoopExitEvent. We do this by reserving this type ID for this
    // case.
    static const uint64_t BACKWARDS_COMPATABLE_ID = 0;


    // The core constructor for the Exit event.
    ExitEvent(
        Tick when,
        u_int64_t type_id,
        const std::map<std::string, std::string> &payload
    );

    // For case when we want to exit immediately.
    ExitEvent(u_int64_t type_id,
    const std::map<std::string, std::string> &payload);

    // For backwards compatibility with the GlobalSimLoopExitEvent approach
    // to handling SimLoop ExitEvents.
    ExitEvent(const GlobalSimLoopExitEvent &event);

    ~ExitEvent();

    std::map<std::string, std::string> getPayload() const;
    u_int64_t getType() const;

    const char *description() const override;
    void process() override;
};
//
// Event to terminate simulation at a particular cycle/instruction
//
class GlobalSimLoopExitEvent : public ExitEvent
{
  protected:
    // string explaining why we're terminating
    std::string cause;
    int code;
    Tick repeat;

  public:
    GlobalSimLoopExitEvent(
        Tick when, const std::string &_cause, int c, Tick repeat = 0);
    GlobalSimLoopExitEvent(const std::string &_cause, int c, Tick repeat = 0);

    int getCode() const { return code; }
    std::string getCause() const { return cause; }
    virtual void process() override;

    ~GlobalSimLoopExitEvent (){
      DPRINTF(Event,"GlobalSimLoopExitEvent destructed\n");
    };
    virtual const char *description() const override;
};

/**

class LocalSimLoopExitEvent : public Event
{
  protected:
    // string explaining why we're terminating
    std::string cause;
    int code;
    Tick repeat;

  public:
    LocalSimLoopExitEvent();
    LocalSimLoopExitEvent(const std::string &_cause, int c, Tick repeat = 0);

    const std::string getCause() const { return cause; }
    int getCode() const { return code; }

    void process() override;     // process event

    const char *description() const override;

    void serialize(CheckpointOut &cp) const override;
    void unserialize(CheckpointIn &cp) override;
};

//
// Event class to terminate simulation after 'n' related events have
// occurred using a shared counter: used to terminate when *all*
// threads have reached a particular instruction count
//
class CountedExitEvent : public Event
{
  private:
    std::string cause;  // string explaining why we're terminating
    int &downCounter;   // decrement & terminate if zero

  public:
    CountedExitEvent(const std::string &_cause, int &_downCounter);

    void process() override;     // process event

    const char *description() const override;
};

} // namespace gem5

#endif  // __SIM_SIM_EVENTS_HH__
