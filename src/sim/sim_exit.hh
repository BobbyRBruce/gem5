/*
 * Copyright (c) 2003-2005 The Regents of The University of Michigan
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

#ifndef __SIM_EXIT_HH__
#define __SIM_EXIT_HH__

#include <functional>
#include <map>
#include <string>
#include <utility>

#include "base/types.hh"
#include "gem5/exit_hypercalls.hh"

namespace gem5
{

Tick curTick();

/// Hypercall identifiers used when scheduling exits with
/// exitSimulationLoop(). These values mirror the IDs consumed by the
/// standard library ``ExitHandler`` implementations. ID 0 is reserved for the
/// legacy generator-based exit handling path.
enum class ExitHypercall : uint64_t
{
#define GEM5_DEFINE_EXIT_ENUM(enum_name, macro_name, value, desc)             \
    enum_name = value,
    GEM5_FOREACH_EXIT_HYPERCALL(GEM5_DEFINE_EXIT_ENUM)
#undef GEM5_DEFINE_EXIT_ENUM
};

#define GEM5_DEFINE_EXIT_CONST(enum_name, macro_name, value, desc)            \
    inline constexpr uint64_t kExitHypercall##enum_name =                     \
        static_cast<uint64_t>(ExitHypercall::enum_name);
GEM5_FOREACH_EXIT_HYPERCALL(GEM5_DEFINE_EXIT_CONST)
#undef GEM5_DEFINE_EXIT_CONST

struct ExitHypercallDescriptor
{
    ExitHypercall id;
    const char *name;
    const char *description;
};

#define GEM5_DEFINE_EXIT_DESCRIPTOR(enum_name, macro_name, value, desc)       \
    {ExitHypercall::enum_name, #enum_name, desc},
inline constexpr ExitHypercallDescriptor kExitHypercallDescriptors[] = {
    GEM5_FOREACH_EXIT_HYPERCALL(GEM5_DEFINE_EXIT_DESCRIPTOR)};
#undef GEM5_DEFINE_EXIT_DESCRIPTOR

/// Register a callback to be called when Python exits.  Defined in
/// sim/main.cc.
void registerExitCallback(const std::function<void()> &);

/**
 * Legacy “classic” exit path that predates hypercalls. This schedules a
 * generator-based exit event that reports only a human-readable message and an
 * optional integer code. Modern stdlib simulations should prefer the
 * hypercall-based APIs below so Python ExitHandlers can dispatch on a specific
 * ID and payload.
 */
/// DEPRECATED: Prefer exitSimulationLoop (hypercall-based).
void exitSimLoop(const std::string &message, int exit_code = 0,
                 Tick when = curTick(), Tick repeat = 0,
                 bool serialize = false);

/// Legacy high-priority variant of ``exitSimLoop`` (see note above).
void exitSimLoopNow(const std::string &message, int exit_code = 0,
                    Tick repeat = 0, bool serialize = false);

/**
 * Transitional helper that emits the legacy message/code *and* a hypercall
 * payload. This is what pseudo instructions such as ``m5_hypercall`` use: the
 * simulator still records the textual cause, but ExitHandlers can make
 * decisions based on ``hypercall_id`` and ``payload``.
 *
 * Prefer the ``exitSimulationLoop*`` helpers below when you do not need the
 * legacy string/code at all.
 *
 * @param message     Human-readable reason displayed when the exit triggers.
 * @param exit_code   Optional numeric code paired with ``message``.
 * @param when        Tick at which to queue the exit event (defaults to now).
 * @param repeat      Interval to reschedule the event; 0 disables repetition.
 * @param payload     Key/value data forwarded to the ExitHandler.
 * @param hypercall_id Identifier that selects the ExitHandler implementation.
 * @param serialize   Request automatic checkpointing before exiting.
 */
void exitSimLoopWithHypercall(const std::string &message, int exit_code,
                              Tick when, Tick repeat,
                              std::map<std::string, std::string> payload,
                              uint64_t hypercall_id, bool serialize);

/**
 * Preferred hypercall-based exit API. ``type_id`` should be one of the
 * ``ExitHypercall`` enum values (or a custom ID if you've registered your own
 * ExitHandler).
 *
 * The ``payload`` map is copied so Python handlers can inspect arbitrary
 * metadata (justification strings, schedule time, etc.).
 *
 * Examples:
 *
 * ```
 * exitSimulationLoop(ExitHypercall::ScheduledExit,
 *     { {"justification", "Stop after ROI"} },
 *     curTick() + 1000);
 * exitSimulationLoop(static_cast<uint64_t>(myCustomId), {{"foo", "bar"}});
 * ```
 *
 * @param type_id   Identifier for the ExitHandler to run.
 * @param payload   Optional metadata consumed by the ExitHandler.
 * @param when      Tick at which the exit event should fire.
 */
void exitSimulationLoop(uint64_t type_id,
    std::map<std::string, std::string> payload=
        std::map<std::string, std::string>(),
    Tick when=curTick());

/**
 * Immediate variant of ``exitSimulationLoop`` that schedules the event for
 * the current tick and does not repeat.
 */
void
exitSimulationLoopNow(uint64_t type_id,
    std::map<std::string, std::string> payload=
        std::map<std::string, std::string>());

/// Convenience overloads so callers can pass ``ExitHypercall`` directly.
inline void
exitSimLoopWithHypercall(const std::string &message, int exit_code, Tick when,
                         Tick repeat,
                         std::map<std::string, std::string> payload,
                         ExitHypercall hypercall_id, bool serialize)
{
    exitSimLoopWithHypercall(message, exit_code, when, repeat,
                             std::move(payload),
                             static_cast<uint64_t>(hypercall_id), serialize);
}

inline void
exitSimulationLoop(ExitHypercall type_id,
                   std::map<std::string, std::string> payload =
                       std::map<std::string, std::string>(),
                   Tick when = curTick())
{
    exitSimulationLoop(static_cast<uint64_t>(type_id), std::move(payload),
                       when);
}

inline void
exitSimulationLoopNow(ExitHypercall type_id,
                      std::map<std::string, std::string> payload =
                          std::map<std::string, std::string>())
{ exitSimulationLoopNow(static_cast<uint64_t>(type_id), std::move(payload)); }

} // namespace gem5

#endif // __SIM_EXIT_HH__
