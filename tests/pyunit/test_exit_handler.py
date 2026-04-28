# Copyright (c) 2026 The Regents of the University of California
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

import unittest


class TestExitHandlerCompatibility(unittest.TestCase):
    def test_resolve_hypercall_id(self):
        # Import inside test so gem5 python paths are in effect when run
        import _m5.event as _m5_event

        from gem5.simulate import exit_handler as exit_handler_mod

        # Enum selector
        enum_sel = _m5_event.ExitHypercall.ScheduledExit
        resolved_enum = exit_handler_mod._resolve_hypercall_id(enum_sel)
        self.assertEqual(resolved_enum, int(enum_sel.value))

        # String selectors in multiple formats for a known member
        member = _m5_event.ExitHypercall.ClassicGenerator
        for form in (
            "classic-generator",
            "classic_generator",
            "ClassicGenerator",
        ):
            resolved = exit_handler_mod._resolve_hypercall_id(form)
            self.assertEqual(resolved, int(member.value))

        # Integer selector
        resolved_int = exit_handler_mod._resolve_hypercall_id(
            int(member.value)
        )
        self.assertEqual(resolved_int, int(member.value))

    def test_register_exit_handler_and_invocation(self):
        import _m5.event as _m5_event

        from gem5.simulate import exit_handler as exit_handler_mod

        h_id = int(_m5_event.ExitHypercall.ClassicGenerator.value)

        invocations = []

        def handler_func(simulator, payload):
            invocations.append((simulator, payload))
            return True

        # Save existing handler to restore after test
        handler_map = exit_handler_mod.ExitHandler.get_handler_map()
        existing = handler_map.get(h_id)

        try:
            handler_cls = exit_handler_mod.register_exit_handler(
                _m5_event.ExitHypercall.ClassicGenerator,
                handler_func,
                "test handler",
            )

            # Construct an instance and call handle()
            payload = {"cause": "test", "code": "0"}
            handler = handler_cls(payload)
            result = handler.handle(None)
            self.assertTrue(result)
            self.assertEqual(len(invocations), 1)
            self.assertEqual(invocations[0][1], payload)

        finally:
            # Restore previous mapping to avoid polluting global state
            if existing is None:
                handler_map.pop(h_id, None)
            else:
                handler_map[h_id] = existing

    def test_exitSimLoop_forwards_to_exitSimLoopWithHypercall(self):
        # Ensure the legacy helper preserves legacy scheduling arguments while
        # also forwarding structured metadata to the hypercall-aware path.
        import m5.event as m5_event

        import _m5.event as _m5_event

        original = _m5_event.exitSimLoopWithHypercall
        calls = []

        def fake_exitSimLoopWithHypercall(
            message,
            exit_code,
            when,
            repeat,
            payload,
            hypercall_id,
            serialize,
        ):
            calls.append(
                (
                    message,
                    exit_code,
                    when,
                    repeat,
                    payload,
                    hypercall_id,
                    serialize,
                )
            )

        try:
            _m5_event.exitSimLoopWithHypercall = fake_exitSimLoopWithHypercall

            m5_event.exitSimLoop(
                "legacy-cause",
                exit_code=7,
                when=123,
                repeat=11,
                serialize=True,
            )

            self.assertEqual(len(calls), 1)
            (
                message,
                exit_code,
                tick,
                repeat,
                payload,
                hypercall_id,
                serialize,
            ) = calls[0]
            self.assertEqual(message, "legacy-cause")
            self.assertEqual(exit_code, 7)
            self.assertEqual(tick, 123)
            self.assertEqual(repeat, 11)
            self.assertEqual(payload.get("cause"), "legacy-cause")
            self.assertEqual(payload.get("code"), str(7))
            self.assertEqual(
                hypercall_id,
                int(_m5_event.ExitHypercall.ClassicGenerator.value),
            )
            self.assertTrue(serialize)

        finally:
            _m5_event.exitSimLoopWithHypercall = original

    def test_exitSimLoop_defaults_when_to_native_curTick(self):
        # With no explicit tick, leave the default as None so the native
        # helper resolves curTick() inside the embedded runtime.
        import m5.event as m5_event

        import _m5.event as _m5_event

        original = _m5_event.exitSimLoopWithHypercall
        calls = []

        def fake_exitSimLoopWithHypercall(
            message,
            exit_code,
            when,
            repeat,
            payload,
            hypercall_id,
            serialize,
        ):
            calls.append((when, repeat))

        try:
            _m5_event.exitSimLoopWithHypercall = fake_exitSimLoopWithHypercall

            m5_event.exitSimLoop("legacy-cause")

            self.assertEqual(calls, [(None, 0)])

        finally:
            _m5_event.exitSimLoopWithHypercall = original


if __name__ == "__main__":
    unittest.main()
