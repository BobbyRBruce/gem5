/*
 * Copyright (c) 2025  The Regents of the University of California
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

#ifndef HYPERCALL_IDS_H
#define HYPERCALL_IDS_H

/*
 * Central list of hypercall-driven exit handlers. The list is expressed as a
 * macro so multiple translation units (C and C++) can derive enums, constants,
 * and documentation without duplicating the mapping.
 *
 * OP arguments: (EnumName, MacroName, Value, Description)
 */
#define GEM5_FOREACH_EXIT_HYPERCALL(OP)                                       \
    OP(ClassicGenerator, CLASSIC_GENERATOR, 0,                                \
       "Legacy generator-based exit handling (ExitEvent translation)")        \
    OP(KernelBooted, KERNEL_BOOTED, 1, "Guest kernel reported it has booted") \
    OP(AfterBoot, AFTER_BOOT, 2, "Guest entered the after_boot hook")         \
    OP(AfterBootScript, AFTER_BOOT_SCRIPT, 3,                                 \
       "Guest completed after_boot.sh")                                       \
    OP(WorkBegin, WORK_BEGIN, 4, "Entered a region-of-interest (workbegin)")  \
    OP(WorkEnd, WORK_END, 5, "Exited a region-of-interest (workend)")         \
    OP(ScheduledExit, SCHEDULED_EXIT, 6,                                      \
       "Simulator scheduled tick/max-tick exit")                              \
    OP(Checkpoint, CHECKPOINT, 7, "Take a checkpoint and continue running")   \
    OP(Orchestrator, ORCHESTRATOR, 1000,                                      \
       "Orchestrator control/status hypercall")

/*
 * Define C-compatible M5_HYPERCALL_* constants from the central mapping
 */
enum
{
#define GEM5_DECLARE_M5_HYPERCALL(enum_name, macro_name, value, desc)         \
    M5_HYPERCALL_##macro_name = value, /* desc */
    GEM5_FOREACH_EXIT_HYPERCALL(GEM5_DECLARE_M5_HYPERCALL)
#undef GEM5_DECLARE_M5_HYPERCALL
};

/*
 * Backwards-compatible C-style names (preserve older header semantics).
 * Keep these as enum constants so including this header from multiple C or
 * C++ translation units does not emit duplicate object definitions.
 */
enum
{
    KERNEL_BOOTED_EXIT = 1,
    STARTED_AFTERBOOT_SCRIPT_EXIT = 2,
    FINISHED_AFTERBOOT_SCRIPT_EXIT = 3,
    WORK_BEGIN_EXIT = 4,
    WORK_END_EXIT = 5,
    SCHEDULED_EXIT = 6,
    CHECKPOINT_EXIT = 7,
    ORCHESTRATOR_EXIT = 1000,
};

#endif // HYPERCALL_IDS_H
