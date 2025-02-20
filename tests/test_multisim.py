import unittest
from unittest.mock import patch, MagicMock
from pathlib import Path
from gem5.utils.multisim import (
    add_simulator,
    get_simulator_ids,
    get_num_processes,
    run,
)
from gem5.simulate.simulator import Simulator

class TestMultiSim(unittest.TestCase):

    @patch("gem5.utils.multisim._multi_sim", set())
    def test_add_simulator(self):
        simulator = MagicMock(spec=Simulator)
        simulator.get_id.return_value = "sim_1"
        add_simulator(simulator)
        self.assertIn(simulator, _multi_sim)
        self.assertEqual(simulator.get_id(), "sim_1")

    @patch("gem5.utils.multisim._multi_sim", set())
    def test_get_simulator_ids(self):
        simulator1 = MagicMock(spec=Simulator)
        simulator1.get_id.return_value = "sim_1"
        simulator2 = MagicMock(spec=Simulator)
        simulator2.get_id.return_value = "sim_2"
        add_simulator(simulator1)
        add_simulator(simulator2)
        ids = get_simulator_ids(Path("dummy_path"))
        self.assertIn("sim_1", ids)
        self.assertIn("sim_2", ids)

    @patch("gem5.utils.multisim._num_processes", None)
    def test_get_num_processes(self):
        global _num_processes
        _num_processes = 4
        num_processes = get_num_processes(Path("dummy_path"))
        self.assertEqual(num_processes, 4)

    @patch("gem5.utils.multisim._multi_sim", set())
    @patch("gem5.utils.multisim._load_module")
    @patch("gem5.utils.multisim._run")
    def test_run(self, mock_run, mock_load_module):
        simulator1 = MagicMock(spec=Simulator)
        simulator1.get_id.return_value = "sim_1"
        simulator2 = MagicMock(spec=Simulator)
        simulator2.get_id.return_value = "sim_2"
        add_simulator(simulator1)
        add_simulator(simulator2)
        run(Path("dummy_path"), processes=2)
        mock_run.assert_any_call(Path("dummy_path"), "sim_1")
        mock_run.assert_any_call(Path("dummy_path"), "sim_2")

if __name__ == "__main__":
    unittest.main()
