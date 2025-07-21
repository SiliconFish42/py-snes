import pytest
import numpy as np
# Try importing the C++ extension module
try:
    from pysnes.pysnes_cpp import SNES
except ImportError as e:
    pytest.fail(f"Could not import pysnes_cpp: {e}")

def test_framebuffer_rgb_shape_and_type():
    snes = SNES()
    snes.power_on()
    # Step a few times to ensure framebuffer is updated
    for _ in range(10):
        snes.step()
    fb = snes.get_framebuffer_rgb()
    assert isinstance(fb, np.ndarray), "Returned framebuffer is not a NumPy array"
    assert fb.shape == (224, 256, 3), f"Unexpected framebuffer shape: {fb.shape}"
    assert fb.dtype == np.uint8, f"Unexpected framebuffer dtype: {fb.dtype}"
    # Optionally, check that the framebuffer is not all zeros
    assert np.any(fb != 0), "Framebuffer appears to be all zeros after stepping"

def test_input_handling_stub():
    pytest.skip("Not yet implemented: input handling test stub.")

def test_state_save_load_stub():
    pytest.skip("Not yet implemented: state save/load test stub.") 